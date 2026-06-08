#pragma once

#include "Tools/Logger.h" // 使用自己的日志系统
#include "Tools/IO.h"

#include "BuildPipeline.h"

namespace MBT {
	class MagicBuildTool
	{
	public:
		MagicBuildTool();
		~MagicBuildTool();

		bool readGenerateInfoFile(const std::wstring& generateInfoPath);
		bool readHeaderFiles();

		bool RunBuildPipeline();
	private:
		std::wstring generateInfoFile;
		std::vector<std::string> headers_;
	};
}