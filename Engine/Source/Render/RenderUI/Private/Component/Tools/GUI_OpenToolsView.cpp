#include <Component/Tools/OpenToolsView.h>
#include <EditorUIWarehouse.h>

namespace RenderUI {
	void OpenToolsView::Tick()
	{
		switch (OpenTools.Open)
		{
		case OpenToolsWindows::OpenTools::FBXImport:
			DrawFBXImportWindow();
			break;
		case OpenToolsWindows::OpenTools::NewFolder:
			DrawNewFolderWindow();
			break;
		default:
			break;
		}
	}

	void OpenToolsView::DrawFBXImportWindow()
	{
        if (!ImGui::BeginPopupModal("FBX Importer", NULL, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize)) {
            // set windows center
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            // set windows size 
            ImGui::SetNextWindowSize(ImVec2(480, 160), ImGuiCond_Appearing);

            ImGui::OpenPopup("FBX Importer");
            ImGui::BeginPopupModal("FBX Importer", NULL, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);
        }

        ImGui::Spacing();

        static char filePath[256] = "";
        ImGui::Text("Select FBX File:");
        ImGui::SetNextItemWidth(-70);  // path - Browse button
        ImGui::InputText("##FilePath", filePath, sizeof(filePath));
        ImGui::SameLine();

        if (ImGui::Button("Browse")) {
            // open file choose windows
            OPENFILENAMEA ofn;
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFilter = "FBX Files\0*.fbx\0All Files\0*.*\0";
            ofn.lpstrFile = filePath;
            ofn.nMaxFile = sizeof(filePath);
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn)) {
                // user choose file 
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        {
            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 100.0f;
            float spacing = 10.0f;
            ImGui::SetCursorPosX(windowWidth - (buttonWidth * 2 + spacing * 2));

            // Import Button
            if (ImGui::Button("Import", ImVec2(buttonWidth, 0))) {
                if (strlen(filePath) > 0) {
                    // Import FBX Function
                    LOG_INFO("Import FBX: " , filePath);
                    FBXImport(filePath);
                    CloseToolsWindow();
                }
                else {
                    LOG_WARNING("Please select the FBX file to import first!");
                }
            }

            ImGui::SameLine(0, spacing);

            // Cancel button
            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                CloseToolsWindow();
            }
        }

        ImGui::EndPopup();
	}

	void OpenToolsView::DrawNewFolderWindow()
	{
        if (!ImGui::BeginPopupModal("New Folder", NULL, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize)) {
            // set windows center
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            // set windows size 
            ImGui::SetNextWindowSize(ImVec2(480, 160), ImGuiCond_Appearing);

            ImGui::OpenPopup("New Folder");
            ImGui::BeginPopupModal("New Folder", NULL, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);
        }

        ImGui::Spacing();

        static char FolderName[256] = "";
        ImGui::Text("New Folder Name:");
        ImGui::InputText("##FolderName", FolderName, sizeof(FolderName));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        {
            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 100.0f;
            float spacing = 10.0f;
            ImGui::SetCursorPosX(windowWidth - (buttonWidth * 2 + spacing * 2));

            // Create Button
            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                if (strlen(FolderName) > 0) {
                    // Create 
                    LOG_INFO("Create Folder: " + std::string(FolderName));
                    CreateFolder(FolderName);
                    CloseToolsWindow();
                }
                else {
                    LOG_WARNING("Please select the FBX file to import first!");
                }
            }

            ImGui::SameLine(0, spacing);

            // Cancel button
            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                CloseToolsWindow();
            }
        }

        ImGui::EndPopup();
	}

	void OpenToolsView::CloseToolsWindow()
	{
		OpenTools.Open = OpenToolsWindows::OpenTools::NOT;
		ImGui::CloseCurrentPopup(); // end popup
	}
}