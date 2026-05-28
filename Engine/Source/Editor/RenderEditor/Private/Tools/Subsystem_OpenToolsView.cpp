#include "Tools/OpenToolsView.h"
#include <ILog.h>

#include <AssetPipeline.h> // fbx import 
#include "Editor/FileBrowserUI.h"

///////////////////////
#include <IO.h> 
///////////////////////

namespace RenderEditor {
	RENDERUI_REGISTER(OpenToolsView);

	void OpenToolsView::Init()
	{
	}

	void OpenToolsView::Uninstall()
	{
	}

	void* OpenToolsView::PublicData(uint8_t type)
	{
		return nullptr;
	}

	void OpenToolsView::OpenToolsWindow(OpenToolsWindows::OpenTools toolwindow)
	{
		if (OpenTools.Open == OpenToolsWindows::OpenTools::NOT) {
			OpenTools.Open = toolwindow;
		}
		else {
			LOG_WARNING("A modal window is already open, multiple openings are not allowed!");
		}
	}

	void OpenToolsView::FBXImport(const char* Src)
	{
		AssetPipeline::FBXImportSettings settings;
		settings.path = std::string(Src);
		AssetPipeline::GetFBXAssetImport()->Import(settings);
	}

	void OpenToolsView::CreateFolder(const char* Src)
	{
		std::wstring currentPath = FileBrowserUI::Get().GetAbsolutePath(FileBrowserUI::Get().GetState().currentPath);
		std::wstring Folder = IO::ToWideString(std::string(Src));
		Folder = currentPath + L"\\" + Folder;
		IO::MakeDirectory(Folder);
	}
}