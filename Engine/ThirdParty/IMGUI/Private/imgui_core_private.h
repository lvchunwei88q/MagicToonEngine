#pragma once

// Implementation details shared by the core translation units.
namespace ImGui
{
    void ItemHandleShortcut(ImGuiID id);
    void NavProcessItem();
    ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window);
    ImGuiInputSource NavCalcPreferredRefPosSource(ImGuiWindowFlags window_type);
    ImVec2 NavCalcPreferredRefPos(ImGuiWindowFlags window_type);
    int FindWindowFocusIndex(ImGuiWindow* window);
    void NavUpdate();
    void NavUpdateAnyRequestFlag();
    void NavEndFrame();
    void NavApplyItemToResult(ImGuiNavItemData* result);
    ImGuiWindow* NavRestoreLastChildNavWindow(ImGuiWindow* window);
}

inline bool IsWindowActiveAndVisible(ImGuiWindow* window)
{
    return window->Active && !window->Hidden;
}

#define IMGUI_DEBUG_NAV_SCORING     0   // Display navigation scoring preview when hovering items. Hold Ctrl to display for all candidates. Ctrl+Arrow to change last direction.
#define IMGUI_DEBUG_NAV_RECTS       0   // Display the reference navigation rectangle for each window

inline ImGuiDir ImGetDirQuadrantFromDelta(float dx, float dy)
{
    if (ImFabs(dx) > ImFabs(dy))
        return (dx > 0.0f) ? ImGuiDir_Right : ImGuiDir_Left;
    return (dy > 0.0f) ? ImGuiDir_Down : ImGuiDir_Up;
}
