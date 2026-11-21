// Dear ImGui implementation - see LICENSE.txt.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"

namespace ImGui
{
static void             UpdateWindowInFocusOrderList(ImGuiWindow* window, bool just_created, ImGuiWindowFlags new_flags);
static int              UpdateWindowManualResize(ImGuiWindow* window, int* border_hovered, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect);
static void             RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size);
static void             RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open);
static void             SetLastItemDataForWindow(ImGuiWindow* window, const ImRect& rect);
static void             SetLastItemDataForChildWindowItem(ImGuiWindow* window, const ImRect& rect);
}

// Docking
static const float DOCKING_TRANSPARENT_PAYLOAD_ALPHA        = 0.50f;    // For use with io.ConfigDockingTransparentPayload. Apply to Viewport _or_ WindowBg in host viewport.

// ImGuiWindow is mostly a dumb struct. It merely has a constructor and a few helper methods
ImGuiWindow::ImGuiWindow(ImGuiContext* ctx, const char* name) : DrawListInst(NULL)
{
    memset((void*)this, 0, sizeof(*this));
    Ctx = ctx;
    Name = ImStrdup(name);
    NameBufLen = (int)ImStrlen(name) + 1;
    ID = ImHashStr(name);
    IDStack.push_back(ID);
    ViewportAllowPlatformMonitorExtend = -1;
    ViewportPos = ImVec2(FLT_MAX, FLT_MAX);
    MoveId = GetID("#MOVE");
    TabId = GetID("#TAB");
    ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
    ScrollTargetCenterRatio = ImVec2(0.5f, 0.5f);
    AutoPosLastDirection = ImGuiDir_None;
    AutoFitFramesX = AutoFitFramesY = -1;
    SetWindowPosAllowFlags = SetWindowSizeAllowFlags = SetWindowCollapsedAllowFlags = SetWindowDockAllowFlags = 0;
    SetWindowPosVal = SetWindowPosPivot = ImVec2(FLT_MAX, FLT_MAX);
    LastFrameActive = -1;
    LastFrameJustFocused = -1;
    LastTimeActive = -1.0f;
    FontRefSize = 0.0f;
    FontWindowScale = FontWindowScaleParents = 1.0f;
    SettingsOffset = -1;
    DockOrder = -1;
    DrawList = &DrawListInst;
    DrawList->_OwnerName = Name;
    DrawList->_SetDrawListSharedData(&Ctx->DrawListSharedData);
    NavPreferredScoringPosRel[0] = NavPreferredScoringPosRel[1] = ImVec2(FLT_MAX, FLT_MAX);
    IM_PLACEMENT_NEW(&WindowClass) ImGuiWindowClass();
}

ImGuiWindow::~ImGuiWindow()
{
    IM_ASSERT(DrawList == &DrawListInst);
    IM_DELETE(Name);
    ColumnsStorage.clear_destruct();
}

static void SetCurrentWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow = window;
    g.StackSizesInBeginForCurrentWindow = g.CurrentWindow ? &g.CurrentWindowStack.back().StackSizesInBegin : NULL;
    g.CurrentTable = window && window->DC.CurrentTableIdx != -1 ? g.Tables.GetByIndex(window->DC.CurrentTableIdx) : NULL;
    if (window)
    {
        if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        {
            ImGuiViewport* viewport = window->Viewport;
            g.FontRasterizerDensity = (viewport->FramebufferScale.x != 0.0f) ? viewport->FramebufferScale.x : g.IO.DisplayFramebufferScale.x; // == SetFontRasterizerDensity()
        }
        const bool backup_skip_items = window->SkipItems;
        window->SkipItems = false;
        ImGui::UpdateCurrentFontSize(0.0f);
        window->SkipItems = backup_skip_items;
        ImGui::NavUpdateCurrentWindowIsScrollPushableX();
    }
}

static void ImGui::SetLastItemDataForWindow(ImGuiWindow* window, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    if (window->DockIsActive)
        SetLastItemData(window->MoveId, g.CurrentItemFlags, window->DC.DockTabItemStatusFlags, window->DC.DockTabItemRect);
    else
        SetLastItemData(window->MoveId, g.CurrentItemFlags, window->DC.WindowItemStatusFlags, rect);
}

static void ImGui::SetLastItemDataForChildWindowItem(ImGuiWindow* window, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    SetLastItemData(window->ChildId, g.CurrentItemFlags, window->DC.ChildItemStatusFlags, rect);
}

void ImGui::StartMouseMovingWindow(ImGuiWindow* window)
{
    // Set ActiveId even if the _NoMove flag is set. Without it, dragging away from a window with _NoMove would activate hover on other windows.
    // We _also_ call this when clicking in a window empty space when io.ConfigWindowsMoveFromTitleBarOnly is set, but clear g.MovingWindow afterward.
    // This is because we want ActiveId to be set even when the window is not permitted to move.
    ImGuiContext& g = *GImGui;
    FocusWindow(window);
    SetActiveID(window->MoveId, window);
    if (g.IO.ConfigNavCursorVisibleAuto)
        g.NavCursorVisible = false;
    g.ActiveIdClickOffset = g.IO.MouseClickedPos[0] - window->RootWindowDockTree->Pos;
    g.ActiveIdNoClearOnFocusLoss = true;
    SetActiveIdUsingAllKeyboardKeys();

    bool can_move_window = true;
    if ((window->Flags & ImGuiWindowFlags_NoMove) || (window->RootWindowDockTree->Flags & ImGuiWindowFlags_NoMove))
        can_move_window = false;
    if (ImGuiDockNode* node = window->DockNodeAsHost)
        if (node->VisibleWindow && (node->VisibleWindow->Flags & ImGuiWindowFlags_NoMove))
            can_move_window = false;
    if (can_move_window)
        g.MovingWindow = window;
}

// We use 'undock == false' when dragging from title bar to allow moving groups of floating nodes without undocking them.
void ImGui::StartMouseMovingWindowOrNode(ImGuiWindow* window, ImGuiDockNode* node, bool undock)
{
    ImGuiContext& g = *GImGui;
    bool can_undock_node = false;
    if (undock && node != NULL && node->VisibleWindow && (node->VisibleWindow->Flags & ImGuiWindowFlags_NoMove) == 0 && (node->MergedFlags & ImGuiDockNodeFlags_NoUndocking) == 0)
    {
        // Can undock if:
        // - part of a hierarchy with more than one visible node (if only one is visible, we'll just move the root window)
        // - part of a dockspace node hierarchy: so we can undock the last single visible node too. Undocking from a fixed/central node will create a new node and copy windows.
        ImGuiDockNode* root_node = DockNodeGetRootNode(node);
        if (root_node->OnlyNodeWithWindows != node || root_node->CentralNode != NULL)   // -V1051 PVS-Studio thinks node should be root_node and is wrong about that.
            can_undock_node = true;
    }

    const bool clicked = IsMouseClicked(0);
    const bool dragging = IsMouseDragging(0);
    if (can_undock_node && dragging)
        DockContextQueueUndockNode(&g, node); // Will lead to DockNodeStartMouseMovingWindow() -> StartMouseMovingWindow() being called next frame
    else if (!can_undock_node && (clicked || dragging) && g.MovingWindow != window)
        StartMouseMovingWindow(window);
}

// This is not 100% symmetric with StartMouseMovingWindow().
// We do NOT clear ActiveID, because:
// - It would lead to rather confusing recursive code paths. Caller can call ClearActiveID() if desired.
// - Some code intentionally cancel moving but keep the ActiveID to lock inputs (e.g. code path taken when clicking a disabled item).
void ImGui::StopMouseMovingWindow()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.MovingWindow;

    // Ref commits 6b7766817, 36055213c for some partial history on checking if viewport != NULL.
    if (window && window->Viewport)
    {
        // Try to merge the window back into the main viewport.
        // This works because MouseViewport should be != MovingWindow->Viewport on release (as per code in UpdateViewports)
        if (g.ConfigFlagsCurrFrame & ImGuiConfigFlags_ViewportsEnable)
            UpdateTryMergeWindowIntoHostViewport(window->RootWindowDockTree, g.MouseViewport);

        // Restore the mouse viewport so that we don't hover the viewport _under_ the moved window during the frame we released the mouse button.
        if (!IsDragDropPayloadBeingAccepted())
            g.MouseViewport = window->Viewport;

        // Clear the NoInputs window flag set by the Viewport system in AddUpdateViewport()
        const bool window_can_use_inputs = ((window->Flags & ImGuiWindowFlags_NoMouseInputs) && (window->Flags & ImGuiWindowFlags_NoNavInputs)) == false;
        if (window_can_use_inputs)
            window->Viewport->Flags &= ~ImGuiViewportFlags_NoInputs;
    }
    g.MovingWindow = NULL;
}

// Handle mouse moving window
// Note: moving window with the navigation keys (Square + d-pad / Ctrl+Tab + Arrows) are processed in NavUpdateWindowing()
// FIXME: We don't have strong guarantee that g.MovingWindow stay synced with g.ActiveId == g.MovingWindow->MoveId.
// This is currently enforced by the fact that BeginDragDropSource() is setting all g.ActiveIdUsingXXXX flags to inhibit navigation inputs,
// but if we should more thoroughly test cases where g.ActiveId or g.MovingWindow gets changed and not the other.
void ImGui::UpdateMouseMovingWindowNewFrame()
{
    ImGuiContext& g = *GImGui;
    if (g.MovingWindow != NULL)
    {
        // We actually want to move the root window. g.MovingWindow == window we clicked on (could be a child window).
        // We track it to preserve Focus and so that generally ActiveIdWindow == MovingWindow and ActiveId == MovingWindow->MoveId for consistency.
        KeepAliveID(g.ActiveId);
        IM_ASSERT(g.MovingWindow && g.MovingWindow->RootWindowDockTree);
        ImGuiWindow* moving_window = g.MovingWindow->RootWindowDockTree;

        // When a window stop being submitted while being dragged, it may will its viewport until next Begin()
        const bool window_disappeared = (!moving_window->WasActive && !moving_window->Active);
        if (g.IO.MouseDown[0] && IsMousePosValid(&g.IO.MousePos) && !window_disappeared)
        {
            ImVec2 pos = g.IO.MousePos - g.ActiveIdClickOffset;
            if (moving_window->Pos.x != pos.x || moving_window->Pos.y != pos.y)
            {
                SetWindowPos(moving_window, pos, ImGuiCond_Always);
                if (moving_window->Viewport && moving_window->ViewportOwned) // Synchronize viewport immediately because some overlays may relies on clipping rectangle before we Begin() into the window.
                {
                    moving_window->Viewport->Pos = pos;
                    moving_window->Viewport->UpdateWorkRect();
                }
            }
            FocusWindow(g.MovingWindow);
        }
        else
        {
            StopMouseMovingWindow();
            ClearActiveID();
        }
    }
    else
    {
        // When clicking/dragging from a window that has the _NoMove flag, we still set the ActiveId in order to prevent hovering others.
        if (g.ActiveIdWindow && g.ActiveIdWindow->MoveId == g.ActiveId)
        {
            KeepAliveID(g.ActiveId);
            if (!g.IO.MouseDown[0])
                ClearActiveID();
        }
    }
}

// Initiate focusing and moving window when clicking on empty space or title bar.
// Initiate focusing window when clicking on a disabled item.
// Handle left-click and right-click focus.
void ImGui::UpdateMouseMovingWindowEndFrame()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId != 0 || (g.HoveredId != 0 && !g.HoveredIdIsDisabled))
        return;

    // Unless we just made a window/popup appear
    if (g.NavWindow && g.NavWindow->Appearing)
        return;

    ImGuiWindow* hovered_window = g.HoveredWindow;

    // Click on empty space to focus window and start moving
    // (after we're done with all our widgets, so e.g. clicking on docking tab-bar which have set HoveredId already and not get us here!)
    if (g.IO.MouseClicked[0])
    {
        // Handle the edge case of a popup being closed while clicking in its empty space.
        // If we try to focus it, FocusWindow() > ClosePopupsOverWindow() will accidentally close any parent popups because they are not linked together any more.
        ImGuiWindow* hovered_root = hovered_window ? hovered_window->RootWindow : NULL;
        const bool is_closed_popup = hovered_root && (hovered_root->Flags & ImGuiWindowFlags_Popup) && !IsPopupOpen(hovered_root->PopupId, ImGuiPopupFlags_AnyPopupLevel);

        if (hovered_window != NULL && !is_closed_popup)
        {
            StartMouseMovingWindow(hovered_window); //-V595

            // FIXME: In principle we might be able to call StopMouseMovingWindow() below.
            // Please note how StartMouseMovingWindow() and StopMouseMovingWindow() and not entirely symmetrical, at the later doesn't clear ActiveId.

            // Cancel moving if clicked outside of title bar
            if ((hovered_window->BgClickFlags & ImGuiWindowBgClickFlags_Move) == 0) // set by io.ConfigWindowsMoveFromTitleBarOnly
                if (!(hovered_root->Flags & ImGuiWindowFlags_NoTitleBar) || hovered_root->DockIsActive)
                    if (!hovered_root->TitleBarRect().Contains(g.IO.MouseClickedPos[0]))
                        g.MovingWindow = NULL;

            // Cancel moving if clicked over an item which was disabled or inhibited by popups
            // (when g.HoveredIdIsDisabled == true && g.HoveredId == 0 we are inhibited by popups, when g.HoveredIdIsDisabled == true && g.HoveredId != 0 we are over a disabled item)
            if (g.HoveredIdIsDisabled)
            {
                g.MovingWindow = NULL;
                g.ActiveIdDisabledId = g.HoveredId;
            }
        }
        else if (hovered_window == NULL && g.NavWindow != NULL)
        {
            // Clicking on void disable focus
            FocusWindow(NULL, ImGuiFocusRequestFlags_UnlessBelowModal);
        }
    }

    // With right mouse button we close popups without changing focus based on where the mouse is aimed
    // Instead, focus will be restored to the window under the bottom-most closed popup.
    // (The left mouse button path calls FocusWindow on the hovered window, which will lead NewFrame->ClosePopupsOverWindow to trigger)
    if (g.IO.MouseClicked[1] && g.HoveredId == 0)
    {
        // Find the top-most window between HoveredWindow and the top-most Modal Window.
        // This is where we can trim the popup stack.
        ImGuiWindow* modal = GetTopMostPopupModal();
        bool hovered_window_above_modal = hovered_window && (modal == NULL || IsWindowAbove(hovered_window, modal));
        ClosePopupsOverWindow(hovered_window_above_modal ? hovered_window : modal, true);
    }
}

// Prior to v1.90 2023/10/16, the BeginChild() function took a 'bool border = false' parameter instead of 'ImGuiChildFlags child_flags = 0'.
// ImGuiChildFlags_Borders is defined as always == 1 in order to allow old code passing 'true'. Read comments in imgui.h for details!
bool ImGui::BeginChild(const char* str_id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGuiID id = GetCurrentWindow()->GetID(str_id);
    return BeginChildEx(str_id, id, size_arg, child_flags, window_flags);
}

bool ImGui::BeginChild(ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    return BeginChildEx(NULL, id, size_arg, child_flags, window_flags);
}

