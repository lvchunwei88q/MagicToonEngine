// Dear ImGui implementation - see LICENSE.txt.
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"

//-----------------------------------------------------------------------------
// [SECTION] ITEM SUBMISSION
//-----------------------------------------------------------------------------
// - KeepAliveID()
// - ItemAdd()
//-----------------------------------------------------------------------------

// Code not using ItemAdd() may need to call this manually otherwise ActiveId will be cleared. In IMGUI_VERSION_NUM < 18717 this was called by GetID().
void ImGui::KeepAliveID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId == id)
        g.ActiveIdIsAlive = id;
    if (g.DeactivatedItemData.ID == id)
        g.DeactivatedItemData.IsAlive = true;
}

// Declare item bounding box for clipping and interaction.
// Note that the size can be different than the one provided to ItemSize(). Typically, widgets that spread over available surface
// declare their minimum size requirement to ItemSize() and provide a larger region to ItemAdd() which is used drawing/interaction.
// THIS IS IN THE PERFORMANCE CRITICAL PATH (UNTIL THE CLIPPING TEST AND EARLY-RETURN)
IM_MSVC_RUNTIME_CHECKS_OFF
bool ImGui::ItemAdd(const ImRect& bb, ImGuiID id, const ImRect* nav_bb_arg, ImGuiItemFlags extra_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Set item data
    // (DisplayRect is left untouched, made valid when ImGuiItemStatusFlags_HasDisplayRect is set)
    g.LastItemData.ID = id;
    g.LastItemData.Rect = bb;
    g.LastItemData.NavRect = nav_bb_arg ? *nav_bb_arg : bb;
    g.LastItemData.ItemFlags = g.CurrentItemFlags | g.NextItemData.ItemFlags | extra_flags;
    g.LastItemData.StatusFlags = ImGuiItemStatusFlags_None;
    // Note: we don't copy 'g.NextItemData.SelectionUserData' to an hypothetical g.LastItemData.SelectionUserData: since the former is not cleared.

    if (id != 0)
    {
        KeepAliveID(id);

        // Directional navigation processing
        // Runs prior to clipping early-out
        //  (a) So that NavInitRequest can be honored, for newly opened windows to select a default widget
        //  (b) So that we can scroll up/down past clipped items. This adds a small O(N) cost to regular navigation requests
        //      unfortunately, but it is still limited to one window. It may not scale very well for windows with ten of
        //      thousands of item, but at least NavMoveRequest is only set on user interaction, aka maximum once a frame.
        //      We could early out with "if (is_clipped && !g.NavInitRequest) return false;" but when we wouldn't be able
        //      to reach unclipped widgets. This would work if user had explicit scrolling control (e.g. mapped on a stick).
        // We intentionally don't check if g.NavWindow != NULL because g.NavAnyRequest should only be set when it is non null.
        // If we crash on a NULL g.NavWindow we need to fix the bug elsewhere.
        if (!(g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav))
        {
            // FIXME-NAV: investigate changing the window tests into a simple 'if (g.NavFocusScopeId == g.CurrentFocusScopeId)' test.
            window->DC.NavLayersActiveMaskNext |= (1 << window->DC.NavLayerCurrent);
            if (g.NavId == id || g.NavAnyRequest)
                if (g.NavWindow->RootWindowForNav == window->RootWindowForNav)
                    if (window == g.NavWindow || ((window->ChildFlags | g.NavWindow->ChildFlags) & ImGuiChildFlags_NavFlattened))
                        NavProcessItem();
        }

        if (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasShortcut)
            ItemHandleShortcut(id);
    }

    // Lightweight clear of SetNextItemXXX data.
    g.NextItemData.HasFlags = ImGuiNextItemDataFlags_None;
    g.NextItemData.ItemFlags = ImGuiItemFlags_None;

#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (id != 0)
        IMGUI_TEST_ENGINE_ITEM_ADD(id, g.LastItemData.NavRect, &g.LastItemData);
#endif

    // Clipping test
    // (this is an inline copy of IsClippedEx() so we can reuse the is_rect_visible value, otherwise we'd do 'if (IsClippedEx(bb, id)) return false')
    // g.NavActivateId is not necessarily == g.NavId, in the case of remote activation (e.g. shortcuts)
    const bool is_rect_visible = bb.Overlaps(window->ClipRect);
    if (!is_rect_visible)
        if (id == 0 || (id != g.ActiveId && id != g.ActiveIdPreviousFrame && id != g.NavId && id != g.NavActivateId))
            if (!g.ItemUnclipByLog)
                return false;

    // [DEBUG]
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0)
    {
        if (id == g.DebugLocateId)
            DebugLocateItemResolveWithLastItem();

        // [DEBUG] People keep stumbling on this problem and using "" as identifier in the root of a window instead of "##something".
        // Empty identifier are valid and useful in a small amount of cases, but 99.9% of the time you want to use "##something".
        // READ THE FAQ: https://dearimgui.com/faq
        IM_ASSERT(id != window->ID && "Cannot have an empty ID at the root of a window. If you need an empty label, use ## and read the FAQ about how the ID Stack works!");

        // [DEBUG] Highlight all conflicts WITHOUT needing to hover. THIS WILL SLOW DOWN DEAR IMGUI. DON'T KEEP ACTIVATED.
        // This will only work for items submitted with ItemAdd(). Some very rare/odd/unrecommended code patterns are calling ButtonBehavior() without ItemAdd().
#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_AllowDuplicateId) == 0)
        {
            int* p_alive = g.DebugDrawIdConflictsAliveCount.GetIntRef(id, -1); // Could halve lookups if we knew ImGuiStorage can store 64-bit, or by storing FrameCount as 30-bits + highlight as 2-bits. But the point is that we should not pretend that this is fast.
            int* p_highlight = g.DebugDrawIdConflictsHighlightSet.GetIntRef(id, -1);
            if (*p_alive == g.FrameCount)
                *p_highlight = g.FrameCount;
            *p_alive = g.FrameCount;
            if (*p_highlight >= g.FrameCount - 1)
                window->DrawList->AddRect(bb.Min - ImVec2(1, 1), bb.Max + ImVec2(1, 1), IM_COL32(255, 0, 0, 255), 0.0f, ImDrawFlags_None, 2.0f);
        }
#endif
    }
    //if (g.IO.KeyAlt) window->DrawList->AddRect(bb.Min, bb.Max, IM_COL32(255,255,0,120)); // [DEBUG]
    //if ((g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav) == 0)
    //    window->DrawList->AddRect(g.LastItemData.NavRect.Min, g.LastItemData.NavRect.Max, IM_COL32(255,255,0,255)); // [DEBUG]
