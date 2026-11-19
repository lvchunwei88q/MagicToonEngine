#include "Clang/CompileCommands.h"

#include "Clang/ClangPathUtility.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "MagicConfig.h"

namespace MCT {
	namespace ClangSupport {
		namespace {
			/**
			 * Split a command-line string into arguments, handling quotes and Windows-style backslashes.
			 * @param command The command-line string to split.
			 * @param arguments Receives the parsed arguments; existing contents are cleared first.
			 * @return True if at least one argument was parsed; otherwise false.
			 */
			bool ParseCommandLine(const std::string& command, std::vector<std::string>& arguments)
			{
				arguments.clear();
				size_t position = 0;
				while (position < command.size())
				{
					while (position < command.size() &&
						std::isspace(static_cast<unsigned char>(command[position])))
					{
						++position;
					}
					if (position == command.size())
					{
						break;
					}

					std::string argument;
					bool inQuotes = false;
					while (position < command.size() &&
						(inQuotes || !std::isspace(static_cast<unsigned char>(command[position]))))
					{
						size_t backslashes = 0;
						while (position < command.size() && command[position] == '\\')
						{
							++backslashes;
							++position;
						}

						if (position < command.size() && command[position] == '"')
						{
							argument.append(backslashes / 2, '\\');
							if (backslashes % 2 != 0)
							{
								argument.push_back('"');
								++position;
							}
							else
							{
								++position;
								if (inQuotes && position < command.size() &&
									command[position] == '"')
								{
									argument.push_back('"');
									++position;
								}
								else
								{
									inQuotes = !inQuotes;
								}
							}
						}
						else
						{
							argument.append(backslashes, '\\');
							if (position < command.size() &&
								(inQuotes || !std::isspace(static_cast<unsigned char>(command[position]))))
							{
								argument.push_back(command[position++]);
							}
						}
					}
					arguments.push_back(std::move(argument));
				}
				return !arguments.empty();
			}

			/**
			 * Walk up from a header's directory to find the module root.
			 * @param headerPath The header path from which to begin the search.
			 * @param moduleName The directory name identifying the module root.
			 * @return The matching module root, or an empty path if none is found.
			 */
			IO::path FindModuleRoot(
				const IO::path& headerPath,
				const std::string& moduleName)
			{
				IO::path directory = headerPath.parent_path();
				while (!directory.empty())
				{
					if (directory.filename().string() == moduleName)
					{
						return directory;
					}
					const IO::path parent = directory.parent_path();
					if (parent == directory)
					{
						break;
					}
					directory = parent;
				}
				return {};
			}

			/**
			 * Check whether a file is inside a directory, excluding paths that only share its name prefix.
			 * @param filePath The file path to test.
			 * @param directory The directory that should contain the file.
			 * @return True if the file is a descendant of the directory; otherwise false.
			 */
			bool IsPathWithinDirectory(
				const IO::path& filePath,
				const IO::path& directory)
			{
				const std::string normalizedFile = NormalizePath(
					std::filesystem::absolute(filePath));
				const std::string normalizedDirectory = NormalizePath(
					std::filesystem::absolute(directory));
				return normalizedFile.size() > normalizedDirectory.size() &&
					normalizedFile.compare(0, normalizedDirectory.size(), normalizedDirectory) == 0 &&
					normalizedFile[normalizedDirectory.size()] == '/';
			}

			/**
			 * Append an include directory as a libclang -I argument.
			 * @param arguments Receives the generated -I argument.
			 * @param directory The compile command's working directory for resolving relative paths.
			 * @param includePath The include directory to append.
			 * @return This function does not return a value; it updates arguments.
			 */
			void AddIncludeArgument(
				std::vector<std::string>& arguments,
				const std::string& directory,
				const std::string& includePath)
			{
				IO::path path = std::filesystem::u8path(includePath);
				if (path.is_relative())
				{
					path = std::filesystem::u8path(directory) / path;
				}
				arguments.push_back("-I" + path.lexically_normal().string());
			}