bool ImGui::BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    IM_ASSERT(id != 0);

    // Sanity check as it is likely that some user will accidentally pass ImGuiWindowFlags into the ImGuiChildFlags argument.
    const ImGuiChildFlags ImGuiChildFlags_SupportedMask_ = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_FrameStyle | ImGuiChildFlags_NavFlattened;
    IM_UNUSED(ImGuiChildFlags_SupportedMask_);
    IM_ASSERT((child_flags & ~ImGuiChildFlags_SupportedMask_) == 0 && "Illegal ImGuiChildFlags value. Did you pass ImGuiWindowFlags values instead of ImGuiChildFlags?");
    IM_ASSERT((window_flags & ImGuiWindowFlags_AlwaysAutoResize) == 0 && "Cannot specify ImGuiWindowFlags_AlwaysAutoResize for BeginChild(). Use ImGuiChildFlags_AlwaysAutoResize!");
    if (child_flags & ImGuiChildFlags_AlwaysAutoResize)
    {
        IM_ASSERT((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0 && "Cannot use ImGuiChildFlags_ResizeX or ImGuiChildFlags_ResizeY with ImGuiChildFlags_AlwaysAutoResize!");
        IM_ASSERT((child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) != 0 && "Must use ImGuiChildFlags_AutoResizeX or ImGuiChildFlags_AutoResizeY with ImGuiChildFlags_AlwaysAutoResize!");
    }
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    //if (window_flags & ImGuiWindowFlags_AlwaysUseWindowPadding) { child_flags |= ImGuiChildFlags_AlwaysUseWindowPadding; }
    //if (window_flags & ImGuiWindowFlags_NavFlattened) { child_flags |= ImGuiChildFlags_NavFlattened; }
#endif
    if (child_flags & ImGuiChildFlags_AutoResizeX)
        child_flags &= ~ImGuiChildFlags_ResizeX;
    if (child_flags & ImGuiChildFlags_AutoResizeY)
        child_flags &= ~ImGuiChildFlags_ResizeY;

    // Set window flags
    window_flags |= ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking;
    window_flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove); // Inherit the NoMove flag
    if (child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize))
        window_flags |= ImGuiWindowFlags_AlwaysAutoResize; // FIXME: Would be sane to not make single-axis flag set this. (#9355)
    if ((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    // Special framed style
    if (child_flags & ImGuiChildFlags_FrameStyle)
    {
        PushStyleColor(ImGuiCol_ChildBg, g.Style.Colors[ImGuiCol_FrameBg]);
        PushStyleVar(ImGuiStyleVar_ChildRounding, g.Style.FrameRounding);
        PushStyleVar(ImGuiStyleVar_ChildBorderSize, g.Style.FrameBorderSize);
        PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.FramePadding);
        child_flags |= ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
        window_flags |= ImGuiWindowFlags_NoMove;
    }

    // Forward size
    // Important: Begin() has special processing to switch condition to ImGuiCond_FirstUseEver for a given axis when ImGuiChildFlags_ResizeXXX is set.
    // (the alternative would to store conditional flags per axis, which is possible but more code)
    const ImVec2 size_avail = GetContentRegionAvail();
    const ImVec2 size_default((child_flags & ImGuiChildFlags_AutoResizeX) ? 0.0f : size_avail.x, (child_flags & ImGuiChildFlags_AutoResizeY) ? 0.0f : size_avail.y);
    ImVec2 size = CalcItemSize(size_arg, size_default.x, size_default.y);

    // A SetNextWindowSize() call always has priority (#8020)
    // (since the code in Begin() never supported SizeVal==0.0f aka auto-resize via SetNextWindowSize() call, we don't support it here for now)
    // FIXME: We only support ImGuiCond_Always in this path. Supporting other paths would requires to obtain window pointer.
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) != 0 && (g.NextWindowData.SizeCond & ImGuiCond_Always) != 0)
    {
        if (g.NextWindowData.SizeVal.x > 0.0f)
        {
            size.x = g.NextWindowData.SizeVal.x;
            child_flags &= ~ImGuiChildFlags_ResizeX;
        }
        if (g.NextWindowData.SizeVal.y > 0.0f)
        {
            size.y = g.NextWindowData.SizeVal.y;
            child_flags &= ~ImGuiChildFlags_ResizeY;
        }
    }
    SetNextWindowSize(size);

    // Forward child flags (we allow prior settings to merge but it'll only work for adding flags)
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags)
        g.NextWindowData.ChildFlags |= child_flags;
    else
        g.NextWindowData.ChildFlags = child_flags;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasChildFlags;

    // Build up name. If you need to append to a same child from multiple location in the ID stack, use BeginChild(ImGuiID id) with a stable value.
    // FIXME: 2023/11/14: commented out shorted version. We had an issue with multiple ### in child window path names, which the trailing hash helped workaround.
    // e.g. "ParentName###ParentIdentifier/ChildName###ChildIdentifier" would get hashed incorrectly by ImHashStr(), trailing _%08X somehow fixes it.
    const char* temp_window_name;
    /*if (name && parent_window->IDStack.back() == parent_window->ID)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s", parent_window->Name, name); // May omit ID if in root of ID stack
    else*/
    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    // Set style
    const float backup_border_size = g.Style.ChildBorderSize;
    if ((child_flags & ImGuiChildFlags_Borders) == 0)
        g.Style.ChildBorderSize = 0.0f;

    // Begin into window
    const bool ret = Begin(temp_window_name, NULL, window_flags);

    // Restore style
    g.Style.ChildBorderSize = backup_border_size;
    if (child_flags & ImGuiChildFlags_FrameStyle)
    {
        PopStyleVar(3);
        PopStyleColor();
    }

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;

    // Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
    // While this is not really documented/defined, it seems that the expected thing to do.
    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    // Process navigation-in immediately so NavInit can run on first frame
    // Can enter a child if (A) it has navigable items or (B) it can be scrolled.
    const ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
    if (g.ActiveId == temp_id_for_activation)
        ClearActiveID();
    if (g.NavActivateId == id && !(child_flags & ImGuiChildFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(temp_id_for_activation, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
        g.ActiveIdSource = g.NavInputSource;
    }
    return ret;
}

void ImGui::EndChild()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* child_window = g.CurrentWindow;

    const ImGuiID backup_within_end_child_id = g.WithinEndChildID;
    IM_ASSERT(child_window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() calls

    g.WithinEndChildID = child_window->ID;
    ImVec2 child_size = child_window->Size;
    End();
    if (child_window->BeginCount == 1)
    {
        ImGuiWindow* parent_window = g.CurrentWindow;
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + child_size);
        ItemSize(child_size);
        const bool nav_flattened = (child_window->ChildFlags & ImGuiChildFlags_NavFlattened) != 0;
        if ((child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY) && !nav_flattened)
        {
            ItemAdd(bb, child_window->ChildId);
            RenderNavCursor(bb, child_window->ChildId);

            // When browsing a window that has no activable items (scroll only) we keep a highlight on the child (pass g.NavId to trick into always displaying)
            if (child_window->DC.NavLayersActiveMask == 0 && child_window == g.NavWindow)
                RenderNavCursor(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavRenderCursorFlags_Compact);
        }
        else
        {
            // Not navigable into
            // - This is a bit of a fringe use case, mostly useful for undecorated, non-scrolling contents childs, or empty childs.
            // - We could later decide to not apply this path if ImGuiChildFlags_FrameStyle or ImGuiChildFlags_Borders is set.
            ItemAdd(bb, child_window->ChildId, NULL, ImGuiItemFlags_NoNav);

            // But when flattened we directly reach items, adjust active layer mask accordingly
            if (nav_flattened)
                parent_window->DC.NavLayersActiveMaskNext |= child_window->DC.NavLayersActiveMaskNext;
        }
        if (g.HoveredWindow == child_window)
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
        child_window->DC.ChildItemStatusFlags = g.LastItemData.StatusFlags;
        //SetLastItemDataForChildWindowItem(child_window, child_window->Rect()); // Not needed, effectively done by ItemAdd()
    }
    else
    {
        SetLastItemDataForChildWindowItem(child_window, child_window->Rect());
    }

    g.WithinEndChildID = backup_within_end_child_id;
    g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

ImGuiWindow* ImGui::FindFrontMostVisibleChildWindow(ImGuiWindow* window)
{
    for (int n = window->DC.ChildWindows.Size - 1; n >= 0; n--)
        if (IsWindowActiveAndVisible(window->DC.ChildWindows[n]))
            return FindFrontMostVisibleChildWindow(window->DC.ChildWindows[n]);
    return window;
}

static void SetWindowConditionAllowFlags(ImGuiWindow* window, ImGuiCond flags, bool enabled)
{
    window->SetWindowPosAllowFlags       = enabled ? (window->SetWindowPosAllowFlags       | flags) : (window->SetWindowPosAllowFlags       & ~flags);
    window->SetWindowSizeAllowFlags      = enabled ? (window->SetWindowSizeAllowFlags      | flags) : (window->SetWindowSizeAllowFlags      & ~flags);
    window->SetWindowCollapsedAllowFlags = enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
    window->SetWindowDockAllowFlags      = enabled ? (window->SetWindowDockAllowFlags      | flags) : (window->SetWindowDockAllowFlags      & ~flags);
}

ImGuiWindow* ImGui::FindWindowByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    return (ImGuiWindow*)g.WindowsById.GetVoidPtr(id);
}

ImGuiWindow* ImGui::FindWindowByName(const char* name)
{
    ImGuiID id = ImHashStr(name);
    return FindWindowByID(id);
}

void ImGui::ApplyWindowSettings(ImGuiWindow* window, ImGuiWindowSettings* settings)
{
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    window->ViewportPos = main_viewport->Pos;
    if (settings->ViewportId)
    {
        window->ViewportId = settings->ViewportId;
        window->ViewportPos = ImVec2(settings->ViewportPos.x, settings->ViewportPos.y);
    }
    window->Pos = ImTrunc(ImVec2(settings->Pos.x + window->ViewportPos.x, settings->Pos.y + window->ViewportPos.y));
    if (settings->Size.x > 0 && settings->Size.y > 0)
        window->Size = window->SizeFull = ImTrunc(ImVec2(settings->Size.x, settings->Size.y));
    window->Collapsed = settings->Collapsed;
    window->DockId = settings->DockId;
    window->DockOrder = settings->DockOrder;
}

void ImGui::InitOrLoadWindowSettings(ImGuiWindow* window, ImGuiWindowSettings* settings)
{
    // Initial window state with e.g. default/arbitrary window position
    // Use SetNextWindowPos() with the appropriate condition flag to change the initial position of a window.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    window->Pos = main_viewport->Pos + ImVec2(60, 60);
    window->Size = window->SizeFull = ImVec2(0, 0);
    window->ViewportPos = main_viewport->Pos;
    window->SetWindowPosAllowFlags = window->SetWindowSizeAllowFlags = window->SetWindowCollapsedAllowFlags = window->SetWindowDockAllowFlags = ImGuiCond_Always | ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing;

    if (settings != NULL)
    {
        SetWindowConditionAllowFlags(window, ImGuiCond_FirstUseEver, false);
        ApplyWindowSettings(window, settings);
    }
    window->DC.CursorStartPos = window->DC.CursorMaxPos = window->DC.IdealMaxPos = window->Pos; // So first call to CalcWindowContentSizes() doesn't return crazy values

    if ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
    {
        window->AutoFitFramesX = window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
    else
    {
        if (window->Size.x <= 0.0f)
            window->AutoFitFramesX = 2;
        if (window->Size.y <= 0.0f)
            window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
    }
}

static ImGuiWindow* CreateNewWindow(const char* name, ImGuiWindowFlags flags)
{
    // Create window the first time
    //IMGUI_DEBUG_LOG("CreateNewWindow '%s', flags = 0x%08X\n", name, flags);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = IM_NEW(ImGuiWindow)(&g, name);
    window->Flags = flags;
    g.WindowsById.SetVoidPtr(window->ID, window);

    ImGuiWindowSettings* settings = NULL;
    if (!(flags & ImGuiWindowFlags_NoSavedSettings))
        if ((settings = ImGui::FindWindowSettingsByWindow(window)) != 0)
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);

    ImGui::InitOrLoadWindowSettings(window, settings);

    if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
        g.Windows.push_front(window); // Quite slow but rare and only once
    else
        g.Windows.push_back(window);

    return window;
}

static ImGuiWindow* GetWindowForTitleAndMenuHeight(ImGuiWindow* window)
{
    return (window->DockNodeAsHost && window->DockNodeAsHost->VisibleWindow) ? window->DockNodeAsHost->VisibleWindow : window;
}

static inline ImVec2 CalcWindowMinSize(ImGuiWindow* window)
{
    // We give windows non-zero minimum size to facilitate understanding problematic cases (e.g. empty popups)
    // FIXME: Essentially we want to restrict manual resizing to WindowMinSize+Decoration, and allow api resizing to be smaller.
    // Perhaps should tend further a neater test for this.
    ImGuiContext& g = *GImGui;
    ImVec2 size_min;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_Popup))
    {
        size_min.x = (window->ChildFlags & ImGuiChildFlags_ResizeX) ? g.Style.WindowMinSize.x : IMGUI_WINDOW_HARD_MIN_SIZE;
        size_min.y = (window->ChildFlags & ImGuiChildFlags_ResizeY) ? g.Style.WindowMinSize.y : IMGUI_WINDOW_HARD_MIN_SIZE;
    }
    else
    {
        size_min.x = ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) == 0) ? g.Style.WindowMinSize.x : IMGUI_WINDOW_HARD_MIN_SIZE;
        size_min.y = ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) == 0) ? g.Style.WindowMinSize.y : IMGUI_WINDOW_HARD_MIN_SIZE;
    }

    // Reduce artifacts with very small windows
    ImGuiWindow* window_for_height = GetWindowForTitleAndMenuHeight(window);
    size_min.y = ImMax(size_min.y, window_for_height->TitleBarHeight + window_for_height->MenuBarHeight + ImMax(0.0f, g.Style.WindowRounding - 1.0f));
    return size_min;
}

static ImVec2 CalcWindowSizeAfterConstraint(ImGuiWindow* window, const ImVec2& size_desired)
{
    ImGuiContext& g = *GImGui;
    ImVec2 new_size = size_desired;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        // See comments in SetNextWindowSizeConstraints() for details about setting size_min an size_max.
        ImRect cr = g.NextWindowData.SizeConstraintRect;
        new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
        new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
        if (g.NextWindowData.SizeCallback)
        {
            ImGuiSizeCallbackData data;
            data.UserData = g.NextWindowData.SizeCallbackUserData;
            data.Pos = window->Pos;
            data.CurrentSize = window->SizeFull;
            data.DesiredSize = new_size;
            g.NextWindowData.SizeCallback(&data);
            new_size = data.DesiredSize;
        }
        new_size.x = IM_TRUNC(new_size.x);
        new_size.y = IM_TRUNC(new_size.y);
    }

    // Minimum size
    ImVec2 size_min = CalcWindowMinSize(window);
    return ImMax(new_size, size_min);
}

static void CalcWindowContentSizes(ImGuiWindow* window, ImVec2* content_size_current, ImVec2* content_size_ideal)
{
    bool preserve_old_content_sizes = false;
    if (window->Collapsed && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
        preserve_old_content_sizes = true;
    else if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 && window->HiddenFramesCanSkipItems > 0)
        preserve_old_content_sizes = true;
    if (preserve_old_content_sizes)
    {
        *content_size_current = window->ContentSize;
        *content_size_ideal = window->ContentSizeIdeal;
        return;
    }

    content_size_current->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : ImTrunc64(window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
    content_size_current->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : ImTrunc64(window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
    content_size_ideal->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : ImTrunc64(ImMax(window->DC.CursorMaxPos.x, window->DC.IdealMaxPos.x) - window->DC.CursorStartPos.x);
    content_size_ideal->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : ImTrunc64(ImMax(window->DC.CursorMaxPos.y, window->DC.IdealMaxPos.y) - window->DC.CursorStartPos.y);
}

static ImVec2 CalcWindowAutoFitSize(ImGuiWindow* window, const ImVec2& size_contents, int axis_mask)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    const float decoration_w_without_scrollbars = window->DecoOuterSizeX1 + window->DecoOuterSizeX2 - window->ScrollbarSizes.x;
    const float decoration_h_without_scrollbars = window->DecoOuterSizeY1 + window->DecoOuterSizeY2 - window->ScrollbarSizes.y;
    ImVec2 size_pad = window->WindowPadding * 2.0f;
    ImVec2 size_desired;
    size_desired.x = (axis_mask & 1) ? size_contents.x + size_pad.x + decoration_w_without_scrollbars : window->Size.x;
    size_desired.y = (axis_mask & 2) ? size_contents.y + size_pad.y + decoration_h_without_scrollbars : window->Size.y;

    // Determine maximum window size
    // Child windows are laid within their parent (unless they are also popups/menus) and thus have no restriction
    ImVec2 size_max = ImVec2(FLT_MAX, FLT_MAX);
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || (window->Flags & ImGuiWindowFlags_Popup) != 0)
    {
        if (!window->ViewportOwned)
            size_max = ImGui::GetMainViewport()->WorkSize - style.DisplaySafeAreaPadding * 2.0f;
        const int monitor_idx = window->ViewportAllowPlatformMonitorExtend;
        if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size)
            size_max = g.PlatformIO.Monitors[monitor_idx].WorkSize - style.DisplaySafeAreaPadding * 2.0f;
    }

    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {
        // Tooltip always resize (up to maximum size)
        return ImMin(size_desired, size_max);
    }
    else
    {
        ImVec2 size_min = CalcWindowMinSize(window);
        ImVec2 size_auto_fit = ImClamp(size_desired, ImMin(size_min, size_max), size_max);

        // When the window cannot fit all contents (either because of constraints, either because screen is too small),
        // we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than ViewportSize-WindowPadding.
        ImVec2 size_auto_fit_after_constraint = CalcWindowSizeAfterConstraint(window, size_auto_fit);
        float size_contents_for_scrollbar_x = (axis_mask & 1) ? size_contents.x : window->ContentSize.x; // See #9352. In theory this should use same logic as `window->ScrollbarY = ...` codepath in Begin(). Needs some plumbling.
        float size_contents_for_scrollbar_y = (axis_mask & 2) ? size_contents.y : window->ContentSize.y;
        bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x < size_contents_for_scrollbar_x + size_pad.x + decoration_w_without_scrollbars && !(window->Flags & ImGuiWindowFlags_NoScrollbar) && (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        bool will_have_scrollbar_y = (size_auto_fit_after_constraint.y < size_contents_for_scrollbar_y + size_pad.y + decoration_h_without_scrollbars && !(window->Flags & ImGuiWindowFlags_NoScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);
        if (will_have_scrollbar_x)
            size_auto_fit.y += style.ScrollbarSize;
        if (will_have_scrollbar_y)
            size_auto_fit.x += style.ScrollbarSize;
        return size_auto_fit;
    }
}

ImVec2 ImGui::CalcWindowNextAutoFitSize(ImGuiWindow* window)
{
    ImVec2 size_contents_current;
    ImVec2 size_contents_ideal;
    CalcWindowContentSizes(window, &size_contents_current, &size_contents_ideal);
    ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, size_contents_ideal, ~0);
    ImVec2 size_final = CalcWindowSizeAfterConstraint(window, size_auto_fit);
    return size_final;
}

static ImGuiCol GetWindowBgColorIdx(ImGuiWindow* window)
{
    if (window->Flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
        return ImGuiCol_PopupBg;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !window->DockIsActive)
        return ImGuiCol_ChildBg;
    return ImGuiCol_WindowBg;
}

static void CalcResizePosSizeFromAnyCorner(ImGuiWindow* window, const ImVec2& corner_target_arg, const ImVec2& corner_norm, ImVec2* out_pos, ImVec2* out_size)
{
    ImVec2 corner_target = corner_target_arg;
    if (window->Flags & ImGuiWindowFlags_ChildWindow) // Clamp resizing of childs within parent
    {
        ImGuiWindow* parent_window = window->ParentWindow;
        ImGuiWindowFlags parent_flags = parent_window->Flags;
        ImRect limit_rect = parent_window->InnerRect;
        limit_rect.Expand(ImVec2(-ImMax(parent_window->WindowPadding.x, parent_window->WindowBorderSize), -ImMax(parent_window->WindowPadding.y, parent_window->WindowBorderSize)));
        if ((parent_flags & (ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar)) == 0 || (parent_flags & ImGuiWindowFlags_NoScrollbar))
            corner_target.x = ImClamp(corner_target.x, limit_rect.Min.x, limit_rect.Max.x);
        if (parent_flags & ImGuiWindowFlags_NoScrollbar)
            corner_target.y = ImClamp(corner_target.y, limit_rect.Min.y, limit_rect.Max.y);
    }
    ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
    ImVec2 pos_max = ImLerp(window->Pos + window->Size, corner_target, corner_norm); // Expected window lower-right
    ImVec2 size_expected = pos_max - pos_min;
    ImVec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
    *out_pos = pos_min;
    if (corner_norm.x == 0.0f)
        out_pos->x -= (size_constrained.x - size_expected.x);
    if (corner_norm.y == 0.0f)
        out_pos->y -= (size_constrained.y - size_expected.y);
    *out_size = size_constrained;
}