#endif

    if (id != 0 && g.DeactivatedItemData.ID == id)
        g.DeactivatedItemData.ElapseFrame = g.FrameCount;

    // We need to calculate this now to take account of the current clipping rectangle (as items like Selectable may change them)
    if (is_rect_visible)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Visible;
    if (IsMouseHoveringRect(bb.Min, bb.Max))
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredRect;
    return true;
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] LAYOUT
//-----------------------------------------------------------------------------
// - ItemSize()
// - SameLine()
// - GetCursorScreenPos()
// - SetCursorScreenPos()
// - GetCursorPos(), GetCursorPosX(), GetCursorPosY()
// - SetCursorPos(), SetCursorPosX(), SetCursorPosY()
// - GetCursorStartPos()
// - Indent()
// - Unindent()
// - SetNextItemWidth()
// - PushItemWidth()
// - PushMultiItemsWidths()
// - PopItemWidth()
// - CalcItemWidth()
// - CalcItemSize()
// - GetTextLineHeight()
// - GetTextLineHeightWithSpacing()
// - GetFrameHeight()
// - GetFrameHeightWithSpacing()
// - GetContentRegionMax()
// - GetContentRegionAvail(),
// - BeginGroup()
// - EndGroup()
// Also see in imgui_widgets: tab bars, and in imgui_tables: tables, columns.
//-----------------------------------------------------------------------------

