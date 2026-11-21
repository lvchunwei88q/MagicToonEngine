// Dear ImGui implementation - see LICENSE.txt.
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"

static const ImVec2 TOOLTIP_DEFAULT_OFFSET_MOUSE = ImVec2(16, 10);      // Multiplied by g.Style.MouseCursorScale
static const ImVec2 TOOLTIP_DEFAULT_OFFSET_TOUCH = ImVec2(0, -20);      // Multiplied by g.Style.MouseCursorScale
static const ImVec2 TOOLTIP_DEFAULT_PIVOT_TOUCH = ImVec2(0.5f, 1.0f);   // Multiplied by g.Style.MouseCursorScale

//-----------------------------------------------------------------------------
// [SECTION] TOOLTIPS
//-----------------------------------------------------------------------------

bool ImGui::BeginTooltip()
{
    return BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_None);
}

bool ImGui::BeginItemTooltip()
{
    if (!IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        return false;
    return BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_None);
}

bool ImGui::BeginTooltipEx(ImGuiTooltipFlags tooltip_flags, ImGuiWindowFlags extra_window_flags)
{
    ImGuiContext& g = *GImGui;

    const bool is_dragdrop_tooltip = g.DragDropWithinSource || g.DragDropWithinTarget;
    if (is_dragdrop_tooltip)
    {
        // Drag and Drop tooltips are positioning differently than other tooltips:
        // - offset visibility to increase visibility around mouse.
        // - never clamp within outer viewport boundary.
        // We call SetNextWindowPos() to enforce position and disable clamping.
        // See FindBestWindowPosForPopup() for positioning logic of other tooltips (not drag and drop ones).
        //ImVec2 tooltip_pos = g.IO.MousePos - g.ActiveIdClickOffset - g.Style.WindowPadding;
        const bool is_touchscreen = (g.IO.MouseSource == ImGuiMouseSource_TouchScreen);
        if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos) == 0)
        {
            ImVec2 tooltip_pos = is_touchscreen ? (g.IO.MousePos + TOOLTIP_DEFAULT_OFFSET_TOUCH * g.Style.MouseCursorScale) : (g.IO.MousePos + TOOLTIP_DEFAULT_OFFSET_MOUSE * g.Style.MouseCursorScale);
            ImVec2 tooltip_pivot = is_touchscreen ? TOOLTIP_DEFAULT_PIVOT_TOUCH : ImVec2(0.0f, 0.0f);
            SetNextWindowPos(tooltip_pos, ImGuiCond_None, tooltip_pivot);
        }

        SetNextWindowBgAlpha(g.Style.Colors[ImGuiCol_PopupBg].w * 0.60f);
        //PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * 0.60f); // This would be nice but e.g ColorButton with checkerboard has issue with transparent colors :(
        tooltip_flags |= ImGuiTooltipFlags_OverridePrevious;
    }

    // Hide previous tooltip from being displayed. We can't easily "reset" the content of a window so we create a new one.
    if ((tooltip_flags & ImGuiTooltipFlags_OverridePrevious) && g.TooltipPreviousWindow != NULL && g.TooltipPreviousWindow->Active && !IsWindowInBeginStack(g.TooltipPreviousWindow))
    {
        //IMGUI_DEBUG_LOG("[tooltip] '%s' already active, using +1 for this frame\n", window_name);
        SetWindowHiddenAndSkipItemsForCurrentFrame(g.TooltipPreviousWindow);
        g.TooltipOverrideCount++;
    }

    const char* window_name_template = is_dragdrop_tooltip ? "##Tooltip_DragDrop_%02d" : "##Tooltip_%02d";
    char window_name[32];
    ImFormatString(window_name, IM_COUNTOF(window_name), window_name_template, g.TooltipOverrideCount);
    ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking;
    Begin(window_name, NULL, flags | extra_window_flags);
    // 2023-03-09: Added bool return value to the API, but currently always returning true.
    // If this ever returns false we need to update BeginDragDropSource() accordingly.
    //if (!ret)
    //    End();
    //return ret;
    return true;
}

void ImGui::EndTooltip()
{
    IM_ASSERT(GetCurrentWindowRead()->Flags & ImGuiWindowFlags_Tooltip);   // Mismatched BeginTooltip()/EndTooltip() calls
    End();
}

void ImGui::SetTooltip(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SetTooltipV(fmt, args);
    va_end(args);
}