// Data for resizing from resize grip / corner
struct ImGuiResizeGripDef
{
    ImVec2  CornerPosN;
    ImVec2  InnerDir;
    int     AngleMin12, AngleMax12;
};
static const ImGuiResizeGripDef resize_grip_def[4] =
{
    { ImVec2(1, 1), ImVec2(-1, -1), 0, 3 },  // Lower-right
    { ImVec2(0, 1), ImVec2(+1, -1), 3, 6 },  // Lower-left
    { ImVec2(0, 0), ImVec2(+1, +1), 6, 9 },  // Upper-left (Unused)
    { ImVec2(1, 0), ImVec2(-1, +1), 9, 12 }  // Upper-right (Unused)
};

// Data for resizing from borders
struct ImGuiResizeBorderDef
{
    ImVec2  InnerDir;               // Normal toward inside
    ImVec2  SegmentN1, SegmentN2;   // End positions, normalized (0,0: upper left)
    float   OuterAngle;             // Angle toward outside
};
static const ImGuiResizeBorderDef resize_border_def[4] =
{
    { ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
    { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
    { ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
    { ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
};

static ImRect GetResizeBorderRect(ImGuiWindow* window, int border_n, float perp_padding, float thickness)
{
    ImRect rect = window->Rect();
    if (thickness == 0.0f)
        rect.Max -= ImVec2(1, 1);
    if (border_n == ImGuiDir_Left)  { return ImRect(rect.Min.x - thickness,    rect.Min.y + perp_padding, rect.Min.x + thickness,    rect.Max.y - perp_padding); }
    if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness,    rect.Min.y + perp_padding, rect.Max.x + thickness,    rect.Max.y - perp_padding); }
    if (border_n == ImGuiDir_Up)    { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness,    rect.Max.x - perp_padding, rect.Min.y + thickness);    }
    if (border_n == ImGuiDir_Down)  { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness,    rect.Max.x - perp_padding, rect.Max.y + thickness);    }
    IM_ASSERT(0);
    return ImRect();
}

// 0..3: corners (Lower-right, Lower-left, Unused, Unused)
ImGuiID ImGui::GetWindowResizeCornerID(ImGuiWindow* window, int n)
{
    IM_ASSERT(n >= 0 && n < 4);
    ImGuiID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
    id = ImHashStr("#RESIZE", 0, id);
    id = ImHashData(&n, sizeof(int), id);
    return id;
}

// Borders (Left, Right, Up, Down)
ImGuiID ImGui::GetWindowResizeBorderID(ImGuiWindow* window, ImGuiDir dir)
{
    IM_ASSERT(dir >= 0 && dir < 4);
    int n = (int)dir + 4;
    ImGuiID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
    id = ImHashStr("#RESIZE", 0, id);
    id = ImHashData(&n, sizeof(int), id);
    return id;
}

// Handle resize for: Resize Grips, Borders, Gamepad
// Return true when using auto-fit (double-click on resize grip)
static int ImGui::UpdateWindowManualResize(ImGuiWindow* window, int* border_hovered, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindowFlags flags = window->Flags;

    if ((flags & ImGuiWindowFlags_NoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
        return false;
    if ((flags & ImGuiWindowFlags_AlwaysAutoResize) && (window->ChildFlags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
        return false;
    if (window->WasActive == false) // Early out to avoid running this code for e.g. a hidden implicit/fallback Debug window.
        return false;

    int ret_auto_fit_mask = 0x00;
    const float grip_draw_size = IM_TRUNC(ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
    const float grip_hover_inner_size = (resize_grip_count > 0) ? IM_TRUNC(grip_draw_size * 0.75f) : 0.0f;
    const float grip_hover_outer_size = g.WindowsBorderHoverPadding;

    ImRect clamp_rect = visibility_rect;
    const bool window_move_from_title_bar = !(window->BgClickFlags & ImGuiWindowBgClickFlags_Move) && !(window->Flags & ImGuiWindowFlags_NoTitleBar);
    if (window_move_from_title_bar)
        clamp_rect.Min.y -= window->TitleBarHeight;

    ImVec2 pos_target(FLT_MAX, FLT_MAX);
    ImVec2 size_target(FLT_MAX, FLT_MAX);

    // Clip mouse interaction rectangles within the viewport rectangle (in practice the narrowing is going to happen most of the time).
    // - Not narrowing would mostly benefit the situation where OS windows _without_ decoration have a threshold for hovering when outside their limits.
    //   This is however not the case with current backends under Win32, but a custom borderless window implementation would benefit from it.
    // - When decoration are enabled we typically benefit from that distance, but then our resize elements would be conflicting with OS resize elements, so we also narrow.
    // - Note that we are unable to tell if the platform setup allows hovering with a distance threshold (on Win32, decorated window have such threshold).
    // We only clip interaction so we overwrite window->ClipRect, cannot call PushClipRect() yet as DrawList is not yet setup.
    const bool clip_with_viewport_rect = !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport) || (g.IO.MouseHoveredViewport != window->ViewportId) || !(window->Viewport->Flags & ImGuiViewportFlags_NoDecoration);
    if (clip_with_viewport_rect)
        window->ClipRect = window->Viewport->GetMainRect();

    // Resize grips and borders are on layer 1
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Manual resize grips
    PushID("#RESIZE");
    for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
    {
        const ImGuiResizeGripDef& def = resize_grip_def[resize_grip_n];
        const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

        // Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
        bool hovered, held;
        ImRect resize_rect(corner - def.InnerDir * grip_hover_outer_size, corner + def.InnerDir * grip_hover_inner_size);
        if (resize_rect.Min.x > resize_rect.Max.x) ImSwap(resize_rect.Min.x, resize_rect.Max.x);
        if (resize_rect.Min.y > resize_rect.Max.y) ImSwap(resize_rect.Min.y, resize_rect.Max.y);
        ImGuiID resize_grip_id = window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
        ItemAdd(resize_rect, resize_grip_id, NULL, ImGuiItemFlags_NoNav);
        ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
        //GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, IM_COL32(255, 255, 0, 255));
        if (hovered || held)
            SetMouseCursor((resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE);

        if (held && g.IO.MouseDoubleClicked[0])
        {
            // Auto-fit when double-clicking
            ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, ~0);
            size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
            ret_auto_fit_mask = 0x03; // Both axes
            ClearActiveID();
        }
        else if (held)
        {
            // Resize from any of the four corners
            // We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
            ImVec2 clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? clamp_rect.Min.x : -FLT_MAX, (def.CornerPosN.y == 1.0f || (def.CornerPosN.y == 0.0f && window_move_from_title_bar)) ? clamp_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? clamp_rect.Max.x : +FLT_MAX, def.CornerPosN.y == 0.0f ? clamp_rect.Max.y : +FLT_MAX);
            ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset + ImLerp(def.InnerDir * grip_hover_outer_size, def.InnerDir * -grip_hover_inner_size, def.CornerPosN); // Corner of the window corresponding to our corner grip
            corner_target = ImClamp(corner_target, clamp_min, clamp_max);
            CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
        }

        // Only lower-left grip is visible before hovering/activating
        const bool resize_grip_visible = held || hovered || (resize_grip_n == 0 && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0);
        if (resize_grip_visible)
            resize_grip_col[resize_grip_n] = GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
    }

    int resize_border_mask = 0x00;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        resize_border_mask |= ((window->ChildFlags & ImGuiChildFlags_ResizeX) ? 0x02 : 0) | ((window->ChildFlags & ImGuiChildFlags_ResizeY) ? 0x08 : 0);
    else
        resize_border_mask = g.IO.ConfigWindowsResizeFromEdges ? 0x0F : 0x00;
    for (int border_n = 0; border_n < 4; border_n++)
    {
        if ((resize_border_mask & (1 << border_n)) == 0)
            continue;
        const ImGuiResizeBorderDef& def = resize_border_def[border_n];
        const ImGuiAxis axis = (border_n == ImGuiDir_Left || border_n == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;

        bool hovered, held;
        ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, g.WindowsBorderHoverPadding);
        ImGuiID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
        ItemAdd(border_rect, border_id, NULL, ImGuiItemFlags_NoNav);
        ButtonBehavior(border_rect, border_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
        //GetForegroundDrawList(window)->AddRect(border_rect.Min, border_rect.Max, IM_COL32(255, 255, 0, 255));
        if (hovered && g.HoveredIdTimer <= WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER)
            hovered = false;
        if (hovered || held)
            SetMouseCursor((axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
        if (held && g.IO.MouseDoubleClicked[0])
        {
            // Double-clicking bottom or right border auto-fit on this axis
            // FIXME: Support top and right borders: rework CalcResizePosSizeFromAnyCorner() to be reusable in both cases.
            if (border_n == 1 || border_n == 3) // Right and bottom border
            {
                ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, 1 << axis);
                size_target[axis] = CalcWindowSizeAfterConstraint(window, size_auto_fit)[axis];
                ret_auto_fit_mask |= (1 << axis);
                hovered = held = false; // So border doesn't show highlighted at new position
            }
            ClearActiveID();
        }
        else if (held)
        {
            // Switch to relative resizing mode when border geometry moved (e.g. resizing a child altering parent scroll), in order to avoid resizing feedback loop.
            // Currently only using relative mode on resizable child windows, as the problem to solve is more likely noticeable for them, but could apply for all windows eventually.
            // FIXME: May want to generalize this idiom at lower-level, so more widgets can use it!
            const bool just_scrolled_manually_while_resizing = (g.WheelingWindow != NULL && g.WheelingWindowScrolledFrame == g.FrameCount && IsWindowChildOf(window, g.WheelingWindow, false, true));
            if (g.ActiveIdIsJustActivated || just_scrolled_manually_while_resizing)
            {
                g.WindowResizeBorderExpectedRect = border_rect;
                g.WindowResizeRelativeMode = false;
            }
            if ((window->Flags & ImGuiWindowFlags_ChildWindow) && memcmp(&g.WindowResizeBorderExpectedRect, &border_rect, sizeof(ImRect)) != 0)
                g.WindowResizeRelativeMode = true;

            const ImVec2 border_curr = (window->Pos + ImMin(def.SegmentN1, def.SegmentN2) * window->Size);
            const float border_target_rel_mode_for_axis = border_curr[axis] + g.IO.MouseDelta[axis];
            const float border_target_abs_mode_for_axis = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + g.WindowsBorderHoverPadding; // Match ButtonBehavior() padding above.

            // Use absolute mode position
            ImVec2 border_target = window->Pos;
            border_target[axis] = border_target_abs_mode_for_axis;

            // Use relative mode target for child window, ignore resize when moving back toward the ideal absolute position.
            bool ignore_resize = false;
            if (g.WindowResizeRelativeMode)
            {
                //GetForegroundDrawList()->AddText(GetMainViewport()->WorkPos, IM_COL32_WHITE, "Relative Mode");
                border_target[axis] = border_target_rel_mode_for_axis;
                if (g.IO.MouseDelta[axis] == 0.0f || (g.IO.MouseDelta[axis] > 0.0f) == (border_target_rel_mode_for_axis > border_target_abs_mode_for_axis))
                    ignore_resize = true;
            }

            // Clamp, apply
            ImVec2 clamp_min(border_n == ImGuiDir_Right ? clamp_rect.Min.x : -FLT_MAX, border_n == ImGuiDir_Down || (border_n == ImGuiDir_Up && window_move_from_title_bar) ? clamp_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max(border_n == ImGuiDir_Left ? clamp_rect.Max.x : +FLT_MAX, border_n == ImGuiDir_Up ? clamp_rect.Max.y : +FLT_MAX);
            border_target = ImClamp(border_target, clamp_min, clamp_max);
            if (!ignore_resize)
                CalcResizePosSizeFromAnyCorner(window, border_target, ImMin(def.SegmentN1, def.SegmentN2), &pos_target, &size_target);
        }
        if (hovered)
            *border_hovered = border_n;
        if (held)
            *border_held = border_n;
    }
    PopID();

    // Restore nav layer
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

    // Navigation resize (keyboard/gamepad)
    // FIXME: This cannot be moved to NavUpdateWindowing() because CalcWindowSizeAfterConstraint() need to callback into user.
    // Not even sure the callback works here.
    if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindowDockTree == window)
    {
        ImVec2 nav_resize_dir;
        if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
            nav_resize_dir = GetKeyMagnitude2d(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
        if (g.NavInputSource == ImGuiInputSource_Gamepad)
            nav_resize_dir = GetKeyMagnitude2d(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadDpadDown);
        if (nav_resize_dir.x != 0.0f || nav_resize_dir.y != 0.0f)
        {
            const float NAV_RESIZE_SPEED = 600.0f;
            const float resize_step = NAV_RESIZE_SPEED * g.IO.DeltaTime * GetScale();
            g.NavWindowingAccumDeltaSize += nav_resize_dir * resize_step;
            g.NavWindowingAccumDeltaSize = ImMax(g.NavWindowingAccumDeltaSize, clamp_rect.Min - window->Pos - window->Size); // We need Pos+Size >= clmap_rect.Min, so Size >= clmap_rect.Min - Pos, so size_delta >= clmap_rect.Min - window->Pos - window->Size
            g.NavWindowingToggleLayer = false;
            g.NavHighlightItemUnderNav = true;
            resize_grip_col[0] = GetColorU32(ImGuiCol_ResizeGripActive);
            ImVec2 accum_floored = ImTrunc(g.NavWindowingAccumDeltaSize);
            if (accum_floored.x != 0.0f || accum_floored.y != 0.0f)
            {
                // FIXME-NAV: Should store and accumulate into a separate size buffer to handle sizing constraints properly, right now a constraint will make us stuck.
                size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull + accum_floored);
                g.NavWindowingAccumDeltaSize -= accum_floored;
            }
        }
    }

    // Apply back modified position/size to window
    const ImVec2 old_pos = window->Pos;
    const ImVec2 old_size = window->SizeFull;
    if (size_target.x != FLT_MAX && (window->Size.x != size_target.x || window->SizeFull.x != size_target.x))
        window->Size.x = window->SizeFull.x = size_target.x;
    if (size_target.y != FLT_MAX && (window->Size.y != size_target.y || window->SizeFull.y != size_target.y))
        window->Size.y = window->SizeFull.y = size_target.y;
    if (pos_target.x != FLT_MAX && window->Pos.x != ImTrunc(pos_target.x))
        window->Pos.x = ImTrunc(pos_target.x);
    if (pos_target.y != FLT_MAX && window->Pos.y != ImTrunc(pos_target.y))
        window->Pos.y = ImTrunc(pos_target.y);
    if (old_pos.x != window->Pos.x || old_pos.y != window->Pos.y || old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
        MarkIniSettingsDirty(window);

    // Recalculate next expected border expected coordinates
    if (*border_held != -1)
        g.WindowResizeBorderExpectedRect = GetResizeBorderRect(window, *border_held, grip_hover_inner_size, g.WindowsBorderHoverPadding);

    return ret_auto_fit_mask;
}

static inline void ClampWindowPos(ImGuiWindow* window, const ImRect& visibility_rect)
{
    ImVec2 size_for_clamping = window->Size;
    const bool move_from_title_bar_only = (window->BgClickFlags & ImGuiWindowBgClickFlags_Move) == 0;
    if (move_from_title_bar_only && window->DockNodeAsHost)
        size_for_clamping.y = ImGui::GetFrameHeight(); // Not using window->TitleBarHeight() as DockNodeAsHost will report 0.0f here.
    else if (move_from_title_bar_only && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        size_for_clamping.y = window->TitleBarHeight;
    window->Pos = ImClamp(window->Pos, visibility_rect.Min - size_for_clamping, visibility_rect.Max);
}

static void RenderWindowOuterSingleBorder(ImGuiWindow* window, int border_n, ImU32 border_col, float border_size)
{
    const ImGuiResizeBorderDef& def = resize_border_def[border_n];
    const float rounding = window->WindowRounding;
    const ImRect border_r = GetResizeBorderRect(window, border_n, rounding, 0.0f);
    window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN1) + ImVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
    window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN2) + ImVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
    window->DrawList->PathStroke(border_col, ImDrawFlags_None, border_size);
}

void ImGui::RenderWindowOuterBorders(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    const float border_size = window->WindowBorderSize;
    const ImU32 border_col = GetColorU32(ImGuiCol_Border);
    if (border_size > 0.0f && (window->Flags & ImGuiWindowFlags_NoBackground) == 0)
        window->DrawList->AddRect(window->Pos, window->Pos + window->Size, border_col, window->WindowRounding, 0, window->WindowBorderSize);
    else if (border_size > 0.0f)
    {
        if (window->ChildFlags & ImGuiChildFlags_ResizeX) // Similar code as 'resize_border_mask' computation in UpdateWindowManualResize() but we specifically only always draw explicit child resize border.
            RenderWindowOuterSingleBorder(window, 1, border_col, border_size);
        if (window->ChildFlags & ImGuiChildFlags_ResizeY)
            RenderWindowOuterSingleBorder(window, 3, border_col, border_size);
    }
    if (window->ResizeBorderHovered != -1 || window->ResizeBorderHeld != -1)
    {
        const int border_n = (window->ResizeBorderHeld != -1) ? window->ResizeBorderHeld : window->ResizeBorderHovered;
        const ImU32 border_col_resizing = GetColorU32((window->ResizeBorderHeld != -1) ? ImGuiCol_SeparatorActive : ImGuiCol_SeparatorHovered);
        RenderWindowOuterSingleBorder(window, border_n, border_col_resizing, ImMax(2.0f, window->WindowBorderSize)); // Thicker than usual
    }
    if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
    {
        float y = window->Pos.y + window->TitleBarHeight - 1;
        window->DrawList->AddLine(ImVec2(window->Pos.x + border_size * 0.5f, y), ImVec2(window->Pos.x + window->Size.x - border_size * 0.5f, y), border_col, g.Style.FrameBorderSize);
    }
}

// Draw background and borders
// Draw and handle scrollbars
void ImGui::RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    // Ensure that Scrollbar() doesn't read last frame's SkipItems
    IM_ASSERT(window->BeginCount == 0);
    window->SkipItems = false;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Draw window + handle manual resize
    // As we highlight the title bar when want_focus is set, multiple reappearing windows will have their title bar highlighted on their reappearing frame.
    const float window_rounding = window->WindowRounding;
    const float window_border_size = window->WindowBorderSize;
    if (window->Collapsed)
    {
        // Title bar only
        const float backup_border_size = style.FrameBorderSize;
        g.Style.FrameBorderSize = window->WindowBorderSize;
        ImU32 title_bar_col = GetColorU32((title_bar_is_highlight && g.NavCursorVisible) ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBgCollapsed);
        if (window->ViewportOwned)
            title_bar_col |= IM_COL32_A_MASK; // No alpha (we don't support is_docking_transparent_payload here because simpler and less meaningful, but could with a bit of code shuffle/reuse)
        RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
        g.Style.FrameBorderSize = backup_border_size;
    }
    else
    {
        // Window background
        if (!(flags & ImGuiWindowFlags_NoBackground))
        {
            bool is_docking_transparent_payload = false;
            if (g.DragDropActive && (g.FrameCount - g.DragDropAcceptFrameCount) <= 1 && g.IO.ConfigDockingTransparentPayload)
                if (g.DragDropPayload.IsDataType(IMGUI_PAYLOAD_TYPE_WINDOW) && *(ImGuiWindow**)g.DragDropPayload.Data == window)
                    is_docking_transparent_payload = true;

            ImU32 bg_col = GetColorU32(GetWindowBgColorIdx(window));
            if (window->ViewportOwned)
            {
                bg_col |= IM_COL32_A_MASK; // No alpha
                if (is_docking_transparent_payload)
                    window->Viewport->Alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA;
            }
            else
            {
                // Adjust alpha. For docking
                bool override_alpha = false;
                float alpha = 1.0f;
                if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasBgAlpha)
                {
                    alpha = g.NextWindowData.BgAlphaVal;
                    override_alpha = true;
                }
                if (is_docking_transparent_payload)
                {
                    alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA; // FIXME-DOCK: Should that be an override?
                    override_alpha = true;
                }
                if (override_alpha)
                    bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(alpha) << IM_COL32_A_SHIFT);
            }

            // Render, for docked windows and host windows we ensure BG goes before decorations
            if (window->DockIsActive)
                window->DockNode->LastBgColor = bg_col;
            if (flags & ImGuiWindowFlags_DockNodeHost)
                bg_col = 0;
            if (bg_col & IM_COL32_A_MASK)
            {
                ImRect bg_rect(window->Pos + ImVec2(0, window->TitleBarHeight), window->Pos + window->Size);
                ImDrawFlags bg_rounding_flags;
                if (window->DockIsActive)
                    bg_rounding_flags = CalcRoundingFlagsForRectInRect(bg_rect, window->DockNode->HostWindow->Rect(), 0.0f);
                else
                    bg_rounding_flags = (flags & ImGuiWindowFlags_NoTitleBar) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersBottom;
                ImDrawList* bg_draw_list = window->DockIsActive ? window->DockNode->HostWindow->DrawList : window->DrawList;
                if (window->DockIsActive)
                    bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
                bg_draw_list->AddRectFilled(bg_rect.Min, bg_rect.Max, bg_col, window_rounding, bg_rounding_flags);
                if (window->DockIsActive)
                    bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG);
            }
        }
        if (window->DockIsActive)
            window->DockNode->IsBgDrawnThisFrame = true;

        // Title bar
        // (when docked, DockNode are drawing their own title bar. Individual windows however do NOT set the _NoTitleBar flag,
        // in order for their pos/size to be matching their undocking state.)
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
        {
            ImU32 title_bar_col = GetColorU32(title_bar_is_highlight ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg);
            if (window->ViewportOwned)
                title_bar_col |= IM_COL32_A_MASK; // No alpha
            window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding, ImDrawFlags_RoundCornersTop);
        }

        // Menu bar
        if (flags & ImGuiWindowFlags_MenuBar)
        {
            ImRect menu_bar_rect = window->MenuBarRect();
            menu_bar_rect.ClipWith(window->Rect());  // Soft clipping, in particular child window don't have minimum size covering the menu bar so this is useful for them.
            window->DrawList->AddRectFilled(menu_bar_rect.Min, menu_bar_rect.Max, GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, ImDrawFlags_RoundCornersTop);
            if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
                window->DrawList->AddLine(menu_bar_rect.GetBL() + ImVec2(window_border_size * 0.5f, 0.0f), menu_bar_rect.GetBR() - ImVec2(window_border_size * 0.5f, 0.0f), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
        }

        // Docking: Unhide tab bar (small triangle in the corner), drag from small triangle to quickly undock
        ImGuiDockNode* node = window->DockNode;
        if (window->DockIsActive && node->IsHiddenTabBar() && !node->IsNoTabBar())
        {
            float unhide_sz_draw = ImTrunc(g.FontSize * 0.70f);
            float unhide_sz_hit = ImTrunc(g.FontSize * 0.55f);
            ImVec2 p = node->Pos;
            ImRect r(p, p + ImVec2(unhide_sz_hit, unhide_sz_hit));
            ImGuiID unhide_id = window->GetID("#UNHIDE");
            KeepAliveID(unhide_id);
            bool hovered, held;
            if (ButtonBehavior(r, unhide_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren))
                node->WantHiddenTabBarToggle = true;
            else if (held && IsMouseDragging(0))
                StartMouseMovingWindowOrNode(window, node, true); // Undock from tab-bar triangle = same as window/collapse menu button

            // FIXME-DOCK: Ideally we'd use ImGuiCol_TitleBgActive/ImGuiCol_TitleBg here, but neither is guaranteed to be visible enough at this sort of size..
            ImU32 col = GetColorU32(((held && hovered) || (node->IsFocused && !hovered)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            window->DrawList->AddTriangleFilled(p, p + ImVec2(unhide_sz_draw, 0.0f), p + ImVec2(0.0f, unhide_sz_draw), col);
        }

        // Scrollbars
        if (window->ScrollbarX)
            Scrollbar(ImGuiAxis_X);
        if (window->ScrollbarY)
            Scrollbar(ImGuiAxis_Y);

        // Render resize grips (after their input handling so we don't have a frame of latency)
        if (handle_borders_and_resize_grips && !(flags & ImGuiWindowFlags_NoResize))
        {
            for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
            {
                const ImU32 col = resize_grip_col[resize_grip_n];
                if ((col & IM_COL32_A_MASK) == 0)
                    continue;
                const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
                const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
                const float border_inner = IM_ROUND(window_border_size * 0.5f);
                window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(border_inner, resize_grip_draw_size) : ImVec2(resize_grip_draw_size, border_inner)));
                window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(resize_grip_draw_size, border_inner) : ImVec2(border_inner, resize_grip_draw_size)));
                window->DrawList->PathArcToFast(ImVec2(corner.x + grip.InnerDir.x * (window_rounding + border_inner), corner.y + grip.InnerDir.y * (window_rounding + border_inner)), window_rounding, grip.AngleMin12, grip.AngleMax12);
                window->DrawList->PathFillConvex(col);
            }
        }

        // Borders (for dock node host they will be rendered over after the tab bar)
        if (handle_borders_and_resize_grips && !window->DockNodeAsHost)
            RenderWindowOuterBorders(window);
    }
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
}