// Advance cursor given item size for layout.
// Register minimum needed size so it can extend the bounding box used for auto-fit calculation.
// See comments in ItemAdd() about how/why the size provided to ItemSize() vs ItemAdd() may often different.
// THIS IS IN THE PERFORMANCE CRITICAL PATH.
IM_MSVC_RUNTIME_CHECKS_OFF
void ImGui::ItemSize(const ImVec2& size, float text_baseline_y)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    // We increase the height in this function to accommodate for baseline offset.
    // In theory we should be offsetting the starting position (window->DC.CursorPos), that will be the topic of a larger refactor,
    // but since ItemSize() is not yet an API that moves the cursor (to handle e.g. wrapping) enlarging the height has the same effect.
    const float offset_to_match_baseline_y = (text_baseline_y >= 0) ? ImMax(0.0f, window->DC.CurrLineTextBaseOffset - text_baseline_y) : 0.0f;

    const float line_y1 = window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y : window->DC.CursorPos.y;
    const float line_height = ImMax(window->DC.CurrLineSize.y, /*ImMax(*/window->DC.CursorPos.y - line_y1/*, 0.0f)*/ + size.y + offset_to_match_baseline_y);

    // Always align ourselves on pixel boundaries
    //if (g.IO.KeyAlt) window->DrawList->AddRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(size.x, line_height), IM_COL32(255,0,0,200)); // [DEBUG]
    window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x + size.x;
    window->DC.CursorPosPrevLine.y = line_y1;
    window->DC.CursorPos.x = IM_TRUNC(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);    // Next line
    window->DC.CursorPos.y = IM_TRUNC(line_y1 + line_height + g.Style.ItemSpacing.y);                       // Next line
    window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPosPrevLine.x);
    window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y - g.Style.ItemSpacing.y);
    //if (g.IO.KeyAlt) window->DrawList->AddCircle(window->DC.CursorMaxPos, 3.0f, IM_COL32(255,0,0,255), 4); // [DEBUG]

    window->DC.PrevLineSize.y = line_height;
    window->DC.CurrLineSize.y = 0.0f;
    window->DC.PrevLineTextBaseOffset = ImMax(window->DC.CurrLineTextBaseOffset, text_baseline_y);
    window->DC.CurrLineTextBaseOffset = 0.0f;
    window->DC.IsSameLine = window->DC.IsSetPos = false;

    // Horizontal layout mode
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
        SameLine();
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

// Gets back to previous line and continue with horizontal layout
//      offset_from_start_x == 0 : follow right after previous item
//      offset_from_start_x != 0 : align to specified x position (relative to window/group left)
//      spacing_w < 0            : use default spacing if offset_from_start_x == 0, no spacing if offset_from_start_x != 0
//      spacing_w >= 0           : enforce spacing amount
void ImGui::SameLine(float offset_from_start_x, float spacing_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    if (offset_from_start_x != 0.0f)
    {
        if (spacing_w < 0.0f)
            spacing_w = 0.0f;
        window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + offset_from_start_x + spacing_w + window->DC.GroupOffset.x + window->DC.ColumnsOffset.x;
        window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
    }
    else
    {
        if (spacing_w < 0.0f)
            spacing_w = g.Style.ItemSpacing.x;
        window->DC.CursorPos.x = window->DC.CursorPosPrevLine.x + spacing_w;
        window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
    }
    window->DC.CurrLineSize = window->DC.PrevLineSize;
    window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
    window->DC.IsSameLine = true;
}

ImVec2 ImGui::GetCursorScreenPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos;
}

void ImGui::SetCursorScreenPos(const ImVec2& pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos = pos;
    //window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
    window->DC.IsSetPos = true;
}

// User generally sees positions in window coordinates. Internally we store CursorPos in absolute screen coordinates because it is more convenient.
// Conversion happens as we pass the value to user, but it makes our naming convention confusing because GetCursorPos() == (DC.CursorPos - window.Pos). May want to rename 'DC.CursorPos'.
ImVec2 ImGui::GetCursorPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos - window->Pos + window->Scroll;
}

float ImGui::GetCursorPosX()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos.x - window->Pos.x + window->Scroll.x;
}

float ImGui::GetCursorPosY()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos.y - window->Pos.y + window->Scroll.y;
}

void ImGui::SetCursorPos(const ImVec2& local_pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos = window->Pos - window->Scroll + local_pos;
    //window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
    window->DC.IsSetPos = true;
}

void ImGui::SetCursorPosX(float x)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + x;
    //window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPos.x);
    window->DC.IsSetPos = true;
}

void ImGui::SetCursorPosY(float y)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos.y = window->Pos.y - window->Scroll.y + y;
    //window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y);
    window->DC.IsSetPos = true;
}

ImVec2 ImGui::GetCursorStartPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorStartPos - window->Pos;
}

