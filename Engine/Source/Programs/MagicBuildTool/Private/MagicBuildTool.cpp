#include "MagicBuildTool.h"
#include <sstream>

namespace MBT{
	namespace {
		void GetLine(std::string& Content,std::vector<std::string>& lines) {
			std::istringstream stream(Content);
			std::string line;
			while (std::getline(stream, line)) {
				// 去除行首行尾的空白字符
				line.erase(0, line.find_first_not_of(" \t\r\n"));
				line.erase(line.find_last_not_of(" \t\r\n") + 1);

				if (!line.empty()) {
					lines.push_back(line);
				}
			}
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
		Log::Info("Start reading header files with " + std::to_string(JobNum) + " jobs");
		for (size_t i = 0; i < headers_.size(); i++)
		{
			std::wstring HeaderPath = IO::ToWideString(headers_[i]);
			if (IO::Exists(HeaderPath)) {
				MagicEngineHeaders magicHeader;
				std::string headerContent = IO::ReadAllText(HeaderPath);
				GetLine(headerContent, magicHeader.lines);
				magicHeader.headerName = headers_[i];

				magicEngineHeaders.push_back(std::move(magicHeader));
			}
			else {
				Log::Error("Header file not found at: " + headers_[i]);
				return false;
			}
		}
		return false;
	}
}