// When inside a dock node, this is handled in DockNodeCalcTabBarLayout() instead.
// Render title text, collapse button, close button
void ImGui::RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    const bool has_close_button = (p_open != NULL);
    const bool has_collapse_button = !(flags & ImGuiWindowFlags_NoCollapse) && (style.WindowMenuButtonPosition != ImGuiDir_None);

    // Close & Collapse button are on the Menu NavLayer and don't default focus (unless there's nothing else on that layer)
    // FIXME-NAV: Might want (or not?) to set the equivalent of ImGuiButtonFlags_NoNavFocus so that mouse clicks on standard title bar items don't necessarily set nav/keyboard ref?
    const ImGuiItemFlags item_flags_backup = g.CurrentItemFlags;
    g.CurrentItemFlags |= ImGuiItemFlags_NoNavDefaultFocus;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Layout buttons
    // FIXME: Would be nice to generalize the subtleties expressed here into reusable code.
    float pad_l = style.FramePadding.x;
    float pad_r = style.FramePadding.x;
    float button_sz = g.FontSize;
    ImVec2 close_button_pos;
    ImVec2 collapse_button_pos;
    if (has_close_button)
    {
        close_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        pad_r += button_sz + style.ItemInnerSpacing.x;
    }
    if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Right)
    {
        collapse_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        pad_r += button_sz + style.ItemInnerSpacing.x;
    }
    if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Left)
    {
        collapse_button_pos = ImVec2(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y + style.FramePadding.y);
        pad_l += button_sz + style.ItemInnerSpacing.x;
    }

    // Collapse button (submitting first so it gets priority when choosing a navigation init fallback)
    if (has_collapse_button)
        if (CollapseButton(window->GetID("#COLLAPSE"), collapse_button_pos, NULL))
            window->WantCollapseToggle = true; // Defer actual collapsing to next frame as we are too far in the Begin() function

    // Close button
    if (has_close_button)
    {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_NoFocus;
        if (CloseButton(window->GetID("#CLOSE"), close_button_pos))
            *p_open = false;
        g.CurrentItemFlags = backup_item_flags;
    }

    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    g.CurrentItemFlags = item_flags_backup;

    // Title bar text (with: horizontal alignment, avoiding collapse/close button, optional "unsaved document" marker)
    // FIXME: Refactor text alignment facilities along with RenderText helpers, this is WAY too much messy code..
    const float marker_size_x = (flags & ImGuiWindowFlags_UnsavedDocument) ? button_sz * 0.80f : 0.0f;
    const ImVec2 text_size = CalcTextSize(name, NULL, true) + ImVec2(marker_size_x, 0.0f);

    // As a nice touch we try to ensure that centered title text doesn't get affected by visibility of Close/Collapse button,
    // while uncentered title text will still reach edges correctly.
    if (pad_l > style.FramePadding.x)
        pad_l += g.Style.ItemInnerSpacing.x;
    if (pad_r > style.FramePadding.x)
        pad_r += g.Style.ItemInnerSpacing.x;
    if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f)
    {
        float centerness = ImSaturate(1.0f - ImFabs(style.WindowTitleAlign.x - 0.5f) * 2.0f); // 0.0f on either edges, 1.0f on center
        float pad_extend = ImMin(ImMax(pad_l, pad_r), title_bar_rect.GetWidth() - pad_l - pad_r - text_size.x);
        pad_l = ImMax(pad_l, pad_extend * centerness);
        pad_r = ImMax(pad_r, pad_extend * centerness);
    }

    ImRect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y, title_bar_rect.Max.x - pad_r, title_bar_rect.Max.y);
    ImRect clip_r(layout_r.Min.x, layout_r.Min.y, ImMin(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x), layout_r.Max.y);
    if (flags & ImGuiWindowFlags_UnsavedDocument)
    {
        ImVec2 marker_pos;
        marker_pos.x = ImClamp(layout_r.Min.x + (layout_r.GetWidth() - text_size.x) * style.WindowTitleAlign.x + text_size.x, layout_r.Min.x, layout_r.Max.x);
        marker_pos.y = (layout_r.Min.y + layout_r.Max.y) * 0.5f;
        if (marker_pos.x > layout_r.Min.x)
        {
            RenderBullet(window->DrawList, marker_pos, GetColorU32(ImGuiCol_UnsavedMarker));
            clip_r.Max.x = ImMin(clip_r.Max.x, marker_pos.x - (int)(marker_size_x * 0.5f));
        }
    }
    //if (g.IO.KeyShift) window->DrawList->AddRect(layout_r.Min, layout_r.Max, IM_COL32(255, 128, 0, 255)); // [DEBUG]
    //if (g.IO.KeyCtrl) window->DrawList->AddRect(clip_r.Min, clip_r.Max, IM_COL32(255, 128, 0, 255)); // [DEBUG]
    RenderTextClipped(layout_r.Min, layout_r.Max, name, NULL, &text_size, style.WindowTitleAlign, &clip_r);
}

void ImGui::UpdateWindowParentAndRootLinks(ImGuiWindow* window, ImGuiWindowFlags flags, ImGuiWindow* parent_window)
{
    window->ParentWindow = parent_window;
    window->RootWindow = window->RootWindowPopupTree = window->RootWindowDockTree = window->RootWindowForTitleBarHighlight = window->RootWindowForNav = window;
    if (parent_window && (flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Tooltip))
    {
        window->RootWindowDockTree = parent_window->RootWindowDockTree;
        if (!window->DockIsActive && !(parent_window->Flags & ImGuiWindowFlags_DockNodeHost))
            window->RootWindow = parent_window->RootWindow;
    }
    if (parent_window && (flags & ImGuiWindowFlags_Popup))
        window->RootWindowPopupTree = parent_window->RootWindowPopupTree;
    if (parent_window && !(flags & ImGuiWindowFlags_Modal) && (flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip))) // FIXME: simply use _NoTitleBar ?
        window->RootWindowForTitleBarHighlight = parent_window->RootWindowForTitleBarHighlight;
    while (window->RootWindowForNav->ChildFlags & ImGuiChildFlags_NavFlattened)
    {
        IM_ASSERT(window->RootWindowForNav->ParentWindow != NULL);
        window->RootWindowForNav = window->RootWindowForNav->ParentWindow;
    }
}

// [EXPERIMENTAL] Called by Begin(). NextWindowData is valid at this point.
// This is designed as a toy/test-bed for
void ImGui::UpdateWindowSkipRefresh(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    window->SkipRefresh = false;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasRefreshPolicy) == 0)
        return;
    if (g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_TryToAvoidRefresh)
    {
        // FIXME-IDLE: Tests for e.g. mouse clicks or keyboard while focused.
        if (window->Appearing) // If currently appearing
            return;
        if (window->Hidden) // If was hidden (previous frame)
            return;
        if ((g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_RefreshOnHover) && g.HoveredWindow)
            if (window->RootWindow == g.HoveredWindow->RootWindow || IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, window))
                return;
        if ((g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_RefreshOnFocus) && g.NavWindow)
            if (window->RootWindow == g.NavWindow->RootWindow || IsWindowWithinBeginStackOf(g.NavWindow->RootWindow, window))
                return;
        window->DrawList = NULL;
        window->SkipRefresh = true;
    }
}

static void SetWindowActiveForSkipRefresh(ImGuiWindow* window)
{
    window->Active = true;
    for (ImGuiWindow* child : window->DC.ChildWindows)
        if (!child->Hidden)
        {
            child->Active = child->SkipRefresh = true;
            SetWindowActiveForSkipRefresh(child);
        }
}

