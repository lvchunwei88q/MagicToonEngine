#include "Editor/EditorGeneralLayout.h"
#include <RenderUIWarehouse.h>
#include "Tools/OpenToolsView.h"

namespace RenderEditor {
	void EditorGeneralLayout::Tick()
	{
        ImGuiIO& io = ImGui::GetIO();

        // 获取主视口的工作区域
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // 设置下一个窗口填满整个视口
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        // 主窗口：无标题栏、无边框、不可移动、不可调整大小
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_MenuBar |           // 有菜单栏
            ImGuiWindowFlags_NoTitleBar |        // 无标题栏
            ImGuiWindowFlags_NoCollapse |        // 不可折叠
            ImGuiWindowFlags_NoResize |          // 不可调整大小
            ImGuiWindowFlags_NoMove |            // 不可移动
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("MainDockSpace", nullptr, window_flags);

        ImGui::PopStyleVar(3);

        // --- 菜单栏 ---
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(Lang::Get("editor.menu.file").c_str()))
            {
                if (ImGui::MenuItem(Lang::Get("editor.menu.new").c_str())) { /* TODO */ }
                if (ImGui::MenuItem(Lang::Get("editor.menu.open").c_str())) { /* TODO */ }
                if (ImGui::MenuItem(Lang::Get("editor.menu.save").c_str())) { /* TODO */ }
                if (ImGui::MenuItem(Lang::Get("editor.menu.import_fbx").c_str())) { OpenToolsView::Get().OpenToolsWindow(OpenToolsWindows::OpenTools::FBXImport); }
                ImGui::Separator();
                if (ImGui::MenuItem(Lang::Get("editor.menu.exit").c_str())) { ExitProgram(); }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(Lang::Get("editor.menu.edit").c_str()))
            {
                ImGui::MenuItem(Lang::Get("editor.menu.editor_settings").c_str(), nullptr, &Switch.EditorSettingsView);
                ImGui::Separator();
                if (ImGui::MenuItem(Lang::Get("editor.menu.undo").c_str())) { Core::GetSubsystemContext()->Notification("COMMAND", { encodeToSizeT("Undo"), nullptr }); }
                if (ImGui::MenuItem(Lang::Get("editor.menu.redo").c_str())) { Core::GetSubsystemContext()->Notification("COMMAND", { encodeToSizeT("Redo"), nullptr }); }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(Lang::Get("editor.menu.view").c_str()))
            {
                ImGui::MenuItem(Lang::Get("editor.menu.lua_control").c_str(), nullptr, &Switch.LuaControl);
                ImGui::MenuItem(Lang::Get("editor.menu.details_panel").c_str(), nullptr, &Switch.DetailsWindow);
                ImGui::Separator();
                ImGui::MenuItem(Lang::Get("editor.menu.engine_log").c_str(), nullptr, &Switch.LoggerWindow);
                ImGui::MenuItem(Lang::Get("editor.menu.render_view").c_str(), nullptr, &Switch.RenderViewWindow);
                ImGui::MenuItem(Lang::Get("editor.menu.content_browser").c_str(), nullptr, &Switch.ContentBrowserWindow);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // --- 下方停靠区域 ---
        // 获取菜单栏下方剩余空间
        ImVec2 dockSpacePos = ImGui::GetCursorPos();
        ImVec2 dockSpaceSize = ImGui::GetContentRegionAvail();

        // 创建停靠空间
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();
	}
}