void ImGui::Indent(float indent_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.Indent.x += (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
    window->DC.CursorPos.x = window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

void ImGui::Unindent(float indent_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.Indent.x -= (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
    window->DC.CursorPos.x = window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

// Affect large frame+labels widgets only.
void ImGui::SetNextItemWidth(float item_width)
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.HasFlags |= ImGuiNextItemDataFlags_HasWidth;
    g.NextItemData.Width = item_width;
}

// FIXME: Remove the == 0.0f behavior?
void ImGui::PushItemWidth(float item_width)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DC.ItemWidthStack.push_back(window->DC.ItemWidth); // Backup current width
    window->DC.ItemWidth = (item_width == 0.0f ? window->DC.ItemWidthDefault : item_width);
    g.NextItemData.HasFlags &= ~ImGuiNextItemDataFlags_HasWidth;
}

void ImGui::PushMultiItemsWidths(int components, float w_full)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(components > 0);
    const ImGuiStyle& style = g.Style;
    window->DC.ItemWidthStack.push_back(window->DC.ItemWidth); // Backup current width
    float w_items = w_full - style.ItemInnerSpacing.x * (components - 1);
    float prev_split = w_items;
    for (int i = components - 1; i > 0; i--)
    {
        float next_split = IM_TRUNC(w_items * i / components);
        window->DC.ItemWidthStack.push_back(ImMax(prev_split - next_split, 1.0f));
        prev_split = next_split;
    }
    window->DC.ItemWidth = ImMax(prev_split, 1.0f);
    g.NextItemData.HasFlags &= ~ImGuiNextItemDataFlags_HasWidth;
}

void ImGui::PopItemWidth()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->DC.ItemWidthStack.Size <= 0)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopItemWidth() too many times!");
        return;
    }
    window->DC.ItemWidth = window->DC.ItemWidthStack.back();
    window->DC.ItemWidthStack.pop_back();
}

// Calculate default item width given value passed to PushItemWidth() or SetNextItemWidth().
// The SetNextItemWidth() data is generally cleared/consumed by ItemAdd() or NextItemData.ClearFlags()
float ImGui::CalcItemWidth()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float w;
    if (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasWidth)
        w = g.NextItemData.Width;
    else
        w = window->DC.ItemWidth;
    if (w < 0.0f)
    {
        float region_avail_x = GetContentRegionAvail().x;
        w = ImMax(1.0f, region_avail_x + w);
    }
    w = IM_TRUNC(w);
    return w;
}

// [Internal] Calculate full item size given user provided 'size' parameter and default width/height. Default width is often == CalcItemWidth().
// Those two functions CalcItemWidth vs CalcItemSize are awkwardly named because they are not fully symmetrical.
// Note that only CalcItemWidth() is publicly exposed.
// The 4.0f here may be changed to match CalcItemWidth() and/or BeginChild() (right now we have a mismatch which is harmless but undesirable)
ImVec2 ImGui::CalcItemSize(ImVec2 size, float default_w, float default_h)
{
    ImVec2 avail;
    if (size.x < 0.0f || size.y < 0.0f)
        avail = GetContentRegionAvail();

    if (size.x == 0.0f)
        size.x = default_w;
    else if (size.x < 0.0f)
        size.x = ImMax(4.0f, avail.x + size.x); // <-- size.x is negative here so we are subtracting

    if (size.y == 0.0f)
        size.y = default_h;
    else if (size.y < 0.0f)
        size.y = ImMax(4.0f, avail.y + size.y); // <-- size.y is negative here so we are subtracting

    return size;
}

float ImGui::GetTextLineHeight()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize;
}

float ImGui::GetTextLineHeightWithSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.ItemSpacing.y;
}

float ImGui::GetFrameHeight()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.FramePadding.y * 2.0f;
}

float ImGui::GetFrameHeightWithSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.FramePadding.y * 2.0f + g.Style.ItemSpacing.y;
}

ImVec2 ImGui::GetContentRegionAvail()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 mx = (window->DC.CurrentColumns || g.CurrentTable) ? window->WorkRect.Max : window->ContentRegionRect.Max;
    return mx - window->DC.CursorPos;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// You should never need those functions. Always use GetCursorScreenPos() and GetContentRegionAvail()!
// They are bizarre local-coordinates which don't play well with scrolling.
ImVec2 ImGui::GetContentRegionMax()
{
    return GetContentRegionAvail() + GetCursorScreenPos() - GetWindowPos();
}

ImVec2 ImGui::GetWindowContentRegionMin()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ContentRegionRect.Min - window->Pos;
}

ImVec2 ImGui::GetWindowContentRegionMax()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ContentRegionRect.Max - window->Pos;
}
#endif

