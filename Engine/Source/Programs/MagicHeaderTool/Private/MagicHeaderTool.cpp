#include "MagicHeaderTool.h"
#include <sstream>
#include <utility>
#include "Tools/Tool.h"

namespace MHT{
	namespace {
		TOOL::Timer timer; // 计时器

		void GetLine(std::string& Content, std::vector<std::string>& lines) {
			std::istringstream stream(Content);
			std::string line;
			while (std::getline(stream, line)) {
				// 使用 trim 函数
				auto trim = [](std::string& s) {
					// 去除行首
					size_t start = s.find_first_not_of(" \t\r\n");
					if (start == std::string::npos) {
						s.clear();
						return;
					}
					// 去除行尾
					size_t end = s.find_last_not_of(" \t\r\n");
					s = s.substr(start, end - start + 1);
					};

				trim(line);
				lines.push_back(line);  // 空行也会被保留（空字符串）
			}
		}

		std::pair<std::wstring, std::wstring> SplitInput(const std::wstring& input) {
			size_t pos = input.find(L" || ");
			if (pos != std::string::npos) {
				std::wstring first = input.substr(0, pos);
				std::wstring second = input.substr(pos + 4); // 4 是 " || " 的长度
				return { first, second };
			}
			return { input, L"" };
		}
	}

	struct LocalCheckFile
	{
		std::wstring file;
		size_t time;
	};

	MagicHeaderTool::MagicHeaderTool() {

	}

	MagicHeaderTool::~MagicHeaderTool() {

	}

	bool MagicHeaderTool::readGenerateInfoFile(const std::wstring& generateInfoPath,const std::wstring& generateDir)
	{
		if (TOOL::Exists(generateInfoPath)) {
			generateInfoFile = generateInfoPath;
			this->generateDir = generateDir;
			MagicBuildData::Get().SetGeneratePath(this->generateDir);

			std::string GenerateInfo = TOOL::ReadAllText(generateInfoFile);
			std::vector<std::string> headers_lines_file;
			GetLine(GenerateInfo, headers_lines_file);

			// 下面我们看看哪些文件是修改过的即改变了修改时间
			std::vector<std::wstring> NeedGeneratedFiles = FilterFilesNoNeedGenerated(headers_lines_file);

			CF::MHT_JobNum = (NeedGeneratedFiles.size() + CF::SingleJobCapacity - 1) / CF::SingleJobCapacity;

			// 预先分配空间
			headers_.resize(CF::MHT_JobNum);
			for (size_t i = 0; i < CF::MHT_JobNum; i++)
			{
				if (i + 1 < CF::MHT_JobNum) {
					headers_[i].headers_array.resize(CF::SingleJobCapacity);
					headers_[i].headerForMoudel_.resize(CF::SingleJobCapacity);
				}
				else {
					size_t Remaining = NeedGeneratedFiles.size() - i * CF::SingleJobCapacity;
					headers_[i].headers_array.resize(Remaining);
					headers_[i].headerForMoudel_.resize(Remaining);
				}
			}
			// End

			// 写入
			for (size_t i = 0; i < CF::MHT_JobNum; i++) {
				auto& headers = headers_[i];

				for (size_t y = 0; y < headers.headers_array.size(); y++)
				{
					size_t startindex = i * CF::SingleJobCapacity;
					auto [path, moudel] = SplitInput(NeedGeneratedFiles[startindex + y]);
					headers.headers_array[y] = path;
					headers.headerForMoudel_[y] = TOOL::ToNarrowString(moudel);
				}
			}

			TOOL::Log::Info("MagicEngine Header Job Num: " + std::to_string(headers_.size()));

			return true;
		}
		else {
			TOOL::Log::Error("Header list file not found at: " + TOOL::ToNarrowString(generateInfoPath)
																+ "\\" MAGICENGINE_FILE_INFO_PATH "\\engine_headers.buildmeta");
			return false;
		}
	}

	bool MagicHeaderTool::Run() {
		for (size_t i = 0; i < CF::MHT_JobNum; i++)
		{
			// Set Current Job
			CF::MHT_CurrentJob = i + 1;

			auto& headers = headers_[i];
			TOOL::Log::Info("Current number of tasks being processed: " + std::to_string(headers.headers_array.size()));
			Pipeline::ResetStatus();

			MagicBuildData::Get().clear();
			if (!readHeaderFiles(i)) {
				return false;
			}
			if (!Pipeline::RunBuildPipeline()) {
				return false;
			}
		}

		return true;
	}