			/**
			 * Visit every non-empty entry of the INCLUDE environment variable.
			 * @param sink Receives each directory as it is found.
			 * @return This function does not return a value.
			 */
			template <typename Sink>
			void ForEachIncludeEnvironmentPath(Sink&& sink)
			{
				const char* includeEnvironment = std::getenv("INCLUDE");
				if (includeEnvironment == nullptr)
				{
					return;
				}

				// INCLUDE is a semicolon separated list; empty entries are ignored.
				std::string paths(includeEnvironment);
				size_t start = 0;
				while (start <= paths.size())
				{
					const size_t separator = paths.find(';', start);
					const std::string path = paths.substr(
						start,
						separator == std::string::npos ? std::string::npos : separator - start);
					if (!path.empty())
					{
						sink(path);
					}
					if (separator == std::string::npos)
					{
						break;
					}
					start = separator + 1;
				}
			}
		}

		IO::path GetCompileCommandsPath(const IO::path& generatePath)
		{
			return generatePath
				/ MAGICENGINE_FILE_INFO_PATH
				/ MAGICENGINE_COMPILE_COMMANDS_FILE;
		}

		bool LoadCompileCommands(const IO::path& buildMetaPath,
			std::vector<CompileCommandData>& outCommands)
		{
			std::ifstream input(buildMetaPath, std::ios::binary);
			if (!input)
			{
				Utilitie::Log::Error(
					"Could not open CMake-generated compile metadata: " +
					buildMetaPath.string());
				return false;
			}

			try
			{
				const nlohmann::json database = nlohmann::json::parse(input);
				if (!database.is_array())
				{
					Utilitie::Log::Error(
						"Compile metadata must contain a compilation database array: " +
						buildMetaPath.string());
					return false;
				}

				outCommands.reserve(database.size());
				for (const nlohmann::json& item : database)
				{
					if (!item.is_object() ||
						!item.contains("directory") || !item["directory"].is_string() ||
						!item.contains("file") || !item["file"].is_string())
					{
						Utilitie::Log::Error(
							"Compile metadata entry is missing a valid directory or file field");
						return false;
					}

					CompileCommandData entry;
					entry.directory = item["directory"].get<std::string>();
					entry.fileName = item["file"].get<std::string>();
					if (item.contains("arguments") && item["arguments"].is_array())
					{
						for (const nlohmann::json& argument : item["arguments"])
						{
							if (!argument.is_string())
							{
								Utilitie::Log::Error(
									"Compile metadata contains a non-string argument");
								return false;
							}
							entry.arguments.push_back(argument.get<std::string>());
						}
					}
					else if (item.contains("command") && item["command"].is_string())
					{
						if (!ParseCommandLine(
							item["command"].get<std::string>(), entry.arguments))
						{
							Utilitie::Log::Error(
								"Could not parse a compile command from CMake metadata");
							return false;
						}
					}
					else
					{
						Utilitie::Log::Error(
							"Compile metadata entry has neither an arguments array nor a command");
						return false;
					}
					outCommands.push_back(std::move(entry));
				}
			}
			catch (const nlohmann::json::exception& error)
			{
				Utilitie::Log::Error(
					"Could not parse CMake-generated compile metadata: " +
					std::string(error.what()));
				return false;
			}

			return true;
		}

		const CompileCommandData* FindModuleCompileCommand(
			const std::vector<CompileCommandData>& commands,
			const MagicEngineHeader& header)
		{
			const IO::path headerPath = header.headerPath;
			const std::string normalizedHeaderPath = NormalizePath(
				std::filesystem::absolute(headerPath));
			const IO::path moduleRoot =
				FindModuleRoot(header.headerPath, header.moduleName);

			const CompileCommandData* bestCommand = nullptr;
			size_t bestDirectoryLength = 0;
			for (const CompileCommandData& command : commands)
			{
				const IO::path sourcePath = std::filesystem::u8path(command.fileName);
				const IO::path absoluteSource = sourcePath.is_absolute()
					? sourcePath
					: std::filesystem::u8path(command.directory) / sourcePath;
				if (NormalizePath(std::filesystem::absolute(absoluteSource)) ==
					normalizedHeaderPath)
				{
					return &command;
				}

				if (moduleRoot.empty())
				{
					continue;
				}
				if (!IsPathWithinDirectory(absoluteSource, moduleRoot))
				{
					continue;
				}

				const std::string extension = absoluteSource.extension().string();
				if (extension != ".cpp" && extension != ".cc" &&
					extension != ".cxx" && extension != ".c")
				{
					continue;
				}
				if (moduleRoot.native().size() > bestDirectoryLength)
				{
					bestDirectoryLength = moduleRoot.native().size();
					bestCommand = &command;
				}
			}
			return bestCommand;
		}

