// Dear ImGui implementation - see LICENSE.txt.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"

//-----------------------------------------------------------------------------
// [SECTION] DRAG AND DROP
//-----------------------------------------------------------------------------

bool ImGui::IsDragDropActive()
{
    ImGuiContext& g = *GImGui;
    return g.DragDropActive;
}

void ImGui::ClearDragDrop()
{
    ImGuiContext& g = *GImGui;
    if (g.DragDropActive)
        IMGUI_DEBUG_LOG_ACTIVEID("[dragdrop] ClearDragDrop()\n");
    g.DragDropActive = false;
    g.DragDropPayload.Clear();
    g.DragDropAcceptFlagsCurr = ImGuiDragDropFlags_None;
    g.DragDropAcceptIdCurr = g.DragDropAcceptIdPrev = 0;
    g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    g.DragDropAcceptFrameCount = -1;

    g.DragDropPayloadBufHeap.clear();
    memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
}

bool ImGui::BeginTooltipHidden()
{
    ImGuiContext& g = *GImGui;
    bool ret = Begin("##Tooltip_Hidden", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    SetWindowHiddenAndSkipItemsForCurrentFrame(g.CurrentWindow);
    return ret;
}

// When this returns true you need to: a) call SetDragDropPayload() exactly once, b) you may render the payload visual/description, c) call EndDragDropSource()
// If the item has an identifier:
// - This assume/require the item to be activated (typically via ButtonBehavior).
// - Therefore if you want to use this with a mouse button other than left mouse button, it is up to the item itself to activate with another button.
// - We then pull and use the mouse button that was used to activate the item and use it to carry on the drag.
// If the item has no identifier:
// - Currently always assume left mouse button.
bool ImGui::BeginDragDropSource(ImGuiDragDropFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // FIXME-DRAGDROP: While in the common-most "drag from non-zero active id" case we can tell the mouse button,
    // in both SourceExtern and id==0 cases we may requires something else (explicit flags or some heuristic).
    ImGuiMouseButton mouse_button = ImGuiMouseButton_Left;

    bool source_drag_active = false;
    ImGuiID source_id = 0;
    ImGuiID source_parent_id = 0;
    if ((flags & ImGuiDragDropFlags_SourceExtern) == 0)
    {
        source_id = g.LastItemData.ID;
        if (source_id != 0)
        {
            // Common path: items with ID
            if (g.ActiveId != source_id)
                return false;
            if (g.ActiveIdMouseButton != -1)
                mouse_button = g.ActiveIdMouseButton;
            if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
                return false;
            g.ActiveIdAllowOverlap = false;
        }
        else
        {
            // Uncommon path: items without ID
            if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
                return false;
            if ((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) == 0 && (g.ActiveId == 0 || g.ActiveIdWindow != window))
                return false;

            // If you want to use BeginDragDropSource() on an item with no unique identifier for interaction, such as Text() or Image(), you need to:
            // A) Read the explanation below, B) Use the ImGuiDragDropFlags_SourceAllowNullID flag.
            if (!(flags & ImGuiDragDropFlags_SourceAllowNullID))
            {
                IM_ASSERT(0);
                return false;
            }

            // Magic fallback to handle items with no assigned ID, e.g. Text(), Image()
            // We build a throwaway ID based on current ID stack + relative AABB of items in window.
            // THE IDENTIFIER WON'T SURVIVE ANY REPOSITIONING/RESIZING OF THE WIDGET, so if your widget moves your dragging operation will be canceled.
            // We don't need to maintain/call ClearActiveID() as releasing the button will early out this function and trigger !ActiveIdIsAlive.
            // Rely on keeping other window->LastItemXXX fields intact.
            source_id = g.LastItemData.ID = window->GetIDFromRectangle(g.LastItemData.Rect);
            KeepAliveID(source_id);
            bool is_hovered = ItemHoverable(g.LastItemData.Rect, source_id, g.LastItemData.ItemFlags);
            if (is_hovered && g.IO.MouseClicked[mouse_button])
            {
                SetActiveID(source_id, window);
                FocusWindow(window);
            }
            if (g.ActiveId == source_id) // Allow the underlying widget to display/return hovered during the mouse release frame, else we would get a flicker.
                g.ActiveIdAllowOverlap = is_hovered;
        }
        if (g.ActiveId != source_id)
            return false;
        source_parent_id = window->IDStack.back();
        source_drag_active = IsMouseDragging(mouse_button);

        // Disable navigation and key inputs while dragging + cancel existing request if any
        SetActiveIdUsingAllKeyboardKeys();
    }
    else
    {
        // When ImGuiDragDropFlags_SourceExtern is set:
        window = NULL;
        source_id = ImHashStr("#SourceExtern");
        source_drag_active = true;
        mouse_button = g.IO.MouseDown[0] ? 0 : -1;
        KeepAliveID(source_id);
        SetActiveID(source_id, NULL);
    }

    IM_ASSERT(g.DragDropWithinTarget == false); // Can't nest BeginDragDropSource() and BeginDragDropTarget()
    if (!source_drag_active)
        return false;

    // Activate drag and drop
    if (!g.DragDropActive)
    {
        IM_ASSERT(source_id != 0);
        ClearDragDrop();
        IMGUI_DEBUG_LOG_ACTIVEID("[dragdrop] BeginDragDropSource() DragDropActive = true, source_id = 0x%08X%s\n",
            source_id, (flags & ImGuiDragDropFlags_SourceExtern) ? " (EXTERN)" : "");
        ImGuiPayload& payload = g.DragDropPayload;
        payload.SourceId = source_id;
        payload.SourceParentId = source_parent_id;
        g.DragDropActive = true;
        g.DragDropSourceFlags = flags;
        g.DragDropMouseButton = mouse_button;
        if (payload.SourceId == g.ActiveId)
            g.ActiveIdNoClearOnFocusLoss = true;
    }
    g.DragDropSourceFrameCount = g.FrameCount;
    g.DragDropWithinSource = true;

    if (!(flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    {
        // Target can request the Source to not display its tooltip (we use a dedicated flag to make this request explicit)
        // We unfortunately can't just modify the source flags and skip the call to BeginTooltip, as caller may be emitting contents.
        bool ret;
        if (g.DragDropAcceptIdPrev && (g.DragDropAcceptFlagsPrev & ImGuiDragDropFlags_AcceptNoPreviewTooltip))
            ret = BeginTooltipHidden();
        else
            ret = BeginTooltip();
        IM_ASSERT(ret); // FIXME-NEWBEGIN: If this ever becomes false, we need to Begin("##Hidden", NULL, ImGuiWindowFlags_NoSavedSettings) + SetWindowHiddenAndSkipItemsForCurrentFrame().
        IM_UNUSED(ret);
    }

    if (!(flags & ImGuiDragDropFlags_SourceNoDisableHover) && !(flags & ImGuiDragDropFlags_SourceExtern))
        g.LastItemData.StatusFlags &= ~ImGuiItemStatusFlags_HoveredRect;

    return true;
}

void ImGui::EndDragDropSource()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(g.DragDropWithinSource && "Not after a BeginDragDropSource()?");

    if (!(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
        EndTooltip();

    // Discard the drag if have not called SetDragDropPayload()
    if (g.DragDropPayload.DataFrameCount == -1)
        ClearDragDrop();
    g.DragDropWithinSource = false;
}

// Use 'cond' to choose to submit payload on drag start or every frame
bool ImGui::SetDragDropPayload(const char* type, const void* data, size_t data_size, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    ImGuiPayload& payload = g.DragDropPayload;
    if (cond == 0)
        cond = ImGuiCond_Always;

    IM_ASSERT(type != NULL);
    IM_ASSERT(ImStrlen(type) < IM_COUNTOF(payload.DataType) && "Payload type can be at most 32 characters long");
    IM_ASSERT((data != NULL && data_size > 0) || (data == NULL && data_size == 0));
    IM_ASSERT(cond == ImGuiCond_Always || cond == ImGuiCond_Once);
    IM_ASSERT(payload.SourceId != 0); // Not called between BeginDragDropSource() and EndDragDropSource()

    if (cond == ImGuiCond_Always || payload.DataFrameCount == -1)
    {
        // Copy payload
        ImStrncpy(payload.DataType, type, IM_COUNTOF(payload.DataType));
        g.DragDropPayloadBufHeap.resize(0);
        if (data_size > sizeof(g.DragDropPayloadBufLocal))
        {
            // Store in heap
            g.DragDropPayloadBufHeap.resize((int)data_size);
            payload.Data = g.DragDropPayloadBufHeap.Data;
            memcpy(payload.Data, data, (size_t)(int)data_size);
        }
        else if (data_size > 0)
        {
            // Store locally
            memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
            payload.Data = g.DragDropPayloadBufLocal;
            memcpy(payload.Data, data, (size_t)(int)data_size);
        }
        else
        {
            payload.Data = NULL;
        }
        payload.DataSize = (int)data_size;
    }
    payload.DataFrameCount = g.FrameCount;

    // Return whether the payload has been accepted
    return (g.DragDropAcceptFrameCount == g.FrameCount) || (g.DragDropAcceptFrameCount == g.FrameCount - 1);
}

bool ImGui::BeginDragDropTargetCustom(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow* window = g.CurrentWindow;
    ImGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
    if (hovered_window == NULL || window->RootWindowDockTree != hovered_window->RootWindowDockTree)
        return false;
    IM_ASSERT(id != 0);
    if (!IsMouseHoveringRect(bb.Min, bb.Max) || (id == g.DragDropPayload.SourceId))
        return false;
    if (window->SkipItems)
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false && g.DragDropWithinSource == false); // Can't nest BeginDragDropSource() and BeginDragDropTarget()
    g.DragDropTargetRect = bb;
    g.DragDropTargetClipRect = window->ClipRect; // May want to be overridden by user depending on use case?
    g.DragDropTargetId = id;
    g.DragDropTargetFullViewport = 0;
    g.DragDropWithinTarget = true;
    return true;
}

// Typical usage would be:
//   if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
//       if (ImGui::BeginDragDropTargetViewport(ImGui::GetMainViewport(), NULL))
// But we are leaving the hover test to the caller for maximum flexibility.
bool ImGui::BeginDragDropTargetViewport(ImGuiViewport* viewport, const ImRect* p_bb)
{
    ImGuiContext& g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImRect bb = p_bb ? *p_bb : ((ImGuiViewportP*)viewport)->GetWorkRect();
    ImGuiID id = viewport->ID;
    if (g.MouseViewport != viewport || !IsMouseHoveringRect(bb.Min, bb.Max, false) || (id == g.DragDropPayload.SourceId))
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false && g.DragDropWithinSource == false); // Can't nest BeginDragDropSource() and BeginDragDropTarget()
    g.DragDropTargetRect = bb;
    g.DragDropTargetClipRect = bb;
    g.DragDropTargetId = id;
    g.DragDropTargetFullViewport = id;
    g.DragDropWithinTarget = true;
    return true;
}

// We don't use BeginDragDropTargetCustom() and duplicate its code because:
// 1) we use LastItemData's ImGuiItemStatusFlags_HoveredRect which handles items that push a temporarily clip rectangle in their code. Calling BeginDragDropTargetCustom(LastItemRect) would not handle them.
// 2) and it's faster. as this code may be very frequently called, we want to early out as fast as we can.
// Also note how the HoveredWindow test is positioned differently in both functions (in both functions we optimize for the cheapest early out case)
bool ImGui::BeginDragDropTarget()
{
    ImGuiContext& g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow* window = g.CurrentWindow;
    if (!(g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect))
        return false;
    ImGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
    if (hovered_window == NULL || window->RootWindowDockTree != hovered_window->RootWindowDockTree || window->SkipItems)
        return false;

    const ImRect& display_rect = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) ? g.LastItemData.DisplayRect : g.LastItemData.Rect;
    ImGuiID id = g.LastItemData.ID;
    if (id == 0)
    {
        id = window->GetIDFromRectangle(display_rect);
        KeepAliveID(id);
    }
    if (g.DragDropPayload.SourceId == id)
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false && g.DragDropWithinSource == false); // Can't nest BeginDragDropSource() and BeginDragDropTarget()
    g.DragDropTargetRect = display_rect;
    g.DragDropTargetClipRect = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasClipRect) ? g.LastItemData.ClipRect : window->ClipRect;
    g.DragDropTargetId = id;
    g.DragDropWithinTarget = true;
    return true;
}