void ImGui::SetTooltipV(const char* fmt, va_list args)
{
    if (!BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
        return;
    TextV(fmt, args);
    EndTooltip();
}

// Shortcut to use 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav'.
// Defaults to == ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort when using the mouse.
void ImGui::SetItemTooltip(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        SetTooltipV(fmt, args);
    va_end(args);
}

void ImGui::SetItemTooltipV(const char* fmt, va_list args)
{
    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        SetTooltipV(fmt, args);
}


//-----------------------------------------------------------------------------
// [SECTION] POPUPS
//-----------------------------------------------------------------------------

// Supported flags: ImGuiPopupFlags_AnyPopupId, ImGuiPopupFlags_AnyPopupLevel
bool ImGui::IsPopupOpen(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    if (popup_flags & ImGuiPopupFlags_AnyPopupId)
    {
        // Return true if any popup is open at the current BeginPopup() level of the popup stack
        // This may be used to e.g. test for another popups already opened to handle popups priorities at the same level.
        IM_ASSERT(id == 0);
        if (popup_flags & ImGuiPopupFlags_AnyPopupLevel)
            return g.OpenPopupStack.Size > 0;
        else
            return g.OpenPopupStack.Size > g.BeginPopupStack.Size;
    }
    else
    {
        if (popup_flags & ImGuiPopupFlags_AnyPopupLevel)
        {
            // Return true if the popup is open anywhere in the popup stack
            for (ImGuiPopupData& popup_data : g.OpenPopupStack)
                if (popup_data.PopupId == id)
                    return true;
            return false;
        }
        else
        {
            // Return true if the popup is open at the current BeginPopup() level of the popup stack (this is the most-common query)
            return g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].PopupId == id;
        }
    }
}

bool ImGui::IsPopupOpen(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = (popup_flags & ImGuiPopupFlags_AnyPopupId) ? 0 : g.CurrentWindow->GetID(str_id);
    if ((popup_flags & ImGuiPopupFlags_AnyPopupLevel) && id != 0)
        IM_ASSERT(0 && "Cannot use IsPopupOpen() with a string id and ImGuiPopupFlags_AnyPopupLevel."); // But non-string version is legal and used internally
    return IsPopupOpen(id, popup_flags);
}

// Also see FindBlockingModal(NULL)
ImGuiWindow* ImGui::GetTopMostPopupModal()
{
    ImGuiContext& g = *GImGui;
    for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
        if (ImGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
            if (popup->Flags & ImGuiWindowFlags_Modal)
                return popup;
    return NULL;
}

// See Demo->Stacked Modal to confirm what this is for.
ImGuiWindow* ImGui::GetTopMostAndVisiblePopupModal()
{
    ImGuiContext& g = *GImGui;
    for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
        if (ImGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
            if ((popup->Flags & ImGuiWindowFlags_Modal) && IsWindowActiveAndVisible(popup))
                return popup;
    return NULL;
}


// When a modal popup is open, newly created windows that want focus (i.e. are not popups and do not specify ImGuiWindowFlags_NoFocusOnAppearing)
// should be positioned behind that modal window, unless the window was created inside the modal begin-stack.
// In case of multiple stacked modals newly created window honors begin stack order and does not go below its own modal parent.
// - WindowA            // FindBlockingModal() returns Modal1
//   - WindowB          //                  .. returns Modal1
//   - Modal1           //                  .. returns Modal2
//      - WindowC       //                  .. returns Modal2
//          - WindowD   //                  .. returns Modal2
//          - Modal2    //                  .. returns Modal2
//            - WindowE //                  .. returns NULL
// Notes:
// - FindBlockingModal(NULL) == NULL is generally equivalent to GetTopMostPopupModal() == NULL.
//   Only difference is here we check for ->Active/WasActive but it may be unnecessary.
ImGuiWindow* ImGui::FindBlockingModal(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size <= 0)
        return NULL;

    // Find a modal that has common parent with specified window. Specified window should be positioned behind that modal.
    for (ImGuiPopupData& popup_data : g.OpenPopupStack)
    {
        ImGuiWindow* popup_window = popup_data.Window;
        if (popup_window == NULL || !(popup_window->Flags & ImGuiWindowFlags_Modal))
            continue;
        if (!popup_window->Active && !popup_window->WasActive)  // Check WasActive, because this code may run before popup renders on current frame, also check Active to handle newly created windows.
            continue;
        if (window == NULL)                                     // FindBlockingModal(NULL) test for if FocusWindow(NULL) is naturally possible via a mouse click.
            return popup_window;
        if (IsWindowWithinBeginStackOf(window, popup_window))   // Window may be over modal
            continue;
        return popup_window;                                    // Place window right below first block modal
    }
    return NULL;
}

void ImGui::OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = g.CurrentWindow->GetID(str_id);
    IMGUI_DEBUG_LOG_POPUP("[popup] OpenPopup(\"%s\" -> 0x%08X)\n", str_id, id);
    OpenPopupEx(id, popup_flags);
}

