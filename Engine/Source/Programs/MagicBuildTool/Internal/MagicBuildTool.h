#pragma once

#include "Tools/Tool.h"

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
		std::vector<std::wstring> headers_;
		std::vector<std::string> headerForMoudel_;
	};
}