#include <SceneAsset/EngineAsset.h>
#include <IBufferManager.h>
#include <AbsolutePath.h>

#include <SceneAsset/Asset.h>

namespace EngineScene {

	void EngineAsset::LoadAsset()
	{
		const std::wstring content = IO::AbsolutePath::Get().GetContentPath();

		std::wstring content_folder = content + L"\\Editor\\folder.png";
		RenderCore::LoadTextureFromFile(content_folder.c_str(), EDITOR_NOHOVER_FOLDER_ID)
			;
		std::wstring content_hover_folder = content + L"\\Editor\\hover_folder.png";
		RenderCore::LoadTextureFromFile(content_hover_folder.c_str(), EDITOR_HOVER_FOLDER_ID);
	}

}
