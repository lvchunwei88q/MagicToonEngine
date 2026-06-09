#pragma once

#include "Tools/Tool.h"

#include "BuildPipeline.h"

namespace MHT {

	struct HeadersData {
		std::vector<std::wstring > headers_array;
		std::vector<std::string> headerForMoudel_;
	};

	class MagicHeaderTool
	{
	public:
		MagicHeaderTool();
		~MagicHeaderTool();

		bool readGenerateInfoFile(const std::wstring& generateInfoPath);

		bool Run();
	private:
		bool readHeaderFiles(size_t index);
		bool RunBuildPipeline();

		std::wstring generateInfoFile;
		std::vector <HeadersData> headers_;
	};
}