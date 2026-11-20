#pragma once

#include "Common/CORE_API.h"
#include "Common/Checker.h"
#include "Utilities/Singleton.h"
#include "Utilities/BinaryRW.h"

#include <filesystem>
#include <string>
#include <vector>

#define ENGINE_CACHE IO::AbsP::Get().GetEngineDirectory() / u8"Cache"
#define ENGINE_CONFIG IO::AbsP::Get().GetEngineDirectory() / u8"Config"

// Get rid of Windows macro clutter
#undef CreateFile
#undef CreateDirectory
#undef MoveFile

DISABLE_DLL_WARNINGS_PUSH;

/*
* Note that the 'bool' here doesn't indicate whether an operation succeeded or failed;
* in some cases, it might mean something different. For details, check the STL source code.
*/

namespace IO {
	// Define it in a namespace to prevent name collisions
	namespace fs = std::filesystem;
	using path = fs::path;

	// Read text to file
	CORE_API std::string ReadAllText(const path& path);
	// Read bytes to file
	CORE_API std::vector<char> ReadAllCharBytes(const path& path);
	CORE_API std::vector<uint8_t> ReadAllUINT8Bytes(const path& path);
	CORE_API BinaryReader ReadAllBinaryReaderBytes(const path& path);
	// Write string to file
	CORE_API void WriteAllText(const path& path, const std::string& content);
	// Write byte to file
	CORE_API void WriteAllBytes(const path& path, const std::vector<char>& data);
	CORE_API void WriteAllBytes(const path& path, const std::vector<uint8_t>& data);
	CORE_API void WriteAllBytes(const path& path, const BinaryWriter& data);
	CORE_API void WriteAllBytes(const path& path, const uint8_t* data, size_t size);
	CORE_API void WriteAllBytes(const path& path, const char* data, size_t size);
	// Append a string to the end of the file
	CORE_API void AppendText(const path& path, const std::string& content);
	// Append bytes to the end of the file
	CORE_API void AppendBytes(const path& path, const std::vector<char>& data);
	CORE_API void AppendBytes(const path& path, const std::vector<uint8_t>& data);
	CORE_API void AppendBytes(const path& path, const BinaryWriter& data);
	CORE_API void AppendBytes(const path& path, const uint8_t* data, size_t size);
	CORE_API void AppendBytes(const path& path, const char* data, size_t size);
	// deleta file
	CORE_API bool DeleteToFile(const path& path);
	// deleta directory
	CORE_API bool DeleteToDirectory(const path& path);
	// Create an empty file (or update its timestamp if the file already exists)
	CORE_API bool CreateFile(const path& path);
	// Move/Rename Files or Folders
	CORE_API bool MoveFile(const path& src, const path& dst);
	// Get the file directory
	CORE_API bool GetFileDirectory(const path& src, path& dst);
	// Get the file name
	CORE_API bool GetFileName(const path& src, path& dst);
	// Create directory (including parent directories)
	CORE_API bool CreateDirectory(const path& path);
	// Check if a file or directory exists
	CORE_API bool Exists(const path& path);
	// Scan all files in the specified directory (excluding subdirectories) and return a list of file names
	CORE_API std::vector<std::string> GetFilesInDirectory(const path& path);
	// Convert narrow characters to wide characters (UTF-8 to UTF-16)
	CORE_API std::wstring ToWideString(const std::string& narrow);
	// Convert wide characters to narrow characters (UTF-16 to UTF-8)
	CORE_API std::string ToNarrowString(const std::wstring& wide);

	struct BaseDirectory {
		explicit BaseDirectory(const path& Root) : Root(Root) {};
		BaseDirectory() = delete;

		path GetRoot() const { return Root; }
	private:
		path Root;
	};
	// WorkingDirectory == Project Directory
	struct WorkingDirectory : public BaseDirectory
	{
		explicit WorkingDirectory(const path& Root) : BaseDirectory(Root) {};
		WorkingDirectory() = delete;

		path GetContent() const { return GetRoot() / u8"Content"; };
		path GetConfig() const { return GetRoot() / u8"Config"; };
		path GetCache() const { return GetRoot() / u8"Cache"; };
	};

	struct ContentDirectory : public BaseDirectory
	{
		explicit ContentDirectory(const path& Root) : BaseDirectory(Root) {};
		ContentDirectory() = delete;

		path GetEditor() const { return GetRoot() / u8"Editor"; };
		path GetConfig() const { return GetRoot() / u8"Config"; };
		path GetLogo() const { return GetRoot() / u8"Logo"; };
		path GetWindow() const { return GetRoot() / u8"Window"; };
	};

	struct ScriptDirectory : public BaseDirectory
	{
		explicit ScriptDirectory(const path& Root) : BaseDirectory(Root) {};
		ScriptDirectory() = delete;
	};

	struct EngineOntologyDirectory : public BaseDirectory
	{
		explicit EngineOntologyDirectory(const path& Root) : BaseDirectory(Root) {};
		EngineOntologyDirectory() = delete;

		path GetConfig() const { return GetRoot() / u8"Config"; };
		path GetCache() const { return GetRoot() / u8"Cache"; };
	};

	class CORE_API AbsolutePath : public Singleton<AbsolutePath> // Absolute Path
	{
	public:
		AbsolutePath() = default;

		// Get the directory where the Engine file is located
		path GetEngineDirectory();
		// Get the full path of the Engine file
		path GetEnginePath();

		// Get current working directory
		WorkingDirectory GetCurrentWorkingDirectory();
		// Get content directory
		ContentDirectory GetContentDirectory();
		// Get script directory
		ScriptDirectory GetScriptDirectory();
		// Get engine ontology directory
		EngineOntologyDirectory GetEngineOntologyDirectory();

		// Clear cache
		void ClearCache();
	private:
		// Cache the path results to avoid recalculating
		path EngineDirectory;
		path EnginePath;
		
		path CurrentWorkingDirectory;
		path ContentPath;
		path ScriptPath;
		path EngineOntologyPath;
	};

	using AbsP = AbsolutePath;
}

DISABLE_DLL_WARNINGS_POP;