		namespace
		{
			/**
			 * Extract the lowercase executable name from a compiler path, dropping the .exe suffix.
			 * @param compilerPath The compiler path stored in the compilation database.
			 * @return The normalized executable name used for dialect detection.
			 */
			std::string GetExecutableName(const std::string& compilerPath)
			{
				const size_t separator = compilerPath.find_last_of("/\\");
				std::string name = separator == std::string::npos
					? compilerPath
					: compilerPath.substr(separator + 1);
				std::transform(name.begin(), name.end(), name.begin(),
					[](unsigned char character)
					{
						return static_cast<char>(std::tolower(character));
					});
				if (name.size() > 4 && name.compare(name.size() - 4, 4, ".exe") == 0)
				{
					name.erase(name.size() - 4);
				}
				return name;
			}

			/**
			 * Check whether an executable name belongs to the GCC driver family.
			 * @param name The lowercase executable name.
			 * @return True for gcc/g++/cc/c++ and for cross drivers such as x86_64-w64-mingw32-g++.
			 */
			bool IsGccDriverName(const std::string& name)
			{
				if (name == "gcc" || name == "g++" || name == "cc" || name == "c++")
				{
					return true;
				}
				return name.size() > 4 &&
					(name.compare(name.size() - 4, 4, "-gcc") == 0 ||
						name.compare(name.size() - 4, 4, "-g++") == 0);
			}

			/**
			 * Replace the MinGW vendor field with the triple clang expects for that environment.
			 * @param machine The triple reported by the compiler, for example x86_64-w64-mingw32.
			 * @return The clang-compatible triple, for example x86_64-w64-windows-gnu.
			 */
			std::string MapToClangTriple(const std::string& machine)
			{
				std::string triple = machine;
				const size_t mingw = triple.find("mingw32");
				if (mingw != std::string::npos)
				{
					triple.replace(mingw, 7, "windows-gnu");
				}
				return triple;
			}

			/**
			 * Run a shell command and collect everything it writes to stdout and stderr.
			 * @details Only the GCC probes use this helper, and GCC is only used on Linux builds:
			 * Windows builds compile with MSVC, whose commands never reach it. Windows therefore
			 * reports failure instead of spawning a shell through the Microsoft CRT.
			 * @param command The command line, already carrying its own redirections.
			 * @param output Receives the captured output; left untouched on Windows.
			 * @return True when the command ran and its output was captured; false otherwise.
			 */
			bool RunCommandCapture(const std::string& command, std::string& output)
			{
#if defined(_WIN32)
				(void)command;
				(void)output;
				return false;
#else
				FILE* pipe = popen(command.c_str(), "r");
				if (pipe == nullptr)
				{
					return false;
				}

				char buffer[256] = {};
				while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr)
				{
					output += buffer;
				}
				pclose(pipe);
				return true;
#endif
			}

			/**
			 * Extract the angle-bracket include directories from a compiler's -v output.
			 * @param output The captured compiler output listing the include search paths.
			 * @return The directories between the search markers, normalized and deduplicated.
			 */
			std::vector<std::string> ParseIncludeSearchList(const std::string& output)
			{
				std::vector<std::string> directories;
				bool collecting = false;
				size_t position = 0;
				while (position < output.size())
				{
					const size_t end = output.find('\n', position);
					const size_t lineEnd = end == std::string::npos ? output.size() : end;
					std::string line = output.substr(position, lineEnd - position);
					position = lineEnd + 1;

					const size_t first = line.find_first_not_of(" \t\r");
					if (first == std::string::npos)
					{
						line.clear();
					}
					else
					{
						line.erase(0, first);
					}

					if (!collecting)
					{
						if (line.find("#include <...> search starts here") != std::string::npos)
						{
							collecting = true;
						}
						continue;
					}
					if (line.find("End of search list.") != std::string::npos)
					{
						break;
					}
					if (line.empty())
					{
						continue;
					}

					const std::string path =
						std::filesystem::u8path(line).lexically_normal().string();
					if (std::find(directories.begin(), directories.end(), path) == directories.end())
					{
						directories.push_back(path);
					}
				}
				return directories;
			}

