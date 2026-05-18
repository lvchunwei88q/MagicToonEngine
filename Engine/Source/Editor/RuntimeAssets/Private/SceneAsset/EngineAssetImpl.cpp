#include "SceneAsset/EngineAssetImpl.h"
#include <IBufferManager.h>
#include <AbsolutePath.h>

namespace RuntimeAssets {

	EngineAsset* GetEngineAsset()
	{
		return &EngineAssetImpl::Get();
	}

	const char* EngineAssetImpl::GetIcon(EngineAssetType type) const
	{
		const auto& pair = m_Icons.at(type);
		return pair.src;
	}

	void EngineAssetImpl::LoadAsset()
	{
		const std::wstring content = IO::AbsolutePath::Get().GetContentPath();

		{
			ICON_PAIR(L"Editor\\content_browser", L"folder.png", Folder);
			ICON_PAIR(L"Editor\\content_browser", L"default_file.png", File);
			ICON_PAIR(L"Editor\\content_browser", L"ion_return_up_back.png", Return);
			ICON_PAIR(L"Editor\\content_browser", L"icons_file_type_json.png", JSON);


			//std::wstring content_folder = content + L"\\Editor\\content_browser\\folder.png";
			//RenderCore::LoadTextureFromFile(content_folder.c_str(), "engineasset_editor_folder");
			//m_Icons[EngineAssetType::Folder] = { "engineasset_editor_folder" };
		}
	}
}
