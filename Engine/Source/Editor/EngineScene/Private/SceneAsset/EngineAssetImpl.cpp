#include <SceneAsset/EngineAssetImpl.h>
#include <IBufferManager.h>
#include <AbsolutePath.h>

namespace EngineScene {

	EngineAsset* GetEngineAsset()
	{
		return &EngineAssetImpl::Get();
	}

	const char* EngineAssetImpl::GetIcon(EngineAssetType type, bool hover) const
	{
		const auto& pair = m_Icons.at(type);
		return hover ? pair.hover : pair.noHover;
	}

	void EngineAssetImpl::LoadAsset()
	{
		const std::wstring content = IO::AbsolutePath::Get().GetContentPath();

		{
			std::wstring content_folder = content + L"\\Editor\\folder.png";
			RenderCore::LoadTextureFromFile(content_folder.c_str(), "engineasset_editor_folder");
			std::wstring content_hover_folder = content + L"\\Editor\\hover_folder.png";
			RenderCore::LoadTextureFromFile(content_hover_folder.c_str(), "engineasset_editor_hover_folder");
			m_Icons[EngineAssetType::Folder] = { "engineasset_editor_hover_folder", "engineasset_editor_folder" };
		}

		{
			std::wstring content_folder = content + L"\\Editor\\default_file.png";
			RenderCore::LoadTextureFromFile(content_folder.c_str(), "engineasset_editor_default_file");
			std::wstring content_hover_folder = content + L"\\Editor\\hover_default_file.png";
			RenderCore::LoadTextureFromFile(content_hover_folder.c_str(), "engineasset_editor_hover_default_file");
			m_Icons[EngineAssetType::File] = { "engineasset_editor_hover_default_file", "engineasset_editor_default_file" };
		}

	}
}
