#include "LauncherGeneralLayout.h"
#include <RenderUIWarehouse.h>

namespace RenderLauncher {
    void LauncherGeneralLayout::Tick()
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("Launcher", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // ===================== 标题栏 =====================
        const float title_bar_height = 35.0f;
        {
            
            ImVec2 title_bar_size = ImVec2(ImGui::GetWindowWidth(), title_bar_height);

            // 绘制标题栏背景
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetWindowPos(),
                ImVec2(ImGui::GetWindowPos().x + title_bar_size.x, ImGui::GetWindowPos().y + title_bar_size.y),
                IM_COL32(30, 30, 30, 255)
            );

            // 创建标题栏区域的 invisible button 用于拖动
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("title_bar_drag", title_bar_size);

            bool is_dragging_title = ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

            // 拖动窗口逻辑
            static ImVec2 drag_offset;
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 window_pos = ImGui::GetWindowPos();
                drag_offset = ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
            }

            if (is_dragging_title)
            {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 new_pos = ImVec2(mouse_pos.x - drag_offset.x, mouse_pos.y - drag_offset.y);

                HWND hwnd = (HWND)ImGui::GetMainViewport()->PlatformHandleRaw;
                SetWindowPos(hwnd, NULL, (int)new_pos.x, (int)new_pos.y, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }

            // 标题栏文字和关闭按钮
            ImGui::SetCursorPos(ImVec2(10, 8));
            ImGui::TextColored(ImVec4(1, 1, 1, 1), "Magic Launcher");

            // 分隔线
            ImGui::SetCursorPosY(title_bar_height);
            ImGui::Separator();
        }

        // ===================== 左右分栏布局 =====================
        const float left_panel_width = 200.0f;
        const ImVec2 button_size = ImVec2(180.0f, 50.0f);

        // 计算内容区域高度
        float content_height = ImGui::GetWindowHeight() - title_bar_height - ImGui::GetStyle().ItemSpacing.y - 2.0f;

        ImGui::BeginChild("LeftPanel",
            ImVec2(left_panel_width, content_height),
            true,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        {
            ImGui::SetCursorPosY(20.0f);
            ImGui::SetCursorPosX((left_panel_width - button_size.x) * 0.5f);
            if (ImGui::Button(Lang::Get("editor.menu.new").c_str(), button_size))
            {
                // 新建逻辑
            }

            ImGui::Spacing();
            ImGui::SetCursorPosX((left_panel_width - button_size.x) * 0.5f);
            if (ImGui::Button(Lang::Get("editor.menu.open").c_str(), button_size))
            {
                // 打开逻辑
            }


            float panel_height = ImGui::GetWindowHeight();
            float cursor_y = panel_height - button_size.y - 10.0f; // 10px 底部边距
            if (cursor_y > ImGui::GetCursorPosY())
            {
                ImGui::SetCursorPosY(cursor_y);
            }

            ImGui::SetCursorPosX((left_panel_width - button_size.x) * 0.5f);
            if (ImGui::Button(Lang::Get("editor.menu.exit").c_str(), button_size))
            {
                ExitProgram();
            }
        }

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel",
            ImVec2(-1.0f, content_height),
            true,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        {
            // ===================== 右侧内容区 =====================
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Content Area");
            ImGui::Separator();

            ImGui::Text("Welcome to Launcher");
        }

        ImGui::EndChild();

        ImGui::End();
    }
}