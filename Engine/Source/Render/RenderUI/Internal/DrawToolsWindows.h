#pragma once

namespace RenderUI {
	struct OpenToolsWindows {
		enum class OpenTools {
			NOT,
			FBXImport,
			NewFolder,

		};

		OpenTools Open = OpenTools::NOT;
	};

	extern OpenToolsWindows opentools;

	// tools function
	void CloseToolsWindow();
	void OpenToolsWindow(OpenToolsWindows::OpenTools toolwindow);

	void DrawFBXImportWindow();
	void DrawNewFolderWindow();
}