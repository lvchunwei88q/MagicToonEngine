#pragma once

#include <clang-c/Index.h>

#include <unordered_set>
#include <vector>

#include <IO.h>

#include "Clang/CompileCommands.h"
#include "Pipeline/GeneralPipeline.h"

namespace MCT {
	namespace ClangSupport {
		/**
		 * @brief Parse one header with libclang and extract marked class information.
		 * @details The translation unit lives in this function only; every libclang
		 * resource is owned by an RAII wrapper, so any early return is leak free.
		 * @param index The libclang index used to create the translation unit.
		 * @param header The header and source lines to parse.
		 * @param allHeaders All known headers, used to add generated module include paths.
		 * @param compileCommands The available compile commands used to configure parsing.
		 * @param generatedRoot The code generation directory holding per-module Object folders.
		 * @param mClassDeclarationLines Zero-based class declaration lines following MCLASS markers.
		 * @param outClasses Receives the classes extracted from this header.
		 * @return True if parsing and AST extraction complete; otherwise false.
		 */
		bool ParseHeaderClasses(CXIndex index,
			const MagicEngineHeader& header,
			const std::vector<MagicEngineHeader>& allHeaders,
			const std::vector<CompileCommandData>& compileCommands,
			const IO::path& generatedRoot,
			const std::vector<size_t>& mClassDeclarationLines,
			std::vector<MagicClassInfo>& outClasses);

		/**
		 * @brief Load the compilation database and parse each header containing an MCLASS marker.
		 * @param headers The headers to inspect and parse.
		 * @param generatePath The code generation directory, also holding the compilation database.
		 * @param outClasses Receives classes extracted from successfully parsed headers.
		 * @param failedHeaders Records headers that could not be parsed.
		 * @return False if the database could not be loaded. Per-header failures are collected in failedHeaders.
		 */
		bool ParseAllHeaderClasses(const std::vector<MagicEngineHeader>& headers,
			const IO::path& generatePath, std::vector<MagicClassInfo>& outClasses,
			std::unordered_set<IO::path>& failedHeaders);
	}
}
