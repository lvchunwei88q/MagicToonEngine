#include "Clang/AstClassCollector.h"

#include "Clang/ClangOwnership.h"
#include "Clang/ClangPathUtility.h"
#include "Clang/HeaderMacroScan.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <clang-c/Index.h>
#include <Jobsystem/JobSystem.h>

#include "MagicConfig.h"

namespace MCT {
	namespace ClangSupport {
		namespace {
			struct LocalCursorLocation
			{
				std::string fileName;
				unsigned line = 0;
			};

			struct LocalParseContext
			{
				std::string targetFile;
				IO::path targetHeaderPath;
				std::unordered_set<size_t> mClassDeclarationLines;
				std::vector<MagicClassInfo> classes;
				std::unordered_map<unsigned, size_t> classIndices;
			};

			/**
			 * Extract the spelling file and line from a libclang source location.
			 * @param location The source location to inspect.
			 * @return The file name and one-based line number, if available.
			 */
			LocalCursorLocation GetLocation(CXSourceLocation location)
			{
				LocalCursorLocation result;
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

			/**
			 * Build a declaration's qualified name from its AST semantic parents.
			 * @param cursor The AST cursor for the declaration.
			 * @return The qualified name, such as Namespace::Class.
			 */
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

			/**
			 * Visit AST declarations and collect information for the target header's classes.
			 * The second CXCursor argument is unused by this callback.
			 * @param cursor The current AST node being visited.
			 * @param clientData A pointer to the ParseContext receiving the collected information.
			 * @return CXChildVisit_Recurse to continue visiting this node's children.
			 */
			enum CXChildVisitResult VisitClassAndMember(CXCursor cursor, CXCursor, CXClientData clientData)
			{
				auto& context = *static_cast<LocalParseContext*>(clientData);
				const CXCursorKind kind = clang_getCursorKind(cursor);
				if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl ||
					kind == CXCursor_ClassTemplate ||
					kind == CXCursor_ClassTemplatePartialSpecialization)
				{
					const LocalCursorLocation start = GetLocation(
						clang_getRangeStart(clang_getCursorExtent(cursor)));
					const LocalCursorLocation end = GetLocation(
						clang_getRangeEnd(clang_getCursorExtent(cursor)));
					const CXCursor definition = clang_getCursorDefinition(cursor);
					if (NormalizePath(start.fileName) == context.targetFile &&
						(clang_Cursor_isNull(definition) || clang_equalCursors(cursor, definition)))
					{
						MagicClassInfo classInfo;
						classInfo.className = ToString(clang_getCursorSpelling(cursor));
						classInfo.qualifiedName = GetQualifiedName(cursor);
						classInfo.kind = kind == CXCursor_StructDecl ? "struct" : "class";
						classInfo.headerPath = context.targetHeaderPath;
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
							const LocalCursorLocation location = GetLocation(
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
							const LocalCursorLocation location = GetLocation(
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

			/**
			 * Assemble the libclang arguments for one header: its module's compile command,
			 * the include paths that resolve generated headers, and the INCLUDE fallback.
			 * @param header The header being parsed.
			 * @param allHeaders All known headers, used to add generated module include paths.
			 * @param compileCommands The compilation database entries.
			 * @param generatedRoot The code generation directory holding per-module Object folders.
			 * @param arguments Receives the libclang arguments.
			 * @return True if the arguments were assembled; otherwise false.
			 */
			bool BuildArgumentsForHeader(
				const MagicEngineHeader& header,
				const std::vector<MagicEngineHeader>& allHeaders,
				const std::vector<CompileCommandData>& compileCommands,
				const IO::path& generatedRoot,
				std::vector<std::string>& arguments)
			{
				const CompileCommandData* compileCommand =
					FindModuleCompileCommand(compileCommands, header);
				if (compileCommand == nullptr)
				{
					Utilitie::Log::Error(
						"No compile command found for module " + header.moduleName +
						" while parsing " + header.headerPath.string());
					return false;
				}
				if (!BuildClangArguments(*compileCommand, arguments))
				{
					return false;
				}

				std::vector<std::string> includePaths;
				AddParseIncludePaths(
					header,
					generatedRoot,
					arguments,
					includePaths);
				for (const MagicEngineHeader& candidate : allHeaders)
				{
					if (!candidate.moduleName.empty())
					{
						AddIncludePath(
							arguments,
							includePaths,
							generatedRoot
								/ "Object"
								/ std::filesystem::u8path(candidate.moduleName));
					}
				}
				return true;
			}
		}

		bool ParseHeaderClasses(CXIndex index,
			const MagicEngineHeader& header,
			const std::vector<MagicEngineHeader>& allHeaders,
			const std::vector<CompileCommandData>& compileCommands,
			const IO::path& generatedRoot,
			const std::vector<size_t>& mClassDeclarationLines,
			std::vector<MagicClassInfo>& outClasses)
		{
			std::vector<std::string> arguments;
			if (!BuildArgumentsForHeader(
				header, allHeaders, compileCommands, generatedRoot, arguments))
			{
				return false;
			}

			std::vector<const char*> argumentPointers;
			argumentPointers.reserve(arguments.size());
			for (const std::string& argument : arguments)
			{
				argumentPointers.push_back(argument.c_str());
			}

			// The unsaved file points into source and headerPath, so both must stay alive
			// until clang_parseTranslationUnit2 has read them.
			const std::string source = BuildParseSource(header.lines);
			const std::string headerPath = header.headerPath.string();
			CXUnsavedFile unsavedFile = {
				headerPath.c_str(),
				source.c_str(),
				static_cast<unsigned long>(source.size())
			};

			CXTranslationUnit rawTranslationUnit = nullptr;
			const CXErrorCode result = clang_parseTranslationUnit2(
				index,
				headerPath.c_str(),
				argumentPointers.data(),
				static_cast<int>(argumentPointers.size()),
				&unsavedFile,
				1,
				CXTranslationUnit_None,
				&rawTranslationUnit);

			// libclang can hand back a partially built unit even when it reports a failure,
			// so ownership is taken before the result is checked; the destructor disposes it
			// on every exit path.
			const ClangTranslationUnit translationUnit(rawTranslationUnit);
			if (result != CXError_Success || !translationUnit)
			{
				Utilitie::Log::Error(
					"libclang failed to parse header: " + header.headerPath.string() +
					" (error " + std::to_string(static_cast<int>(result)) + ")");
				return false;
			}

			bool hasErrors = false;
			const unsigned diagnosticCount = clang_getNumDiagnostics(translationUnit.Get());
			for (unsigned i = 0; i < diagnosticCount; ++i)
			{
				const ClangDiagnostic diagnostic(
					clang_getDiagnostic(translationUnit.Get(), i));
				if (clang_getDiagnosticSeverity(diagnostic.Get()) >= CXDiagnostic_Error)
				{
					const LocalCursorLocation location = GetLocation(
						clang_getDiagnosticLocation(diagnostic.Get()));
					Utilitie::Log::Error(
						"libclang diagnostic in " + header.headerPath.string() +
						" Line:" + std::to_string(location.line) + ": " +
						ToString(clang_getDiagnosticSpelling(diagnostic.Get())));
					hasErrors = true;
				}
			}
			if (hasErrors)
			{
				return false;
			}

			const std::string targetFile = NormalizePath(
				std::filesystem::absolute(header.headerPath));
			LocalParseContext context{
				targetFile,
				header.headerPath,
				std::unordered_set<size_t>(
					mClassDeclarationLines.begin(), mClassDeclarationLines.end()),
				{},
				{}
			};
			clang_visitChildren(
				clang_getTranslationUnitCursor(translationUnit.Get()),
				VisitClassAndMember,
				&context);

			std::erase_if(context.classes, [&context](const MagicClassInfo& classInfo) {
				return !context.mClassDeclarationLines.contains(classInfo.lineNum);
			});

			outClasses.insert(
				outClasses.end(),
				std::make_move_iterator(context.classes.begin()),
				std::make_move_iterator(context.classes.end()));
			return true;
		}

		bool ParseAllHeaderClasses(const std::vector<MagicEngineHeader>& headers,
			const IO::path& generatePath, std::vector<MagicClassInfo>& outClasses,
			std::unordered_set<IO::path>& failedHeaders)
		{
			const IO::path compileCommandsPath = GetCompileCommandsPath(generatePath);
			if (!std::filesystem::is_regular_file(compileCommandsPath))
			{
				Utilitie::Log::Error(
					"CMake-generated " +
					IO::path(MAGICENGINE_COMPILE_COMMANDS_FILE).string() +
					" was not found at " +
					compileCommandsPath.string());
				return false;
			}

			std::vector<CompileCommandData> compileCommands;
			if (!LoadCompileCommands(compileCommandsPath, compileCommands))
			{
				return false;
			}

			struct HeaderJob
			{
				const MagicEngineHeader& header;
				std::vector<size_t> lines;
				std::vector<MagicClassInfo> classes;
				bool succeeded = false;
			};
			std::vector<HeaderJob> jobs;
			for (const MagicEngineHeader& header : headers)
			{
				// Record MCLASS lines so only classes directly following the marker are kept.
				std::vector<size_t> mClassDeclarationLines;
				std::vector<std::string> params;
				for (size_t line = 0; line < header.lines.size(); ++line)
				{
					if (ParseMacro(header.lines[line], "MCLASS", params))
					{
						mClassDeclarationLines.push_back(line + 1);
					}
				}
				if (!mClassDeclarationLines.empty())
				{
					jobs.push_back({ header, std::move(mClassDeclarationLines), {} });
				}
			}

			Utilitie::Log::Info("Parsing in parallel, number of parallels:" + std::to_string(jobs.size()) + " worker(s)");
			Core::JobSystem::Get().ParallelFor(static_cast<uint32_t>(jobs.size()), [&](uint32_t i)
			{
				auto& job = jobs[i];
				try
				{
					const ClangIndex index(clang_createIndex(0, 0));
					if (!index)
					{
						Utilitie::Log::Error("Could not create libclang index for " + job.header.headerPath.string());
						return;
					}
					job.succeeded = ParseHeaderClasses(index.Get(), job.header, headers, compileCommands,
						generatePath, job.lines, job.classes);
				}
				catch (const std::exception& error)
				{
					Utilitie::Log::Error("Failed to parse " + job.header.headerPath.string() + ": " + error.what());
				}
			});

			// Preserve input order regardless of which translation unit finished first.
			for (auto& job : jobs)
			{
				if (!job.succeeded)
				{
					failedHeaders.insert(job.header.headerPath);
					continue;
				}
				outClasses.insert(outClasses.end(), std::make_move_iterator(job.classes.begin()),
					std::make_move_iterator(job.classes.end()));
			}

			return true;
		}
	}
}
