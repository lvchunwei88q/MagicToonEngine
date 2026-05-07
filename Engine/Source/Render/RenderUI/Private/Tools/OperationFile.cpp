#include "DrawToolsWindows.h"
#include <GuiInterface.h>
#include <DrawToolsWindows.h>

#include <ILog.h>
#include <Component/FileBrowserUI.h>

#include <Converter.h>
#include <FileManager.h>

namespace RenderUI {
    auto Bind_BeginPopupModal = []() {
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
        };

	void RenderUI::DrawNewFolderWindow()
	{
        Bind_BeginPopupModal();

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

                    std::wstring currentPath = FileBrowserUI::Get().GetAbsolutePath(FileBrowserUI::Get().GetState().currentPath);
                    std::wstring Folder = IO::Converter::ToWideString(std::string(FolderName));
                    Folder = currentPath + L"\\" + Folder;
                    IO::FileManager::MakeDirectory(Folder);

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
}