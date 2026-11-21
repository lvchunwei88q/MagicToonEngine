#pragma once

// Implementation details shared by the core translation units.
namespace ImGui
{
    void ItemHandleShortcut(ImGuiID id);
    void NavProcessItem();
    ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window);
}
