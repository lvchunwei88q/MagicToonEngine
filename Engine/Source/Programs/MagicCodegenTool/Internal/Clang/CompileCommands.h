#pragma once

#include <string>
#include <vector>

#include <IO.h>

#include "Pipeline/GeneralPipeline.h"

namespace MCT {
	namespace ClangSupport {
		/**
		 * @brief One entry of the CMake generated compilation database.
		 */
		struct CompileCommandData
		{
			std::string directory;
			std::string fileName;
			std::vector<std::string> arguments;
		};

		/**
		 * @brief Build the path to the CMake-generated compilation database.
		 * @param generatePath The configured code generation directory.
		 * @return The compilation database path under the generation directory.
		 */
		IO::path GetCompileCommandsPath(const IO::path& generatePath);

		/**
		 * @brief Read and validate a compilation database in either arguments-array or command-string form.
		 * @param buildMetaPath The path to the compilation database file.
		 * @param outCommands Receives the parsed compile command entries.
		 * @return True if the database was read and parsed successfully; otherwise false.
		 */
		bool LoadCompileCommands(const IO::path& buildMetaPath,
			std::vector<CompileCommandData>& outCommands);

		/**
		 * @brief Find a compile command for a header, falling back to a source file in its module.
		 * @param commands The available compile command entries.
		 * @param header The header whose compile command is being sought.
		 * @return A pointer to the matching entry, or nullptr if no suitable command exists.
		 */
		const CompileCommandData* FindModuleCompileCommand(
			const std::vector<CompileCommandData>& commands,
			const MagicEngineHeader& header);

		/**
		 * @brief The compiler dialect that produced a compilation database entry.
		 */
		enum class CompilerKind
		{
			Unknown,
			Msvc,
			Gcc
		};

		/**
		 * @brief Detect the compiler dialect after any leading sccache/ccache launchers.
		 * @param command The compile command whose compiler is being inspected.
		 * @return Msvc for cl.exe, Gcc for GCC drivers (including cross prefixes and numeric
		 * version suffixes such as g++-13), otherwise Unknown.
		 */
		CompilerKind DetectCompilerKind(const CompileCommandData& command);

		/**
		 * @brief Convert an MSVC (cl.exe) compile command into libclang arguments.
		 * @param command The compile command whose MSVC options are being converted.
		 * @param arguments Receives the libclang arguments, in MSVC compatibility mode.
		 * @return True if the options were converted successfully; otherwise false.
		 */
		bool BuildMsvcClangArguments(const CompileCommandData& command,
			std::vector<std::string>& arguments);

		/**
		 * @brief Convert a GCC (gcc/g++) compile command into libclang arguments.
		 * @param command The compile command whose GCC options are being forwarded.
		 * @param arguments Receives the libclang arguments, in native clang mode.
		 * @return True if the options were converted successfully; otherwise false.
		 */
		bool BuildGccClangArguments(const CompileCommandData& command,
			std::vector<std::string>& arguments);

		/**
		 * @brief Detect the compiler dialect and convert the compile command into libclang arguments.
		 * @details Response files referenced with @file are expanded before detection. The dialect
		 * must be MSVC (cl.exe) or GCC (gcc/g++); any other compiler is reported as an error.
		 * @param command The compile command being converted.
		 * @param arguments Receives the libclang arguments, including default C++ options.
		 * @return False when the dialect is neither MSVC nor GCC, or the conversion fails.
		 */
		bool BuildClangArguments(const CompileCommandData& command,
			std::vector<std::string>& arguments);

		/**
		 * @brief Append an existing include directory if it has not already been recorded.
		 * @param arguments Receives the generated -I argument.
		 * @param knownPaths Tracks paths already added by this helper.
		 * @param path The directory to validate and append.
		 * @return This function does not return a value; it may update arguments and knownPaths.
		 */
		void AddIncludePath(std::vector<std::string>& arguments,
			std::vector<std::string>& knownPaths, const IO::path& path);

		/**
		 * @brief Add include search paths needed to parse a header and its generated dependencies.
		 * @param header The header whose directory and module should be searched.
		 * @param generatedRoot The code generation directory holding per-module Object folders.
		 * @param arguments Receives the generated -I arguments.
		 * @param knownPaths Tracks paths already added by AddIncludePath.
		 * @return This function does not return a value; it may update arguments and knownPaths.
		 */
		void AddParseIncludePaths(const MagicEngineHeader& header,
			const IO::path& generatedRoot, std::vector<std::string>& arguments,
			std::vector<std::string>& knownPaths);
	}
}