void ImGui::OpenPopup(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    OpenPopupEx(id, popup_flags);
}

// Mark popup as open (toggle toward open state).
// Popups are closed when user click outside, or activate a pressable item, or CloseCurrentPopup() is called within a BeginPopup()/EndPopup() block.
// Popup identifiers are relative to the current ID-stack (so OpenPopup and BeginPopup needs to be at the same level).
// One open popup per level of the popup hierarchy (NB: when assigning we reset the Window member of ImGuiPopupRef to NULL)
void ImGui::OpenPopupEx(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    const int current_stack_size = g.BeginPopupStack.Size;

    if (popup_flags & ImGuiPopupFlags_NoOpenOverExistingPopup)
        if (IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId))
            return;

    ImGuiPopupData popup_ref; // Tagged as new ref as Window will be set back to NULL if we write this into OpenPopupStack.
    popup_ref.PopupId = id;
    popup_ref.Window = NULL;
    popup_ref.RestoreNavWindow = g.NavWindow;           // When popup closes focus may be restored to NavWindow (depend on window type).
    popup_ref.OpenFrameCount = g.FrameCount;
    popup_ref.OpenParentId = parent_window->IDStack.back();
    popup_ref.OpenPopupPos = NavCalcPreferredRefPos(ImGuiWindowFlags_Popup);
    popup_ref.OpenMousePos = IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : popup_ref.OpenPopupPos;

    IMGUI_DEBUG_LOG_POPUP("[popup] OpenPopupEx(0x%08X)\n", id);
    if (g.OpenPopupStack.Size < current_stack_size + 1)
    {
        g.OpenPopupStack.push_back(popup_ref);
    }
    else
    {
        // Gently handle the user mistakenly calling OpenPopup() every frames: it is likely a programming mistake!
        // However, if we were to run the regular code path, the ui would become completely unusable because the popup will always be
        // in hidden-while-calculating-size state _while_ claiming focus. Which is extremely confusing situation for the programmer.
        // Instead, for successive frames calls to OpenPopup(), we silently avoid reopening even if ImGuiPopupFlags_NoReopen is not specified.
        bool keep_existing = false;
        if (g.OpenPopupStack[current_stack_size].PopupId == id)
            if ((g.OpenPopupStack[current_stack_size].OpenFrameCount == g.FrameCount - 1) || (popup_flags & ImGuiPopupFlags_NoReopen))
                keep_existing = true;
        if (keep_existing)
        {
            // No reopen
            g.OpenPopupStack[current_stack_size].OpenFrameCount = popup_ref.OpenFrameCount;
        }
        else
        {
            // Reopen: close child popups if any, then flag popup for open/reopen (set position, focus, init navigation)
            ClosePopupToLevel(current_stack_size, true);
            g.OpenPopupStack.push_back(popup_ref);
        }

        // When reopening a popup we first refocus its parent, otherwise if its parent is itself a popup it would get closed by ClosePopupsOverWindow().
        // This is equivalent to what ClosePopupToLevel() does.
        //if (g.OpenPopupStack[current_stack_size].PopupId == id)
        //    FocusWindow(parent_window);
    }
}

