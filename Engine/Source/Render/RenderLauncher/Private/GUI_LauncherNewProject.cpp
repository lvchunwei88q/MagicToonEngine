#include "LauncherGeneralLayout.h"
#include <RenderUIWarehouse.h>
#include <windows.h>
#include <shlobj.h>
#include <IProjectController.h>
#include <Converter.h>

namespace RenderLauncher {
    namespace {
        std::string BrowseFolder(const std::string& title) {
            BROWSEINFOW bi = { 0 };
            bi.lpszTitle = std::wstring(title.begin(), title.end()).c_str();
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
            if (pidl != 0) {
                wchar_t path[MAX_PATH];
                if (SHGetPathFromIDListW(pidl, path)) {
                    std::wstring ws(path);
                    std::string result = IO::Converter::ToNarrowString(ws);
                    CoTaskMemFree(pidl);
                    return result;
                }
                CoTaskMemFree(pidl);
            }
            return "";
        }
    }

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
            std::string selectedPath = BrowseFolder("Select project folder");
            if (!selectedPath.empty() && selectedPath.length() < 256) {
                strcpy_s(projectPath, selectedPath.c_str());
            }
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
                EngineProject::ProJectConfig config;
                config.name = projectName;
                config.path = projectPath;
                EngineProject::GetProjectControllerInterface()->Create(config);

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