// Push a new Dear ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    IM_ASSERT(name != NULL && name[0] != '\0');     // Window name required
    IM_ASSERT(g.WithinFrameScope);                  // Forgot to call ImGui::NewFrame()
    IM_ASSERT(g.FrameCountEnded != g.FrameCount);   // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

    // Find or create
    ImGuiWindow* window = FindWindowByName(name);
    const bool window_just_created = (window == NULL);
    if (window_just_created)
        window = CreateNewWindow(name, flags);

    // [DEBUG] Debug break requested by user
    if (g.DebugBreakInWindow == window->ID)
        IM_DEBUG_BREAK();

    // Automatically disable manual moving/resizing when NoInputs is set
    if ((flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    const int current_frame = g.FrameCount;
    const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
    window->IsFallbackWindow = (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

    // Update the Appearing flag (note: the BeginDocked() path may also set this to true later)
    bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1); // Not using !WasActive because the implicit "Debug" window would always toggle off->on
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
        window_just_activated_by_user |= (window != popup_ref.Window);
    }

    // Update Flags, LastFrameActive, BeginOrderXXX fields
    const bool window_was_appearing = window->Appearing;
    if (first_begin_of_the_frame)
    {
        UpdateWindowInFocusOrderList(window, window_just_created, flags);
        window->Appearing = window_just_activated_by_user;
        if (window->Appearing)
            SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);
        window->FlagsPreviousFrame = window->Flags;
        window->Flags = (ImGuiWindowFlags)flags;
        window->ChildFlags = (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags) ? g.NextWindowData.ChildFlags : 0;
        window->LastFrameActive = current_frame;
        window->LastTimeActive = (float)g.Time;
        window->BeginOrderWithinParent = 0;
        window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
    }
    else
    {
        flags = window->Flags;
    }

    // Docking
    // (NB: during the frame dock nodes are created, it is possible that (window->DockIsActive == false) even though (window->DockNode->Windows.Size > 1)
    IM_ASSERT(window->DockNode == NULL || window->DockNodeAsHost == NULL); // Cannot be both
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasDock)
        SetWindowDock(window, g.NextWindowData.DockId, g.NextWindowData.DockCond);
    if (first_begin_of_the_frame)
    {
        const bool has_dock_node = (window->DockId != 0 || window->DockNode != NULL);
        const bool new_auto_dock_node = !has_dock_node && GetWindowAlwaysWantOwnTabBar(window);
        const bool dock_node_was_visible = window->DockNodeIsVisible;
        const bool dock_tab_was_visible = window->DockTabIsVisible;
        window->DockIsActive = window->DockNodeIsVisible = window->DockTabIsVisible = false;

        if (has_dock_node || new_auto_dock_node)
        {
            BeginDocked(window, p_open);
            flags = window->Flags;
            if (window->DockIsActive)
            {
                IM_ASSERT(window->DockNode != NULL);
                g.NextWindowData.HasFlags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint; // Docking currently override constraints
            }

            // Amend the Appearing flag
            if (window->DockTabIsVisible && !dock_tab_was_visible && dock_node_was_visible && !window->Appearing && !window_was_appearing)
            {
                window->Appearing = true;
                SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);
            }
        }
    }

    // Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from a different window stack
    ImGuiWindow* parent_window_in_stack = (window->DockIsActive && window->DockNode->HostWindow) ? window->DockNode->HostWindow : g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back().Window;
    ImGuiWindow* parent_window = first_begin_of_the_frame ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) ? parent_window_in_stack : NULL) : window->ParentWindow;
    IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

    // We allow window memory to be compacted so recreate the base stack when needed.
    if (window->IDStack.Size == 0)
        window->IDStack.push_back(window->ID);

    // Add to stack
    g.CurrentWindow = window;
    g.CurrentWindowStack.resize(g.CurrentWindowStack.Size + 1);
    ImGuiWindowStackData& window_stack_data = g.CurrentWindowStack.back();
    window_stack_data.Window = window;
    window_stack_data.ParentLastItemDataBackup = g.LastItemData;
    window_stack_data.DisabledOverrideReenable = (flags & ImGuiWindowFlags_Tooltip) && (g.CurrentItemFlags & ImGuiItemFlags_Disabled);
    window_stack_data.DisabledOverrideReenableAlphaBackup = 0.0f;
    ErrorRecoveryStoreState(&window_stack_data.StackSizesInBegin);
    g.StackSizesInBeginForCurrentWindow = &window_stack_data.StackSizesInBegin;
    if (flags & ImGuiWindowFlags_ChildMenu)
        g.BeginMenuDepth++;

    // Update ->RootWindow and others pointers (before any possible call to FocusWindow)
    if (first_begin_of_the_frame)
    {
        UpdateWindowParentAndRootLinks(window, flags, parent_window);
        window->ParentWindowInBeginStack = parent_window_in_stack;

        // Focus route
        // There's little point to expose a flag to set this: because the interesting cases won't be using parent_window_in_stack,
        // Use for e.g. linking a tool window in a standalone viewport to a document window, regardless of their Begin() stack parenting. (#6798)
        window->ParentWindowForFocusRoute = (window->RootWindow != window) ? parent_window_in_stack : NULL;
        if (window->ParentWindowForFocusRoute == NULL && window->DockNode != NULL)
            if (window->DockNode->MergedFlags & ImGuiDockNodeFlags_DockedWindowsInFocusRoute)
                window->ParentWindowForFocusRoute = window->DockNode->HostWindow;

        // Override with SetNextWindowClass() field or direct call to SetWindowParentWindowForFocusRoute()
        if (window->WindowClass.FocusRouteParentWindowId != 0)
        {
            window->ParentWindowForFocusRoute = FindWindowByID(window->WindowClass.FocusRouteParentWindowId);
            IM_ASSERT(window->ParentWindowForFocusRoute != 0); // Invalid value for FocusRouteParentWindowId.
        }

        // Inherit SetWindowFontScale() from parent until we fix this system...
        window->FontWindowScaleParents = parent_window ? parent_window->FontWindowScaleParents * parent_window->FontWindowScale : 1.0f;
    }

    // Add to focus scope stack
    PushFocusScope((window->ChildFlags & ImGuiChildFlags_NavFlattened) ? g.CurrentFocusScopeId : window->ID);
    window->NavRootFocusScopeId = g.CurrentFocusScopeId;

    // Add to popup stacks: update OpenPopupStack[] data, push to BeginPopupStack[]
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        popup_ref.Window = window;
        popup_ref.ParentNavLayer = parent_window_in_stack->DC.NavLayerCurrent;
        g.BeginPopupStack.push_back(popup_ref);
        window->PopupId = popup_ref.PopupId;
    }

    // Process SetNextWindow***() calls
    // (FIXME: Consider splitting the HasXXX flags into X/Y components)
    bool window_pos_set_by_api = false;
    bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos)
    {
        window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
        if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
        {
            // May be processed on the next frame if this is our first frame and we are measuring size
            // FIXME: Look into removing the branch so everything can go through this same code path for consistency.
            window->SetWindowPosVal = g.NextWindowData.PosVal;
            window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
            window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
        }
        else
        {
            SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
        }
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize)
    {
        window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
        window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
        if ((window->ChildFlags & ImGuiChildFlags_ResizeX) && (window->SetWindowSizeAllowFlags & ImGuiCond_FirstUseEver) == 0) // Axis-specific conditions for BeginChild()
            g.NextWindowData.SizeVal.x = window->SizeFull.x;
        if ((window->ChildFlags & ImGuiChildFlags_ResizeY) && (window->SetWindowSizeAllowFlags & ImGuiCond_FirstUseEver) == 0)
            g.NextWindowData.SizeVal.y = window->SizeFull.y;
        SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasScroll)
    {
        if (g.NextWindowData.ScrollVal.x >= 0.0f)
        {
            window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
            window->ScrollTargetCenterRatio.x = 0.0f;
        }
        if (g.NextWindowData.ScrollVal.y >= 0.0f)
        {
            window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
            window->ScrollTargetCenterRatio.y = 0.0f;
        }
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasContentSize)
        window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
    else if (first_begin_of_the_frame)
        window->ContentSizeExplicit = ImVec2(0.0f, 0.0f);
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasWindowClass)
        window->WindowClass = g.NextWindowData.WindowClass;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasCollapsed)
        SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasFocus)
        FocusWindow(window);
    if (window->Appearing)
        SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

    // [EXPERIMENTAL] Skip Refresh mode
    UpdateWindowSkipRefresh(window);

    // Nested root windows (typically tooltips) override disabled state
    if (window_stack_data.DisabledOverrideReenable && window->RootWindow == window)
        BeginDisabledOverrideReenable();

    // We intentionally set g.CurrentWindow to NULL to prevent usage until when the viewport is set, then will call SetCurrentWindow()
    g.CurrentWindow = NULL;

    // When reusing window again multiple times a frame, just append content (don't need to setup again)
    if (first_begin_of_the_frame && !window->SkipRefresh)
    {
        // Initialize
        const bool window_is_child_tooltip = (flags & ImGuiWindowFlags_ChildWindow) && (flags & ImGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)
        const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
        window->Active = true;
        window->HasCloseButton = (p_open != NULL);
        window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
        window->IDStack.resize(1);
        window->DrawList->_ResetForNewFrame();
        window->DC.CurrentTableIdx = -1;
        if (flags & ImGuiWindowFlags_DockNodeHost)
        {
            window->DrawList->ChannelsSplit(2);
            window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG); // Render decorations on channel 1 as we will render the backgrounds manually later
        }

        // Restore buffer capacity when woken from a compacted state, to avoid
        if (window->MemoryCompacted)
            GcAwakeTransientWindowBuffers(window);

        // Update stored window name when it changes (which can _only_ happen with the "###" operator, so the ID would stay unchanged).
        // The title bar always display the 'name' parameter, so we only update the string storage if it needs to be visible to the end-user elsewhere.
        bool window_title_visible_elsewhere = false;
        if ((window->Viewport && window->Viewport->Window == window) || (window->DockIsActive))
            window_title_visible_elsewhere = true;
        else if (g.NavWindowingListWindow != NULL && g.NavWindowingListWindow->WasActive && (flags & ImGuiWindowFlags_NoNavFocus) == 0)   // Window titles visible when using Ctrl+Tab
            window_title_visible_elsewhere = true;
        else if (flags & ImGuiWindowFlags_ChildMenu)
            window_title_visible_elsewhere = true;
        if ((window_title_visible_elsewhere || window_just_activated_by_user) && !window_just_created && strcmp(name, window->Name) != 0)
        {
            size_t buf_len = (size_t)window->NameBufLen;
            window->Name = ImStrdupcpy(window->Name, &buf_len, name);
            window->NameBufLen = (int)buf_len;
        }

        // UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

        // Update contents size from last frame for auto-fitting (or use explicit size)
        CalcWindowContentSizes(window, &window->ContentSize, &window->ContentSizeIdeal);

        // FIXME: These flags are decremented before they are used. This means that in order to have these fields produce their intended behaviors
        // for one frame we must set them to at least 2, which is counter-intuitive. HiddenFramesCannotSkipItems is a more complicated case because
        // it has a single usage before this code block and may be set below before it is finally checked.
        if (window->HiddenFramesCanSkipItems > 0)
            window->HiddenFramesCanSkipItems--;
        if (window->HiddenFramesCannotSkipItems > 0)
            window->HiddenFramesCannotSkipItems--;
        if (window->HiddenFramesForRenderOnly > 0)
            window->HiddenFramesForRenderOnly--;

        // Hide new windows for one frame until they calculate their size
        if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
            window->HiddenFramesCannotSkipItems = 1;

        // Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
        // We reset Size/ContentSize for reappearing popups/tooltips early in this function, so further code won't be tempted to use the old size.
        if (window_just_activated_by_user && (flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0)
        {
            window->HiddenFramesCannotSkipItems = 1;
            if (flags & ImGuiWindowFlags_AlwaysAutoResize)
            {
                if (!window_size_x_set_by_api)
                    window->Size.x = window->SizeFull.x = 0.f;
                if (!window_size_y_set_by_api)
                    window->Size.y = window->SizeFull.y = 0.f;
                window->ContentSize = window->ContentSizeIdeal = ImVec2(0.f, 0.f);
            }
        }

        // SELECT VIEWPORT
        // We need to do this before using any style/font sizes, as viewport with a different DPI may affect font sizes.

        WindowSelectViewport(window);
        SetCurrentViewport(window, window->Viewport);
        SetCurrentWindow(window);
        flags = window->Flags;

        // LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't cause inconsistencies)
        // We read Style data after the call to UpdateSelectWindowViewport() which might be swapping the style.

        if (!window->DockIsActive && (flags & ImGuiWindowFlags_ChildWindow))
            window->WindowBorderSize = style.ChildBorderSize;
        else
            window->WindowBorderSize = ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
        window->WindowPadding = style.WindowPadding;
        if (!window->DockIsActive && (flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !(window->ChildFlags & ImGuiChildFlags_AlwaysUseWindowPadding) && window->WindowBorderSize == 0.0f)
            window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

        // Lock menu offset so size calculation can use it as menu-bar windows need a minimum size.
        window->DC.MenuBarOffset.x = ImMax(ImMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
        window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;
        window->TitleBarHeight = (flags & ImGuiWindowFlags_NoTitleBar) ? 0.0f : g.FontSize + g.Style.FramePadding.y * 2.0f;
        window->MenuBarHeight = (flags & ImGuiWindowFlags_MenuBar) ? window->DC.MenuBarOffset.y + g.FontSize + g.Style.FramePadding.y * 2.0f : 0.0f;
        window->FontRefSize = g.FontSize; // Lock this to discourage calling window->CalcFontSize() outside of current window.

        // Depending on condition we use previous or current window size to compare against contents size to decide if a scrollbar should be visible.
        // Those flags will be altered further down in the function depending on more conditions.
        bool use_current_size_for_scrollbar_x = window_just_created;
        bool use_current_size_for_scrollbar_y = window_just_created;
        if (window_size_x_set_by_api && window->ContentSizeExplicit.x != 0.0f)
            use_current_size_for_scrollbar_x = true;
        if (window_size_y_set_by_api && window->ContentSizeExplicit.y != 0.0f) // #7252
            use_current_size_for_scrollbar_y = true;

        // Collapse window by double-clicking on title bar
        // At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse) && !window->DockIsActive)
        {
            // We don't use a regular button+id to test for double-click on title bar (mostly due to legacy reason, could be fixed),
            // so verify that we don't have items over the title bar.
            ImRect title_bar_rect = window->TitleBarRect();
            if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 && g.ActiveId == 0 && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max))
                if (g.IO.MouseClickedCount[0] == 2 && GetKeyOwner(ImGuiKey_MouseLeft) == ImGuiKeyOwner_NoOwner)
                    window->WantCollapseToggle = true;
            if (window->WantCollapseToggle)
            {
                window->Collapsed = !window->Collapsed;
                if (!window->Collapsed)
                    use_current_size_for_scrollbar_y = true;
                MarkIniSettingsDirty(window);
            }
        }
        else
        {
            window->Collapsed = false;
        }
        window->WantCollapseToggle = false;

        // SIZE

        // Outer Decoration Sizes
        // (we need to clear ScrollbarSize immediately as CalcWindowAutoFitSize() needs it and can be called from other locations).
        const ImVec2 scrollbar_sizes_from_last_frame = window->ScrollbarSizes;
        window->DecoOuterSizeX1 = 0.0f;
        window->DecoOuterSizeX2 = 0.0f;
        window->DecoOuterSizeY1 = window->TitleBarHeight + window->MenuBarHeight;
        window->DecoOuterSizeY2 = 0.0f;
        window->ScrollbarSizes = ImVec2(0.0f, 0.0f);

        // Calculate auto-fit size, handle automatic resize
        // - Using SetNextWindowSize() overrides ImGuiWindowFlags_AlwaysAutoResize, so it can be used on tooltips/popups, etc.
        // - We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
        // - Auto-fit may only grow window during the first few frames.
        {
            const bool size_auto_fit_x_always = !window_size_x_set_by_api && (flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed;
            const bool size_auto_fit_y_always = !window_size_y_set_by_api && (flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed;
            const bool size_auto_fit_x_current = !window_size_x_set_by_api && (window->AutoFitFramesX > 0);
            const bool size_auto_fit_y_current = !window_size_y_set_by_api && (window->AutoFitFramesY > 0);
            int size_auto_fit_mask = 0;
            if (size_auto_fit_x_always || size_auto_fit_x_current)
                size_auto_fit_mask |= (1 << ImGuiAxis_X);
            if (size_auto_fit_y_always || size_auto_fit_y_current)
                size_auto_fit_mask |= (1 << ImGuiAxis_Y);
            const ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, size_auto_fit_mask);

            const ImVec2 old_size = window->SizeFull;
            if (size_auto_fit_x_always || size_auto_fit_x_current)
            {
                if (size_auto_fit_x_always)
                    window->SizeFull.x = size_auto_fit.x;
                else
                    window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
                use_current_size_for_scrollbar_x = true;
            }
            if (size_auto_fit_y_always || size_auto_fit_y_current)
            {
                if (size_auto_fit_y_always)
                    window->SizeFull.y = size_auto_fit.y;
                else
                    window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
                use_current_size_for_scrollbar_y = true;
            }
            if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
                MarkIniSettingsDirty(window);
        }

        // Apply minimum/maximum window size constraints and final size
        window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
        window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

        // POSITION

        // Popup latch its initial position, will position itself when it appears next frame
        if (window_just_activated_by_user)
        {
            window->AutoPosLastDirection = ImGuiDir_None;
            if ((flags & ImGuiWindowFlags_Popup) != 0 && !(flags & ImGuiWindowFlags_Modal) && !window_pos_set_by_api) // FIXME: BeginPopup() could use SetNextWindowPos()
                window->Pos = g.BeginPopupStack.back().OpenPopupPos;
        }

        // Position child window
        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            IM_ASSERT(parent_window && parent_window->Active);
            window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
            parent_window->DC.ChildWindows.push_back(window);
            if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
                window->Pos = parent_window->DC.CursorPos;
        }

        const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
        if (window_pos_with_pivot)
            SetWindowPos(window, window->SetWindowPosVal - window->Size * window->SetWindowPosPivot, 0); // Position given a pivot (e.g. for centering)
        else if ((flags & ImGuiWindowFlags_ChildMenu) != 0)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
            window->Pos = FindBestWindowPosForPopup(window);

        // Late create viewport if we don't fit within our current host viewport.
        if (window->ViewportAllowPlatformMonitorExtend >= 0 && !window->ViewportOwned && !(window->Viewport->Flags & ImGuiViewportFlags_IsMinimized))
            if (!window->Viewport->GetMainRect().Contains(window->Rect()))
            {
                // This is based on the assumption that the DPI will be known ahead (same as the DPI of the selection done in UpdateSelectWindowViewport)
                //ImGuiViewport* old_viewport = window->Viewport;
                window->Viewport = AddUpdateViewport(window, window->ID, window->Pos, window->Size, ImGuiViewportFlags_NoFocusOnAppearing);

                // FIXME-DPI
                //IM_ASSERT(old_viewport->DpiScale == window->Viewport->DpiScale); // FIXME-DPI: Something went wrong
                SetCurrentViewport(window, window->Viewport);
                SetCurrentWindow(window);
            }

        if (window->ViewportOwned)
            WindowSyncOwnedViewport(window, parent_window_in_stack);

        // Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
        // When clamping to stay visible, we will enforce that window->Pos stays inside of visibility_rect.
        ImRect viewport_rect(window->Viewport->GetMainRect());
        ImRect viewport_work_rect(window->Viewport->GetWorkRect());
        ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
        ImRect visibility_rect(viewport_work_rect.Min + visibility_padding, viewport_work_rect.Max - visibility_padding);

        // Clamp position/size so window stays visible within its viewport or monitor
        // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
        // FIXME: Similar to code in GetWindowAllowedExtentRect()
        if (!window_pos_set_by_api && !(flags & ImGuiWindowFlags_ChildWindow))
        {
            if (!window->ViewportOwned && viewport_rect.GetWidth() > 0 && viewport_rect.GetHeight() > 0.0f)
            {
                ClampWindowPos(window, visibility_rect);
            }
            else if (window->ViewportOwned && g.PlatformIO.Monitors.Size > 0)
            {
                if (g.MovingWindow != NULL && window->RootWindowDockTree == g.MovingWindow->RootWindowDockTree)
                {
                    // While moving windows we allow them to straddle monitors (#7299, #3071)
                    visibility_rect = g.PlatformMonitorsFullWorkRect;
                }
                else
                {
                    // When not moving ensure visible in its monitor
                    // Lost windows (e.g. a monitor disconnected) will naturally moved to the fallback/dummy monitor aka the main viewport.
                    const ImGuiPlatformMonitor* monitor = GetViewportPlatformMonitor(window->Viewport);
                    visibility_rect = ImRect(monitor->WorkPos, monitor->WorkPos + monitor->WorkSize);
                }
                visibility_rect.Expand(-visibility_padding);
                ClampWindowPos(window, visibility_rect);
            }
        }
        window->Pos = ImTrunc(window->Pos);

        // Lock window rounding for the frame (so that altering them doesn't cause inconsistencies)
        // Large values tend to lead to variety of artifacts and are not recommended.
        if ((flags & ImGuiWindowFlags_ChildWindow) && !window->DockIsActive)
            window->WindowRounding = style.ChildRounding;
        else if (window->RootWindowDockTree->ViewportOwned)
            window->WindowRounding = 0.0f;
        else
            window->WindowRounding = ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupRounding : style.WindowRounding;

        // For windows with title bar or menu bar, we clamp to FrameHeight(FontSize + FramePadding.y * 2.0f) to completely hide artifacts.
        //if ((window->Flags & ImGuiWindowFlags_MenuBar) || !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        //    window->WindowRounding = ImMin(window->WindowRounding, g.FontSize + style.FramePadding.y * 2.0f);

        // Apply window focus (new and reactivated windows are moved to front)
        bool want_focus = false;
        if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
        {
            if (flags & ImGuiWindowFlags_Popup)
                want_focus = true;
            else if ((window->DockIsActive || (flags & ImGuiWindowFlags_ChildWindow) == 0) && !(flags & ImGuiWindowFlags_Tooltip))
                want_focus = true;
        }

        // [Test Engine] Register whole window in the item system (before submitting further decorations)
#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (g.TestEngineHookItems)
        {
            IM_ASSERT(window->IDStack.Size == 1);
            window->IDStack.Size = 0; // As window->IDStack[0] == window->ID here, make sure TestEngine doesn't erroneously see window as parent of itself.
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            IMGUI_TEST_ENGINE_ITEM_ADD(window->ID, window->Rect(), NULL);
            IMGUI_TEST_ENGINE_ITEM_INFO(window->ID, window->Name, (g.HoveredWindow == window) ? ImGuiItemStatusFlags_HoveredRect : 0);
            window->IDStack.Size = 1;
            window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

        }
#endif

        // Decide if we are going to handle borders and resize grips
        // 'window->SkipItems' is not updated yet so for child windows we rely on ParentWindow to avoid submitting decorations. (#8815)
        // Whenever we add support for full decorated child windows we will likely make this logic more general.
        bool handle_borders_and_resize_grips = (window->DockNodeAsHost || !window->DockIsActive);
        if ((flags & ImGuiWindowFlags_ChildWindow) && window->ParentWindow->SkipItems)
            handle_borders_and_resize_grips = false;

        // Handle manual resize: Resize Grips, Borders, Gamepad
        // Child windows can only be resized when they have the flags set. The resize grip allows resizing in both directions, so it should appear only if both flags are set.
        int border_hovered = -1, border_held = -1;
        ImU32 resize_grip_col[4] = {};
        int resize_grip_count;
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup))
            resize_grip_count = ((window->ChildFlags & ImGuiChildFlags_ResizeX) && (window->ChildFlags & ImGuiChildFlags_ResizeY)) ? 1 : 0;
        else
            resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.

        const float resize_grip_draw_size = IM_TRUNC(ImMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
        if (handle_borders_and_resize_grips && !window->Collapsed)
            if (int auto_fit_mask = UpdateWindowManualResize(window, &border_hovered, &border_held, resize_grip_count, &resize_grip_col[0], visibility_rect))
            {
                if (auto_fit_mask & (1 << ImGuiAxis_X))
                    use_current_size_for_scrollbar_x = true;
                if (auto_fit_mask & (1 << ImGuiAxis_Y))
                    use_current_size_for_scrollbar_y = true;
            }
        window->ResizeBorderHovered = (signed char)border_hovered;
        window->ResizeBorderHeld = (signed char)border_held;

        // Synchronize window --> viewport again and one last time (clamping and manual resize may have affected either)
        if (window->ViewportOwned)
        {
            if (!window->Viewport->PlatformRequestMove)
                window->Viewport->Pos = window->Pos;
            if (!window->Viewport->PlatformRequestResize)
                window->Viewport->Size = window->Size;
            window->Viewport->UpdateWorkRect();
            viewport_rect = window->Viewport->GetMainRect();
        }

        // Save last known viewport position within the window itself (so it can be saved in .ini file and restored)
        window->ViewportPos = window->Viewport->Pos;

        // SCROLLBAR VISIBILITY

        // Update scrollbar visibility (based on the Size that was effective during last frame or the auto-resized Size).
        if (!window->Collapsed)
        {
            // When reading the current size we need to read it after size constraints have been applied.
            // Intentionally use previous frame values for InnerRect and ScrollbarSizes.
            // And when we use window->DecorationUp here it doesn't have ScrollbarSizes.y applied yet.
            ImVec2 avail_size_from_current_frame = ImVec2(window->SizeFull.x, window->SizeFull.y - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
            ImVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + scrollbar_sizes_from_last_frame;
            ImVec2 needed_size_from_last_frame = window_just_created ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
            float size_for_scrollbars_x = use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
            float size_for_scrollbars_y = use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
            bool scrollbar_x_prev = window->ScrollbarX;
            //bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May want to use that in the ScrollbarX expression? How many pros vs cons?
            window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((needed_size_from_last_frame.y > size_for_scrollbars_y) && !(flags & ImGuiWindowFlags_NoScrollbar));
            window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((needed_size_from_last_frame.x > size_for_scrollbars_x - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));

            // Track when ScrollbarX visibility keeps toggling, which is a sign of a feedback loop, and stabilize by enforcing visibility (#3285, #8488)
            // (Feedback loops of this sort can manifest in various situations, but combining horizontal + vertical scrollbar + using a clipper with varying width items is one frequent cause.
            //  The better solution is to, either (1) enforce visibility by using ImGuiWindowFlags_AlwaysHorizontalScrollbar or (2) declare stable contents width with SetNextWindowContentSize(), if you can compute it)
            window->ScrollbarXStabilizeToggledHistory <<= 1;
            window->ScrollbarXStabilizeToggledHistory |= (scrollbar_x_prev != window->ScrollbarX) ? 0x01 : 0x00;
            const bool scrollbar_x_stabilize = (window->ScrollbarXStabilizeToggledHistory != 0) && ImCountSetBits(window->ScrollbarXStabilizeToggledHistory) >= 4; // 4 == half of bits in our U8 history.
            if (scrollbar_x_stabilize)
                window->ScrollbarX = true;
            //if (scrollbar_x_stabilize && !window->ScrollbarXStabilizeEnabled)
            //    IMGUI_DEBUG_LOG("[scroll] Stabilize ScrollbarX for Window '%s'\n", window->Name);
            window->ScrollbarXStabilizeEnabled = scrollbar_x_stabilize;

            if (window->ScrollbarX && !window->ScrollbarY)
                window->ScrollbarY = (needed_size_from_last_frame.y > size_for_scrollbars_y - style.ScrollbarSize) && !(flags & ImGuiWindowFlags_NoScrollbar);
            window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);

            // Amend the partially filled window->DecorationXXX values.
            window->DecoOuterSizeX2 += window->ScrollbarSizes.x;
            window->DecoOuterSizeY2 += window->ScrollbarSizes.y;
        }

        // UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
        // Update various regions. Variables they depend on should be set above in this function.
        // We set this up after processing the resize grip so that our rectangles doesn't lag by a frame.

        // Outer rectangle
        // Not affected by window border size. Used by:
        // - FindHoveredWindow() (w/ extra padding when border resize is enabled)
        // - Begin() initial clipping rect for drawing window background and borders.
        // - Begin() clipping whole child
        const ImRect host_rect = ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip) ? parent_window->ClipRect : viewport_rect;
        const ImRect outer_rect = window->Rect();
        const ImRect title_bar_rect = window->TitleBarRect();
        window->OuterRectClipped = outer_rect;
        if (window->DockIsActive)
            window->OuterRectClipped.Min.y += window->TitleBarHeight;
        window->OuterRectClipped.ClipWith(host_rect);

        // Inner rectangle
        // Not affected by window border size. Used by:
        // - InnerClipRect
        // - ScrollToRectEx()
        // - NavUpdatePageUpPageDown()
        // - Scrollbar()
        window->InnerRect.Min.x = window->Pos.x + window->DecoOuterSizeX1;
        window->InnerRect.Min.y = window->Pos.y + window->DecoOuterSizeY1;
        window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->DecoOuterSizeX2;
        window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->DecoOuterSizeY2;

        // Inner clipping rectangle.
        // - Extend a outside of normal work region up to borders.
        // - This is to allow e.g. Selectable or CollapsingHeader or some separators to cover that space.
        // - It also makes clipped items be more noticeable.
        // - And is consistent on both axis (prior to 2024/05/03 ClipRect used WindowPadding.x * 0.5f on left and right edge), see #3312
        // - Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
        // Note that if our window is collapsed we will end up with an inverted (~null) clipping rectangle which is the correct behavior.
        // Affected by window/frame border size. Used by:
        // - Begin() initial clip rect
        float top_border_size = (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);

        // Try to match the fact that our border is drawn centered over the window rectangle, rather than inner.
        // This is why we do a *0.5f here. We don't currently even technically support large values for WindowBorderSize,
        // see e.g #7887 #7888, but may do after we move the window border to become an inner border (and then we can remove the 0.5f here).
        window->InnerClipRect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x + window->WindowBorderSize * 0.5f);
        window->InnerClipRect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y + top_border_size * 0.5f);
        window->InnerClipRect.Max.x = ImFloor(window->InnerRect.Max.x - window->WindowBorderSize * 0.5f);
        window->InnerClipRect.Max.y = ImFloor(window->InnerRect.Max.y - window->WindowBorderSize * 0.5f);
        window->InnerClipRect.ClipWithFull(host_rect);

        // SCROLLING

        // Lock down maximum scrolling
        // The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is intentionally using InnerRect from previous rect in order to accommodate
        // for right/bottom aligned items without creating a scrollbar.
        window->ScrollMax.x = ImMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
        window->ScrollMax.y = ImMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

        // Apply scrolling
        window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
        window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
        window->DecoInnerSizeX1 = window->DecoInnerSizeY1 = 0.0f;

        // DRAWING

        // Setup draw list and outer clipping rectangle
        IM_ASSERT(window->DrawList->CmdBuffer.Size == 1 && window->DrawList->CmdBuffer[0].ElemCount == 0);
        window->DrawList->PushTexture(g.Font->OwnerAtlas->TexRef);
        PushClipRect(host_rect.Min, host_rect.Max, false);

        // Child windows can render their decoration (bg color, border, scrollbars, etc.) within their parent to save a draw call (since 1.71)
        // When using overlapping child windows, this will break the assumption that child z-order is mapped to submission order.
        // FIXME: User code may rely on explicit sorting of overlapping child window and would need to disable this somehow. Please get in contact if you are affected (github #4493)
        const bool is_undocked_or_docked_visible = !window->DockIsActive || window->DockTabIsVisible;
        if (is_undocked_or_docked_visible)
        {
            bool render_decorations_in_parent = false;
            if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
            {
                // - We test overlap with the previous child window only (testing all would end up being O(log N) not a good investment here)
                // - We disable this when the parent window has zero vertices, which is a common pattern leading to laying out multiple overlapping childs
                ImGuiWindow* previous_child = parent_window->DC.ChildWindows.Size >= 2 ? parent_window->DC.ChildWindows[parent_window->DC.ChildWindows.Size - 2] : NULL;
                bool previous_child_overlapping = previous_child ? previous_child->Rect().Overlaps(window->Rect()) : false;
                bool parent_is_empty = (parent_window->DrawList->VtxBuffer.Size == 0);
                if (window->DrawList->CmdBuffer.back().ElemCount == 0 && !parent_is_empty && !previous_child_overlapping)
                    render_decorations_in_parent = true;
            }
            if (render_decorations_in_parent)
                window->DrawList = parent_window->DrawList;

            // Handle title bar, scrollbar, resize grips and resize borders
            const ImGuiWindow* window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
            const bool title_bar_is_highlight = want_focus || (window_to_highlight && (window->RootWindowForTitleBarHighlight == window_to_highlight->RootWindowForTitleBarHighlight || (window->DockNode && window->DockNode == window_to_highlight->DockNode)));
            RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight, handle_borders_and_resize_grips, resize_grip_count, resize_grip_col, resize_grip_draw_size);

            if (render_decorations_in_parent)
                window->DrawList = &window->DrawListInst;
        }

        // UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

        // Work rectangle.
        // Affected by window padding and border size. Used by:
        // - Columns() for right-most edge
        // - TreeNode(), CollapsingHeader() for right-most edge
        // - BeginTabBar() for right-most edge
        const bool allow_scrollbar_x = !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar);
        const bool allow_scrollbar_y = !(flags & ImGuiWindowFlags_NoScrollbar);
        const float work_rect_size_x = (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : ImMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f, window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
        const float work_rect_size_y = (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : ImMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f, window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));
        window->WorkRect.Min.x = ImTrunc(window->InnerRect.Min.x - window->Scroll.x + ImMax(window->WindowPadding.x, window->WindowBorderSize));
        window->WorkRect.Min.y = ImTrunc(window->InnerRect.Min.y - window->Scroll.y + ImMax(window->WindowPadding.y, window->WindowBorderSize));
        window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
        window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
        window->ParentWorkRect = window->WorkRect;

        // [LEGACY] Content Region
        // FIXME-OBSOLETE: window->ContentRegionRect.Max is currently very misleading / partly faulty, but some BeginChild() patterns relies on it.
        // Unless explicit content size is specified by user, this currently represent the region leading to no scrolling.
        // Used by:
        // - Mouse wheel scrolling + many other things
        window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x + window->DecoOuterSizeX1;
        window->ContentRegionRect.Min.y = window->Pos.y - window->Scroll.y + window->WindowPadding.y + window->DecoOuterSizeY1;
        window->ContentRegionRect.Max.x = window->ContentRegionRect.Min.x + (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : (window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
        window->ContentRegionRect.Max.y = window->ContentRegionRect.Min.y + (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : (window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));

        // Setup drawing context
        // (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient data only. Nowadays difference between window-> and window->DC-> is dubious.)
        window->DC.Indent.x = window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
        window->DC.GroupOffset.x = 0.0f;
        window->DC.ColumnsOffset.x = 0.0f;

        // Record the loss of precision of CursorStartPos which can happen due to really large scrolling amount.
        // This is used by clipper to compensate and fix the most common use case of large scroll area. Easy and cheap, next best thing compared to switching everything to double or ImU64.
        double start_pos_highp_x = (double)window->Pos.x + window->WindowPadding.x - (double)window->Scroll.x + window->DecoOuterSizeX1 + window->DC.ColumnsOffset.x;
        double start_pos_highp_y = (double)window->Pos.y + window->WindowPadding.y - (double)window->Scroll.y + window->DecoOuterSizeY1;
        window->DC.CursorStartPos  = ImVec2((float)start_pos_highp_x, (float)start_pos_highp_y);
        window->DC.CursorStartPosLossyness = ImVec2((float)(start_pos_highp_x - window->DC.CursorStartPos.x), (float)(start_pos_highp_y - window->DC.CursorStartPos.y));
        window->DC.CursorPos = window->DC.CursorStartPos;
        window->DC.CursorPosPrevLine = window->DC.CursorPos;
        window->DC.CursorMaxPos = window->DC.CursorStartPos;
        window->DC.IdealMaxPos = window->DC.CursorStartPos;
        window->DC.CurrLineSize = window->DC.PrevLineSize = ImVec2(0.0f, 0.0f);
        window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
        window->DC.IsSameLine = window->DC.IsSetPos = false;

        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
        window->DC.NavLayersActiveMaskNext = 0x00;
        window->DC.NavIsScrollPushableX = true;
        window->DC.NavHideHighlightOneFrame = false;
        window->DC.NavWindowHasScrollY = (window->ScrollMax.y > 0.0f);

        window->DC.MenuBarAppending = false;
        window->DC.MenuColumns.Update(style.ItemSpacing.x, window_just_activated_by_user);
        window->DC.TreeDepth = 0;
        window->DC.TreeHasStackDataDepthMask = window->DC.TreeRecordsClippedNodesY2Mask = 0x00;
        window->DC.ChildWindows.resize(0);
        window->DC.StateStorage = &window->StateStorage;
        window->DC.CurrentColumns = NULL;
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;

        // Default item width. Make it proportional to window size if window can be manually resized.
        // (we cannot use AutoFitFramesX/AutoFitFramesY which is a temporary state)
        bool is_resizable_width;
        if (flags & ImGuiWindowFlags_ChildWindow)
            is_resizable_width = (window->Size.x > 0.0f) && !(window->ChildFlags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysAutoResize));
        else
            is_resizable_width = (window->Size.x > 0.0f) && !(flags & ImGuiWindowFlags_AlwaysAutoResize);
        if (is_resizable_width)
            window->DC.ItemWidthDefault = ImTrunc(window->Size.x * 0.65f);
        else
            window->DC.ItemWidthDefault = ImTrunc(g.FontSize * 16.0f);
        window->DC.ItemWidth = window->DC.ItemWidthDefault;
        window->DC.ItemWidthStack.resize(0);
        window->DC.TextWrapPos = -1.0f; // Disabled
        window->DC.TextWrapPosStack.resize(0);
        if (flags & ImGuiWindowFlags_Modal)
            window->DC.ModalDimBgColor = ColorConvertFloat4ToU32(GetStyleColorVec4(ImGuiCol_ModalWindowDimBg));

        if (window->AutoFitFramesX > 0)
            window->AutoFitFramesX--;
        if (window->AutoFitFramesY > 0)
            window->AutoFitFramesY--;

        // Clear SetNextWindowXXX data (can aim to move this higher in the function)
        g.NextWindowData.ClearFlags();

        // Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
        // We ImGuiFocusRequestFlags_UnlessBelowModal to:
        // - Avoid focusing a window that is created outside of a modal. This will prevent active modal from being closed.
        // - Position window behind the modal that is not a begin-parent of this window.
        if (want_focus)
            FocusWindow(window, ImGuiFocusRequestFlags_UnlessBelowModal);
        if (want_focus && window == g.NavWindow)
            NavInitWindow(window, false); // <-- this is in the way for us to be able to defer and sort reappearing FocusWindow() calls

        // Close requested by platform window (apply to all windows in this viewport)
        // FIXME: Investigate removing the 'window->Viewport != GetMainViewport()' test, which seems superfluous.
        if (p_open != NULL && window->Viewport->PlatformRequestClose && window->Viewport != GetMainViewport())
            if (window->DockNode == NULL || (window->DockNode->MergedFlags & ImGuiDockNodeFlags_DockSpace) == 0)
            {
                IMGUI_DEBUG_LOG_VIEWPORT("[viewport] Window '%s' closed by PlatformRequestClose\n", window->Name);
                *p_open = false;
                g.NavWindowingToggleLayer = false; // Assume user mapped PlatformRequestClose on Alt-F4 so we disable Alt for menu toggle. False positive not an issue. // FIXME-NAV: Try removing.
            }

        // Pressing Ctrl+C copy window content into the clipboard
        // [EXPERIMENTAL] Breaks on nested Begin/End pairs. We need to work that out and add better logging scope.
        // [EXPERIMENTAL] Text outputs has many issues.
        if (g.IO.ConfigWindowsCopyContentsWithCtrlC)
            if (g.NavWindow && g.NavWindow->RootWindow == window && g.ActiveId == 0 && Shortcut(ImGuiMod_Ctrl | ImGuiKey_C))
                LogToClipboard(0);

        // Title bar
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
            RenderWindowTitleBarContents(window, ImRect(title_bar_rect.Min.x + window->WindowBorderSize, title_bar_rect.Min.y, title_bar_rect.Max.x - window->WindowBorderSize, title_bar_rect.Max.y), name, p_open);
        else if (!(flags & ImGuiWindowFlags_NoTitleBar) && window->DockIsActive)
            LogText("%.*s\n", (int)(FindRenderedTextEnd(window->Name) - window->Name), window->Name);

        // Clear hit test shape every frame
        window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

        if (flags & ImGuiWindowFlags_Tooltip)
            g.TooltipPreviousWindow = window;

        if (g.IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            // Docking: Dragging a dockable window (or any of its child) turns it into a drag and drop source.
            // We need to do this _before_ we overwrite window->DC.LastItemId below because BeginDockableDragDropSource() also overwrites it.
            if (g.MovingWindow == window && (window->RootWindowDockTree->Flags & ImGuiWindowFlags_NoDocking) == 0)
                BeginDockableDragDropSource(window);

            // Docking: Any dockable window can act as a target. For dock node hosts we call BeginDockableDragDropTarget() in DockNodeUpdate() instead.
            if (g.DragDropActive && !(flags & ImGuiWindowFlags_NoDocking))
                if (g.MovingWindow == NULL || g.MovingWindow->RootWindowDockTree != window)
                    if ((window == window->RootWindowDockTree) && !(window->Flags & ImGuiWindowFlags_DockNodeHost))
                        BeginDockableDragDropTarget(window);
        }

        // Set default BgClickFlags
        // This is set at the end of this function, so UpdateWindowManualResize()/ClampWindowPos() may use last-frame value if overridden by user code.
        // FIXME: The general intent is that we will later expose config options to default to enable scrolling + select scrolling mouse button.
        window->BgClickFlags = (flags & ImGuiWindowFlags_ChildWindow) ? parent_window->BgClickFlags : (g.IO.ConfigWindowsMoveFromTitleBarOnly ? ImGuiWindowBgClickFlags_None : ImGuiWindowBgClickFlags_Move);

        // We fill last item data based on Title Bar/Tab, in order for IsItemHovered() and IsItemActive() to be usable after Begin().
        // This is useful to allow creating context menus on title bar only, etc.
        window->DC.WindowItemStatusFlags = ImGuiItemStatusFlags_None;
        window->DC.WindowItemStatusFlags |= IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? ImGuiItemStatusFlags_HoveredRect : 0;
        SetLastItemDataForWindow(window, title_bar_rect);

        // [DEBUG]
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
        if (g.DebugLocateId != 0 && (window->ID == g.DebugLocateId || window->MoveId == g.DebugLocateId))
            DebugLocateItemResolveWithLastItem();
#endif

        // [Test Engine] Register title bar / tab with MoveId.
#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
        {
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            IMGUI_TEST_ENGINE_ITEM_ADD(g.LastItemData.ID, g.LastItemData.Rect, &g.LastItemData);
            window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        }
#endif
    }
    else
    {
        // Skip refresh always mark active
        if (window->SkipRefresh)
            SetWindowActiveForSkipRefresh(window);

        // Append
        SetCurrentViewport(window, window->Viewport);
        SetCurrentWindow(window);
        g.NextWindowData.ClearFlags();
        SetLastItemDataForWindow(window, window->TitleBarRect());
    }

    if (!(flags & ImGuiWindowFlags_DockNodeHost) && !window->SkipRefresh)
        PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

    // Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when the default "Debug" window is unused)
    window->WriteAccessed = false;
    window->BeginCount++;

    // Update visibility
    if (first_begin_of_the_frame && !window->SkipRefresh)
    {
        // When we are about to select this tab (which will only be visible on the _next frame_), flag it with a non-zero HiddenFramesCannotSkipItems.
        // This will have the important effect of actually returning true in Begin() and not setting SkipItems, allowing an earlier submission of the window contents.
        // This is analogous to regular windows being hidden from one frame.
        // It is especially important as e.g. nested TabBars would otherwise generate flicker in the form of one empty frame, or focus requests won't be processed.
        if (window->DockIsActive && !window->DockTabIsVisible)
        {
            if (window->LastFrameJustFocused == g.FrameCount)
                window->HiddenFramesCannotSkipItems = 1;
            else
                window->HiddenFramesCanSkipItems = 1;
        }

        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_ChildMenu))
        {
            // Child window can be out of sight and have "negative" clip windows.
            // Mark them as collapsed so commands are skipped earlier (we can't manually collapse them because they have no title bar).
            IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0 || window->DockIsActive);
            const bool nav_request = (window->ChildFlags & ImGuiChildFlags_NavFlattened) && (g.NavAnyRequest && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
            if (!g.LogEnabled && !nav_request)
                if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x || window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
                {
                    if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
                        window->HiddenFramesCannotSkipItems = 1;
                    else
                        window->HiddenFramesCanSkipItems = 1;
                }

            // Hide along with parent or if parent is collapsed
            if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
                window->HiddenFramesCanSkipItems = 1;
            if (parent_window && parent_window->HiddenFramesCannotSkipItems > 0)
                window->HiddenFramesCannotSkipItems = 1;
        }

        // Don't render if style alpha is 0.0 at the time of Begin(). This is arbitrary and inconsistent but has been there for a long while (may remove at some point)
        if (style.Alpha <= 0.0f)
            window->HiddenFramesCanSkipItems = 1;

        // Update the Hidden flag
        bool hidden_regular = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0);
        window->Hidden = hidden_regular || (window->HiddenFramesForRenderOnly > 0);

        // Disable inputs for requested number of frames
        if (window->DisableInputsFrames > 0)
        {
            window->DisableInputsFrames--;
            window->Flags |= ImGuiWindowFlags_NoInputs;
        }

        // Update the SkipItems flag, used to early out of all items functions (no layout required)
        bool skip_items = false;
        if (window->Collapsed || !window->Active || hidden_regular)
            if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
                skip_items = true;
        window->SkipItems = skip_items;

        // Restore NavLayersActiveMaskNext to previous value when not visible, so a CTRL+Tab back can use a safe value.
        if (window->SkipItems)
            window->DC.NavLayersActiveMaskNext = window->DC.NavLayersActiveMask;

        // Sanity check: there are two spots which can set Appearing = true
        // - when 'window_just_activated_by_user' is set -> HiddenFramesCannotSkipItems is set -> SkipItems always false
        // - in BeginDocked() path when DockNodeIsVisible == DockTabIsVisible == true -> hidden _should_ be all zero // FIXME: Not formally proven, hence the assert.
        if (window->SkipItems && !window->Appearing)
            IM_ASSERT(window->Appearing == false); // Please report on GitHub if this triggers: https://github.com/ocornut/imgui/issues/4177
    }
    else if (first_begin_of_the_frame)
    {
        // Skip refresh mode
        window->SkipItems = true;
    }

    // [DEBUG] io.ConfigDebugBeginReturnValue override return value to test Begin/End and BeginChild/EndChild behaviors.
    // (The implicit fallback window is NOT automatically ended allowing it to always be able to receive commands without crashing)
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (!window->IsFallbackWindow)
        if ((g.IO.ConfigDebugBeginReturnValueOnce && window_just_created) || (g.IO.ConfigDebugBeginReturnValueLoop && g.DebugBeginReturnValueCullDepth == g.CurrentWindowStack.Size))
        {
            if (window->AutoFitFramesX > 0) { window->AutoFitFramesX++; }
            if (window->AutoFitFramesY > 0) { window->AutoFitFramesY++; }
            return false;
        }