// When popups are stacked, clicking on a lower level popups puts focus back to it and close popups above it.
// This function closes any popups that are over 'ref_window'.
void ImGui::ClosePopupsOverWindow(ImGuiWindow* ref_window, bool restore_focus_to_window_under_popup)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size == 0)
        return;

    // Don't close our own child popup windows.
    //IMGUI_DEBUG_LOG_POPUP("[popup] ClosePopupsOverWindow(\"%s\") restore_under=%d\n", ref_window ? ref_window->Name : "<NULL>", restore_focus_to_window_under_popup);
    int popup_count_to_keep = 0;
    if (ref_window)
    {
        // Find the highest popup which is a descendant of the reference window (generally reference window = NavWindow)
        for (; popup_count_to_keep < g.OpenPopupStack.Size; popup_count_to_keep++)
        {
            ImGuiPopupData& popup = g.OpenPopupStack[popup_count_to_keep];
            if (!popup.Window)
                continue;
            IM_ASSERT((popup.Window->Flags & ImGuiWindowFlags_Popup) != 0);

            // Trim the stack unless the popup is a direct parent of the reference window (the reference window is often the NavWindow)
            // - Clicking/Focusing Window2 won't close Popup1:
            //     Window -> Popup1 -> Window2(Ref)
            // - Clicking/focusing Popup1 will close Popup2 and Popup3:
            //     Window -> Popup1(Ref) -> Popup2 -> Popup3
            // - Each popups may contain child windows, which is why we compare ->RootWindowDockTree!
            //     Window -> Popup1 -> Popup1_Child -> Popup2 -> Popup2_Child
            // We step through every popup from bottom to top to validate their position relative to reference window.
            bool ref_window_is_descendant_of_popup = false;
            for (int n = popup_count_to_keep; n < g.OpenPopupStack.Size; n++)
                if (ImGuiWindow* popup_window = g.OpenPopupStack[n].Window)
                    //if (popup_window->RootWindowDockTree == ref_window->RootWindowDockTree) // FIXME-MERGE
                    if (IsWindowWithinBeginStackOf(ref_window, popup_window))
                    {
                        ref_window_is_descendant_of_popup = true;
                        break;
                    }
            if (!ref_window_is_descendant_of_popup)
                break;
        }
    }
    if (popup_count_to_keep < g.OpenPopupStack.Size) // This test is not required but it allows to set a convenient breakpoint on the statement below
    {
        IMGUI_DEBUG_LOG_POPUP("[popup] ClosePopupsOverWindow(\"%s\")\n", ref_window ? ref_window->Name : "<NULL>");
        ClosePopupToLevel(popup_count_to_keep, restore_focus_to_window_under_popup);
    }
}

void ImGui::ClosePopupsExceptModals()
{
    ImGuiContext& g = *GImGui;

    int popup_count_to_keep;
    for (popup_count_to_keep = g.OpenPopupStack.Size; popup_count_to_keep > 0; popup_count_to_keep--)
    {
        ImGuiWindow* window = g.OpenPopupStack[popup_count_to_keep - 1].Window;
        if (!window || (window->Flags & ImGuiWindowFlags_Modal))
            break;
    }
    if (popup_count_to_keep < g.OpenPopupStack.Size) // This test is not required but it allows to set a convenient breakpoint on the statement below
        ClosePopupToLevel(popup_count_to_keep, true);
}

void ImGui::ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup)
{
    ImGuiContext& g = *GImGui;
    IMGUI_DEBUG_LOG_POPUP("[popup] ClosePopupToLevel(%d), restore_under=%d\n", remaining, restore_focus_to_window_under_popup);
    IM_ASSERT(remaining >= 0 && remaining < g.OpenPopupStack.Size);
    if (g.DebugLogFlags & ImGuiDebugLogFlags_EventPopup)
        for (int n = remaining; n < g.OpenPopupStack.Size; n++)
            IMGUI_DEBUG_LOG_POPUP("[popup] - Closing PopupID 0x%08X Window \"%s\"\n", g.OpenPopupStack[n].PopupId, g.OpenPopupStack[n].Window ? g.OpenPopupStack[n].Window->Name : NULL);

    // Trim open popup stack
    ImGuiPopupData prev_popup = g.OpenPopupStack[remaining];
    g.OpenPopupStack.resize(remaining);

    // Restore focus (unless popup window was not yet submitted, and didn't have a chance to take focus anyhow. See #7325 for an edge case)
    if (restore_focus_to_window_under_popup && prev_popup.Window)
    {
        ImGuiWindow* popup_window = prev_popup.Window;
        ImGuiWindow* focus_window = (popup_window->Flags & ImGuiWindowFlags_ChildMenu) ? popup_window->ParentWindow : prev_popup.RestoreNavWindow;
        if (focus_window && !focus_window->WasActive)
            FocusTopMostWindowUnderOne(popup_window, NULL, NULL, ImGuiFocusRequestFlags_RestoreFocusedChild); // Fallback
        else
            FocusWindow(focus_window, (g.NavLayer == ImGuiNavLayer_Main) ? ImGuiFocusRequestFlags_RestoreFocusedChild : ImGuiFocusRequestFlags_None);
    }
}

