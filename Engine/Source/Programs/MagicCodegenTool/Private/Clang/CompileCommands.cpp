#include "Clang/CompileCommands.h"

#include "Clang/ClangPathUtility.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
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

		bool BuildClangArguments(
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