#endif

    return !window->SkipItems;
}

void ImGui::End()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Error checking: verify that user hasn't called End() too many times!
    if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow)
    {
        IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size > 1, "Calling End() too many times!");
        return;
    }
    ImGuiWindowStackData& window_stack_data = g.CurrentWindowStack.back();

    // Error checking: verify that user doesn't directly call End() on a child window.
    if (window->Flags & ImGuiWindowFlags_Popup)
        IM_ASSERT_USER_ERROR(g.WithinEndPopupID == window->ID, "Must call EndPopup() and not End()!");
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_DockNodeHost) && !window->DockIsActive)
        IM_ASSERT_USER_ERROR(g.WithinEndChildID == window->ID, "Must call EndChild() and not End()!");

    // Close anything that is open
    if (window->DC.CurrentColumns)
        EndColumns();
    if (!(window->Flags & ImGuiWindowFlags_DockNodeHost) && !window->SkipRefresh)   // Pop inner window clip rectangle
        PopClipRect();
    PopFocusScope();
    if (window_stack_data.DisabledOverrideReenable && window->RootWindow == window)
        EndDisabledOverrideReenable();

    if (window->SkipRefresh)
    {
        IM_ASSERT(window->DrawList == NULL);
        window->DrawList = &window->DrawListInst;
    }

    // Stop logging
    if (g.LogWindow == window) // FIXME: add more options for scope of logging
        LogFinish();

    if (window->DC.IsSetPos)
        ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

    // Docking: report contents sizes to parent to allow for auto-resize
    if (window->DockNode && window->DockTabIsVisible)
        if (ImGuiWindow* host_window = window->DockNode->HostWindow)         // FIXME-DOCK
            host_window->DC.CursorMaxPos = window->DC.CursorMaxPos + window->WindowPadding - host_window->WindowPadding;

    // Pop from window stack
    g.LastItemData = window_stack_data.ParentLastItemDataBackup;
    if (window->Flags & ImGuiWindowFlags_ChildMenu)
        g.BeginMenuDepth--;
    if (window->Flags & ImGuiWindowFlags_Popup)
        g.BeginPopupStack.pop_back();

    // Error handling, state recovery
    if (g.IO.ConfigErrorRecovery)
        ErrorRecoveryTryToRecoverWindowState(&window_stack_data.StackSizesInBegin);

    g.CurrentWindowStack.pop_back();
    SetCurrentWindow(g.CurrentWindowStack.Size == 0 ? NULL : g.CurrentWindowStack.back().Window);
    if (g.CurrentWindow)
        SetCurrentViewport(g.CurrentWindow, g.CurrentWindow->Viewport);
}

