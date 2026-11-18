#include "Pipeline/GeneralPipeline.h"

#include "Clang/AstClassCollector.h"
#include "Clang/HeaderMacroScan.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace MCT {
	namespace GeneralPipeline {
		/**
		 * @brief Parse engine headers and store the discovered class metadata in the pipeline data.
		 * @details The libclang work (compile database, translation units, AST walking) lives in
		 * Clang/AstClassCollector.cpp. This stage only orchestrates: parse the headers, then
		 * attach the MCLASS / GENERATE_BODY parameters to every discovered class.
		 * @return True if parsing completes without errors; otherwise false.
		 */
		bool FindEngineClass()
		{
			MGC context = MagicGlobalContext::Get();
			MagicGPD pipelineData = MagicGeneralPipelinedData::Get();
			const auto& headers = pipelineData.GetRefMagicEngineHeaders();

			std::vector<MagicClassInfo> parsedClasses;
			std::unordered_set<IO::path> failedHeaders;
			// Use libclang to read target class declarations, members, and functions;
			// continue parsing other headers if one fails.
			if (!ClangSupport::ParseAllHeaderClasses(
				headers, pipelineData.GetGeneratePath(), parsedClasses, failedHeaders))
			{
				return false;
			}

			bool hasErrors = !failedHeaders.empty();
			for (const MagicEngineHeader& header : headers)
			{
				if (failedHeaders.contains(header.headerPath))
				{
					continue;
				}

				for (size_t line = 0; line < header.lines.size(); ++line)
				{
					std::vector<std::string> classParams;
					if (!ClangSupport::ParseMacro(header.lines[line], "MCLASS", classParams))
					{
						continue;
					}

					const auto classInfo = std::find_if(
						parsedClasses.begin(),
						parsedClasses.end(),
						[&header, line](const MagicClassInfo& candidate) {
							return candidate.headerPath == header.headerPath &&
								candidate.lineNum == line + 1;
						});
					if (classInfo == parsedClasses.end())
					{
						Utilitie::Log::Error(
							"libclang could not find a class declaration after MCLASS: " +
							header.headerPath.string() + " Line:" + std::to_string(line + 1));
						hasErrors = true;
						continue;
					}

					std::vector<std::string> bodyParams;
					// MCLASS describes class capabilities; GENERATE_BODY supplies the class name
					// and object type used during code generation.
					if (!ClangSupport::ParseGenerateBody(
						header.lines, classInfo->lineNum, classInfo->endLineNum, bodyParams) ||
						bodyParams.empty())
					{
						Utilitie::Log::Error(
							"No valid GENERATE_BODY was found in class " +
							classInfo->className + " in " + header.headerPath.string());
						hasErrors = true;
						continue;
					}

					classInfo->mclassParams = classParams;
					classInfo->moduleName = header.moduleName;
					classInfo->generateBodyParams = bodyParams;
				}
			}

			pipelineData.SetMagicClasses(parsedClasses);
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
