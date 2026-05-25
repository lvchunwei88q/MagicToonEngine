#include "LauncherGeneralLayout.h"
#include <RenderUIWarehouse.h>

namespace RenderLauncher {
    void LauncherGeneralLayout::DrawOpenProjectUI() {
        static char searchFilter[64] = "";

        ImGui::Spacing();
        ImGui::Text("Open Existing Project");
        ImGui::Separator();
        ImGui::Spacing();

        // Search
        ImGui::InputTextWithHint("##Search", "Search by project name...", searchFilter, sizeof(searchFilter));
        ImGui::Spacing();

        // 可滚动的项目列表区域
        ImGui::BeginChild("ProjectList", ImVec2(0, 0), true);
        {
            bool hasMatch = false;
            std::string filterLower = searchFilter;

            for (const auto& proj : projects) {
                // 应用搜索过滤（简单包含匹配，不区分大小写）
                bool match = (strlen(searchFilter) == 0);
                if (!match) {
                    // 转为小写匹配（此处略，可自行添加）
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
                    ImGui::Text("Path: %s", proj.path.c_str());
                    ImGui::Text("Version: %s", proj.version.c_str());
                    ImGui::SameLine(ImGui::GetWindowWidth() - 80.0f);
                    if (ImGui::Button("Open", ImVec2(60, 0))) {
                        // 这里执行打开项目的逻辑
                        // 例如：加载项目配置、切换到编辑器界面等
                        ImGui::OpenPopup("OpenProjectInfo");
                        // 实际开发时可用一个静态变量记录当前打开的项目名称，这里简单弹窗示意
                    }
                }
                ImGui::EndGroup();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PopID();
            }

            if (!hasMatch) {
                ImGui::TextDisabled("No projects match the search criteria.");
            }

            // 打开项目时的信息弹窗（仅示意）
            if (ImGui::BeginPopup("OpenProjectInfo")) {
                ImGui::Text("Opening project... (placeholder)");
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
    }
}