	bool MagicHeaderTool::readHeaderFiles(size_t index)
	{
		timer.reset();

		auto& headers = headers_[index];

		std::vector<MagicEngineHeader> MagicEngineHeaders;

		std::vector<std::string> AllHeadersContent = TOOL::ReadFilesParallel(headers.headers_array);
		for (size_t i = 0; i < AllHeadersContent.size(); i++)
		{
			MagicEngineHeader magicHeader;
			GetLine(AllHeadersContent[i], magicHeader.lines);

			magicHeader.headerName = TOOL::ToNarrowString(headers.headers_array[i]);
			magicHeader.moudelName = headers.headerForMoudel_[i];
			MagicEngineHeaders.push_back(std::move(magicHeader));
		}

		MagicBuildData::Get().SetMagicEngineHeaders(MagicEngineHeaders);

		TOOL::Log::Info("Time taken to read engine file: " + std::to_string(timer.elapsed_ms()) + "ms");
		return true;
	}

	const std::vector<std::wstring> MagicHeaderTool::FilterFilesNoNeedGenerated(const std::vector<std::string>& files)
	{
		std::vector<std::wstring> NeedGeneratedFiles;
		std::wstring TimeCheckFilePath = generateDir + L"\\" MAGICENGINE_FILE_INFO_PATH_W L"\\engine_headers_check_file.buildmeta";
		std::vector<LocalCheckFile> CheckFiles;
		std::vector<LocalCheckFile> NewCheckFileCache;

		// 读取缓存文件
		if (TOOL::Exists(TimeCheckFilePath)) {
			std::string CheckInfo = TOOL::ReadAllText(TimeCheckFilePath);
			std::vector<std::string> FileLine;
			GetLine(CheckInfo, FileLine);

			for (size_t i = 0; i < FileLine.size(); i++)
			{
				auto& Line = FileLine[i];
				std::wstring LineW = TOOL::ToWideString(Line);
				auto[path, time] = SplitInput(LineW);

				size_t time_number = std::stoull(time);

				LocalCheckFile CheckFile = { path,time_number };
				CheckFiles.push_back(std::move(CheckFile));
			}
		}
		else {
			// 没有缓存文件全部生成
			TOOL::MakeFile(TimeCheckFilePath);
		}

		for (size_t i = 0; i < files.size(); i++)
		{
			auto& file = files[i];
			std::wstring fileW = TOOL::ToWideString(file);
			auto [path, moudel] = SplitInput(fileW);
			size_t CheckTime = TOOL::GetFileModificationTime(path);

			// 没有检测到的文件直接添加，检测到的但是时间不同的也添加
			bool CheckCurrentFile = false;
			for (size_t y = 0; y < CheckFiles.size(); y++)
			{
				auto& CheckFile = CheckFiles[y];
				if (path == CheckFile.file) {
					CheckCurrentFile = true;
					// 之后我们将这些文件的修改时间保存
					NewCheckFileCache.push_back({ path , CheckTime });
					if (CheckTime != CheckFile.time) { // 时间不一致
						NeedGeneratedFiles.push_back(fileW);
					}
				}
			}
			if (!CheckCurrentFile) {
				NewCheckFileCache.push_back({ path , CheckTime });
				NeedGeneratedFiles.push_back(fileW);
			}
		}

		std::wstring CheckFileCache_ContentsW = L"";
		for (size_t i = 0; i < NewCheckFileCache.size(); i++)
		{
			auto& CheckFileCache = NewCheckFileCache[i];
			std::wstring Content = CheckFileCache.file + L" || " + std::to_wstring(CheckFileCache.time) + L"\n";
			CheckFileCache_ContentsW += Content;

#if 0
			TOOL::Log::Info(TOOL::ToNarrowString(CheckFileCache.file) + " - Time: " + std::to_string(CheckFileCache.time));
#endif
		}

		std::string CheckFileCache_Contents = TOOL::ToNarrowString(CheckFileCache_ContentsW);
		TOOL::WriteAllText(TimeCheckFilePath , CheckFileCache_Contents);

		return NeedGeneratedFiles;
	}
}