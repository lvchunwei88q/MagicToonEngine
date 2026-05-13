#include <Component/Editor/EditorGeneralLayout.h>
#include <EditorUIWarehouse.h>
#include <Component/Tools/OpenToolsView.h>

namespace RenderUI {
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
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) { /* TODO */ }
                if (ImGui::MenuItem("Open")) { /* TODO */ }
                if (ImGui::MenuItem("Save")) { /* TODO */ }
                if (ImGui::MenuItem("Import FBX")) { OpenToolsView::Get().OpenToolsWindow(OpenToolsWindows::OpenTools::FBXImport); }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { ExitProgram(); }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Editor Settings", nullptr, &Switch.EditorSettingsView);
                ImGui::Separator();
                if (ImGui::MenuItem("Undo")) { /* TODO */ }
                if (ImGui::MenuItem("Redo")) { /* TODO */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Lua Control", nullptr, &Switch.LuaControl);
                ImGui::MenuItem("Details Panel", nullptr, &Switch.DetailsWindow);
                ImGui::Separator();
                ImGui::MenuItem("Engine Log", nullptr, &Switch.LoggerWindow);
                ImGui::MenuItem("Render View", nullptr, &Switch.RenderViewWindow);
                ImGui::MenuItem("Content Browser View", nullptr, &Switch.ContentBrowserWindow);
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