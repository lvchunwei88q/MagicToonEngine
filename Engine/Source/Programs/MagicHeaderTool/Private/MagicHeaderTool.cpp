#include "MagicHeaderTool.h"
#include <sstream>
#include <utility>
#include "Pipeline/GeneralPipeline.h"
#include "MagicGlobalContext.h"
#include "MagicUtilities.h"

// Core Libraries
#include <IO.h>

namespace MHT{
	namespace {
		struct LocalCheckModificationTime
		{
			std::wstring file;
			size_t time;
		};

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

		std::pair<std::wstring, std::wstring> SplitInput(const std::wstring& input) {
			size_t pos = input.find(L" || ");
			if (pos != std::string::npos) {
				std::wstring first = input.substr(0, pos);
				std::wstring second = input.substr(pos + 4); // 4 is the length of 
				return { first, second };
			}
			return { input, L"" };
		}

		const std::vector<std::wstring> FilterFilesNoNeedGenerated(const std::vector<std::string>& files)
		{
			MGC ctx = MagicGlobalContext::Get();

			std::vector<std::wstring> NeedGeneratedFiles;
			std::wstring TimeCheckFilePath = ctx.GetgenerateDir() + L"\\" MAGICENGINE_FILE_INFO_PATH_W L"\\" MAGICENGINE_HEADER_TIME_CHECK_FILE_W;
			
			std::vector<LocalCheckModificationTime> BufferForReadingTimeCheckFile;
			std::vector<LocalCheckModificationTime> BufferForWritingTimeCheckFile;

			// Read cache file
			if (IO::Exists(TimeCheckFilePath)) {
				std::string CheckInfo = IO::ReadAllText(TimeCheckFilePath);
				std::vector<std::string> FileLine;
				GetLine(CheckInfo, FileLine);

				for (size_t i = 0; i < FileLine.size(); i++)
				{
					auto& Line = FileLine[i];
					std::wstring LineW = IO::ToWideString(Line);
					auto[path, time] = SplitInput(LineW);
					size_t time_number = std::stoull(time);
					LocalCheckModificationTime CheckModificationTime = { path,time_number };
					// add to BufferForReadingTimeCheckFile
					BufferForReadingTimeCheckFile.push_back(std::move(CheckModificationTime));
				}
			}
			// No cache files, everything generated
			else {
				IO::CreateFile(TimeCheckFilePath);
			}

			for (size_t i = 0; i < files.size(); i++)
			{
				auto& file = files[i];
				std::wstring fileW = IO::ToWideString(file);
				auto [path, moudel] = SplitInput(fileW);
				// get this file modification time
				size_t ThisFileModificationTime = Utilitie::GetFileModificationTime(path);

				// If the current file does not exist in the time check cache file, add it to the time check buffer so it can be written to the cache file later. 
				// If it exists, update the time and then add it.
				bool CurrentFileAlreadyExistsInTimestampCache = false;
				for (size_t y = 0; y < BufferForReadingTimeCheckFile.size(); y++)
				{
					auto& CheckFile = BufferForReadingTimeCheckFile[y];
					if (path == CheckFile.file) {
						CurrentFileAlreadyExistsInTimestampCache = true;
						// Save these with the file's last modified time afterwards
						BufferForWritingTimeCheckFile.push_back({ path , ThisFileModificationTime });
						// Indicates that reflection code needs to be generated when the timing doesn’t match
						if (ThisFileModificationTime != CheckFile.time) {
							NeedGeneratedFiles.push_back(fileW);
						}
					}
				}
				// add to BufferForWritingTimeCheckFile
				if (!CurrentFileAlreadyExistsInTimestampCache) {
					BufferForWritingTimeCheckFile.push_back({ path , ThisFileModificationTime });
					// If it didn't exist before, the time-check file should also be updated as mentioned.
					NeedGeneratedFiles.push_back(fileW);
				}
			}

			// Update File ModificationTime
			std::wstring CheckModificationTime_ContentsW = L"";
			for (size_t i = 0; i < BufferForWritingTimeCheckFile.size(); i++)
			{
				auto& CheckFileCache = BufferForWritingTimeCheckFile[i];
				std::wstring Content = CheckFileCache.file + L" || " + std::to_wstring(CheckFileCache.time) + L"\n";
				CheckModificationTime_ContentsW += Content;

#if MAGIC_DEV
				Utilitie::Log::Info(IO::ToNarrowString(CheckFileCache.file) + " - Time: " + std::to_string(CheckFileCache.time));
#endif
			}

			// Write to file
			std::string CheckModificationTime_Contents = IO::ToNarrowString(CheckModificationTime_ContentsW);
			IO::WriteAllText(TimeCheckFilePath , CheckModificationTime_Contents);

			return NeedGeneratedFiles;
		}
	}

	bool MagicHeaderTool::readGenerateInfoFile()
	{
		MGC ctx = MagicGlobalContext::Get();
		if (IO::Exists(ctx.GetgenerateInfoFilePath())) {
			MagicGeneralPipelinedData::Get().SetGeneratePath(ctx.GetgenerateDir());

			std::string GenerateInfo = IO::ReadAllText(ctx.GetgenerateInfoFilePath());
			std::vector<std::string> headers_lines_file;
			GetLine(GenerateInfo, headers_lines_file);

			// To compile faster and avoid unnecessary builds, it will check the modification time to decide whether reflection code needs to be generated.
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
			Utilitie::Log::Error("Header list file not found at: " + IO::ToNarrowString(ctx.GetgenerateInfoFilePath())
																+ "\\" MAGICENGINE_FILE_INFO_PATH "\\" MAGICENGINE_HEADER_INFO_FILE);
			return false;
		}
	}

	bool MagicHeaderTool::Run() {
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

		return true;
	}

	bool MagicHeaderTool::readHeaderFiles()
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

		std::vector<std::string> AllHeadersContent = Utilitie::ReadFilesParallel(HeaderDescriptorData.ArrayOfHeaderFilePaths);
		for (size_t i = 0; i < AllHeadersContent.size(); i++)
		{
			MagicEngineHeader magicHeader;
			GetLine(AllHeadersContent[i], magicHeader.lines);

			magicHeader.headerName = IO::ToNarrowString(HeaderDescriptorData.ArrayOfHeaderFilePaths[i]);
			magicHeader.moudelName = HeaderDescriptorData.HeaderFileComesFromModuleArray[i];
			MagicEngineHeaders.push_back(std::move(magicHeader));
		}

		MagicGeneralPipelinedData::Get().SetMagicEngineHeaders(MagicEngineHeaders);

		Utilitie::Log::Info("Time taken to read engine file: " + std::to_string(GTimer.elapsed_ms()) + "ms");
		return true;
	}
}