bool ImGui::IsDragDropPayloadBeingAccepted()
{
    ImGuiContext& g = *GImGui;
    return g.DragDropActive && g.DragDropAcceptIdPrev != 0;
}

const ImGuiPayload* ImGui::AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiPayload& payload = g.DragDropPayload;
    IM_ASSERT(g.DragDropActive);                        // Not called between BeginDragDropTarget() and EndDragDropTarget() ?
    IM_ASSERT(payload.DataFrameCount != -1);            // Forgot to call EndDragDropTarget() ?
    if (type != NULL && !payload.IsDataType(type))
        return NULL;

    // Accept smallest drag target bounding box, this allows us to nest drag targets conveniently without ordering constraints.
    // NB: We currently accept NULL id as target. However, overlapping targets requires a unique ID to function!
    const bool was_accepted_previously = (g.DragDropAcceptIdPrev == g.DragDropTargetId);
    ImRect r = g.DragDropTargetRect;
    float r_surface = r.GetWidth() * r.GetHeight();
    if (r_surface > g.DragDropAcceptIdCurrRectSurface)
        return NULL;

    g.DragDropAcceptFlagsCurr = flags;
    g.DragDropAcceptIdCurr = g.DragDropTargetId;
    g.DragDropAcceptIdCurrRectSurface = r_surface;
    //IMGUI_DEBUG_LOG("AcceptDragDropPayload(): %08X: accept\n", g.DragDropTargetId);

    // Render default drop visuals
    payload.Preview = was_accepted_previously;
    flags |= (g.DragDropSourceFlags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect); // Source can also inhibit the preview (useful for external sources that live for 1 frame)
    const bool draw_target_rect = payload.Preview && !(flags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
    if (draw_target_rect && g.DragDropTargetFullViewport != 0)
    {
        ImGuiViewport* viewport = FindViewportByID(g.DragDropTargetFullViewport);
        IM_ASSERT(viewport != NULL);
        ImRect bb = g.DragDropTargetRect;
        bb.Expand(-3.5f);
        RenderDragDropTargetRectEx(GetForegroundDrawList(viewport), bb);
    }
    else if (draw_target_rect)
    {
        RenderDragDropTargetRectForItem(r);
    }

    g.DragDropAcceptFrameCount = g.FrameCount;
    if ((g.DragDropSourceFlags & ImGuiDragDropFlags_SourceExtern) && g.DragDropMouseButton == -1)
        payload.Delivery = was_accepted_previously && (g.DragDropSourceFrameCount < g.FrameCount);
    else
        payload.Delivery = was_accepted_previously && !IsMouseDown(g.DragDropMouseButton); // For extern drag sources affecting OS window focus, it's easier to just test !IsMouseDown() instead of IsMouseReleased()
    if (!payload.Delivery && !(flags & ImGuiDragDropFlags_AcceptBeforeDelivery))
        return NULL;

    if (payload.Delivery)
        IMGUI_DEBUG_LOG_ACTIVEID("[dragdrop] AcceptDragDropPayload(): 0x%08X: payload delivery\n", g.DragDropTargetId);
    return &payload;
}

