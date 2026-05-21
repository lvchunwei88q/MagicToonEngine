#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>

using namespace RenderUI;

namespace RenderEditor {
	struct OpenToolsWindows {
		enum class OpenTools {
			NOT,
			FBXImport,
			NewFolder,

		};

		OpenTools Open = OpenTools::NOT;
	};

	class OpenToolsView : public RSubsystemTemplate<OpenToolsView, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

		// tools function
		void OpenToolsWindow(OpenToolsWindows::OpenTools toolwindow);

	private:
		void CloseToolsWindow();

		void DrawFBXImportWindow();
		void DrawNewFolderWindow();

		void FBXImport(const char* Src);
		void CreateFolder(const char* Src);
	private:
		OpenToolsWindows OpenTools;
	};
}