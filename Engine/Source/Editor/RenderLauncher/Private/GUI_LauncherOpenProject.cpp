#include "LauncherGeneralLayout.h"
#include <RenderUIWarehouse.h>

namespace RenderLauncher {
    void LauncherGeneralLayout::DrawOpenProjectUI() {
        static char searchFilter[64] = "";

        ImGui::Spacing();
        ImGui::Text(Lang::Get("launcher.menu.open").c_str());
        ImGui::Separator();
        ImGui::Spacing();

        // Search
        ImGui::InputTextWithHint("##Search", Lang::Get("launcher.open.search_hint").c_str(), searchFilter, sizeof(searchFilter));
        ImGui::Spacing();

        // 可滚动的项目列表区域
        ImGui::BeginChild("ProjectList", ImVec2(0, 0), true);
        {
            bool hasMatch = false;
            std::string filterLower = searchFilter;

            for (const auto& proj : projects) {
                bool match = (strlen(searchFilter) == 0);
                if (!match) {
                    if (proj.name.find(searchFilter) != std::string::npos) {
                        match = true;
                    }
                }
                if (!match) continue;

                hasMatch = true;

                // 绘制每个项目卡片
                ImGui::PushID(proj.name.c_str());  // 确保每个项目有独立 ID

                // 使用 Group 模拟卡片效果
                ImGui::BeginGroup();
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "%s", proj.name.c_str());
                    ImGui::Text("%s: %s", Lang::Get("launcher.open.path_label").c_str(), proj.path.c_str());
                    ImGui::Text("%s: %s", Lang::Get("launcher.open.version_label").c_str(), proj.version.c_str());
                    ImGui::SameLine(ImGui::GetWindowWidth() - 80.0f);
                    if (ImGui::Button(Lang::Get("launcher.open.open_button").c_str(), ImVec2(60, 0))) {
                        ImGui::OpenPopup("OpenProjectInfo");
                    }
                }
                ImGui::EndGroup();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::BeginPopupModal("OpenProjectInfo", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text(Lang::Get("launcher.open.opening_message").c_str());
                    ImGui::Separator();

                    ImGui::Indent(20.0f);
                    ImGui::Text(("Name: " + proj.name).c_str());
                    ImGui::Text(("Path: " + proj.path).c_str());
                    ImGui::Text(("Version: " + proj.version).c_str());
                    ImGui::Unindent(20.0f);
                    ImGui::Separator();
                    ImGui::Indent(10.0f);

                    if (ImGui::Button("Open", ImVec2(80, 0))) {
                        OpenProject(proj.path, proj.name);
                        ImGui::CloseCurrentPopup();
                    }
					ImGui::SameLine();
                    if (ImGui::Button("Close", ImVec2(80, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::Unindent(10.0f);

                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            if (!hasMatch) {
                ImGui::TextDisabled(Lang::Get("launcher.open.no_match").c_str());
            }
        }
        ImGui::EndChild();
    }
}