static ImGuiWindow* GetCombinedRootWindow(ImGuiWindow* window, bool popup_hierarchy, bool dock_hierarchy)
{
    ImGuiWindow* last_window = NULL;
    while (last_window != window)
    {
        last_window = window;
        window = window->RootWindow;
        if (popup_hierarchy)
            window = window->RootWindowPopupTree;
		if (dock_hierarchy)
			window = window->RootWindowDockTree;
	}
    return window;
}

bool ImGui::IsWindowChildOf(ImGuiWindow* window, ImGuiWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy)
{
    ImGuiWindow* window_root = GetCombinedRootWindow(window, popup_hierarchy, dock_hierarchy);
    if (window_root == potential_parent)
        return true;
    while (window != NULL)
    {
        if (window == potential_parent)
            return true;
        if (window == window_root) // end of chain
            return false;
        window = window->ParentWindow;
    }
    return false;
}

bool ImGui::IsWindowInBeginStack(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    for (int n = g.CurrentWindowStack.Size - 1; n >= 0; n--)
        if (g.CurrentWindowStack[n].Window == window)
            return true;
    return false;
}

bool ImGui::IsWindowWithinBeginStackOf(ImGuiWindow* window, ImGuiWindow* potential_parent)
{
    if (window->RootWindow == potential_parent)
        return true;
    while (window != NULL)
    {
        if (window == potential_parent)
            return true;
        window = window->ParentWindowInBeginStack;
    }
    return false;
}

bool ImGui::IsWindowAbove(ImGuiWindow* potential_above, ImGuiWindow* potential_below)
{
    ImGuiContext& g = *GImGui;

    // It would be saner to ensure that display layer is always reflected in the g.Windows[] order, which would likely requires altering all manipulations of that array
    const int display_layer_delta = GetWindowDisplayLayer(potential_above) - GetWindowDisplayLayer(potential_below);
    if (display_layer_delta != 0)
        return display_layer_delta > 0;

    for (int i = g.Windows.Size - 1; i >= 0; i--)
    {
        ImGuiWindow* candidate_window = g.Windows[i];
        if (candidate_window == potential_above)
            return true;
        if (candidate_window == potential_below)
            return false;
    }
    return false;
}

// Is current window hovered and hoverable (e.g. not blocked by a popup/modal)? See ImGuiHoveredFlags_ for options.
// IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear ImGui or to your underlying app,
// you should not use this function! Use the 'io.WantCaptureMouse' boolean for that!
// Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?" for details.
bool ImGui::IsWindowHovered(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR((flags & ~ImGuiHoveredFlags_AllowedMaskForIsWindowHovered) == 0, "Invalid flags for IsWindowHovered()!");

    ImGuiWindow* ref_window = g.HoveredWindow;
    ImGuiWindow* cur_window = g.CurrentWindow;
    if (ref_window == NULL)
        return false;

    if ((flags & ImGuiHoveredFlags_AnyWindow) == 0)
    {
        IM_ASSERT(cur_window); // Not inside a Begin()/End()
        const bool popup_hierarchy = (flags & ImGuiHoveredFlags_NoPopupHierarchy) == 0;
        const bool dock_hierarchy = (flags & ImGuiHoveredFlags_DockHierarchy) != 0;
        if (flags & ImGuiHoveredFlags_RootWindow)
            cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

        bool result;
        if (flags & ImGuiHoveredFlags_ChildWindows)
            result = IsWindowChildOf(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
        else
            result = (ref_window == cur_window);
        if (!result)
            return false;
    }

    if (!IsWindowContentHoverable(ref_window, flags))
        return false;
    if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != ref_window->MoveId)
            return false;

    // When changing hovered window we requires a bit of stationary delay before activating hover timer.
    // FIXME: We don't support delay other than stationary one for now, other delay would need a way
    // to fulfill the possibility that multiple IsWindowHovered() with varying flag could return true
    // for different windows of the hierarchy. Possibly need a Hash(Current+Flags) ==> (Timer) cache.
    // We can implement this for _Stationary because the data is linked to HoveredWindow rather than CurrentWindow.
    if (flags & ImGuiHoveredFlags_ForTooltip)
        flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);
    if ((flags & ImGuiHoveredFlags_Stationary) != 0 && g.HoverWindowUnlockedStationaryId != ref_window->ID)
        return false;

    return true;
}

ImGuiID ImGui::GetWindowDockID()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentWindow->DockId;
}

bool ImGui::IsWindowDocked()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentWindow->DockIsActive;
}

float ImGui::GetWindowWidth()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.x;
}

float ImGui::GetWindowHeight()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.y;
}

ImVec2 ImGui::GetWindowPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    return window->Pos;
}

void ImGui::SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
    window->SetWindowPosVal = ImVec2(FLT_MAX, FLT_MAX);

    // Set
    const ImVec2 old_pos = window->Pos;
    window->Pos = ImTrunc(pos);
    ImVec2 offset = window->Pos - old_pos;
    if (offset.x == 0.0f && offset.y == 0.0f)
        return;
    MarkIniSettingsDirty(window);
    // FIXME: share code with TranslateWindow(), need to confirm whether the 3 rect modified by TranslateWindow() are desirable here.
    window->DC.CursorPos += offset;         // As we happen to move the window while it is being appended to (which is a bad idea - will smear) let's at least offset the cursor
    window->DC.CursorMaxPos += offset;      // And more importantly we need to offset CursorMaxPos/CursorStartPos this so ContentSize calculation doesn't get affected.
    window->DC.IdealMaxPos += offset;
    window->DC.CursorStartPos += offset;
}

void ImGui::SetWindowPos(const ImVec2& pos, ImGuiCond cond)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    SetWindowPos(window, pos, cond);
}

