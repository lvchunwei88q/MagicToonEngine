#pragma once
#include <SceneAsset/EngineAsset.h>
#include <Tools/Singleton.h>
#include <unordered_map>

namespace RuntimeAssets {
	// 加载 Engine Assets

#define ICON(content_path,name, type) \
    std::wstring _p##type = content + L"\\" content_path L"\\" name; \
    RenderCore::LoadTextureFromFile(_p##type.c_str(), "engineasset_editor_" #type);

#define ICON_PAIR(content_path,name, type) \
    ICON(content_path,name, type) \
    m_Icons[EngineAssetType::type] = { "engineasset_editor_" #type};

	class EngineAssetImpl : public EngineAsset, public Singleton<EngineAssetImpl>
	{
	public:
		EngineAssetImpl() {};
		void LoadAsset();

		virtual const char* GetIcon(EngineAssetType type) const override;
	private:
		std::unordered_map<EngineAssetType, EngineAssetPair> m_Icons;
	};
}