// Close the popup we have begin-ed into.
void ImGui::CloseCurrentPopup()
{
    ImGuiContext& g = *GImGui;
    int popup_idx = g.BeginPopupStack.Size - 1;
    if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size || g.BeginPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
        return;

    // Closing a menu closes its top-most parent popup (unless a modal)
    while (popup_idx > 0)
    {
        ImGuiWindow* popup_window = g.OpenPopupStack[popup_idx].Window;
        ImGuiWindow* parent_popup_window = g.OpenPopupStack[popup_idx - 1].Window;
        bool close_parent = false;
        if (popup_window && (popup_window->Flags & ImGuiWindowFlags_ChildMenu))
            if (parent_popup_window && !(parent_popup_window->Flags & ImGuiWindowFlags_MenuBar))
                close_parent = true;
        if (!close_parent)
            break;
        popup_idx--;
    }
    IMGUI_DEBUG_LOG_POPUP("[popup] CloseCurrentPopup %d -> %d\n", g.BeginPopupStack.Size - 1, popup_idx);
    ClosePopupToLevel(popup_idx, true);

    // A common pattern is to close a popup when selecting a menu item/selectable that will open another window.
    // To improve this usage pattern, we avoid nav highlight for a single frame in the parent window.
    // Similarly, we could avoid mouse hover highlight in this window but it is less visually problematic.
    if (ImGuiWindow* window = g.NavWindow)
        window->DC.NavHideHighlightOneFrame = true;
}

// Attention! BeginPopup() adds default flags when calling BeginPopupEx()!
bool ImGui::BeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_window_flags)
{
    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }

    char name[20];
    IM_ASSERT((extra_window_flags & ImGuiWindowFlags_ChildMenu) == 0); // Use BeginPopupMenuEx()
    ImFormatString(name, IM_COUNTOF(name), "##Popup_%08x", id); // No recycling, so we can close/open during the same frame

    bool is_open = Begin(name, NULL, extra_window_flags | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoDocking);
    if (!is_open) // NB: Begin can return false when the popup is completely clipped (e.g. zero size display)
        EndPopup();
    //g.CurrentWindow->FocusRouteParentWindow = g.CurrentWindow->ParentWindowInBeginStack;
    return is_open;
}

bool ImGui::BeginPopupMenuEx(ImGuiID id, const char* label, ImGuiWindowFlags extra_window_flags)
{
    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }

    char name[128];
    IM_ASSERT(extra_window_flags & ImGuiWindowFlags_ChildMenu);
    ImFormatString(name, IM_COUNTOF(name), "%s###Menu_%02d", label, g.BeginMenuDepth); // Recycle windows based on depth
    bool is_open = Begin(name, NULL, extra_window_flags | ImGuiWindowFlags_Popup);
    if (!is_open) // NB: Begin can return false when the popup is completely clipped (e.g. zero size display)
        EndPopup();
    //g.CurrentWindow->FocusRouteParentWindow = g.CurrentWindow->ParentWindowInBeginStack;
    return is_open;
}

bool ImGui::BeginPopup(const char* str_id, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size <= g.BeginPopupStack.Size) // Early out for performance
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }
    flags |= ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
    ImGuiID id = g.CurrentWindow->GetID(str_id);
    return BeginPopupEx(id, flags);
}

// If 'p_open' is specified for a modal popup window, the popup will have a regular close button which will close the popup.
// Note that popup visibility status is owned by Dear ImGui (and manipulated with e.g. OpenPopup).
// - *p_open set back to false in BeginPopupModal() when popup is not open.
// - if you set *p_open to false before calling BeginPopupModal(), it will close the popup.
bool ImGui::BeginPopupModal(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const ImGuiID id = window->GetID(name);
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        if (p_open && *p_open)
            *p_open = false;
        return false;
    }

    // Center modal windows by default for increased visibility
    // (this won't really last as settings will kick in, and is mostly for backward compatibility. user may do the same themselves)
    // FIXME: Should test for (PosCond & window->SetWindowPosAllowFlags) with the upcoming window.
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos) == 0)
    {
        const ImGuiViewport* viewport = window->WasActive ? window->Viewport : GetMainViewport(); // FIXME-VIEWPORT: What may be our reference viewport?
        SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    }

    flags |= ImGuiWindowFlags_Popup | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    const bool is_open = Begin(name, p_open, flags);
    if (!is_open || (p_open && !*p_open)) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    {
        EndPopup();
        if (is_open)
            ClosePopupToLevel(g.BeginPopupStack.Size, true);
        return false;
    }
    return is_open;
}

void ImGui::EndPopup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR_RET((window->Flags & ImGuiWindowFlags_Popup) != 0 && g.BeginPopupStack.Size > 0, "Calling EndPopup() in wrong window!");

    // Make all menus and popups wrap around for now, may need to expose that policy (e.g. focus scope could include wrap/loop policy flags used by new move requests)
    if (g.NavWindow == window)
        NavMoveRequestTryWrapping(window, ImGuiNavMoveFlags_LoopY);

    // Child-popups don't need to be laid out
    const ImGuiID backup_within_end_popup_id = g.WithinEndPopupID;
    const ImGuiID backup_within_end_child_id = g.WithinEndChildID;
    g.WithinEndPopupID = window->ID;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        g.WithinEndChildID = window->ID;
    End();
    g.WithinEndPopupID = backup_within_end_popup_id;
    g.WithinEndChildID = backup_within_end_child_id;
}

