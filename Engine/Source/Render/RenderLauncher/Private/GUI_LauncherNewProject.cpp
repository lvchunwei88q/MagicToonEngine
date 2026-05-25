#include "LauncherGeneralLayout.h"
#include <RenderUIWarehouse.h>

namespace RenderLauncher {
    void LauncherGeneralLayout::DrawNewProjectUI() {
        // 持久化输入状态
        static char projectName[128] = "";
        static char projectPath[256] = "";
        static int selectedAPI = 0;          // 0: DX12, 1: DX11
        static const char* apiOptions[] = { "DX12", "DX11" };

        ImGui::Spacing();
        ImGui::Text("Create New Project");
        ImGui::Separator();
        ImGui::Spacing();

        // 项目名称输入
        ImGui::InputText("Project Name", projectName, sizeof(projectName));

        ImGui::InputText("Project Path", projectPath, sizeof(projectPath));
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            // TODO: 集成文件对话框，获取用户选择的路径并填充到 projectPath 中
        }

        ImGui::Combo("Graphics API", &selectedAPI, apiOptions, IM_ARRAYSIZE(apiOptions));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 创建按钮
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            if (strlen(projectName) == 0 || strlen(projectPath) == 0) {
                ImGui::OpenPopup("CreateError");
            }
            else {
				// TODO : 实际创建项目的逻辑（文件夹结构、配置文件等）
                ImGui::OpenPopup("CreateSuccess");

            }
        }

        // 错误提示弹窗
        if (ImGui::BeginPopupModal("CreateError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Project name and path cannot be empty!");
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // 成功提示弹窗
        if (ImGui::BeginPopupModal("CreateSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Project \"%s\" created at %s (API: %s)",
                projectName, projectPath, apiOptions[selectedAPI]);
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                // 可选：清空输入框，方便新建下一个项目
                memset(projectName, 0, sizeof(projectName));
                memset(projectPath, 0, sizeof(projectPath));
            }
            ImGui::EndPopup();
        }
    }
}