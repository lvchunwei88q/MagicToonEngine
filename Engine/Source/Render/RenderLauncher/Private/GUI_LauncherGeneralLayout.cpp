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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

        ImGui::Begin("Launcher", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        float button_width = 250.0f;
        float button_height = 50.0f;


        float window_width = ImGui::GetWindowWidth();
        float offset_x = (window_width - button_width) * 0.5f;

        ImGui::SetCursorPosX(offset_x);
        if (ImGui::Button(Lang::Get("editor.menu.new").c_str(), ImVec2(button_width, button_height)))
        {
            
        }

        ImGui::SetCursorPosX(offset_x);
        if (ImGui::Button(Lang::Get("editor.menu.open").c_str(), ImVec2(button_width, button_height)))
        {
           
        }

        ImGui::SetCursorPosX(offset_x);
        if (ImGui::Button(Lang::Get("editor.menu.exit").c_str(), ImVec2(button_width, button_height)))
        {

        }

        ImGui::End();
    }
}