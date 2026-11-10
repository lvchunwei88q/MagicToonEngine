#include "Pipeline/GeneralPipeline.h"

#include <algorithm>
#include <string_view>
#include <vector>

namespace MHT {
	namespace {
		bool ParseMarker(
			const std::string& line,
			std::string_view markerName,
			std::vector<std::string>& outParams)
		{
			// Parse a single-line MMEMBER/MFUNCTION marker and its parameters for matching
			// markers to libclang declarations.
			const size_t macroPosition = line.find(markerName);
			if (macroPosition == std::string::npos ||
				line.find_first_not_of(" \t", 0) != macroPosition)
			{
				return false;
			}

			size_t position = macroPosition + markerName.size();
			while (position < line.size() && (line[position] == ' ' || line[position] == '\t'))
			{
				++position;
			}
			if (position >= line.size() || line[position] != '(')
			{
				return false;
			}

			const size_t argumentStart = position + 1;
			size_t close = std::string::npos;
			size_t depth = 1;
			char quote = '\0';
			bool escaped = false;
			for (size_t i = argumentStart; i < line.size(); ++i)
			{
				const char current = line[i];
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
					++depth;
				}
				else if (current == ')' && --depth == 0)
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
			const std::string_view arguments(
				line.data() + argumentStart, close - argumentStart);
			size_t start = 0;
			size_t parentheses = 0;
			size_t brackets = 0;
			size_t braces = 0;
			size_t angles = 0;
			quote = '\0';
			escaped = false;
			for (size_t i = 0; i <= arguments.size(); ++i)
			{
				const char current = i < arguments.size() ? arguments[i] : ',';
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
					const std::string_view param = arguments.substr(start, i - start);
					const size_t first = param.find_first_not_of(" \t");
					if (first != std::string_view::npos)
					{
						const size_t last = param.find_last_not_of(" \t");
						outParams.emplace_back(param.substr(first, last - first + 1));
					}
					start = i + 1;
				}
			}
			return true;
		}

		const MagicEngineHeader* FindHeader(
			const std::vector<MagicEngineHeader>& headers,
			const std::string& headerName)
		{
			for (const MagicEngineHeader& header : headers)
			{
				if (header.headerName == headerName)
				{
					return &header;
				}
			}
			return nullptr;
		}

	}

	namespace GeneralPipeline {
		bool FindClassMember()
		{
			auto& pipelineData = MagicGeneralPipelinedData::Get();
			auto& classes = pipelineData.GetRefMagicClasses();
			const auto& headers = pipelineData.GetRefMagicEngineHeaders();
			bool hasErrors = false;

			// Copy annotation parameters from the preceding marker line to libclang members
			// and functions.
			for (MagicClassInfo& classInfo : classes)
			{
				const MagicEngineHeader* header = FindHeader(headers, classInfo.headerName);
				if (header == nullptr)
				{
					Utilitie::Log::Error(
						"Source lines are missing for " + classInfo.headerName);
					hasErrors = true;
					continue;
				}

				for (MemberVariable& member : classInfo.members)
				{
					member.hasMMember = false;
					member.annotationParams.clear();
					if (member.lineNum == 0 || member.lineNum >= header->lines.size())
					{
						continue;
					}

					std::vector<std::string> params;
					if (!ParseMarker(
						header->lines[member.lineNum - 1], "MMEMBER", params))
					{
						continue;
					}
					member.hasMMember = true;
					member.annotationParams = std::move(params);
				}

				for (MagicClassFunction& function : classInfo.functions)
				{
					function.hasMFunction = false;
					function.annotationParams.clear();
					if (function.lineNum == 0 || function.lineNum >= header->lines.size())
					{
						continue;
					}

					std::vector<std::string> params;
					if (!ParseMarker(
						header->lines[function.lineNum - 1], "MFUNCTION", params))
					{
						continue;
					}
					function.hasMFunction = true;
					function.annotationParams = std::move(params);
				}
			}

			// Check every marker in each class body to ensure libclang resolved its declaration.
			for (const MagicClassInfo& classInfo : classes)
			{
				const MagicEngineHeader* header = FindHeader(headers, classInfo.headerName);
				if (header == nullptr)
				{
					Utilitie::Log::Error("Class source data is missing for " + classInfo.className);
					hasErrors = true;
					continue;
				}

				for (size_t line = classInfo.lineNum;
					line <= classInfo.endLineNum && line < header->lines.size();
					++line)
				{
					std::vector<std::string> params;
					if (ParseMarker(header->lines[line], "MMEMBER", params))
					{
						const bool hasParsedMember = std::any_of(
							classInfo.members.begin(),
							classInfo.members.end(),
							[line](const MemberVariable& member) {
								return member.lineNum == line + 1 && member.hasMMember;
							});
						if (!hasParsedMember)
						{
							Utilitie::Log::Error(
								"libclang could not resolve declaration after MMEMBER in " +
								classInfo.headerName + " Line:" + std::to_string(line + 1));
							hasErrors = true;
						}
					}
					else if (ParseMarker(header->lines[line], "MFUNCTION", params))
					{
						const bool hasParsedFunction = std::any_of(
							classInfo.functions.begin(),
							classInfo.functions.end(),
							[line](const MagicClassFunction& function) {
								return function.lineNum == line + 1 &&
									function.hasMFunction;
							});
						if (!hasParsedFunction)
						{
							Utilitie::Log::Error(
								"libclang could not resolve declaration after MFUNCTION in " +
								classInfo.headerName + " Line:" + std::to_string(line + 1));
							hasErrors = true;
						}
					}
				}
			}

			if (hasErrors)
			{
				Utilitie::Log::Error(
					"Member/function marker parsing found errors");
				return false;
			}
			return true;
		}
	}
}
