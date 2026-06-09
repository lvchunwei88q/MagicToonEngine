#pragma once

#include "Tools/Tool.h"

#include "BuildPipeline.h"

namespace MHT {
	class MagicHeaderTool
	{
	public:
		MagicHeaderTool();
		~MagicHeaderTool();

		bool readGenerateInfoFile(const std::wstring& generateInfoPath);
		bool readHeaderFiles();

		bool RunBuildPipeline();
	private:
		std::wstring generateInfoFile;
		std::vector<std::wstring> headers_;
		std::vector<std::string> headerForMoudel_;
	};
}