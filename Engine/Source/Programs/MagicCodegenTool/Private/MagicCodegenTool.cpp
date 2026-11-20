#include "MagicCodegenTool.h"
#include <sstream>
#include <utility>
#include <unordered_map>
#include <limits>
#include "Pipeline/GeneralPipeline.h"
#include "MagicGlobalContext.h"
#include "MagicUtilities.h"

// Core Libraries
#include <IO.h>

namespace MCT{
	namespace {
		void GetLine(std::string& Content, std::vector<std::string>& lines) {
			std::istringstream stream(Content);
			std::string line;
			while (std::getline(stream, line)) {
				// use trim function
				auto trim = [](std::string& s) {
					// Remove leading spaces
					size_t start = s.find_first_not_of(" \t\r\n");
					if (start == std::string::npos) {
						s.clear();
						return;
					}
					// Remove line endings
					size_t end = s.find_last_not_of(" \t\r\n");
					s = s.substr(start, end - start + 1);
					};

				trim(line);
				lines.push_back(line);  // Blank lines will also be kept (empty string)
			}
		}

		std::pair<IO::path, std::wstring> SplitInput(const std::wstring& input) {
			size_t pos = input.find(L" || ");
			if (pos != std::string::npos) {
				std::wstring first = input.substr(0, pos);
				std::wstring second = input.substr(pos + 4); // 4 is the length of 
				return { IO::path(first), second };
			}
			return { input, L"" };
		}

		// Hex is easier to eyeball in the .buildmeta file than a 20 digit decimal.
		std::wstring ToHexHash(uint64_t hash) {
			std::wstringstream stream;
			stream << std::hex << hash;
			return stream.str();
		}

		const std::vector<std::wstring> FilterFilesNoNeedGenerated(const std::vector<std::string>& files)
		{
			// Get the global context
			MGC ctx = MagicGlobalContext::Get();

			std::vector<std::wstring> NeedGeneratedFiles;
			const IO::path hashCheckFilePath =
				ctx.GetgenerateDir() / MAGICENGINE_FILE_INFO_PATH / MAGICENGINE_HEADER_HASH_CHECK_FILE;

			// Path -> content hash recorded by the last successful generation.
			std::unordered_map<std::wstring, uint64_t> CachedHashes;

			// Read cache file
			if (IO::Exists(hashCheckFilePath.native())) {
				std::string CheckInfo = IO::ReadAllText(hashCheckFilePath.native());
				std::vector<std::string> FileLine;
				GetLine(CheckInfo, FileLine);

				for (size_t i = 0; i < FileLine.size(); i++)
				{
					const std::string& Line = FileLine[i];
					if (Line.empty()) {
						continue;
					}

					auto [path, hashText] = SplitInput(IO::ToWideString(Line));
					if (path.empty() || hashText.empty()) {
						continue;
					}

					uint64_t CachedHash = 0;
					try {
						const std::string HashText = IO::ToNarrowString(hashText);
						if (HashText.empty() || HashText.front() == '-' || HashText.front() == '+') {
							continue;
						}

						size_t ParsedLength = 0;
						const uint64_t ParsedHash =
							std::stoull(HashText, &ParsedLength, 16);
						if (ParsedLength != HashText.size() ||
							ParsedHash > std::numeric_limits<uint64_t>::max()) {
							continue;
						}

						CachedHash = ParsedHash;
					}
					catch (const std::exception&) {
						// Unreadable line (old format or corruption): drop it, the file is regenerated once.
						continue;
					}

					CachedHashes[path] = CachedHash;
				}
			}
			// No cache files, everything generated
			else {
				IO::CreateFile(hashCheckFilePath.native());
			}

			// Lines are "<path> || <module>": split once, then hash every file's content in parallel.
			std::vector<IO::path> FilePaths(files.size());
			for (size_t i = 0; i < files.size(); i++)
			{
				// Only the path part may reach the file system - the raw line still contains
				// " || <module>", so opening it directly would always fail and yield hash 0.
				FilePaths[i] = IO::path(SplitInput(IO::ToWideString(files[i])).first);
			}

			std::vector<uint64_t> CurrentHashes;
			Utilitie::GetFileContentHashesParallel(FilePaths, CurrentHashes);

			// Rebuild the cache contents; it is written only after a successful generation.
			std::wstring HashCheck_ContentsW = L"";
			for (size_t i = 0; i < files.size(); i++)
			{
				// Copy out of the path: a const reference here would bind to a temporary
				// produced by path -> wstring conversion.
				const std::wstring path = FilePaths[i].wstring();
				const uint64_t ThisFileHash = CurrentHashes[i];

				HashCheck_ContentsW += path + L" || " + ToHexHash(ThisFileHash) + L"\n";

				auto CachedHashIt = CachedHashes.find(path);
				const bool bGeneratedBefore = (CachedHashIt != CachedHashes.end());

				// The content hash decides: a file that was only touched (mtime changed,
				// bytes unchanged) no longer triggers a regeneration.
				// Hash 0 means the file could not be read - regenerate so the error surfaces.
				if (!bGeneratedBefore || ThisFileHash == 0 || CachedHashIt->second != ThisFileHash) {
					NeedGeneratedFiles.push_back(IO::ToWideString(files[i]));
				}

#if MAGIC_DEV
				Utilitie::Log::Info(IO::ToNarrowString(path) + " - Hash: " + IO::ToNarrowString(ToHexHash(ThisFileHash)));
#endif
			}

			// Save to GlobalContext
			ctx.SetHeaderHashCheckContents(IO::ToNarrowString(HashCheck_ContentsW));

			return NeedGeneratedFiles;
		}
	}

