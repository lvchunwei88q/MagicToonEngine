#pragma once

// Implementation details shared by the core translation units.
namespace ImGui
{
    void ItemHandleShortcut(ImGuiID id);
    void NavProcessItem();
    ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window);
    ImGuiInputSource NavCalcPreferredRefPosSource(ImGuiWindowFlags window_type);
    ImVec2 NavCalcPreferredRefPos(ImGuiWindowFlags window_type);
}

inline bool IsWindowActiveAndVisible(ImGuiWindow* window)
{
    return window->Active && !window->Hidden;
}