// Lock horizontal starting position + capture group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
// Groups are currently a mishmash of functionalities which should perhaps be clarified and separated.
// FIXME-OPT: Could we safely early out on ->SkipItems?
void ImGui::BeginGroup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    g.GroupStack.resize(g.GroupStack.Size + 1);
    ImGuiGroupData& group_data = g.GroupStack.back();
    group_data.WindowID = window->ID;
    group_data.BackupCursorPos = window->DC.CursorPos;
    group_data.BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
    group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
    group_data.BackupIndent = window->DC.Indent;
    group_data.BackupGroupOffset = window->DC.GroupOffset;
    group_data.BackupCurrLineSize = window->DC.CurrLineSize;
    group_data.BackupCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
    group_data.BackupActiveIdIsAlive = g.ActiveIdIsAlive;
    group_data.BackupHoveredIdIsAlive = g.HoveredId != 0;
    group_data.BackupIsSameLine = window->DC.IsSameLine;
    group_data.BackupActiveIdHasBeenEditedThisFrame = g.ActiveIdHasBeenEditedThisFrame;
    group_data.BackupDeactivatedIdIsAlive = g.DeactivatedItemData.IsAlive;
    group_data.EmitItem = true;

    window->DC.GroupOffset.x = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffset.x;
    window->DC.Indent = window->DC.GroupOffset;
    window->DC.CursorMaxPos = window->DC.CursorPos;
    window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);
    if (g.LogEnabled)
        g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

void ImGui::EndGroup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(g.GroupStack.Size > 0); // Mismatched BeginGroup()/EndGroup() calls

    ImGuiGroupData& group_data = g.GroupStack.back();
    IM_ASSERT(group_data.WindowID == window->ID); // EndGroup() in wrong window?

    if (window->DC.IsSetPos)
        ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

    // Include LastItemData.Rect.Max as a workaround for e.g. EndTable() undershooting with CursorMaxPos report. (#7543)
    ImRect group_bb(group_data.BackupCursorPos, ImMax(ImMax(window->DC.CursorMaxPos, g.LastItemData.Rect.Max), group_data.BackupCursorPos));
    window->DC.CursorPos = group_data.BackupCursorPos;
    window->DC.CursorPosPrevLine = group_data.BackupCursorPosPrevLine;
    window->DC.CursorMaxPos = ImMax(group_data.BackupCursorMaxPos, group_bb.Max);
    window->DC.Indent = group_data.BackupIndent;
    window->DC.GroupOffset = group_data.BackupGroupOffset;
    window->DC.CurrLineSize = group_data.BackupCurrLineSize;
    window->DC.CurrLineTextBaseOffset = group_data.BackupCurrLineTextBaseOffset;
    window->DC.IsSameLine = group_data.BackupIsSameLine;
    if (g.LogEnabled)
        g.LogLinePosY = -FLT_MAX; // To enforce a carriage return

    if (!group_data.EmitItem)
    {
        g.GroupStack.pop_back();
        return;
    }

    window->DC.CurrLineTextBaseOffset = ImMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrLineTextBaseOffset); // FIXME: Incorrect, we should grab the base offset from the *first line* of the group but it is hard to obtain now.
    ItemSize(group_bb.GetSize());
    ItemAdd(group_bb, 0, NULL, ImGuiItemFlags_NoTabStop);

    // If the current ActiveId was declared within the boundary of our group, we copy it to LastItemId so IsItemActive(), IsItemDeactivated() etc. will be functional on the entire group.
    // It would be neater if we replaced window.DC.LastItemId by e.g. 'bool LastItemIsActive', but would put a little more burden on individual widgets.
    // Also if you grep for LastItemId you'll notice it is only used in that context.
    // (The two tests not the same because ActiveIdIsAlive is an ID itself, in order to be able to handle ActiveId being overwritten during the frame.)
    const bool group_contains_curr_active_id = (group_data.BackupActiveIdIsAlive != g.ActiveId) && (g.ActiveIdIsAlive == g.ActiveId) && g.ActiveId;
    const bool group_contains_deactivated_id = (group_data.BackupDeactivatedIdIsAlive == false) && (g.DeactivatedItemData.IsAlive == true);
    if (group_contains_curr_active_id)
        g.LastItemData.ID = g.ActiveId;
    else if (group_contains_deactivated_id)
        g.LastItemData.ID = g.DeactivatedItemData.ID;
    g.LastItemData.Rect = group_bb;

    // Forward Hovered flag
    const bool group_contains_curr_hovered_id = (group_data.BackupHoveredIdIsAlive == false) && g.HoveredId != 0;
    if (group_contains_curr_hovered_id)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;

    // Forward Edited flag
    if (g.ActiveIdHasBeenEditedThisFrame && !group_data.BackupActiveIdHasBeenEditedThisFrame)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;

    // Forward Deactivated flag
    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDeactivated;
    if (group_contains_deactivated_id)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Deactivated;

    g.GroupStack.pop_back();
    if (g.DebugShowGroupRects)
        window->DrawList->AddRect(group_bb.Min, group_bb.Max, IM_COL32(255,0,255,255));   // [Debug]
}