ImGuiMouseButton ImGui::GetMouseButtonFromPopupFlags(ImGuiPopupFlags flags)
{
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    if ((flags & ImGuiPopupFlags_InvalidMask_) != 0) // 1,2 --> ImGuiMouseButton_Right, ImGuiMouseButton_Middle
        return (flags & ImGuiPopupFlags_InvalidMask_);
#else
    IM_ASSERT((flags & ImGuiPopupFlags_InvalidMask_) == 0);
#endif
    if (flags & ImGuiPopupFlags_MouseButtonMask_)
        return ((flags & ImGuiPopupFlags_MouseButtonMask_) >> ImGuiPopupFlags_MouseButtonShift_) - 1;
    return ImGuiMouseButton_Right; // Default == 1
}

bool ImGui::IsPopupOpenRequestForItem(ImGuiPopupFlags popup_flags, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        return true;
    if (g.NavOpenContextMenuItemId == id && (IsItemFocused() || id == g.CurrentWindow->MoveId))
        return true;
    return false;
}

bool ImGui::IsPopupOpenRequestForWindow(ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        if (!(popup_flags & ImGuiPopupFlags_NoOpenOverItems) || !IsAnyItemHovered())
            return true;
    if (g.NavOpenContextMenuWindowId && g.CurrentWindow->ID)
        if (IsWindowChildOf(g.NavWindow, g.CurrentWindow, false, false)) // This enable ordering to be used to disambiguate item vs window (#8803)
            return true;
    return false;
}

// Helper to open a popup if mouse button is released over the item
// - This is essentially the same as BeginPopupContextItem() but without the trailing BeginPopup()
void ImGui::OpenPopupOnItemClick(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (IsPopupOpenRequestForItem(popup_flags, g.LastItemData.ID))
    {
        ImGuiID id = str_id ? window->GetID(str_id) : g.LastItemData.ID; // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
        IM_ASSERT(id != 0);                                              // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
        OpenPopupEx(id, popup_flags);
    }
}

