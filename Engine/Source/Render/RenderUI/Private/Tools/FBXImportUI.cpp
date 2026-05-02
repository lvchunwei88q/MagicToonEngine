#include <GuiInterface.h>
#include <DrawToolsWindows.h>
#include <Log.h>

#include <FBXImport.h>

namespace RenderUI {

    void DrawFBXImportWindow() {  // 添加参数控制开关

        // 设置窗口居中
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // 弹出窗口标志：带关闭按钮、模态
        ImGui::SetNextWindowSize(ImVec2(480, 160), ImGuiCond_Appearing);

        // 改用 p_open 参数，这样叉号就会出现并可点击
        ImGui::OpenPopup("FBX Importer");

        ImGui::BeginPopupModal("FBX Importer", NULL, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);

        static char filePath[256] = "";  // 需要是static，保持数据持久

        // 添加一些间距
        ImGui::Spacing();

        // 文件路径输入框和浏览按钮
        ImGui::Text("Select FBX File:");
        ImGui::SetNextItemWidth(-70);  // 为浏览按钮留出空间
        ImGui::InputText("##FilePath", filePath, sizeof(filePath));
        ImGui::SameLine();

        if (ImGui::Button("Browse")) {
            // 打开文件对话框
            OPENFILENAMEA ofn;       // Windows 文件对话框结构体
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;  // 如果有窗口句柄可以传入
            ofn.lpstrFilter = "FBX Files\0*.fbx\0All Files\0*.*\0";
            ofn.lpstrFile = filePath;
            ofn.nMaxFile = sizeof(filePath);
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn)) {
                // 用户选择了文件，filePath 已经被更新
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 按钮布局
        float windowWidth = ImGui::GetWindowWidth();
        float buttonWidth = 100.0f;
        float spacing = 10.0f;

        // 从右侧开始放置按钮
        ImGui::SetCursorPosX(windowWidth - (buttonWidth * 2 + spacing * 2));

        // 导入按钮
        if (ImGui::Button("Import", ImVec2(buttonWidth, 0))) {
            if (strlen(filePath) > 0) {
                // TODO: 调用 FBX 导入函数
                LOG_INFO("Import FBX: " + std::string(filePath));
                
                EngineAsset::FBXImportSettings settings;
                settings.path = std::string(filePath);
                EngineAsset::FBXAssetImport::Get().Import(settings);

                // 导入成功后关闭窗口
                ImGui::CloseCurrentPopup();
                CloseToolsWindow();
            }
            else {
                LOG_WARNING("Please select the FBX file to import first!");
            }
        }

        ImGui::SameLine(0, spacing);

        // 取消按钮
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
            CloseToolsWindow();
        }

        ImGui::EndPopup();
    }
}