//-----------------------------------------------------------------------------
// [SECTION] SCROLLING
//-----------------------------------------------------------------------------

// Helper to snap on edges when aiming at an item very close to the edge,
// So the difference between WindowPadding and ItemSpacing will be in the visible area after scrolling.
// When we refactor the scrolling API this may be configurable with a flag?
// Note that the effect for this won't be visible on X axis with default Style settings as WindowPadding.x == ItemSpacing.x by default.
static float CalcScrollEdgeSnap(float target, float snap_min, float snap_max, float snap_threshold, float center_ratio)
{
    if (target <= snap_min + snap_threshold)
        return ImLerp(snap_min, target, center_ratio);
    if (target >= snap_max - snap_threshold)
        return ImLerp(target, snap_max, center_ratio);
    return target;
}

ImVec2 ImGui::CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window)
{
    ImVec2 scroll = window->Scroll;
    ImVec2 decoration_size(window->DecoOuterSizeX1 + window->DecoInnerSizeX1 + window->DecoOuterSizeX2, window->DecoOuterSizeY1 + window->DecoInnerSizeY1 + window->DecoOuterSizeY2);
    for (int axis = 0; axis < 2; axis++)
    {
        if (window->ScrollTarget[axis] < FLT_MAX)
        {
            float center_ratio = window->ScrollTargetCenterRatio[axis];
            float scroll_target = window->ScrollTarget[axis];
            if (window->ScrollTargetEdgeSnapDist[axis] > 0.0f)
            {
                float snap_min = 0.0f;
                float snap_max = window->ScrollMax[axis] + window->SizeFull[axis] - decoration_size[axis];
                scroll_target = CalcScrollEdgeSnap(scroll_target, snap_min, snap_max, window->ScrollTargetEdgeSnapDist[axis], center_ratio);
            }
            scroll[axis] = scroll_target - center_ratio * (window->SizeFull[axis] - decoration_size[axis]);
        }
        scroll[axis] = ImRound64(ImMax(scroll[axis], 0.0f));
        if (!window->Collapsed && !window->SkipItems)
            scroll[axis] = ImMin(scroll[axis], window->ScrollMax[axis]);
    }
    return scroll;
}

void ImGui::ScrollToItem(ImGuiScrollFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ScrollToRectEx(window, g.LastItemData.NavRect, flags);
}

void ImGui::ScrollToRect(ImGuiWindow* window, const ImRect& item_rect, ImGuiScrollFlags flags)
{
    ScrollToRectEx(window, item_rect, flags);
}