	bool MagicCodegenTool::readGenerateInfoFile()
	{
		MGC ctx = MagicGlobalContext::Get();
		if (IO::Exists(ctx.GetgenerateInfoFilePath().native())) {
			MagicGeneralPipelinedData::Get().SetGeneratePath(ctx.GetgenerateDir());

			std::string GenerateInfo = IO::ReadAllText(ctx.GetgenerateInfoFilePath().native());
			std::vector<std::string> headers_lines_file;
			GetLine(GenerateInfo, headers_lines_file);

			// To compile faster and avoid unnecessary builds, the content hash of every
			// file decides whether reflection code needs to be generated.
#if !MAGIC_DEV
			std::vector<std::wstring> NeedGeneratedFileLists = FilterFilesNoNeedGenerated(headers_lines_file);
#else
			std::vector<std::wstring> NeedGeneratedFileLists;
			for (const auto& header_file : headers_lines_file)
			{
				std::wstring fileW = IO::ToWideString(header_file);
				NeedGeneratedFileLists.push_back(fileW);
			}
#endif

			// Get All Task Num
			ctx.GetRefInfo().TaskNum = (NeedGeneratedFileLists.size() + MAXIMUM_CAPACITY_PER_JOB - 1) / MAXIMUM_CAPACITY_PER_JOB;
			Context::TaskInfo TInfo = ctx.GetInfo();
			std::vector<Context::HeaderDescriptorData>& HDDs = ctx.GetRefHDDs();

			// Pre-allocate space
			HDDs.resize(TInfo.TaskNum);
			for (size_t i = 0; i < TInfo.TaskNum; i++)
			{
				// If it's not the last task (when there are multiple tasks)
				if (i + 1 < TInfo.TaskNum) {
					HDDs[i].ArrayOfHeaderFilePaths.resize(MAXIMUM_CAPACITY_PER_JOB);
					HDDs[i].HeaderFileComesFromModuleArray.resize(MAXIMUM_CAPACITY_PER_JOB);
				}
				// Allocate space for the last task here; the amount to handle will be less than or equal to the maximum amount.
				else {
					// Get Remaining Job
					size_t Remaining = NeedGeneratedFileLists.size() - i * MAXIMUM_CAPACITY_PER_JOB;
					HDDs[i].ArrayOfHeaderFilePaths.resize(Remaining);
					HDDs[i].HeaderFileComesFromModuleArray.resize(Remaining);
				}
			}

			// Write the module and path each header file belongs to
			for (size_t i = 0; i < TInfo.TaskNum; i++) {
				auto& headerDescriptorData = HDDs[i];
				size_t startindex = i * MAXIMUM_CAPACITY_PER_JOB;
				size_t CurrentHeaderNum = headerDescriptorData.ArrayOfHeaderFilePaths.size();

				for (size_t y = 0; y < CurrentHeaderNum; y++)
				{
					auto [path, moudel] = SplitInput(NeedGeneratedFileLists[startindex + y]);
					headerDescriptorData.ArrayOfHeaderFilePaths[y] = path;
					headerDescriptorData.HeaderFileComesFromModuleArray[y] = IO::ToNarrowString(moudel);
				}
			}

			Utilitie::Log::Info("MagicEngine Header Job Num: " + std::to_string(HDDs.size()));

			return true;
		}
		else {
			Utilitie::Log::Error(
				"Header list file not found at: " +
				ctx.GetgenerateInfoFilePath().string());
			return false;
		}
	}

