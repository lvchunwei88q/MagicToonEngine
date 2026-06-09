#include "MagicBuildTool.h"
#include <sstream>
#include <utility>
#include "Tools/Tool.h"

namespace MBT{
	namespace {
		TOOL::Timer timer; // 计时器

		void GetLine(std::string& Content,std::vector<std::string>& lines) {
			std::istringstream stream(Content);
			std::string line;
			while (std::getline(stream, line)) {
				// 去除行首行尾的空白字符
				line.erase(0, line.find_first_not_of(" \t\r\n"));
				line.erase(line.find_last_not_of(" \t\r\n") + 1);

				lines.push_back(line); // 不要去除空行因为我们需要告诉用户具体行数
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

	MagicBuildTool::MagicBuildTool() {

	}

	MagicBuildTool::~MagicBuildTool() {

	}

	bool MagicBuildTool::readGenerateInfoFile(const std::wstring& generateInfoPath)
	{
		if (TOOL::Exists(generateInfoPath)) {
			generateInfoFile = generateInfoPath;
			std::string GenerateInfo = TOOL::ReadAllText(generateInfoFile);
			std::vector<std::string> headers_lines;
			GetLine(GenerateInfo, headers_lines);

			headerForMoudel_.resize(headers_lines.size());
			headers_.resize(headers_lines.size());
			for (size_t i = 0; i < headers_lines.size(); i++)
			{
				auto [path, moudel] = SplitInput(headers_lines[i]);
				headers_[i] = TOOL::ToWideString(path);
				headerForMoudel_[i] = moudel;
			}

			TOOL::Log::Info("MagicEngine Header Num: " + std::to_string(headers_.size()));

			return true;
		}
		else {
			TOOL::Log::Error("Header list file not found at: " + TOOL::ToNarrowString(generateInfoPath)
																+ "\\engine_info\\engine_headers.txt");
			return false;
		}
	}

	bool MagicBuildTool::readHeaderFiles()
	{
		timer.reset();

		constexpr int JobSize = 100;
		size_t JobNum = (headers_.size() / JobSize) + (headers_.size() % JobSize > 0 ? 1 : 0); // 每20个文件分成一组
		std::vector<MagicEngineHeader> MagicEngineHeaders;
		TOOL::Log::Info("Start reading header files with " + std::to_string(JobNum) + " jobs");

		std::vector<std::string> AllHeadersContent = TOOL::ReadFilesParallel(headers_);
		for (size_t i = 0; i < AllHeadersContent.size(); i++)
		{
			MagicEngineHeader magicHeader;
			GetLine(AllHeadersContent[i], magicHeader.lines);

			magicHeader.headerName = TOOL::ToNarrowString(headers_[i]);
			magicHeader.moudelName = headerForMoudel_[i];
			MagicEngineHeaders.push_back(std::move(magicHeader));
		}

		MagicBuildData::Get().SetMagicEngineHeaders(MagicEngineHeaders);

		TOOL::Log::Info("Time taken to read engine file: " + std::to_string(timer.elapsed_ms()) + "ms");
		return true;
	}

	bool MagicBuildTool::RunBuildPipeline()
	{
		timer.reset(); // 重置计时器

		if (!Pipeline::FindEngineClass()) {
			TOOL::Log::Error("An error occurred during the Class-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully found Class stage Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		timer.reset(); // 重置计时器

		if (!Pipeline::FindClassMember()) {
			TOOL::Log::Error("An error occurred during the Class Member-finding phase of the build pipeline");
			return false;
		}
		TOOL::Log::Info("Pipeline construction successfully found Class Member stage Time: " + std::to_string(timer.elapsed_ms()) + "ms");
		timer.reset(); // 重置计时器

		return true;
	}
}