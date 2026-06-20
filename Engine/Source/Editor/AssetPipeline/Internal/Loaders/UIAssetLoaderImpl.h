#pragma once
#include "AssetPipeline.h"
#include <Tools/Singleton.h>
#include <unordered_map>

namespace AssetPipeline {
	// 加载 Editor Assets

#define ICON(content_path,name, type) \
    std::wstring _p##type = editor_content + L"\\" content_path L"\\" name; \
    RenderCore::LoadTextureFromFile(_p##type.c_str(), "uiasset_editor_" #type);

#define ICON_PAIR(content_path,name, type) \
    ICON(content_path,name, type) \
    m_Icons[UIAssetType::type] = { "uiasset_editor_" #type};

	class UIAssetImpl : public UIAsset, public Singleton<UIAssetImpl>
	{
	public:
		UIAssetImpl() {};
		void LoadAsset();

		virtual const char* GetIcon(UIAssetType type) const override;
	private:
		std::unordered_map<UIAssetType, UIAssetPair> m_Icons;
	};
}