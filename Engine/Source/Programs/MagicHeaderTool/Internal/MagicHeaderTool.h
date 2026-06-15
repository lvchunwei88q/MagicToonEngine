#pragma once

#include "Tools/Tool.h"

#include "BuildPipeline/BuildPipeline.h"

#define MAGICENGINE_FILE_INFO_PATH_W L"engine_generate_info"
#define MAGICENGINE_FILE_INFO_PATH "engine_generate_info"

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
		const std::vector<std::wstring> FilterFilesNoNeedGenerated(const std::vector<std::string>& files);

		std::wstring generateInfoFile;
		std::wstring generateDir;
		std::vector <HeadersData> headers_;
	};
}