	bool MagicCodegenTool::Run() {
		// Get the global context
		MGC ctx = MagicGlobalContext::Get();
		Context::TaskInfo& TInfo = ctx.GetRefInfo();
		std::vector<Context::HeaderDescriptorData>& HDDs = ctx.GetRefHDDs();
		for (size_t i = 0; i < TInfo.TaskNum; i++)
		{
			// Update Current Job
			TInfo.CurrentTaskIndex = i;

			auto& headers = HDDs[i];
			Utilitie::Log::Info("Current number of tasks being processed: " + std::to_string(headers.ArrayOfHeaderFilePaths.size()));
			ctx.SetGeneratePending();

			MagicGeneralPipelinedData::Get().clearAll();
			if (!readHeaderFiles()) {
				return false;
			}
			if (!GeneralPipeline::RunBuildPipeline()) {
				return false;
			}
		}

		// All tasks completed! Updating the content of the check file.
		const IO::path hashCheckFilePath =
			ctx.GetgenerateDir() / MAGICENGINE_FILE_INFO_PATH / MAGICENGINE_HEADER_HASH_CHECK_FILE;
		if(ctx.HeaderHashCheckContents_isValid())
		{
			IO::WriteAllText(hashCheckFilePath.native(), ctx.GetHeaderHashCheckContents());
		}
		else
		{
			Utilitie::Log::Error("HeaderHashCheckContents is invalid. Cannot write to HashCheckFile.");
			return false;
		}

		return true;
	}

	bool MagicCodegenTool::readHeaderFiles()
	{
		Utilitie::GTimer GTimer = Utilitie::GlobalTimer::Get();
		// Start the timer
		GTimer.start();

		// Get MagicGlobalContext
		MGC ctx = MagicGlobalContext::Get();
		Context::TaskInfo TInfo = ctx.GetInfo();
		std::vector<Context::HeaderDescriptorData>& HDDs = ctx.GetRefHDDs();
		auto& HeaderDescriptorData = ctx.GetCurrentTaskHeaderDescriptorData();

		std::vector<MagicEngineHeader> MagicEngineHeaders;

		std::vector<std::string> AllHeadersContent;
		if (!Utilitie::ReadFilesParallel(HeaderDescriptorData.ArrayOfHeaderFilePaths, AllHeadersContent)) {
			return false;
		}
		for (size_t i = 0; i < AllHeadersContent.size(); i++)
		{
			MagicEngineHeader magicHeader;
			GetLine(AllHeadersContent[i], magicHeader.lines);

			magicHeader.headerPath = HeaderDescriptorData.ArrayOfHeaderFilePaths[i];
			magicHeader.moduleName = HeaderDescriptorData.HeaderFileComesFromModuleArray[i];
			MagicEngineHeaders.push_back(std::move(magicHeader));
		}

		MagicGeneralPipelinedData::Get().SetMagicEngineHeaders(MagicEngineHeaders);

		Utilitie::Log::Info("Time taken to read engine file: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		return true;
	}
}
