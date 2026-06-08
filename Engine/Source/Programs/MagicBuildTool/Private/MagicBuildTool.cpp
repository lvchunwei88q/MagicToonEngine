#include "MagicBuildTool.h"
#include <sstream>
#include <utility>

namespace MBT{
	namespace {
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
		if (IO::Exists(generateInfoPath)) {
			generateInfoFile = generateInfoPath;
			std::string GenerateInfo = IO::ReadAllText(generateInfoFile);
			GetLine(GenerateInfo, headers_);

			headerForMoudel_.resize(headers_.size());
			for (size_t i = 0; i < headers_.size(); i++)
			{
				auto [path, moudel] = SplitInput(headers_[i]);
				headers_[i] = path;
				headerForMoudel_[i] = moudel;
			}

			Log::Info("MagicEngine Header Num: " + std::to_string(headers_.size()));

			return true;
		}
		else {
			Log::Error("Header list file not found at: " + IO::ToNarrowString(generateInfoPath)
																+ "\\engine_info\\engine_headers.txt");
			return false;
		}
	}

	bool MagicBuildTool::readHeaderFiles()
	{
		size_t JobNum = (headers_.size() / 20) + (headers_.size() % 20 > 0 ? 1 : 0); // 每20个文件分成一组
		std::vector<MagicEngineHeader> MagicEngineHeaders;
		Log::Info("Start reading header files with " + std::to_string(JobNum) + " jobs");

		for (size_t i = 0; i < headers_.size(); i++)
		{
			std::wstring HeaderPath = IO::ToWideString(headers_[i]);
			if (IO::Exists(HeaderPath)) {
				MagicEngineHeader magicHeader;
				std::string headerContent = IO::ReadAllText(HeaderPath);
				GetLine(headerContent, magicHeader.lines);

				magicHeader.headerName = headers_[i];
				magicHeader.moudelName = headerForMoudel_[i];
				MagicEngineHeaders.push_back(std::move(magicHeader));
			}
			else {
				Log::Error("Header file not found at: " + headers_[i]);
				return false;
			}
		}

		MagicBuildData::Get().SetMagicEngineHeaders(MagicEngineHeaders);
		return true;
	}

	bool MagicBuildTool::RunBuildPipeline()
	{
		if (!Pipeline::FindClass()) {
			Log::Error("An error occurred during the Class-finding phase of the build pipeline");
			return false;
		}
		Log::Info("Pipeline construction successfully found Class stage");

		return true;
	}
}