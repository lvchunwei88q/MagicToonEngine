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
        ImGui::Text(Lang::Get("launcher.menu.new").c_str());
        ImGui::Separator();
        ImGui::Spacing();

        // 项目名称输入
        ImGui::InputText(Lang::Get("launcher.new.project_name").c_str(), projectName, sizeof(projectName));

        ImGui::InputText(Lang::Get("launcher.new.project_path").c_str(), projectPath, sizeof(projectPath));
        ImGui::SameLine();
        if (ImGui::Button(Lang::Get("launcher.new.browse").c_str())) {
            // TODO: 集成文件对话框，获取用户选择的路径并填充到 projectPath 中
        }

        ImGui::Combo(Lang::Get("launcher.new.graphics_api").c_str(), &selectedAPI, apiOptions, IM_ARRAYSIZE(apiOptions));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 创建按钮
        if (ImGui::Button(Lang::Get("launcher.new.create").c_str(), ImVec2(120, 0))) {
            if (strlen(projectName) == 0 || strlen(projectPath) == 0) {
                ImGui::OpenPopup(Lang::Get("launcher.new.error_popup").c_str());
            }
            else {
                // TODO : 实际创建项目的逻辑（文件夹结构、配置文件等）
                ImGui::OpenPopup(Lang::Get("launcher.new.success_popup").c_str());

            }
        }

        // 错误提示弹窗
        if (ImGui::BeginPopupModal(Lang::Get("launcher.new.error_popup").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(Lang::Get("launcher.new.error_message").c_str());
            if (ImGui::Button(Lang::Get("launcher.common.ok").c_str(), ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // 成功提示弹窗
        if (ImGui::BeginPopupModal(Lang::Get("launcher.new.success_popup").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            char successMsg[512];
            snprintf(successMsg, sizeof(successMsg),
                Lang::Get("launcher.new.success_message").c_str(),
                projectName, projectPath, apiOptions[selectedAPI]);
            ImGui::Text("%s", successMsg);

            if (ImGui::Button(Lang::Get("launcher.common.ok").c_str(), ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                // 清除
                memset(projectName, 0, sizeof(projectName));
                memset(projectPath, 0, sizeof(projectPath));
            }
            ImGui::EndPopup();
        }
    }
}