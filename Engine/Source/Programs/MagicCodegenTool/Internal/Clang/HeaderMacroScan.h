#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace MCT {
	namespace ClangSupport {
		/**
		 * @brief Split a macro argument list at top-level commas.
		 * @param args The argument text to split.
		 * @param outParams Receives the trimmed, non-empty parameters.
		 * @return This function does not return a value; it replaces outParams.
		 */
		void SplitParams(std::string_view args, std::vector<std::string>& outParams);

		/**
		 * @brief Parse a single-line macro call whose name starts the line after optional whitespace.
		 * @param line The source line to inspect.
		 * @param macroName The macro name to find.
		 * @param outParams Receives the parsed macro arguments when parsing succeeds.
		 * @return True if the line contains a complete matching macro call; otherwise false.
		 */
		bool ParseMacro(const std::string& line, std::string_view macroName,
			std::vector<std::string>& outParams);

		/**
		 * @brief Build temporary source for libclang by replacing or removing tool-specific macros.
		 * @param lines The original header contents, split into lines.
		 * @return The transformed source text supplied to libclang.
		 */
		std::string BuildParseSource(const std::vector<std::string>& lines);

		/**
		 * @brief Find GENERATE_BODY within a zero-based line range.
		 * @param lines The original header contents, split into lines.
		 * @param firstLine The zero-based first line of the range to scan.
		 * @param lastLine The zero-based last line of the range to scan.
		 * @param params Receives the macro arguments when GENERATE_BODY is found.
		 * @return True if a valid GENERATE_BODY call is found; otherwise false.
		 */
		bool ParseGenerateBody(const std::vector<std::string>& lines,
			size_t firstLine, size_t lastLine, std::vector<std::string>& params);
	}
}