			/**
			 * Ask the compiler for its default target triple with -dumpmachine.
			 * @param compilerPath The compiler executable from the compile command.
			 * @return The clang-compatible triple, or an empty string when the probe fails.
			 */
			std::string QueryCompilerTriple(const std::string& compilerPath)
			{
				static std::mutex cacheMutex;
				static std::unordered_map<std::string, std::string> cache;

				{
					std::lock_guard<std::mutex> lock(cacheMutex);
					const auto found = cache.find(compilerPath);
					if (found != cache.end())
					{
						return found->second;
					}
				}

				if (!std::filesystem::exists(std::filesystem::u8path(compilerPath)))
				{
					std::lock_guard<std::mutex> lock(cacheMutex);
					cache[compilerPath] = std::string();
					return {};
				}

				const std::string command = "\"" + compilerPath + "\" -dumpmachine";

				std::string output;
				if (!RunCommandCapture(command, output))
				{
					Utilitie::Log::Warning(
						"Could not run the compiler to detect its target triple: " + compilerPath);
					return {};
				}

				const size_t end = output.find_last_not_of(" \t\r\n");
				const std::string machine = end == std::string::npos
					? std::string()
					: output.substr(0, end + 1);
				const std::string triple = machine.empty() ? machine : MapToClangTriple(machine);

				{
					std::lock_guard<std::mutex> lock(cacheMutex);
					cache[compilerPath] = triple;
				}
				return triple;
			}

			/**
			 * Ask the compiler for the include directories it searches on its own.
			 * @details Neither CMake nor libclang knows a GCC installation layout, so the compiler
			 * is asked once per executable and the answer is cached.
			 * @param compilerPath The compiler executable from the compile command.
			 * @return The system include directories, or an empty list when the probe fails.
			 */
			std::vector<std::string> QueryCompilerIncludePaths(const std::string& compilerPath)
			{
				static std::mutex cacheMutex;
				static std::unordered_map<std::string, std::vector<std::string>> cache;

				{
					std::lock_guard<std::mutex> lock(cacheMutex);
					const auto found = cache.find(compilerPath);
					if (found != cache.end())
					{
						return found->second;
					}
				}

				std::vector<std::string> directories;
				const bool exists = std::filesystem::exists(std::filesystem::u8path(compilerPath));
				if (exists)
				{
					// GCC is only used on Linux in this project, Windows builds use MSVC, so the
					// POSIX redirection is intentional; the null device keeps the compiler from
					// waiting on stdin for the "-" translation unit.
					const std::string command =
						"\"" + compilerPath + "\" -E -x c++ - -v < /dev/null 2>&1";
					std::string output;
					if (RunCommandCapture(command, output))
					{
						directories = ParseIncludeSearchList(output);
					}
				}

				if (exists && directories.empty())
				{
					Utilitie::Log::Warning(
						"Could not determine the system include paths of: " + compilerPath);
				}

				{
					std::lock_guard<std::mutex> lock(cacheMutex);
					cache[compilerPath] = directories;
				}
				return directories;
			}

			/**
			 * Work out the target triple libclang should assume for a GCC command.
			 * @param compilerPath The compiler executable from the compile command.
			 * @return The clang-compatible triple, or an empty string when it cannot be determined.
			 */
			std::string ResolveCompilerTriple(const std::string& compilerPath)
			{
				const std::string executable = GetExecutableName(compilerPath);
				const size_t suffix = executable.find_last_of('-');
				if (suffix != std::string::npos &&
					(executable.size() - suffix) > 1)
				{
					std::string prefix = executable.substr(0, suffix);
					if (prefix.find('-') != std::string::npos &&
						prefix.find("mingw") != std::string::npos)
					{
						return MapToClangTriple(prefix);
					}
				}
				return QueryCompilerTriple(compilerPath);
			}

