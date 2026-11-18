#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

#include <IO.h>

namespace MCT {
	namespace ClangSupport {
		/**
		 * @brief Normalize a path for case-insensitive comparison.
		 * @param path The path to normalize.
		 * @return The lexically normalized path with forward slashes and lowercase characters.
		 */
		inline std::string NormalizePath(const IO::path& path)
		{
			std::string normalized = path.lexically_normal().string();
			std::replace(normalized.begin(), normalized.end(), '\\', '/');
			std::transform(normalized.begin(), normalized.end(), normalized.begin(),
				[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
			return normalized;
		}

		inline std::string NormalizePath(const std::string& path)
		{
			return NormalizePath(std::filesystem::u8path(path));
		}
	}
}