// FIXME-STYLE FIXME-DRAGDROP: Settle on a proper default visuals for drop target.
void ImGui::RenderDragDropTargetRectForItem(const ImRect& bb)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImRect bb_display = bb;
    bb_display.ClipWith(g.DragDropTargetClipRect); // Clip THEN expand so we have a way to visualize that target is not entirely visible.
    bb_display.Expand(g.Style.DragDropTargetPadding);
    bool push_clip_rect = !window->ClipRect.Contains(bb_display);
    if (push_clip_rect)
        window->DrawList->PushClipRectFullScreen();
    RenderDragDropTargetRectEx(window->DrawList, bb_display);
    if (push_clip_rect)
        window->DrawList->PopClipRect();
}

void ImGui::RenderDragDropTargetRectEx(ImDrawList* draw_list, const ImRect& bb)
{
    ImGuiContext& g = *GImGui;
    draw_list->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_DragDropTargetBg), g.Style.DragDropTargetRounding, 0);
    draw_list->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_DragDropTarget), g.Style.DragDropTargetRounding, 0, g.Style.DragDropTargetBorderSize);
}

const ImGuiPayload* ImGui::GetDragDropPayload()
{
    ImGuiContext& g = *GImGui;
    return (g.DragDropActive && g.DragDropPayload.DataFrameCount != -1) ? &g.DragDropPayload : NULL;
}

void ImGui::EndDragDropTarget()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(g.DragDropWithinTarget);
    g.DragDropWithinTarget = false;

    // Clear drag and drop state payload right after delivery
    if (g.DragDropPayload.Delivery)
        ClearDragDrop();
}

#endif // #ifndef IMGUI_DISABLE
