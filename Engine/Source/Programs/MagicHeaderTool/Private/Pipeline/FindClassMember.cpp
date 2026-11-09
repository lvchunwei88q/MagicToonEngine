#include "Pipeline/GeneralPipeline.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <vector>

namespace MHT {
	namespace {
		bool ParseMMember(const std::string& line, std::vector<std::string>& outParams)
		{
			const size_t macroPosition = line.find("MMEMBER");
			if (macroPosition == std::string::npos ||
				line.find_first_not_of(" \t", 0) != macroPosition)
			{
				return false;
			}

			size_t position = macroPosition + std::string_view("MMEMBER").size();
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

		bool WriteClassInfoReport(const std::vector<MagicClassInfo>& classes)
		{
			const std::filesystem::path reportPath =
				std::filesystem::path(MagicGeneralPipelinedData::Get().GetGeneratePath())
				/ MAGICENGINE_FILE_INFO_PATH
				/ MAGICENGINE_CLASS_INFO_FILE;
			std::error_code directoryError;
			std::filesystem::create_directories(reportPath.parent_path(), directoryError);
			if (directoryError)
			{
				Utilitie::Log::Error(
					"Could not create class information directory: " +
					directoryError.message());
				return false;
			}
			std::ofstream report(reportPath, std::ios::binary | std::ios::trunc);
			if (!report)
			{
				Utilitie::Log::Error("Could not write class information report: " + reportPath.string());
				return false;
			}

			for (const MagicClassInfo& classInfo : classes)
			{
				if (!classInfo.hasMClass)
				{
					continue;
				}

				report << "Class: " << classInfo.qualifiedName << '\n'
					<< "Kind: " << classInfo.kind << '\n'
					<< "Header: " << classInfo.headerName << '\n'
					<< "Line: " << classInfo.lineNum + 1 << '\n'
					<< "MCLASS: yes";
				for (size_t i = 0; i < classInfo.mclassParams.size(); ++i)
				{
					report << (i == 0 ? " (" : ", ") << classInfo.mclassParams[i];
				}
				if (!classInfo.mclassParams.empty())
				{
					report << ')';
				}
				report << "\nGENERATE_BODY: "
					<< (classInfo.hasGenerateBody ? "yes" : "no");
				for (size_t i = 0; i < classInfo.generateBodyParams.size(); ++i)
				{
					report << (i == 0 ? " (" : ", ") << classInfo.generateBodyParams[i];
				}
				if (!classInfo.generateBodyParams.empty())
				{
					report << ')';
				}
				report << "\nBases:";
				for (const std::string& base : classInfo.baseClasses)
				{
					report << ' ' << base;
				}
				report << "\nFunctions:\n";
				for (const MagicClassFunction& function : classInfo.functions)
				{
					report << "  [" << function.access << "] "
						<< function.signature << " (line " << function.lineNum + 1 << ")\n";
				}
				report << "MMEMBER variables:\n";
				for (const MemberVariable& member : classInfo.members)
				{
					if (!member.hasMMember)
					{
						continue;
					}
					report << "  [" << member.access << "] "
						<< member.type << ' ' << member.name
						<< " (line " << member.lineNum << ") MMEMBER(";
					for (size_t i = 0; i < member.annotationParams.size(); ++i)
					{
						if (i != 0)
						{
							report << ", ";
						}
						report << member.annotationParams[i];
					}
					report << ")\n";
				}
				report << "\n";
			}

			report.close();
			if (!report)
			{
				Utilitie::Log::Error("Failed while writing class information report: " + reportPath.string());
				return false;
			}
			return true;
		}
	}

	namespace GeneralPipeline {
		bool FindClassMember()
		{
			auto& pipelineData = MagicGeneralPipelinedData::Get();
			auto& classes = pipelineData.GetRefMagicClasses();
			const auto& headers = pipelineData.GetRefMagicEngineHeaders();

			for (MagicClassInfo& classInfo : classes)
			{
				if (!classInfo.hasMClass)
				{
					continue;
				}

				const MagicEngineHeader* header = FindHeader(headers, classInfo.headerName);
				if (header == nullptr)
				{
					Utilitie::Log::Error(
						"Source lines are missing for " + classInfo.headerName);
					return false;
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
					if (!ParseMMember(header->lines[member.lineNum - 1], params))
					{
						continue;
					}
					member.hasMMember = true;
					member.annotationParams = std::move(params);
				}
			}

			for (const MagicClassInfo& classInfo : classes)
			{
				if (!classInfo.hasMClass)
				{
					continue;
				}

				const MagicEngineHeader* header = FindHeader(headers, classInfo.headerName);
				if (header == nullptr)
				{
					Utilitie::Log::Error("Class source data is missing for " + classInfo.className);
					return false;
				}

				for (size_t line = classInfo.lineNum;
					line <= classInfo.endLineNum && line < header->lines.size();
					++line)
				{
					std::vector<std::string> params;
					if (!ParseMMember(header->lines[line], params))
					{
						continue;
					}

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
						return false;
					}
				}
			}

			return WriteClassInfoReport(classes);
		}
	}
}
