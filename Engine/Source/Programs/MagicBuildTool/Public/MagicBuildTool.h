#pragma once

#include "Common/MBT_API.h"
#include "Logger.h" // 使用自己的日志系统
#include "IO.h"

namespace MBT {
	struct MagicEngineHeaders
	{
		std::string headerName;
		std::vector<std::string> lines;
	};

	class MagicBuildTool
	{
	public:
		MagicBuildTool();
		~MagicBuildTool();

		bool readGenerateInfoFile(const std::wstring& generateInfoPath);
		bool readHeaderFiles();

	private:
		std::wstring generateInfoFile;
		std::vector<std::string> headers_;
		std::vector<MagicEngineHeaders> magicEngineHeaders;
	};
}