// This is a helper to handle the simplest case of associating one named popup to one given widget.
// - To create a popup associated to the last item, you generally want to pass a NULL value to str_id.
// - To create a popup with a specific identifier, pass it in str_id.
//    - This is useful when using using BeginPopupContextItem() on an item which doesn't have an identifier, e.g. a Text() call.
//    - This is useful when multiple code locations may want to manipulate/open the same popup, given an explicit id.
// - You may want to handle the whole on user side if you have specific needs (e.g. tweaking IsItemHovered() parameters).
//   This is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       OpenPopupOnItemClick(str_id, ImGuiPopupFlags_MouseButtonRight);
//       return BeginPopup(id);
//   Which is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
//           OpenPopup(id);
//       return BeginPopup(id);
//   The main difference being that this is tweaked to avoid computing the ID twice.
bool ImGui::BeginPopupContextItem(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return false;
    ImGuiID id = str_id ? window->GetID(str_id) : g.LastItemData.ID; // If user hasn't passed an ID, we can use the LastItem ID. Using LastItem ID as a Popup ID won't conflict!
    IM_ASSERT(id != 0);                                              // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
    if (IsPopupOpenRequestForItem(popup_flags, g.LastItemData.ID))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextWindow(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!str_id)
        str_id = "window_context";
    ImGuiID id = window->GetID(str_id);
    if (IsPopupOpenRequestForWindow(popup_flags))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextVoid(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!str_id)
        str_id = "void_context";
    ImGuiID id = window->GetID(str_id);
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        if (GetTopMostPopupModal() == NULL)
            OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

// r_avoid = the rectangle to avoid (e.g. for tooltip it is a rectangle around the mouse cursor which we want to avoid. for popups it's a small point around the cursor.)
// r_outer = the visible area rectangle, minus safe area padding. If our popup size won't fit because of safe area padding we ignore it.
// (r_outer is usually equivalent to the viewport rectangle minus padding, but when multi-viewports are enabled and monitor
//  information are available, it may represent the entire platform monitor from the frame of reference of the current viewport.
//  this allows us to have tooltips/popups displayed out of the parent viewport.)
ImVec2 ImGui::FindBestWindowPosForPopupEx(const ImVec2& ref_pos, const ImVec2& size, ImGuiDir* last_dir, const ImRect& r_outer, const ImRect& r_avoid, ImGuiPopupPositionPolicy policy)
{
    ImVec2 base_pos_clamped = ImClamp(ref_pos, r_outer.Min, r_outer.Max - size);
    //GetForegroundDrawList()->AddRect(r_avoid.Min, r_avoid.Max, IM_COL32(255,0,0,255));
    //GetForegroundDrawList()->AddRect(r_outer.Min, r_outer.Max, IM_COL32(0,255,0,255));

    // Combo Box policy (we want a connecting edge)
    if (policy == ImGuiPopupPositionPolicy_ComboBox)
    {
        const ImGuiDir dir_preferred_order[ImGuiDir_COUNT] = { ImGuiDir_Down, ImGuiDir_Right, ImGuiDir_Left, ImGuiDir_Up };
        for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_COUNT; n++)
        {
            const ImGuiDir dir = (n == -1) ? *last_dir : dir_preferred_order[n];
            if (n != -1 && dir == *last_dir) // Already tried this direction?
                continue;
            ImVec2 pos;
            if (dir == ImGuiDir_Down)  pos = ImVec2(r_avoid.Min.x, r_avoid.Max.y);          // Below, Toward Right (default)
            if (dir == ImGuiDir_Right) pos = ImVec2(r_avoid.Min.x, r_avoid.Min.y - size.y); // Above, Toward Right
            if (dir == ImGuiDir_Left)  pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Max.y); // Below, Toward Left
            if (dir == ImGuiDir_Up)    pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Min.y - size.y); // Above, Toward Left
            if (!r_outer.Contains(ImRect(pos, pos + size)))
                continue;
            *last_dir = dir;
            return pos;
        }
    }

    // Tooltip and Default popup policy
    // (Always first try the direction we used on the last frame, if any)
    if (policy == ImGuiPopupPositionPolicy_Tooltip || policy == ImGuiPopupPositionPolicy_Default)
    {
        const ImGuiDir dir_preferred_order[ImGuiDir_COUNT] = { ImGuiDir_Right, ImGuiDir_Down, ImGuiDir_Up, ImGuiDir_Left };
        for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_COUNT; n++)
        {
            const ImGuiDir dir = (n == -1) ? *last_dir : dir_preferred_order[n];
            if (n != -1 && dir == *last_dir) // Already tried this direction?
                continue;

            const float avail_w = (dir == ImGuiDir_Left ? r_avoid.Min.x : r_outer.Max.x) - (dir == ImGuiDir_Right ? r_avoid.Max.x : r_outer.Min.x);
            const float avail_h = (dir == ImGuiDir_Up ? r_avoid.Min.y : r_outer.Max.y) - (dir == ImGuiDir_Down ? r_avoid.Max.y : r_outer.Min.y);

            // If there's not enough room on one axis, there's no point in positioning on a side on this axis (e.g. when not enough width, use a top/bottom position to maximize available width)
            if (avail_w < size.x && (dir == ImGuiDir_Left || dir == ImGuiDir_Right))
                continue;
            if (avail_h < size.y && (dir == ImGuiDir_Up || dir == ImGuiDir_Down))
                continue;

            ImVec2 pos;
            pos.x = (dir == ImGuiDir_Left) ? r_avoid.Min.x - size.x : (dir == ImGuiDir_Right) ? r_avoid.Max.x : base_pos_clamped.x;
            pos.y = (dir == ImGuiDir_Up) ? r_avoid.Min.y - size.y : (dir == ImGuiDir_Down) ? r_avoid.Max.y : base_pos_clamped.y;

            // Clamp top-left corner of popup
            pos.x = ImMax(pos.x, r_outer.Min.x);
            pos.y = ImMax(pos.y, r_outer.Min.y);

            *last_dir = dir;
            return pos;
        }
    }

    // Fallback when not enough room:
    *last_dir = ImGuiDir_None;

    // For tooltip we prefer avoiding the cursor at all cost even if it means that part of the tooltip won't be visible.
    if (policy == ImGuiPopupPositionPolicy_Tooltip)
        return ref_pos + ImVec2(2, 2);

    // Otherwise try to keep within display
    ImVec2 pos = ref_pos;
    pos.x = ImMax(ImMin(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
    pos.y = ImMax(ImMin(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
    return pos;
}

// Note that this is used for popups, which can overlap the non work-area of individual viewports.
ImRect ImGui::GetPopupAllowedExtentRect(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    ImRect r_screen;
    if (window->ViewportAllowPlatformMonitorExtend >= 0)
    {
        // Extent with be in the frame of reference of the given viewport (so Min is likely to be negative here)
        const ImGuiPlatformMonitor& monitor = g.PlatformIO.Monitors[window->ViewportAllowPlatformMonitorExtend];
        r_screen.Min = monitor.WorkPos;
        r_screen.Max = monitor.WorkPos + monitor.WorkSize;
    }
    else
    {
        // Use the full viewport area (not work area) for popups
        r_screen = window->Viewport->GetMainRect();
    }
    ImVec2 padding = g.Style.DisplaySafeAreaPadding;
    r_screen.Expand(ImVec2((r_screen.GetWidth() > padding.x * 2) ? -padding.x : 0.0f, (r_screen.GetHeight() > padding.y * 2) ? -padding.y : 0.0f));
    return r_screen;
}

ImVec2 ImGui::FindBestWindowPosForPopup(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;

    ImRect r_outer = GetPopupAllowedExtentRect(window);
    if (window->Flags & ImGuiWindowFlags_ChildMenu)
    {
        // Child menus typically request _any_ position within the parent menu item, and then we move the new menu outside the parent bounds.
        // This is how we end up with child menus appearing (most-commonly) on the right of the parent menu.
        ImGuiWindow* parent_window = window->ParentWindow;
        float horizontal_overlap = g.Style.ItemInnerSpacing.x; // We want some overlap to convey the relative depth of each menu (currently the amount of overlap is hard-coded to style.ItemSpacing.x).
        ImRect r_avoid;
        if (parent_window->DC.MenuBarAppending)
            r_avoid = ImRect(-FLT_MAX, parent_window->ClipRect.Min.y, FLT_MAX, parent_window->ClipRect.Max.y); // Avoid parent menu-bar. If we wanted multi-line menu-bar, we may instead want to have the calling window setup e.g. a NextWindowData.PosConstraintAvoidRect field
        else
            r_avoid = ImRect(parent_window->Pos.x + horizontal_overlap, -FLT_MAX, parent_window->Pos.x + parent_window->Size.x - horizontal_overlap - parent_window->ScrollbarSizes.x, FLT_MAX);
        return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid, ImGuiPopupPositionPolicy_Default);
    }
    if (window->Flags & ImGuiWindowFlags_Popup)
    {
        return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, ImRect(window->Pos, window->Pos), ImGuiPopupPositionPolicy_Default); // Ideally we'd disable r_avoid here
    }
    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {
        // Position tooltip (always follows mouse + clamp within outer boundaries)
        // FIXME:
        // - Too many paths. One problem is that FindBestWindowPosForPopupEx() doesn't allow passing a suggested position (so touch screen path doesn't use it by default).
        // - Drag and drop tooltips are not using this path either: BeginTooltipEx() manually sets their position.
        // - Require some tidying up. In theory we could handle both cases in same location, but requires a bit of shuffling
        //   as drag and drop tooltips are calling SetNextWindowPos() leading to 'window_pos_set_by_api' being set in Begin().
        IM_ASSERT(g.CurrentWindow == window);
        const float scale = g.Style.MouseCursorScale;
        const ImVec2 ref_pos = NavCalcPreferredRefPos(ImGuiWindowFlags_Tooltip);

        if (g.IO.MouseSource == ImGuiMouseSource_TouchScreen && NavCalcPreferredRefPosSource(ImGuiWindowFlags_Tooltip) == ImGuiInputSource_Mouse)
        {
            ImVec2 tooltip_pos = ref_pos + TOOLTIP_DEFAULT_OFFSET_TOUCH * scale - (TOOLTIP_DEFAULT_PIVOT_TOUCH * window->Size);
            if (r_outer.Contains(ImRect(tooltip_pos, tooltip_pos + window->Size)))
                return tooltip_pos;
        }

        ImVec2 tooltip_pos = ref_pos + TOOLTIP_DEFAULT_OFFSET_MOUSE * scale;
        ImRect r_avoid;
        if (g.NavCursorVisible && g.NavHighlightItemUnderNav && !g.IO.ConfigNavMoveSetMousePos)
            r_avoid = ImRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 16, ref_pos.y + 8);
        else
            r_avoid = ImRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24 * scale, ref_pos.y + 24 * scale); // FIXME: Hard-coded based on mouse cursor shape expectation. Exact dimension not very important.
        //GetForegroundDrawList()->AddRect(r_avoid.Min, r_avoid.Max, IM_COL32(255, 0, 255, 255));

        return FindBestWindowPosForPopupEx(tooltip_pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid, ImGuiPopupPositionPolicy_Tooltip);
    }
    IM_ASSERT(0);
    return window->Pos;
}

#endif // #ifndef IMGUI_DISABLE