// Scroll to keep newly navigated item fully into view
ImVec2 ImGui::ScrollToRectEx(ImGuiWindow* window, const ImRect& item_rect, ImGuiScrollFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImRect scroll_rect(window->InnerRect.Min - ImVec2(1, 1), window->InnerRect.Max + ImVec2(1, 1));
    scroll_rect.Min.x = ImMin(scroll_rect.Min.x + window->DecoInnerSizeX1, scroll_rect.Max.x);
    scroll_rect.Min.y = ImMin(scroll_rect.Min.y + window->DecoInnerSizeY1, scroll_rect.Max.y);
    //GetForegroundDrawList(window)->AddRect(item_rect.Min, item_rect.Max, IM_COL32(255,0,0,255), 0.0f, 0, 5.0f); // [DEBUG]
    //GetForegroundDrawList(window)->AddRect(scroll_rect.Min, scroll_rect.Max, IM_COL32_WHITE); // [DEBUG]

    // Check that only one behavior is selected per axis
    IM_ASSERT((flags & ImGuiScrollFlags_MaskX_) == 0 || ImIsPowerOfTwo(flags & ImGuiScrollFlags_MaskX_));
    IM_ASSERT((flags & ImGuiScrollFlags_MaskY_) == 0 || ImIsPowerOfTwo(flags & ImGuiScrollFlags_MaskY_));

    // Defaults
    ImGuiScrollFlags in_flags = flags;
    if ((flags & ImGuiScrollFlags_MaskX_) == 0 && window->ScrollbarX)
        flags |= ImGuiScrollFlags_KeepVisibleEdgeX;
    if ((flags & ImGuiScrollFlags_MaskY_) == 0)
        flags |= window->Appearing ? ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeY;

    const bool fully_visible_x = item_rect.Min.x >= scroll_rect.Min.x && item_rect.Max.x <= scroll_rect.Max.x;
    const bool fully_visible_y = item_rect.Min.y >= scroll_rect.Min.y && item_rect.Max.y <= scroll_rect.Max.y;
    const bool can_be_fully_visible_x = (item_rect.GetWidth() + g.Style.ItemSpacing.x * 2.0f) <= scroll_rect.GetWidth() || (window->AutoFitFramesX > 0) || (window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0;
    const bool can_be_fully_visible_y = (item_rect.GetHeight() + g.Style.ItemSpacing.y * 2.0f) <= scroll_rect.GetHeight() || (window->AutoFitFramesY > 0) || (window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0;

    if ((flags & ImGuiScrollFlags_KeepVisibleEdgeX) && !fully_visible_x)
    {
        if (item_rect.Min.x < scroll_rect.Min.x || !can_be_fully_visible_x)
            SetScrollFromPosX(window, item_rect.Min.x - g.Style.ItemSpacing.x - window->Pos.x, 0.0f);
        else if (item_rect.Max.x >= scroll_rect.Max.x)
            SetScrollFromPosX(window, item_rect.Max.x + g.Style.ItemSpacing.x - window->Pos.x, 1.0f);
    }
    else if (((flags & ImGuiScrollFlags_KeepVisibleCenterX) && !fully_visible_x) || (flags & ImGuiScrollFlags_AlwaysCenterX))
    {
        if (can_be_fully_visible_x)
            SetScrollFromPosX(window, ImTrunc((item_rect.Min.x + item_rect.Max.x) * 0.5f) - window->Pos.x, 0.5f);
        else
            SetScrollFromPosX(window, item_rect.Min.x - window->Pos.x, 0.0f);
    }

    if ((flags & ImGuiScrollFlags_KeepVisibleEdgeY) && !fully_visible_y)
    {
        if (item_rect.Min.y < scroll_rect.Min.y || !can_be_fully_visible_y)
            SetScrollFromPosY(window, item_rect.Min.y - g.Style.ItemSpacing.y - window->Pos.y, 0.0f);
        else if (item_rect.Max.y >= scroll_rect.Max.y)
            SetScrollFromPosY(window, item_rect.Max.y + g.Style.ItemSpacing.y - window->Pos.y, 1.0f);
    }
    else if (((flags & ImGuiScrollFlags_KeepVisibleCenterY) && !fully_visible_y) || (flags & ImGuiScrollFlags_AlwaysCenterY))
    {
        if (can_be_fully_visible_y)
            SetScrollFromPosY(window, ImTrunc((item_rect.Min.y + item_rect.Max.y) * 0.5f) - window->Pos.y, 0.5f);
        else
            SetScrollFromPosY(window, item_rect.Min.y - window->Pos.y, 0.0f);
    }

    ImVec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
    ImVec2 delta_scroll = next_scroll - window->Scroll;

    // Also scroll parent window to keep us into view if necessary
    if (!(flags & ImGuiScrollFlags_NoScrollParent) && (window->Flags & ImGuiWindowFlags_ChildWindow))
    {
        // FIXME-SCROLL: May be an option?
        if ((in_flags & (ImGuiScrollFlags_AlwaysCenterX | ImGuiScrollFlags_KeepVisibleCenterX)) != 0)
            in_flags = (in_flags & ~ImGuiScrollFlags_MaskX_) | ImGuiScrollFlags_KeepVisibleEdgeX;
        if ((in_flags & (ImGuiScrollFlags_AlwaysCenterY | ImGuiScrollFlags_KeepVisibleCenterY)) != 0)
            in_flags = (in_flags & ~ImGuiScrollFlags_MaskY_) | ImGuiScrollFlags_KeepVisibleEdgeY;
        delta_scroll += ScrollToRectEx(window->ParentWindow, ImRect(item_rect.Min - delta_scroll, item_rect.Max - delta_scroll), in_flags);
    }

    return delta_scroll;
}

float ImGui::GetScrollX()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Scroll.x;
}

float ImGui::GetScrollY()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Scroll.y;
}

float ImGui::GetScrollMaxX()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ScrollMax.x;
}

float ImGui::GetScrollMaxY()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ScrollMax.y;
}