			/**
			 * Resolve a possibly relative path against the compile command's working directory.
			 * @param directory The compile command's working directory.
			 * @param path The path to resolve.
			 * @return The normalized path as a UTF-8 string.
			 */
			std::string ResolveAbsolutePath(const std::string& directory, const std::string& path)
			{
				IO::path resolved = std::filesystem::u8path(path);
				if (resolved.is_relative())
				{
					resolved = std::filesystem::u8path(directory) / resolved;
				}
				return resolved.lexically_normal().string();
			}

			/**
			 * Read a response file and append its arguments, replacing the @file token.
			 * @param directory The compile command's working directory, used to resolve the file.
			 * @param argument The @file token being expanded.
			 * @param depth The current nesting depth, guarding against recursive response files.
			 * @param outArguments Receives the expanded arguments.
			 * @return False when the file cannot be read or nesting is too deep; otherwise true.
			 */
			bool ExpandResponseFile(const std::string& directory, const std::string& argument,
				size_t depth, std::vector<std::string>& outArguments)
			{
				if (depth > 4)
				{
					Utilitie::Log::Error("Compile command response files nest too deeply: " + argument);
					return false;
				}

				const std::string path = ResolveAbsolutePath(directory, argument.substr(1));
				std::ifstream input(path, std::ios::binary);
				if (!input)
				{
					Utilitie::Log::Error("Could not open compile command response file: " + path);
					return false;
				}

				const std::string contents{
					std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
				std::vector<std::string> tokens;
				if (!ParseCommandLine(contents, tokens))
				{
					return true;
				}
				for (const std::string& token : tokens)
				{
					if (token.compare(0, 1, "@") == 0)
					{
						if (!ExpandResponseFile(directory, token, depth + 1, outArguments))
						{
							return false;
						}
					}
					else
					{
						outArguments.push_back(token);
					}
				}
				return true;
			}

			/**
			 * Replace every @file token of a compile command with the arguments it contains.
			 * @param command The compile command being flattened.
			 * @param outArguments Receives the command line without response files.
			 * @return False when a response file cannot be expanded; otherwise true.
			 */
			bool ExpandArguments(const CompileCommandData& command,
				std::vector<std::string>& outArguments)
			{
				outArguments.clear();
				outArguments.reserve(command.arguments.size());
				for (const std::string& argument : command.arguments)
				{
					if (argument.compare(0, 1, "@") == 0)
					{
						if (!ExpandResponseFile(command.directory, argument, 0, outArguments))
						{
							return false;
						}
					}
					else
					{
						outArguments.push_back(argument);
					}
				}
				return true;
			}
		}

		CompilerKind DetectCompilerKind(const CompileCommandData& command)
		{
			if (command.arguments.empty())
			{
				return CompilerKind::Unknown;
			}

			const std::string executable = GetExecutableName(command.arguments.front());
			if (executable == "cl")
			{
				return CompilerKind::Msvc;
			}
			if (IsGccDriverName(executable))
			{
				return CompilerKind::Gcc;
			}
			return CompilerKind::Unknown;
		}

