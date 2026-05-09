#include <GuiInterface.h>
#include <DrawToolsWindows.h>
#include <ILog.h>

#include <FBXImport.h>

namespace RenderUI {

    void DrawFBXImportWindow() {  // 添加参数控制开关

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
                    LOG_INFO("Import FBX: " + std::string(filePath));

                    RuntimeAssets::FBXImportSettings settings;
                    settings.path = std::string(filePath);
                    RuntimeAssets::FBXAssetImport::Get().Import(settings);

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