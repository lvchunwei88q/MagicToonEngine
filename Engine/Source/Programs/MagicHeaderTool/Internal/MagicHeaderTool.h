#pragma once

#include "Tools/Tool.h"

#include "BuildPipeline/BuildPipeline.h"

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

		bool readGenerateInfoFile(const std::wstring& generateInfoPath, const std::wstring& generateDir);

		bool Run();
	private:
		bool readHeaderFiles(size_t index);

		std::wstring generateInfoFile;
		std::wstring generateDir;
		std::vector <HeadersData> headers_;
	};
}