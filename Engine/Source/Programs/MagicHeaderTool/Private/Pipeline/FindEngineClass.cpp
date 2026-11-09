#include "Pipeline/GeneralPipeline.h"

#include <clang-c/Index.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace MHT {
	namespace {
		std::string ToString(CXString value)
		{
			const char* text = clang_getCString(value);
			std::string result = text ? text : "";
			clang_disposeString(value);
			return result;
		}

		std::string NormalizePath(const std::string& path)
		{
			std::string normalized = std::filesystem::path(std::filesystem::u8path(path))
				.lexically_normal()
				.string();
			std::replace(normalized.begin(), normalized.end(), '\\', '/');
			std::transform(normalized.begin(), normalized.end(), normalized.begin(),
				[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
			return normalized;
		}

		std::filesystem::path GetCompileCommandsPath()
		{
			return std::filesystem::u8path(
				MagicGeneralPipelinedData::Get().GetCompileCommandsPath());
		}

		struct CompileCommandData
		{
			std::string directory;
			std::string fileName;
			std::vector<std::string> arguments;
		};

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

		bool LoadCompileCommands(const std::filesystem::path& buildMetaPath,
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

		std::filesystem::path FindModuleRoot(
			const std::string& headerName,
			const std::string& moduleName)
		{
			std::filesystem::path directory =
				std::filesystem::u8path(headerName).parent_path();
			while (!directory.empty())
			{
				if (directory.filename().string() == moduleName)
				{
					return directory;
				}
				const std::filesystem::path parent = directory.parent_path();
				if (parent == directory)
				{
					break;
				}
				directory = parent;
			}
			return {};
		}

		bool IsPathWithinDirectory(
			const std::filesystem::path& filePath,
			const std::filesystem::path& directory)
		{
			const std::string normalizedFile = NormalizePath(
				std::filesystem::absolute(filePath).string());
			const std::string normalizedDirectory = NormalizePath(
				std::filesystem::absolute(directory).string());
			return normalizedFile.size() > normalizedDirectory.size() &&
				normalizedFile.compare(0, normalizedDirectory.size(), normalizedDirectory) == 0 &&
				normalizedFile[normalizedDirectory.size()] == '/';
		}

		const CompileCommandData* FindModuleCompileCommand(
			const std::vector<CompileCommandData>& commands,
			const MagicEngineHeader& header)
		{
			const std::filesystem::path headerPath =
				std::filesystem::u8path(header.headerName);
			const std::string normalizedHeaderPath = NormalizePath(
				std::filesystem::absolute(headerPath).string());
			const std::filesystem::path moduleRoot =
				FindModuleRoot(header.headerName, header.moudelName);

			const CompileCommandData* bestCommand = nullptr;
			size_t bestDirectoryLength = 0;
			for (const CompileCommandData& command : commands)
			{
				const std::filesystem::path sourcePath = std::filesystem::u8path(command.fileName);
				const std::filesystem::path absoluteSource = sourcePath.is_absolute()
					? sourcePath
					: std::filesystem::u8path(command.directory) / sourcePath;
				if (NormalizePath(std::filesystem::absolute(absoluteSource).string()) ==
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

		void AddIncludeArgument(
			std::vector<std::string>& arguments,
			const std::string& directory,
			const std::string& includePath)
		{
			std::filesystem::path path = std::filesystem::u8path(includePath);
			if (path.is_relative())
			{
				path = std::filesystem::u8path(directory) / path;
			}
			arguments.push_back("-I" + path.lexically_normal().string());
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

			if (const char* includeEnvironment = std::getenv("INCLUDE"))
			{
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
						AddIncludeArgument(arguments, command.directory, path);
					}
					if (separator == std::string::npos)
					{
						break;
					}
					start = separator + 1;
				}
			}
			return true;
		}

		void AddIncludePath(std::vector<std::string>& arguments,
			std::vector<std::string>& knownPaths,
			const std::filesystem::path& path)
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

		void AddParseIncludePaths(
			const MagicEngineHeader& header,
			const std::string& moduleName,
			std::vector<std::string>& arguments,
			std::vector<std::string>& knownPaths)
		{
			AddIncludePath(
				arguments,
				knownPaths,
				std::filesystem::u8path(header.headerName).parent_path());

			const std::filesystem::path generatedRoot =
				std::filesystem::path(MagicGeneralPipelinedData::Get().GetGeneratePath());
			if (!moduleName.empty())
			{
				AddIncludePath(
					arguments,
					knownPaths,
					generatedRoot / "Object" / std::filesystem::u8path(moduleName));
			}

			if (const char* includeEnvironment = std::getenv("INCLUDE"))
			{
				std::string includePaths(includeEnvironment);
				size_t start = 0;
				while (start <= includePaths.size())
				{
					const size_t separator = includePaths.find(';', start);
					const std::string path = includePaths.substr(
						start,
						separator == std::string::npos ? std::string::npos : separator - start);
					if (!path.empty())
					{
						AddIncludePath(arguments, knownPaths, std::filesystem::u8path(path));
					}
					if (separator == std::string::npos)
					{
						break;
					}
					start = separator + 1;
				}
			}
		}

		void SplitParams(std::string_view args, std::vector<std::string>& outParams)
		{
			outParams.clear();
			size_t start = 0;
			size_t parentheses = 0;
			size_t brackets = 0;
			size_t braces = 0;
			size_t angles = 0;
			char quote = '\0';
			bool escaped = false;
			for (size_t position = 0; position <= args.size(); ++position)
			{
				const char current = position < args.size() ? args[position] : ',';
				if (quote != '\0')
				{
					if (escaped)
					{
						escaped = false;
					}
					else if (current == '\\')
					{
						escaped = true;
					}
					else if (current == quote)
					{
						quote = '\0';
					}
					continue;
				}

				if (current == '"' || current == '\'')
				{
					quote = current;
				}
				else if (current == '(')
				{
					++parentheses;
				}
				else if (current == ')' && parentheses > 0)
				{
					--parentheses;
				}
				else if (current == '[')
				{
					++brackets;
				}
				else if (current == ']' && brackets > 0)
				{
					--brackets;
				}
				else if (current == '{')
				{
					++braces;
				}
				else if (current == '}' && braces > 0)
				{
					--braces;
				}
				else if (current == '<')
				{
					++angles;
				}
				else if (current == '>' && angles > 0)
				{
					--angles;
				}

				if (current == ',' && parentheses == 0 && brackets == 0 &&
					braces == 0 && angles == 0)
				{
					std::string_view param = args.substr(start, position - start);
					const size_t first = param.find_first_not_of(" \t");
					if (first != std::string_view::npos)
					{
						const size_t last = param.find_last_not_of(" \t");
						outParams.emplace_back(param.substr(first, last - first + 1));
					}
					start = position + 1;
				}
			}
		}

		bool ParseMacro(const std::string& line, std::string_view macroName,
			std::vector<std::string>& outParams)
		{
			const size_t macroPosition = line.find(macroName);
			if (macroPosition == std::string::npos ||
				line.find_first_not_of(" \t", 0) != macroPosition)
			{
				return false;
			}

			size_t position = macroPosition + macroName.size();
			while (position < line.size() && (line[position] == ' ' || line[position] == '\t'))
			{
				++position;
			}
			if (position >= line.size() || line[position] != '(')
			{
				return false;
			}
			size_t close = std::string::npos;
			size_t depth = 1;
			char quote = '\0';
			for (size_t i = position + 1; i < line.size(); ++i)
			{
				if (quote != '\0')
				{
					if (line[i] == quote && (i == 0 || line[i - 1] != '\\'))
					{
						quote = '\0';
					}
					continue;
				}
				if (line[i] == '"' || line[i] == '\'')
				{
					quote = line[i];
				}
				else if (line[i] == '(')
				{
					++depth;
				}
				else if (line[i] == ')' && --depth == 0)
				{
					close = i;
					break;
				}
			}
			if (close == std::string::npos ||
				line.find_first_not_of(" \t;", close + 1) != std::string::npos)
			{
				return false;
			}

			outParams.clear();
			SplitParams(
				std::string_view(line).substr(position + 1, close - position - 1),
				outParams);
			return true;
		}

		bool IsIncludeDirective(const std::string& line)
		{
			size_t position = line.find_first_not_of(" \t");
			if (position == std::string::npos || line[position] != '#')
			{
				return false;
			}

			++position;
			while (position < line.size() && (line[position] == ' ' || line[position] == '\t'))
			{
				++position;
			}
			if (line.compare(position, 7, "include") != 0)
			{
				return false;
			}
			position += 7;
			return position == line.size() ||
				std::isspace(static_cast<unsigned char>(line[position])) ||
				line[position] == '<' ||
				line[position] == '"';
		}

		std::string BuildParseSource(const std::vector<std::string>& lines)
		{
			std::ostringstream source;
			std::vector<std::string> params;
			for (const std::string& line : lines)
			{
				if (IsIncludeDirective(line) &&
					line.find(".object.generate.h") != std::string::npos)
				{
					source << "#undef GetClassName\n";
				}
				else if (ParseMacro(line, "MCLASS", params) ||
					ParseMacro(line, "MMEMBER", params))
				{
					source << '\n';
				}
				else if (ParseMacro(line, "GENERATE_BODY", params))
				{
					source << "public: "
						<< "virtual std::string GetClassName() const override; "
						<< "virtual uint64_t GetClassId() const override; "
						<< "virtual uint64_t GetClassHas() const override; "
						<< "virtual Core::ObjectType GetClassType() const override; "
						<< "virtual Core::ObjectSwitch GetClassSwitch() const override; "
						<< "void generate_object_init_func_();\n";
				}
				else
				{
					source << line << '\n';
				}
			}
			return source.str();
		}

		std::string GetQualifiedName(CXCursor cursor)
		{
			std::vector<std::string> parts;
			for (CXCursor parent = clang_getCursorSemanticParent(cursor);
				!clang_Cursor_isNull(parent);
				parent = clang_getCursorSemanticParent(parent))
			{
				const CXCursorKind kind = clang_getCursorKind(parent);
				if (kind == CXCursor_Namespace ||
					kind == CXCursor_ClassDecl ||
					kind == CXCursor_StructDecl ||
					kind == CXCursor_ClassTemplate)
				{
					std::string name = ToString(clang_getCursorSpelling(parent));
					if (!name.empty())
					{
						parts.push_back(std::move(name));
					}
				}
			}

			std::reverse(parts.begin(), parts.end());
			std::string result;
			for (const std::string& part : parts)
			{
				if (!result.empty())
				{
					result += "::";
				}
				result += part;
			}
			const std::string name = ToString(clang_getCursorSpelling(cursor));
			if (!result.empty())
			{
				result += "::";
			}
			return result + name;
		}

		struct CursorLocation
		{
			std::string fileName;
			unsigned line = 0;
		};

		CursorLocation GetLocation(CXSourceLocation location)
		{
			CursorLocation result;
			CXFile file = nullptr;
			unsigned column = 0;
			unsigned offset = 0;
			clang_getSpellingLocation(location, &file, &result.line, &column, &offset);
			if (file != nullptr)
			{
				result.fileName = ToString(clang_getFileName(file));
			}
			return result;
		}

		struct ParseContext
		{
			std::string targetFile;
			std::string targetHeaderName;
			std::unordered_set<size_t> mClassDeclarationLines;
			std::vector<MagicClassInfo> classes;
			std::unordered_map<unsigned, size_t> classIndices;
		};

		enum CXChildVisitResult VisitClassAndMember(CXCursor cursor, CXCursor, CXClientData clientData)
		{
			auto& context = *static_cast<ParseContext*>(clientData);
			const CXCursorKind kind = clang_getCursorKind(cursor);
			if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl ||
				kind == CXCursor_ClassTemplate ||
				kind == CXCursor_ClassTemplatePartialSpecialization)
			{
				const CursorLocation start = GetLocation(
					clang_getRangeStart(clang_getCursorExtent(cursor)));
				const CursorLocation end = GetLocation(
					clang_getRangeEnd(clang_getCursorExtent(cursor)));
				const CXCursor definition = clang_getCursorDefinition(cursor);
				if (NormalizePath(start.fileName) == context.targetFile &&
					(clang_Cursor_isNull(definition) || clang_equalCursors(cursor, definition)))
				{
					MagicClassInfo classInfo;
					classInfo.className = ToString(clang_getCursorSpelling(cursor));
					classInfo.qualifiedName = GetQualifiedName(cursor);
					classInfo.kind = kind == CXCursor_StructDecl ? "struct" : "class";
					classInfo.headerName = context.targetHeaderName;
					classInfo.lineNum = start.line == 0 ? 0 : start.line - 1;
					classInfo.endLineNum = end.line == 0 ? classInfo.lineNum : end.line - 1;
					const size_t classIndex = context.classes.size();
					context.classIndices.emplace(clang_hashCursor(cursor), classIndex);
					context.classes.push_back(std::move(classInfo));
				}
			}
			else
			{
				const CXCursor parent = clang_getCursorSemanticParent(cursor);
				const auto parentIndex = context.classIndices.find(clang_hashCursor(parent));
				if (parentIndex != context.classIndices.end())
				{
					MagicClassInfo& classInfo = context.classes[parentIndex->second];
					if (kind == CXCursor_CXXBaseSpecifier)
					{
						classInfo.baseClasses.push_back(
							ToString(clang_getTypeSpelling(clang_getCursorType(cursor))));
					}
					else if (kind == CXCursor_FieldDecl || kind == CXCursor_VarDecl)
					{
						const CursorLocation location = GetLocation(
							clang_getRangeStart(clang_getCursorExtent(cursor)));
						if (NormalizePath(location.fileName) == context.targetFile)
						{
							const CX_CXXAccessSpecifier access =
								clang_getCXXAccessSpecifier(cursor);
							const std::string accessName =
								access == CX_CXXPublic ? "public" :
								access == CX_CXXProtected ? "protected" :
								access == CX_CXXPrivate ? "private" :
								classInfo.kind == "struct" ? "public" : "private";
							classInfo.members.emplace_back(
								ToString(clang_getCursorSpelling(cursor)),
								ToString(clang_getTypeSpelling(clang_getCursorType(cursor))),
								location.line == 0 ? 0 : location.line - 1,
								accessName);
						}
					}
					else if (kind == CXCursor_CXXMethod ||
						kind == CXCursor_Constructor ||
						kind == CXCursor_Destructor ||
						kind == CXCursor_ConversionFunction ||
						kind == CXCursor_FunctionTemplate)
					{
						const CursorLocation location = GetLocation(
							clang_getRangeStart(clang_getCursorExtent(cursor)));
						if (NormalizePath(location.fileName) == context.targetFile)
						{
							const CX_CXXAccessSpecifier access =
								clang_getCXXAccessSpecifier(cursor);
							classInfo.functions.push_back({
								ToString(clang_getCursorSpelling(cursor)),
								ToString(clang_getCursorDisplayName(cursor)) + " : " +
									ToString(clang_getTypeSpelling(clang_getCursorType(cursor))),
								access == CX_CXXPublic ? "public" :
									access == CX_CXXProtected ? "protected" :
									access == CX_CXXPrivate ? "private" :
									classInfo.kind == "struct" ? "public" : "private",
								location.line == 0 ? 0 : location.line - 1
							});
						}
					}
				}

			}

			return CXChildVisit_Recurse;
		}

		std::filesystem::path GetAstOutputPath(const MagicEngineHeader& header)
		{
			std::ostringstream fileName;
			fileName << std::filesystem::u8path(header.headerName).stem().string()
				<< '_' << std::hex
				<< std::hash<std::string>{}(NormalizePath(header.headerName))
				<< ".ast";
			return std::filesystem::path(MagicGeneralPipelinedData::Get().GetGeneratePath())
				/ MAGICENGINE_FILE_INFO_PATH
				/ "ast"
				/ std::filesystem::u8path(header.moudelName)
				/ fileName.str();
		}

		bool ParseHeaderWithLibClang(
			CXIndex index,
			const MagicEngineHeader& header,
			const std::vector<MagicEngineHeader>& allHeaders,
			const std::vector<CompileCommandData>& compileCommands,
			const std::vector<size_t>& mClassDeclarationLines,
			std::vector<MagicClassInfo>& outClasses,
			std::unordered_set<std::string>& failedHeaders)
		{
			const CompileCommandData* compileCommand =
				FindModuleCompileCommand(compileCommands, header);
			if (compileCommand == nullptr)
			{
				Utilitie::Log::Error(
					"No compile command found for module " + header.moudelName +
					" while parsing " + header.headerName);
				failedHeaders.insert(header.headerName);
				return false;
			}

			std::vector<std::string> arguments;
			if (!BuildClangArguments(*compileCommand, arguments))
			{
				failedHeaders.insert(header.headerName);
				return false;
			}
			std::vector<std::string> includePaths;
			AddParseIncludePaths(
				header,
				header.moudelName,
				arguments,
				includePaths);
			for (const MagicEngineHeader& candidate : allHeaders)
			{
				if (!candidate.moudelName.empty())
				{
					AddIncludePath(
						arguments,
						includePaths,
						std::filesystem::path(
							MagicGeneralPipelinedData::Get().GetGeneratePath())
							/ "Object"
							/ std::filesystem::u8path(candidate.moudelName));
				}
			}

			std::vector<const char*> argumentPointers;
			argumentPointers.reserve(arguments.size());
			for (const std::string& argument : arguments)
			{
				argumentPointers.push_back(argument.c_str());
			}

			const std::string source = BuildParseSource(header.lines);
			CXUnsavedFile unsavedFile = {
				header.headerName.c_str(),
				source.c_str(),
				static_cast<unsigned long>(source.size())
			};
			CXTranslationUnit translationUnit = nullptr;
			const CXErrorCode result = clang_parseTranslationUnit2(
				index,
				header.headerName.c_str(),
				argumentPointers.data(),
				static_cast<int>(argumentPointers.size()),
				&unsavedFile,
				1,
				CXTranslationUnit_None,
				&translationUnit);
			if (result != CXError_Success || translationUnit == nullptr)
			{
				Utilitie::Log::Error(
					"libclang failed to parse header: " + header.headerName +
					" (error " + std::to_string(static_cast<int>(result)) + ")");
				failedHeaders.insert(header.headerName);
				return false;
			}

			bool hasErrors = false;
			const unsigned diagnosticCount = clang_getNumDiagnostics(translationUnit);
			for (unsigned i = 0; i < diagnosticCount; ++i)
			{
				CXDiagnostic diagnostic = clang_getDiagnostic(translationUnit, i);
				if (clang_getDiagnosticSeverity(diagnostic) >= CXDiagnostic_Error)
				{
					const CursorLocation location = GetLocation(
						clang_getDiagnosticLocation(diagnostic));
					Utilitie::Log::Error(
						"libclang diagnostic in " + header.headerName +
						" Line:" + std::to_string(location.line) + ": " +
						ToString(clang_getDiagnosticSpelling(diagnostic)));
					hasErrors = true;
				}
				clang_disposeDiagnostic(diagnostic);
			}
			if (hasErrors)
			{
				failedHeaders.insert(header.headerName);
				clang_disposeTranslationUnit(translationUnit);
				return false;
			}

			const std::filesystem::path astPath = GetAstOutputPath(header);
			std::error_code directoryError;
			std::filesystem::create_directories(astPath.parent_path(), directoryError);
			if (directoryError)
			{
				Utilitie::Log::Error(
					"Could not create AST output directory: " + directoryError.message());
				failedHeaders.insert(header.headerName);
				clang_disposeTranslationUnit(translationUnit);
				return false;
			}
			const int saveResult = clang_saveTranslationUnit(
				translationUnit,
				astPath.string().c_str(),
				clang_defaultSaveOptions(translationUnit));
			if (saveResult != CXSaveError_None)
			{
				Utilitie::Log::Error(
					"Could not save libclang AST for " + header.headerName +
					" (error " + std::to_string(saveResult) + ")");
				failedHeaders.insert(header.headerName);
				clang_disposeTranslationUnit(translationUnit);
				return false;
			}

			const std::string targetFile = NormalizePath(
				std::filesystem::absolute(
					std::filesystem::u8path(header.headerName)).string());
			ParseContext context{
				targetFile,
				header.headerName,
				std::unordered_set<size_t>(
					mClassDeclarationLines.begin(), mClassDeclarationLines.end()),
				{},
				{}
			};
			clang_visitChildren(
				clang_getTranslationUnitCursor(translationUnit),
				VisitClassAndMember,
				&context);

			std::erase_if(context.classes, [&context](const MagicClassInfo& classInfo) {
				return !context.mClassDeclarationLines.contains(classInfo.lineNum);
			});

			clang_disposeTranslationUnit(translationUnit);
			outClasses.insert(
				outClasses.end(),
				std::make_move_iterator(context.classes.begin()),
				std::make_move_iterator(context.classes.end()));
			return true;
		}

		bool ParseHeadersWithLibClang(
			const std::vector<MagicEngineHeader>& headers,
			std::vector<MagicClassInfo>& outClasses,
			std::unordered_set<std::string>& failedHeaders)
		{
			const std::filesystem::path compileCommandsPath =
				GetCompileCommandsPath();
			if (!std::filesystem::is_regular_file(compileCommandsPath))
			{
				Utilitie::Log::Error(
					"CMake-generated engine_compile_commands.buildmeta was not found at " +
					compileCommandsPath.string());
				return false;
			}

			std::vector<CompileCommandData> compileCommands;
			if (!LoadCompileCommands(compileCommandsPath, compileCommands))
			{
				return false;
			}

			CXIndex index = clang_createIndex(0, 0);
			if (index == nullptr)
			{
				Utilitie::Log::Error("libclang could not create an index");
				return false;
			}

			for (const MagicEngineHeader& header : headers)
			{
				std::vector<size_t> mClassDeclarationLines;
				std::vector<std::string> params;
				for (size_t line = 0; line < header.lines.size(); ++line)
				{
					if (ParseMacro(header.lines[line], "MCLASS", params))
					{
						mClassDeclarationLines.push_back(line + 1);
					}
				}
				if (mClassDeclarationLines.empty())
				{
					continue;
				}

				if (!ParseHeaderWithLibClang(
					index,
					header,
					headers,
					compileCommands,
					mClassDeclarationLines,
					outClasses,
					failedHeaders))
				{
					continue;
				}
			}

			clang_disposeIndex(index);
			return true;
		}

		bool ParseGenerateBody(
			const std::vector<std::string>& lines,
			const MagicClassInfo& classInfo,
			std::vector<std::string>& params)
		{
			const size_t firstLine = classInfo.lineNum;
			const size_t lastLine = std::min(classInfo.endLineNum, lines.empty() ? 0 : lines.size() - 1);
			for (size_t line = firstLine; line <= lastLine && line < lines.size(); ++line)
			{
				if (ParseMacro(lines[line], "GENERATE_BODY", params))
				{
					return true;
				}
			}
			return false;
		}
	}

	namespace GeneralPipeline {
		bool FindEngineClass()
		{
			MGC context = MagicGlobalContext::Get();
			const auto& headers = MagicGeneralPipelinedData::Get().GetRefMagicEngineHeaders();
			std::vector<MagicClassInfo> parsedClasses;
			std::unordered_set<std::string> failedHeaders;
			if (!ParseHeadersWithLibClang(headers, parsedClasses, failedHeaders))
			{
				return false;
			}

			bool hasErrors = !failedHeaders.empty();
			for (const MagicEngineHeader& header : headers)
			{
				if (failedHeaders.contains(header.headerName))
				{
					continue;
				}

				for (size_t line = 0; line < header.lines.size(); ++line)
				{
					std::vector<std::string> classParams;
					if (!ParseMacro(header.lines[line], "MCLASS", classParams))
					{
						continue;
					}

					const auto classInfo = std::find_if(
						parsedClasses.begin(),
						parsedClasses.end(),
						[&header, line](const MagicClassInfo& candidate) {
							return candidate.headerName == header.headerName &&
								candidate.lineNum == line + 1;
						});
					if (classInfo == parsedClasses.end())
					{
						Utilitie::Log::Error(
							"libclang could not find a class declaration after MCLASS: " +
							header.headerName + " Line:" + std::to_string(line + 1));
						hasErrors = true;
						continue;
					}

					std::vector<std::string> bodyParams;
					if (!ParseGenerateBody(header.lines, *classInfo, bodyParams) ||
						bodyParams.empty())
					{
						Utilitie::Log::Error(
							"No valid GENERATE_BODY was found in class " +
							classInfo->className + " in " + header.headerName);
						hasErrors = true;
						continue;
					}

					classInfo->mclassParams = classParams;
					classInfo->moduleName = header.moudelName;
					classInfo->generateBodyParams = bodyParams;
				}
			}

			MagicGeneralPipelinedData::Get().SetMagicClasses(parsedClasses);
			if (hasErrors)
			{
				Utilitie::Log::Error(
					"Class parsing found errors; see the diagnostics above for all affected headers/classes");
				return false;
			}
			if (parsedClasses.empty())
			{
				context.SetGenerateCompleted();
			}
			return true;
		}
	}
}
