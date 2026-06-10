#include "MagicHeaderTool.h"
#include <sstream>
#include <utility>
#include "Tools/Tool.h"

namespace MHT{
	namespace Pipeline{
		GenerationState GenerationState_;
	}
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

		std::pair<std::string, std::string> SplitInput(const std::string& input) {
			size_t pos = input.find(" || ");
			if (pos != std::string::npos) {
				std::string first = input.substr(0, pos);
				std::string second = input.substr(pos + 4); // 4 是 " || " 的长度
				return { first, second };
			}
			return { input, "" };
		}
	}

	MagicHeaderTool::MagicHeaderTool() {

	}

	MagicHeaderTool::~MagicHeaderTool() {

	}

	bool MagicHeaderTool::readGenerateInfoFile(const std::wstring& generateInfoPath,const std::wstring& generateDir)
	{
		if (TOOL::Exists(generateInfoPath)) {
			generateInfoFile = generateInfoPath;
			this->generateDir = generateDir;
			MagicBuildData::Get().SetGeneratePath(generateDir);

			std::string GenerateInfo = TOOL::ReadAllText(generateInfoFile);
			std::vector<std::string> headers_lines;
			GetLine(GenerateInfo, headers_lines);
			CF::MHT_JobNum = (headers_lines.size() + CF::SingleJobCapacity - 1) / CF::SingleJobCapacity;

			headers_.resize(CF::MHT_JobNum);
			for (size_t i = 0; i < CF::MHT_JobNum; i++)
			{
				if (i + 1 < CF::MHT_JobNum) {
					headers_[i].headers_array.resize(CF::SingleJobCapacity);
					headers_[i].headerForMoudel_.resize(CF::SingleJobCapacity);
				}
				else {
					size_t Remaining = headers_lines.size() - i * CF::SingleJobCapacity;
					headers_[i].headers_array.resize(Remaining);
					headers_[i].headerForMoudel_.resize(Remaining);
				}
			}

			for (size_t i = 0; i < CF::MHT_JobNum; i++) {
				auto& headers = headers_[i];

				for (size_t y = 0; y < headers.headers_array.size(); y++)
				{
					size_t startindex = i * CF::SingleJobCapacity;
					auto [path, moudel] = SplitInput(headers_lines[startindex + y]);
					headers.headers_array[y] = TOOL::ToWideString(path);
					headers.headerForMoudel_[y] = moudel;
				}
			}

			TOOL::Log::Info("MagicEngine Header Job Num: " + std::to_string(headers_.size()));

			return true;
		}
		else {
			TOOL::Log::Error("Header list file not found at: " + TOOL::ToNarrowString(generateInfoPath)
																+ "\\engine_info\\engine_headers.txt");
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
			if (!RunBuildPipeline()) {
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

	bool MagicHeaderTool::RunBuildPipeline()
	{
		timer.reset(); // 重置计时器

		if (!Pipeline::FindEngineClass()) {
			TOOL::Log::Error("An error occurred during the Class-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In found Class stage Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if(!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

		if (!Pipeline::FindClassMember()) {
			TOOL::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In found Class Member stage Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

		if (!Pipeline::GenerateObjectMetadata()) {
			TOOL::Log::Error("An error occurred during the Object metadata generation phase of building the pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In Generate Object Metadata Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

		if (!Pipeline::GenerateMetadataFile()) {
			TOOL::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully, In Generate metadata file Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		if (!Pipeline::NeedGenerate())goto end;
		timer.reset(); // 重置计时器

	end:
		return true;
	}
}