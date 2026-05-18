#include "Component/Tools/OpenToolsView.h"
#include <ILog.h>

#include <FBXImport.h> // fbx import 
#include "Component/Editor/FileBrowserUI.h"

///////////////////////
#include <FileManager.h>
// 编码转换
#include <Converter.h>
///////////////////////

namespace RenderUI {
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
		RuntimeAssets::FBXImportSettings settings;
		settings.path = std::string(Src);
		RuntimeAssets::FBXAssetImport::Get().Import(settings);
	}

	void OpenToolsView::CreateFolder(const char* Src)
	{
		std::wstring currentPath = FileBrowserUI::Get().GetAbsolutePath(FileBrowserUI::Get().GetState().currentPath);
		std::wstring Folder = IO::Converter::ToWideString(std::string(Src));
		Folder = currentPath + L"\\" + Folder;
		IO::FileManager::MakeDirectory(Folder);
	}
}