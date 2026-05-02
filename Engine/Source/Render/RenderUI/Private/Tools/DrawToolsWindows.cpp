#include <GuiInterface.h>
#include <DrawToolsWindows.h>

#include <Log.h>

namespace RenderUI {
	OpenToolsWindows opentools;

	void CloseToolsWindow()
	{
		opentools.Open = OpenToolsWindows::OpenTools::NOT;
	}

	void OpenToolsWindow(OpenToolsWindows::OpenTools toolwindow)
	{
		if (opentools.Open == OpenToolsWindows::OpenTools::NOT) {
			opentools.Open = toolwindow;
		}
		else {
			LOG_WARNING("A modal window is already open, multiple openings are not allowed!");
		}
	}

	void DrawToolsWindows() {

		switch (opentools.Open)
		{
			case OpenToolsWindows::OpenTools::FBXImport:
				DrawFBXImportWindow();
			break;
		default:
			break;
		}
	}

}