void ImGui::SetScrollX(ImGuiWindow* window, float scroll_x)
{
    window->ScrollTarget.x = scroll_x;
    window->ScrollTargetCenterRatio.x = 0.0f;
    window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void ImGui::SetScrollY(ImGuiWindow* window, float scroll_y)
{
    window->ScrollTarget.y = scroll_y;
    window->ScrollTargetCenterRatio.y = 0.0f;
    window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void ImGui::SetScrollX(float scroll_x)
{
    ImGuiContext& g = *GImGui;
    SetScrollX(g.CurrentWindow, scroll_x);
}

void ImGui::SetScrollY(float scroll_y)
{
    ImGuiContext& g = *GImGui;
    SetScrollY(g.CurrentWindow, scroll_y);
}

// Note that a local position will vary depending on initial scroll value,
// This is a little bit confusing so bear with us:
//  - local_pos = (absolution_pos - window->Pos)
//  - So local_x/local_y are 0.0f for a position at the upper-left corner of a window,
//    and generally local_x/local_y are >(padding+decoration) && <(size-padding-decoration) when in the visible area.
//  - They mostly exist because of legacy API.
// Following the rules above, when trying to work with scrolling code, consider that:
//  - SetScrollFromPosY(0.0f) == SetScrollY(0.0f + scroll.y) == has no effect!
//  - SetScrollFromPosY(-scroll.y) == SetScrollY(-scroll.y + scroll.y) == SetScrollY(0.0f) == reset scroll. Of course writing SetScrollY(0.0f) directly then makes more sense
// We store a target position so centering and clamping can occur on the next frame when we are guaranteed to have a known window size
void ImGui::SetScrollFromPosX(ImGuiWindow* window, float local_x, float center_x_ratio)
{
    IM_ASSERT(center_x_ratio >= 0.0f && center_x_ratio <= 1.0f);
    window->ScrollTarget.x = IM_TRUNC(local_x - window->DecoOuterSizeX1 - window->DecoInnerSizeX1 + window->Scroll.x); // Convert local position to scroll offset
    window->ScrollTargetCenterRatio.x = center_x_ratio;
    window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void ImGui::SetScrollFromPosY(ImGuiWindow* window, float local_y, float center_y_ratio)
{
    IM_ASSERT(center_y_ratio >= 0.0f && center_y_ratio <= 1.0f);
    window->ScrollTarget.y = IM_TRUNC(local_y - window->DecoOuterSizeY1 - window->DecoInnerSizeY1 + window->Scroll.y); // Convert local position to scroll offset
    window->ScrollTargetCenterRatio.y = center_y_ratio;
    window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void ImGui::SetScrollFromPosX(float local_x, float center_x_ratio)
{
    ImGuiContext& g = *GImGui;
    SetScrollFromPosX(g.CurrentWindow, local_x, center_x_ratio);
}

void ImGui::SetScrollFromPosY(float local_y, float center_y_ratio)
{
    ImGuiContext& g = *GImGui;
    SetScrollFromPosY(g.CurrentWindow, local_y, center_y_ratio);
}

// center_x_ratio: 0.0f left of last item, 0.5f horizontal center of last item, 1.0f right of last item.
void ImGui::SetScrollHereX(float center_x_ratio)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float spacing_x = ImMax(window->WindowPadding.x, g.Style.ItemSpacing.x);
    float target_pos_x = ImLerp(g.LastItemData.Rect.Min.x - spacing_x, g.LastItemData.Rect.Max.x + spacing_x, center_x_ratio);
    SetScrollFromPosX(window, target_pos_x - window->Pos.x, center_x_ratio); // Convert from absolute to local pos

    // Tweak: snap on edges when aiming at an item very close to the edge
    window->ScrollTargetEdgeSnapDist.x = ImMax(0.0f, window->WindowPadding.x - spacing_x);
}

// center_y_ratio: 0.0f top of last item, 0.5f vertical center of last item, 1.0f bottom of last item.
void ImGui::SetScrollHereY(float center_y_ratio)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float spacing_y = ImMax(window->WindowPadding.y, g.Style.ItemSpacing.y);
    float target_pos_y = ImLerp(window->DC.CursorPosPrevLine.y - spacing_y, window->DC.CursorPosPrevLine.y + window->DC.PrevLineSize.y + spacing_y, center_y_ratio);
    SetScrollFromPosY(window, target_pos_y - window->Pos.y, center_y_ratio); // Convert from absolute to local pos

    // Tweak: snap on edges when aiming at an item very close to the edge
    window->ScrollTargetEdgeSnapDist.y = ImMax(0.0f, window->WindowPadding.y - spacing_y);
}

#endif // #ifndef IMGUI_DISABLE