		bool BuildMsvcClangArguments(
			const CompileCommandData& command,
			std::vector<std::string>& arguments)
		{
			arguments = { "-x", "c++", "-fms-extensions", "-fms-compatibility" };
			for (size_t i = command.arguments.empty() ? 0 : 1;
				i < command.arguments.size();
				++i)
			{
				const std::string& argument = command.arguments[i];
				if (argument == "/D" || argument == "-D" ||
					argument == "/U" || argument == "-U")
				{
					if (i + 1 >= command.arguments.size())
					{
						Utilitie::Log::Error("Missing value in compile command option: " + argument);
						return false;
					}
					arguments.push_back(argument[0] == '/' ? "-" + argument.substr(1) : argument);
					arguments.push_back(command.arguments[++i]);
				}
				else if (argument.size() > 2 &&
					(argument.compare(0, 2, "/D") == 0 || argument.compare(0, 2, "-D") == 0 ||
						argument.compare(0, 2, "/U") == 0 || argument.compare(0, 2, "-U") == 0))
				{
					arguments.push_back("-" + argument.substr(1));
				}
				else if (argument == "/I" || argument == "-I")
				{
					if (i + 1 >= command.arguments.size())
					{
						Utilitie::Log::Error("Missing include path in compile command");
						return false;
					}
					AddIncludeArgument(arguments, command.directory, command.arguments[++i]);
				}
				else if (argument.size() > 2 &&
					(argument.compare(0, 2, "/I") == 0 || argument.compare(0, 2, "-I") == 0))
				{
					AddIncludeArgument(arguments, command.directory, argument.substr(2));
				}
				else if (argument.size() > 11 && argument.compare(0, 11, "/external:I") == 0)
				{
					AddIncludeArgument(arguments, command.directory, argument.substr(11));
				}
				else if (argument.rfind("-std=", 0) == 0)
				{
					arguments.push_back(argument);
				}
				else if (argument.rfind("-std:", 0) == 0 || argument.rfind("/std:", 0) == 0)
				{
					arguments.push_back("-std=" + argument.substr(5));
				}
				else if (argument == "/TP" || argument == "-TP")
				{
					continue;
				}
				else if (argument == "/TC" || argument == "-TC")
				{
					Utilitie::Log::Error("A C compile command cannot be used to parse a C++ header");
					return false;
				}
				else if (argument == "/EHsc" || argument == "-EHsc")
				{
					arguments.push_back("-fexceptions");
				}
				else if (argument == "/GR" || argument == "-GR")
				{
					arguments.push_back("-frtti");
				}
				else if (argument.size() > 3 && argument.compare(0, 3, "/FI") == 0)
				{
					arguments.push_back("-include");
					arguments.push_back(
						(std::filesystem::u8path(command.directory) /
							std::filesystem::u8path(argument.substr(3))).lexically_normal().string());
				}
			}

			ForEachIncludeEnvironmentPath([&arguments, &command](const std::string& path)
				{
					AddIncludeArgument(arguments, command.directory, path);
				});
			return true;
		}

