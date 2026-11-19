#include "Clang/HeaderMacroScan.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "Codegen/ObjectInterfaceContract.h"
#include "MagicConfig.h"

namespace MCT {
	namespace ClangSupport {
		namespace {
			/**
			 * Check whether a source line has the form of a preprocessor include directive.
			 * @param line The source line to inspect.
			 * @return True if the line contains an #include directive; otherwise false.
			 */
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
					ParseMacro(line, "MMEMBER", params) ||
					ParseMacro(line, "MFUNCTION", params))
				{
					source << '\n';
				}
				else if (ParseMacro(line, "GENERATE_BODY", params))
				{
					// GENERATE_BODY is replaced by the declarations of every member it defines.
					// The list and the signatures come from Codegen/ObjectInterfaceContract.h,
					// which GenerateClassBodyFunction() renders into definitions - a rename
					// now touches one place instead of two hand-written string blobs.
					//
					// One source line in, one source line out: the unsaved file must keep the
					// original line numbering, because MCLASS line matching compares the AST
					// locations against the header on disk.
					source << "public:";
					for (const Codegen::ClassOverrideMethod& method : Codegen::kClassOverrideMethods)
					{
						source << " " << method.Signature << ";";
					}
					source << " " << Codegen::kGenerateObjectInitFuncSignature << ";\n";
				}
				else
				{
					source << line << '\n';
				}
			}
			return source.str();
		}

		bool ParseGenerateBody(const std::vector<std::string>& lines,
			size_t firstLine, size_t lastLine, std::vector<std::string>& params)
		{
			const size_t effectiveLastLine =
				std::min(lastLine, lines.empty() ? 0 : lines.size() - 1);
			for (size_t line = firstLine; line <= effectiveLastLine && line < lines.size(); ++line)
			{
				if (ParseMacro(lines[line], "GENERATE_BODY", params))
				{
					return true;
				}
			}
			return false;
		}
	}
}