void ImGui::SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowPos(window, pos, cond);
}

ImVec2 ImGui::GetWindowSize()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Size;
}

void ImGui::SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    window->SetWindowSizeAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Enable auto-fit (not done in BeginChild() path unless appearing or combined with ImGuiChildFlags_AlwaysAutoResize)
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || window->Appearing || (window->ChildFlags & ImGuiChildFlags_AlwaysAutoResize) != 0)
        window->AutoFitFramesX = (size.x <= 0.0f) ? 2 : 0;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || window->Appearing || (window->ChildFlags & ImGuiChildFlags_AlwaysAutoResize) != 0)
        window->AutoFitFramesY = (size.y <= 0.0f) ? 2 : 0;

    // Set
    ImVec2 old_size = window->SizeFull;
    if (size.x <= 0.0f)
        window->AutoFitOnlyGrows = false;
    else
        window->SizeFull.x = IM_TRUNC(size.x);
    if (size.y <= 0.0f)
        window->AutoFitOnlyGrows = false;
    else
        window->SizeFull.y = IM_TRUNC(size.y);
    if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
        MarkIniSettingsDirty(window);
}

void ImGui::SetWindowSize(const ImVec2& size, ImGuiCond cond)
{
    SetWindowSize(GImGui->CurrentWindow, size, cond);
}

void ImGui::SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowSize(window, size, cond);
}

void ImGui::SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
        return;
    window->SetWindowCollapsedAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Queue applying in Begin()
    if (window->WantCollapseToggle)
        window->Collapsed ^= 1;
    window->WantCollapseToggle = (window->Collapsed != collapsed);
}

void ImGui::SetWindowHitTestHole(ImGuiWindow* window, const ImVec2& pos, const ImVec2& size)
{
    IM_ASSERT(window->HitTestHoleSize.x == 0);     // We don't support multiple holes/hit test filters
    window->HitTestHoleSize = ImVec2ih(size);
    window->HitTestHoleOffset = ImVec2ih(pos - window->Pos);
}

void ImGui::SetWindowHiddenAndSkipItemsForCurrentFrame(ImGuiWindow* window)
{
    window->Hidden = window->SkipItems = true;
    window->HiddenFramesCanSkipItems = 1;
}

void ImGui::SetWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    SetWindowCollapsed(GImGui->CurrentWindow, collapsed, cond);
}

bool ImGui::IsWindowCollapsed()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Collapsed;
}

bool ImGui::IsWindowAppearing()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Appearing;
}

void ImGui::SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowCollapsed(window, collapsed, cond);
}

void ImGui::SetNextWindowPos(const ImVec2& pos, ImGuiCond cond, const ImVec2& pivot)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasPos;
    g.NextWindowData.PosVal = pos;
    g.NextWindowData.PosPivotVal = pivot;
    g.NextWindowData.PosCond = cond ? cond : ImGuiCond_Always;
    g.NextWindowData.PosUndock = true;
}

void ImGui::SetNextWindowSize(const ImVec2& size, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasSize;
    g.NextWindowData.SizeVal = size;
    g.NextWindowData.SizeCond = cond ? cond : ImGuiCond_Always;
}

// For each axis:
// - Use 0.0f as min or FLT_MAX as max if you don't want limits, e.g. size_min = (500.0f, 0.0f), size_max = (FLT_MAX, FLT_MAX) sets a minimum width.
// - Use -1 for both min and max of same axis to preserve current size which itself is a constraint.
// - See "Demo->Examples->Constrained-resizing window" for examples.
void ImGui::SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback, void* custom_callback_user_data)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
    g.NextWindowData.SizeConstraintRect = ImRect(size_min, size_max);
    g.NextWindowData.SizeCallback = custom_callback;
    g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

// Content size = inner scrollable rectangle, padded with WindowPadding.
// SetNextWindowContentSize(ImVec2(100,100)) + ImGuiWindowFlags_AlwaysAutoResize will always allow submitting a 100x100 item.
void ImGui::SetNextWindowContentSize(const ImVec2& size)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasContentSize;
    g.NextWindowData.ContentSizeVal = ImTrunc(size);
}

void ImGui::SetNextWindowScroll(const ImVec2& scroll)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasScroll;
    g.NextWindowData.ScrollVal = scroll;
}

void ImGui::SetNextWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasCollapsed;
    g.NextWindowData.CollapsedVal = collapsed;
    g.NextWindowData.CollapsedCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowBgAlpha(float alpha)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasBgAlpha;
    g.NextWindowData.BgAlphaVal = alpha;
}

void ImGui::SetNextWindowViewport(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasViewport;
    g.NextWindowData.ViewportId = id;
}

void ImGui::SetNextWindowDockID(ImGuiID id, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasDock;
    g.NextWindowData.DockCond = cond ? cond : ImGuiCond_Always;
    g.NextWindowData.DockId = id;
}

void ImGui::SetNextWindowClass(const ImGuiWindowClass* window_class)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT((window_class->ViewportFlagsOverrideSet & window_class->ViewportFlagsOverrideClear) == 0); // Cannot set both set and clear for the same bit
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasWindowClass;
    g.NextWindowData.WindowClass = *window_class;
}

// This is experimental and meant to be a toy for exploring a future/wider range of features.
void ImGui::SetNextWindowRefreshPolicy(ImGuiWindowRefreshFlags flags)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasRefreshPolicy;
    g.NextWindowData.RefreshFlagsVal = flags;
}

ImDrawList* ImGui::GetWindowDrawList()
{
    ImGuiWindow* window = GetCurrentWindow();
    return window->DrawList;
}

float ImGui::GetWindowDpiScale()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentDpiScale;
}

ImGuiViewport* ImGui::GetWindowViewport()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentViewport != NULL && g.CurrentViewport == g.CurrentWindow->Viewport);
    return g.CurrentViewport;
}

//-----------------------------------------------------------------------------
// [SECTION] WINDOW FOCUS
//----------------------------------------------------------------------------
// - SetWindowFocus()
// - SetNextWindowFocus()
// - IsWindowFocused()
// - UpdateWindowInFocusOrderList() [Internal]
// - BringWindowToFocusFront() [Internal]
// - BringWindowToDisplayFront() [Internal]
// - BringWindowToDisplayBack() [Internal]
// - BringWindowToDisplayBehind() [Internal]
// - FindWindowDisplayIndex() [Internal]
// - FocusWindow() [Internal]
// - FocusTopMostWindowUnderOne() [Internal]
//-----------------------------------------------------------------------------

void ImGui::SetWindowFocus()
{
    FocusWindow(GImGui->CurrentWindow);
}

void ImGui::SetWindowFocus(const char* name)
{
    if (name)
    {
        if (ImGuiWindow* window = FindWindowByName(name))
            FocusWindow(window);
    }
    else
    {
        FocusWindow(NULL);
    }
}

void ImGui::SetNextWindowFocus()
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasFocus;
}

// Similar to IsWindowHovered()
bool ImGui::IsWindowFocused(ImGuiFocusedFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* ref_window = g.NavWindow;
    ImGuiWindow* cur_window = g.CurrentWindow;

    if (ref_window == NULL)
        return false;
    if (flags & ImGuiFocusedFlags_AnyWindow)
        return true;

    IM_ASSERT(cur_window); // Not inside a Begin()/End()
    const bool popup_hierarchy = (flags & ImGuiFocusedFlags_NoPopupHierarchy) == 0;
    const bool dock_hierarchy = (flags & ImGuiFocusedFlags_DockHierarchy) != 0;
    if (flags & ImGuiFocusedFlags_RootWindow)
        cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

    if (flags & ImGuiFocusedFlags_ChildWindows)
        return IsWindowChildOf(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
    else
        return ref_window == cur_window;
}

int ImGui::FindWindowFocusIndex(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(g);
    int order = window->FocusOrder;
    IM_ASSERT(window->RootWindow == window); // No child window (not testing _ChildWindow because of docking)
    IM_ASSERT(g.WindowsFocusOrder[order] == window);
    return order;
}

static void ImGui::UpdateWindowInFocusOrderList(ImGuiWindow* window, bool just_created, ImGuiWindowFlags new_flags)
{
    ImGuiContext& g = *GImGui;

    const bool new_is_explicit_child = (new_flags & ImGuiWindowFlags_ChildWindow) != 0 && ((new_flags & ImGuiWindowFlags_Popup) == 0 || (new_flags & ImGuiWindowFlags_ChildMenu) != 0);
    const bool child_flag_changed = new_is_explicit_child != window->IsExplicitChild;
    if ((just_created || child_flag_changed) && !new_is_explicit_child)
    {
        IM_ASSERT(!g.WindowsFocusOrder.contains(window));
        g.WindowsFocusOrder.push_back(window);
        window->FocusOrder = (short)(g.WindowsFocusOrder.Size - 1);
    }
    else if (!just_created && child_flag_changed && new_is_explicit_child)
    {
        IM_ASSERT(g.WindowsFocusOrder[window->FocusOrder] == window);
        for (int n = window->FocusOrder + 1; n < g.WindowsFocusOrder.Size; n++)
            g.WindowsFocusOrder[n]->FocusOrder--;
        g.WindowsFocusOrder.erase(g.WindowsFocusOrder.Data + window->FocusOrder);
        window->FocusOrder = -1;
    }
    window->IsExplicitChild = new_is_explicit_child;
}

void ImGui::BringWindowToFocusFront(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(window == window->RootWindow);

    const int cur_order = window->FocusOrder;
    IM_ASSERT(g.WindowsFocusOrder[cur_order] == window);
    if (g.WindowsFocusOrder.back() == window)
        return;

    const int new_order = g.WindowsFocusOrder.Size - 1;
    for (int n = cur_order; n < new_order; n++)
    {
        g.WindowsFocusOrder[n] = g.WindowsFocusOrder[n + 1];
        g.WindowsFocusOrder[n]->FocusOrder--;
        IM_ASSERT(g.WindowsFocusOrder[n]->FocusOrder == n);
    }
    g.WindowsFocusOrder[new_order] = window;
    window->FocusOrder = (short)new_order;
}

// Note technically focus related but rather adjacent and close to BringWindowToFocusFront()
// FIXME-FOCUS: Could opt-in/opt-out enable modal check like in FocusWindow().
void ImGui::BringWindowToDisplayFront(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* current_front_window = g.Windows.back();
    if (current_front_window == window || current_front_window->RootWindowDockTree == window) // Cheap early out (could be better)
        return;
    for (int i = g.Windows.Size - 2; i >= 0; i--) // We can ignore the top-most window
        if (g.Windows[i] == window)
        {
            memmove(&g.Windows[i], &g.Windows[i + 1], (size_t)(g.Windows.Size - i - 1) * sizeof(ImGuiWindow*));
            g.Windows[g.Windows.Size - 1] = window;
            break;
        }
}

void ImGui::BringWindowToDisplayBack(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (g.Windows[0] == window)
        return;
    for (int i = 0; i < g.Windows.Size; i++)
        if (g.Windows[i] == window)
        {
            memmove(&g.Windows[1], &g.Windows[0], (size_t)i * sizeof(ImGuiWindow*));
            g.Windows[0] = window;
            break;
        }
}

void ImGui::BringWindowToDisplayBehind(ImGuiWindow* window, ImGuiWindow* behind_window)
{
    IM_ASSERT(window != NULL && behind_window != NULL);
    ImGuiContext& g = *GImGui;
    window = window->RootWindow;
    behind_window = behind_window->RootWindow;
    int pos_wnd = FindWindowDisplayIndex(window);
    int pos_beh = FindWindowDisplayIndex(behind_window);
    if (pos_wnd < pos_beh)
    {
        size_t copy_bytes = (pos_beh - pos_wnd - 1) * sizeof(ImGuiWindow*);
        memmove(&g.Windows.Data[pos_wnd], &g.Windows.Data[pos_wnd + 1], copy_bytes);
        g.Windows[pos_beh - 1] = window;
    }
    else
    {
        size_t copy_bytes = (pos_wnd - pos_beh) * sizeof(ImGuiWindow*);
        memmove(&g.Windows.Data[pos_beh + 1], &g.Windows.Data[pos_beh], copy_bytes);
        g.Windows[pos_beh] = window;
    }
}

int ImGui::FindWindowDisplayIndex(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    return g.Windows.index_from_ptr(g.Windows.find(window));
}

// Moving window to front of display and set focus (which happens to be back of our sorted list)
void ImGui::FocusWindow(ImGuiWindow* window, ImGuiFocusRequestFlags flags)
{
    ImGuiContext& g = *GImGui;

    // Modal check?
    if ((flags & ImGuiFocusRequestFlags_UnlessBelowModal) && (g.NavWindow != window)) // Early out in common case.
        if (ImGuiWindow* blocking_modal = FindBlockingModal(window))
        {
            // This block would typically be reached in two situations:
            // - API call to FocusWindow() with a window under a modal and ImGuiFocusRequestFlags_UnlessBelowModal flag.
            // - User clicking on void or anything behind a modal while a modal is open (window == NULL)
            IMGUI_DEBUG_LOG_FOCUS("[focus] FocusWindow(\"%s\", UnlessBelowModal): prevented by \"%s\".\n", window ? window->Name : "<NULL>", blocking_modal->Name);
            if (window && window == window->RootWindow && (window->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus) == 0)
                BringWindowToDisplayBehind(window, blocking_modal); // Still bring right under modal. (FIXME: Could move in focus list too?)
            ClosePopupsOverWindow(GetTopMostPopupModal(), false); // Note how we need to use GetTopMostPopupModal() aad NOT blocking_modal, to handle nested modals
            return;
        }

    // Find last focused child (if any) and focus it instead.
    if ((flags & ImGuiFocusRequestFlags_RestoreFocusedChild) && window != NULL)
        window = NavRestoreLastChildNavWindow(window);

    // Apply focus
    if (g.NavWindow != window)
    {
        SetNavWindow(window);
        if (window && g.NavHighlightItemUnderNav)
            g.NavMousePosDirty = true;
        g.NavId = window ? window->NavLastIds[0] : 0; // Restore NavId
        g.NavLayer = ImGuiNavLayer_Main;
        SetNavFocusScope(window ? window->NavRootFocusScopeId : 0);
        g.NavIdIsAlive = false;
        g.NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;

        // Close popups if any
        ClosePopupsOverWindow(window, false);
    }

    // Move the root window to the top of the pile
    IM_ASSERT(window == NULL || window->RootWindowDockTree != NULL);
    ImGuiWindow* focus_front_window = window ? window->RootWindow : NULL;
    ImGuiWindow* display_front_window = window ? window->RootWindowDockTree : NULL;
    ImGuiDockNode* dock_node = window ? window->DockNode : NULL;
    bool active_id_window_is_dock_node_host = (g.ActiveIdWindow && dock_node && dock_node->HostWindow == g.ActiveIdWindow);

    // Steal active widgets. Some of the cases it triggers includes:
    // - Focus a window while an InputText in another window is active, if focus happens before the old InputText can run.
    // - When using Nav to activate menu items (due to timing of activating on press->new window appears->losing ActiveId)
    // - Using dock host items (tab, collapse button) can trigger this before we redirect the ActiveIdWindow toward the child window.
    if (g.ActiveId != 0 && g.ActiveIdWindow && g.ActiveIdWindow->RootWindow != focus_front_window)
        if (!g.ActiveIdNoClearOnFocusLoss && !active_id_window_is_dock_node_host)
            ClearActiveID();

    // Passing NULL allow to disable keyboard focus
    if (!window)
        return;
    window->LastFrameJustFocused = g.FrameCount;

    // Select in dock node
    // For #2304 we avoid applying focus immediately before the tabbar is visible.
    //if (dock_node && dock_node->TabBar)
    //    dock_node->TabBar->SelectedTabId = dock_node->TabBar->NextSelectedTabId = window->TabId;

    // Bring to front
    BringWindowToFocusFront(focus_front_window);
    if (((window->Flags | focus_front_window->Flags | display_front_window->Flags) & ImGuiWindowFlags_NoBringToFrontOnFocus) == 0)
        BringWindowToDisplayFront(display_front_window);
}

void ImGui::FocusTopMostWindowUnderOne(ImGuiWindow* under_this_window, ImGuiWindow* ignore_window, ImGuiViewport* filter_viewport, ImGuiFocusRequestFlags flags)
{
    ImGuiContext& g = *GImGui;
    int start_idx = g.WindowsFocusOrder.Size - 1;
    if (under_this_window != NULL)
    {
        // Aim at root window behind us, if we are in a child window that's our own root (see #4640)
        int offset = -1;
        while (under_this_window->Flags & ImGuiWindowFlags_ChildWindow)
        {
            under_this_window = under_this_window->ParentWindow;
            offset = 0;
        }
        start_idx = FindWindowFocusIndex(under_this_window) + offset;
    }
    for (int i = start_idx; i >= 0; i--)
    {
        // We may later decide to test for different NoXXXInputs based on the active navigation input (mouse vs nav) but that may feel more confusing to the user.
        ImGuiWindow* window = g.WindowsFocusOrder[i];
        if (window == ignore_window || !window->WasActive)
            continue;
        if (filter_viewport != NULL && window->Viewport != filter_viewport)
            continue;
        if ((window->Flags & (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs)) != (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs))
        {
            // FIXME-DOCK: When ImGuiFocusRequestFlags_RestoreFocusedChild is set...
            // This is failing (lagging by one frame) for docked windows.
            // If A and B are docked into window and B disappear, at the NewFrame() call site window->NavLastChildNavWindow will still point to B.
            // We might leverage the tab order implicitly stored in window->DockNodeAsHost->TabBar (essentially the 'most_recently_selected_tab' code in tab bar will do that but on next update)
            // to tell which is the "previous" window. Or we may leverage 'LastFrameFocused/LastFrameJustFocused' and have this function handle child window itself?
            FocusWindow(window, flags);
            return;
        }
    }
    FocusWindow(NULL, flags);
}

#endif // #ifndef IMGUI_DISABLE