		bool BuildGccClangArguments(
			const CompileCommandData& command,
			std::vector<std::string>& arguments)
		{
			arguments = { "-x", "c++" };
			bool hasTarget = false;
			for (size_t i = 1; i < command.arguments.size(); ++i)
			{
				const std::string& argument = command.arguments[i];

				// Build output, dependency and linker options say nothing libclang needs.
				if (argument == "-o" || argument == "-MF" || argument == "-MT" ||
					argument == "-MQ" || argument == "-x" || argument == "-arch")
				{
					++i;
					continue;
				}
				if (argument == "-c" || argument == "-S" || argument == "-E" ||
					argument == "-MD" || argument == "-MMD" || argument == "-MP" ||
					argument == "-pipe" || argument == "-save-temps" ||
					argument == "-Werror" || argument.rfind("-Werror=", 0) == 0 ||
					argument.rfind("-Wl,", 0) == 0 || argument.rfind("-Wa,", 0) == 0)
				{
					continue;
				}
				if (argument == "-D" || argument == "-U")
				{
					if (i + 1 >= command.arguments.size())
					{
						Utilitie::Log::Error("Missing value in compile command option: " + argument);
						return false;
					}
					arguments.push_back(argument);
					arguments.push_back(command.arguments[++i]);
					continue;
				}
				if (argument == "--target" || argument == "-target")
				{
					if (i + 1 >= command.arguments.size())
					{
						Utilitie::Log::Error("Missing value in compile command option: " + argument);
						return false;
					}
					arguments.push_back("--target=" + command.arguments[++i]);
					hasTarget = true;
					continue;
				}
				if (argument.rfind("--target=", 0) == 0 || argument.rfind("-target=", 0) == 0)
				{
					const size_t separator = argument.find('=');
					arguments.push_back("--target=" + argument.substr(separator + 1));
					hasTarget = true;
					continue;
				}
				if (argument == "-I" || argument == "-isystem" || argument == "-iquote" ||
					argument == "-idirafter" || argument == "-isysroot" ||
					argument == "--sysroot" || argument == "-include" ||
					argument == "-imacros" || argument == "-include-pch")
				{
					if (i + 1 >= command.arguments.size())
					{
						Utilitie::Log::Error("Missing path in compile command option: " + argument);
						return false;
					}
					arguments.push_back(argument);
					arguments.push_back(
						ResolveAbsolutePath(command.directory, command.arguments[++i]));
					continue;
				}
				if (argument.size() > 2 && argument.compare(0, 2, "-I") == 0)
				{
					arguments.push_back(
						"-I" + ResolveAbsolutePath(command.directory, argument.substr(2)));
					continue;
				}
				if (argument.empty() || argument.front() != '-')
				{
					// The translation unit itself is supplied to libclang as the main file.
					continue;
				}

				// Everything else is forwarded untouched: the clang driver understands the GCC
				// options, and dropping them would change the parsed language semantics.
				arguments.push_back(argument);
			}

			// CMake does not record the compiler's default triple, and libclang would otherwise
			// assume the host's MSVC environment and fail on the MinGW standard library headers.
			if (!hasTarget)
			{
				const std::string triple = ResolveCompilerTriple(command.arguments.front());
				if (!triple.empty())
				{
					arguments.push_back("--target=" + triple);
				}
			}

			// Neither the compilation database nor libclang carries the compiler's implicit system
			// include directories; without them <string> and friends cannot be resolved.
			std::vector<std::string> knownSystemPaths;
			for (size_t i = 0; i + 1 < arguments.size(); ++i)
			{
				if (arguments[i] == "-isystem")
				{
					knownSystemPaths.push_back(NormalizePath(arguments[i + 1]));
				}
			}
			for (const std::string& directory :
				QueryCompilerIncludePaths(command.arguments.front()))
			{
				if (std::find(knownSystemPaths.begin(), knownSystemPaths.end(),
					NormalizePath(directory)) == knownSystemPaths.end())
				{
					arguments.push_back("-isystem");
					arguments.push_back(directory);
				}
			}
			return true;
		}

		bool BuildClangArguments(
			const CompileCommandData& command,
			std::vector<std::string>& arguments)
		{
			if (command.arguments.empty())
			{
				Utilitie::Log::Error(
					"Compile command has no compiler executable: " + command.fileName);
				return false;
			}

			CompileCommandData expanded = command;
			if (!ExpandArguments(command, expanded.arguments))
			{
				return false;
			}

			switch (DetectCompilerKind(expanded))
			{
			case CompilerKind::Msvc:
				return BuildMsvcClangArguments(expanded, arguments);
			case CompilerKind::Gcc:
				return BuildGccClangArguments(expanded, arguments);
			default:
				Utilitie::Log::Error(
					"Unsupported compiler in the compilation database: " +
					expanded.arguments.front() + " (only MSVC cl.exe and GCC gcc/g++ are supported) "
					"while parsing " + expanded.fileName);
				return false;
			}
		}

		void AddIncludePath(std::vector<std::string>& arguments,
			std::vector<std::string>& knownPaths,
			const IO::path& path)
		{
			if (!std::filesystem::is_directory(path))
			{
				return;
			}

			const std::string pathString = path.string();
			if (std::find(knownPaths.begin(), knownPaths.end(), pathString) == knownPaths.end())
			{
				knownPaths.push_back(pathString);
				arguments.push_back("-I" + pathString);
			}
		}

		void AddParseIncludePaths(const MagicEngineHeader& header,
			const IO::path& generatedRoot, std::vector<std::string>& arguments,
			std::vector<std::string>& knownPaths)
		{
			AddIncludePath(
				arguments,
				knownPaths,
				header.headerPath.parent_path());

			if (!header.moduleName.empty())
			{
				AddIncludePath(
					arguments,
					knownPaths,
					generatedRoot / "Object" / std::filesystem::u8path(header.moduleName));
			}

			ForEachIncludeEnvironmentPath([&arguments, &knownPaths](const std::string& path)
				{
					AddIncludePath(arguments, knownPaths, std::filesystem::u8path(path));
				});
		}
	}
}
