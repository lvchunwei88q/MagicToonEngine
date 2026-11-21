// Dear ImGui implementation - see LICENSE.txt.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"
#include "imgui_docking_private.h"
#include "imgui_platform_private.h"

//-----------------------------------------------------------------------------
// [SECTION] ERROR CHECKING, STATE RECOVERY
//-----------------------------------------------------------------------------
// - DebugCheckVersionAndDataLayout() (called via IMGUI_CHECKVERSION() macros)
// - ErrorCheckUsingSetCursorPosToExtendParentBoundaries()
// - ErrorCheckNewFrameSanityChecks()
// - ErrorCheckEndFrameSanityChecks()
// - ErrorRecoveryStoreState()
// - ErrorRecoveryTryToRecoverState()
// - ErrorRecoveryTryToRecoverWindowState()
// - ErrorLog()
//-----------------------------------------------------------------------------

// Verify ABI compatibility between caller code and compiled version of Dear ImGui. This helps detects some build issues.
// Called by IMGUI_CHECKVERSION().
// Verify that the type sizes are matching between the calling file's compilation unit and imgui.cpp's compilation unit
// If this triggers you have mismatched headers and compiled code versions.
// - It could be because of a build issue (using new headers with old compiled code)
// - It could be because of mismatched configuration #define, compilation settings, packing pragma etc.
//   THE CONFIGURATION SETTINGS MENTIONED IN imconfig.h MUST BE SET FOR ALL COMPILATION UNITS INVOLVED WITH DEAR IMGUI.
//   Which is why it is required you put them in your imconfig file (and NOT only before including imgui.h).
//   Otherwise it is possible that different compilation units would see different structure layout.
//   If you don't want to modify imconfig.h you can use the IMGUI_USER_CONFIG define to change filename.
bool ImGui::DebugCheckVersionAndDataLayout(const char* version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx)
{
    bool error = false;
    if (strcmp(version, IMGUI_VERSION) != 0) { error = true; IM_ASSERT(strcmp(version, IMGUI_VERSION) == 0 && "Mismatched version string!"); }
    if (sz_io    != sizeof(ImGuiIO))    { error = true; IM_ASSERT(sz_io == sizeof(ImGuiIO) && "Mismatched struct layout!"); }
    if (sz_style != sizeof(ImGuiStyle)) { error = true; IM_ASSERT(sz_style == sizeof(ImGuiStyle) && "Mismatched struct layout!"); }
    if (sz_vec2  != sizeof(ImVec2))     { error = true; IM_ASSERT(sz_vec2 == sizeof(ImVec2) && "Mismatched struct layout!"); }
    if (sz_vec4  != sizeof(ImVec4))     { error = true; IM_ASSERT(sz_vec4 == sizeof(ImVec4) && "Mismatched struct layout!"); }
    if (sz_vert  != sizeof(ImDrawVert)) { error = true; IM_ASSERT(sz_vert == sizeof(ImDrawVert) && "Mismatched struct layout!"); }
    if (sz_idx   != sizeof(ImDrawIdx))  { error = true; IM_ASSERT(sz_idx == sizeof(ImDrawIdx) && "Mismatched struct layout!"); }
    return !error;
}

// Until 1.89 (August 2022, IMGUI_VERSION_NUM < 18814) it was legal to use SetCursorPos()/SetCursorScreenPos()
// to extend contents size of our parent container (e.g. window contents size, which is used for auto-resizing
// windows, table column contents size used for auto-resizing columns, group size).
// This was causing issues and ambiguities and we needed to retire that.
// 2022/08/05 (1.89): extending contents size boundaries REQUIRES AN ITEM TO BE SUBMITTED. However we gated the new logic behind a '#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS' block.
// 2025/06/25 (1.92): removed the legacy path and turned into an assert. It was a mistake that there was a #ifndef before: our obsolescence schedule gets pushed back a bit more :(
//
//  Previously this would make the window content size ~200x200:
//    Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + ImVec2(200,200)) + End();                      // NOT OK ANYMORE
//  Instead, please submit an item:
//    Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + ImVec2(200,200)) + Dummy(ImVec2(0,0)) + End(); // OK
//  Alternative:
//    Begin(...) + Dummy(ImVec2(200,200)) + End(); // OK
//
// The assert below detects when the _last_ call in a window was a SetCursorPos() not followed by an Item,
// and with a position that would grow the parent contents size.
//
// Advanced:
// - For reference, old logic was causing issues because it meant that SetCursorScreenPos(GetCursorScreenPos())
//   had a side-effect on layout! In particular this caused problem to compute group boundaries.
//   e.g. BeginGroup() + SomeItem() + SetCursorScreenPos(GetCursorScreenPos()) + EndGroup() would cause the
//   group to be taller because auto-sizing generally adds padding on bottom and right side.
// - While this code is a little twisted, no-one would expect SetXXX(GetXXX()) to have a side-effect.
//   Using vertical alignment patterns would frequently trigger this sorts of issue.
// - See https://github.com/ocornut/imgui/issues/5548 for more details.
void ImGui::ErrorCheckUsingSetCursorPosToExtendParentBoundaries()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(window->DC.IsSetPos);
    window->DC.IsSetPos = false;
    if (window->DC.CursorPos.x <= window->DC.CursorMaxPos.x && window->DC.CursorPos.y <= window->DC.CursorMaxPos.y)
        return;
    if (window->SkipItems)
        return;
    IM_ASSERT_USER_ERROR(0, "Code uses SetCursorPos()/SetCursorScreenPos() to extend window/parent boundaries.\nPlease submit an item e.g. Dummy() afterwards in order to grow window/parent boundaries.");

    // For reference, the old behavior was essentially:
    //window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
}

void ImGui::ErrorCheckNewFrameSanityChecks()
{
    ImGuiContext& g = *GImGui;

    // Check user IM_ASSERT macro
    // (IF YOU GET A WARNING OR COMPILE ERROR HERE: it means your assert macro is incorrectly defined!
    //  If your macro uses multiple statements, it NEEDS to be surrounded by a 'do { ... } while (0)' block.
    //  This is a common C/C++ idiom to allow multiple statements macros to be used in control flow blocks.)
    // #define IM_ASSERT(EXPR)   if (SomeCode(EXPR)) SomeMoreCode();                    // Wrong!
    // #define IM_ASSERT(EXPR)   do { if (SomeCode(EXPR)) SomeMoreCode(); } while (0)   // Correct!
    if (true) IM_ASSERT(1); else IM_ASSERT(0);

    // Emscripten backends are often imprecise in their submission of DeltaTime. (#6114, #3644)
    // Ideally the Emscripten app/backend should aim to fix or smooth this value and avoid feeding zero, but we tolerate it.
#ifdef __EMSCRIPTEN__
    if (g.IO.DeltaTime <= 0.0f && g.FrameCount > 0)
        g.IO.DeltaTime = 0.00001f;
#endif

    // Check user data
    // (We pass an error message in the assert expression to make it visible to programmers who are not using a debugger, as most assert handlers display their argument)
    IM_ASSERT(g.Initialized);
    IM_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0)              && "Need a positive DeltaTime!");
    IM_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount)  && "Forgot to call Render() or EndFrame() at the end of the previous frame?");
    IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f  && "Invalid DisplaySize value!");
    IM_ASSERT(g.Style.CurveTessellationTol > 0.0f                       && "Invalid style setting!");
    IM_ASSERT(g.Style.CircleTessellationMaxError > 0.0f                 && "Invalid style setting!");
    IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f            && "Invalid style setting!"); // Allows us to avoid a few clamps in color computations
    IM_ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f && "Invalid style setting!");
    IM_ASSERT(g.Style.WindowBorderHoverPadding > 0.0f                   && "Invalid style setting!"); // Required otherwise cannot resize from borders.
    IM_ASSERT(g.Style.WindowMenuButtonPosition == ImGuiDir_None || g.Style.WindowMenuButtonPosition == ImGuiDir_Left || g.Style.WindowMenuButtonPosition == ImGuiDir_Right);
    IM_ASSERT(g.Style.ColorButtonPosition == ImGuiDir_Left || g.Style.ColorButtonPosition == ImGuiDir_Right);
    IM_ASSERT(g.Style.TreeLinesFlags == ImGuiTreeNodeFlags_DrawLinesNone || g.Style.TreeLinesFlags == ImGuiTreeNodeFlags_DrawLinesFull || g.Style.TreeLinesFlags == ImGuiTreeNodeFlags_DrawLinesToNodes);

    // Error handling: we do not accept 100% silent recovery! Please contact me if you feel this is getting in your way.
    if (g.IO.ConfigErrorRecovery)
        IM_ASSERT(g.IO.ConfigErrorRecoveryEnableAssert || g.IO.ConfigErrorRecoveryEnableDebugLog || g.IO.ConfigErrorRecoveryEnableTooltip || g.ErrorCallback != NULL);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    if (g.IO.FontGlobalScale > 1.0f)
        IM_ASSERT(g.Style.FontScaleMain == 1.0f && "Since 1.92: use style.FontScaleMain instead of g.IO.FontGlobalScale!");

    // Remap legacy names
    if (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)
    {
        g.IO.ConfigNavMoveSetMousePos = true;
        g.IO.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;
    }
    if (g.IO.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)
    {
        g.IO.ConfigNavCaptureKeyboard = false;
        g.IO.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
    }
    if (g.IO.ConfigFlags & ImGuiConfigFlags_DpiEnableScaleFonts)
    {
        g.IO.ConfigDpiScaleFonts = true;
        g.IO.ConfigFlags &= ~ImGuiConfigFlags_DpiEnableScaleFonts;
    }
    if (g.IO.ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
    {
        g.IO.ConfigDpiScaleViewports = true;
        g.IO.ConfigFlags &= ~ImGuiConfigFlags_DpiEnableScaleViewports;
    }

    // Remap legacy clipboard handlers (OBSOLETED in 1.91.1, August 2024)
    if (g.IO.GetClipboardTextFn != NULL && (g.PlatformIO.Platform_GetClipboardTextFn == NULL || g.PlatformIO.Platform_GetClipboardTextFn == Platform_GetClipboardTextFn_DefaultImpl))
        g.PlatformIO.Platform_GetClipboardTextFn = [](ImGuiContext* ctx) { return ctx->IO.GetClipboardTextFn(ctx->IO.ClipboardUserData); };
    if (g.IO.SetClipboardTextFn != NULL && (g.PlatformIO.Platform_SetClipboardTextFn == NULL || g.PlatformIO.Platform_SetClipboardTextFn == Platform_SetClipboardTextFn_DefaultImpl))
        g.PlatformIO.Platform_SetClipboardTextFn = [](ImGuiContext* ctx, const char* text) { return ctx->IO.SetClipboardTextFn(ctx->IO.ClipboardUserData, text); };
#endif

    // Perform simple check: error if Docking or Viewport are enabled _exactly_ on frame 1 (instead of frame 0 or later), which is a common error leading to loss of .ini data.
    if (g.FrameCount == 1 && (g.IO.ConfigFlags & ImGuiConfigFlags_DockingEnable) && (g.ConfigFlagsLastFrame & ImGuiConfigFlags_DockingEnable) == 0)
        IM_ASSERT(0 && "Please set DockingEnable before the first call to NewFrame()! Otherwise you will lose your .ini settings!");
    if (g.FrameCount == 1 && (g.IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && (g.ConfigFlagsLastFrame & ImGuiConfigFlags_ViewportsEnable) == 0)
        IM_ASSERT(0 && "Please set ViewportsEnable before the first call to NewFrame()! Otherwise you will lose your .ini settings!");

    // Perform simple checks: multi-viewport and platform windows support
    if (g.IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        if ((g.IO.BackendFlags & ImGuiBackendFlags_PlatformHasViewports) && (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasViewports))
        {
            IM_ASSERT((g.FrameCount == 0 || g.FrameCount == g.FrameCountPlatformEnded) && "Forgot to call UpdatePlatformWindows() in main loop after EndFrame()? Check examples/ applications for reference.");
            IM_ASSERT(g.PlatformIO.Platform_CreateWindow  != NULL && "Platform init didn't install handlers?");
            IM_ASSERT(g.PlatformIO.Platform_DestroyWindow != NULL && "Platform init didn't install handlers?");
            IM_ASSERT(g.PlatformIO.Platform_GetWindowPos  != NULL && "Platform init didn't install handlers?");
            IM_ASSERT(g.PlatformIO.Platform_SetWindowPos  != NULL && "Platform init didn't install handlers?");
            IM_ASSERT(g.PlatformIO.Platform_GetWindowSize != NULL && "Platform init didn't install handlers?");
            IM_ASSERT(g.PlatformIO.Platform_SetWindowSize != NULL && "Platform init didn't install handlers?");
            IM_ASSERT(g.PlatformIO.Monitors.Size > 0 && "Platform init didn't setup Monitors list?");
            IM_ASSERT((g.Viewports[0]->PlatformUserData != NULL || g.Viewports[0]->PlatformHandle != NULL) && "Platform init didn't setup main viewport.");
            if (g.IO.ConfigDockingTransparentPayload && (g.IO.ConfigFlags & ImGuiConfigFlags_DockingEnable))
                IM_ASSERT(g.PlatformIO.Platform_SetWindowAlpha != NULL && "Platform_SetWindowAlpha handler is required to use io.ConfigDockingTransparent!");
        }
        else
        {
            // Disable feature, our backends do not support it
            g.IO.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
        }

        // Perform simple checks on platform monitor data + compute a total bounding box for quick early outs
        for (ImGuiPlatformMonitor& mon : g.PlatformIO.Monitors)
        {
            IM_UNUSED(mon);
            IM_ASSERT(mon.MainSize.x > 0.0f && mon.MainSize.y > 0.0f && "Monitor main bounds not setup properly.");
            IM_ASSERT(ImRect(mon.MainPos, mon.MainPos + mon.MainSize).Contains(ImRect(mon.WorkPos, mon.WorkPos + mon.WorkSize)) && "Monitor work bounds not setup properly. If you don't have work area information, just copy MainPos/MainSize into them.");
            IM_ASSERT(mon.DpiScale > 0.0f && mon.DpiScale < 99.0f && "Monitor DpiScale is invalid."); // Typical correct values would be between 1.0f and 4.0f
        }
    }
}

void ImGui::ErrorCheckEndFrameSanityChecks()
{
    // Verify that io.KeyXXX fields haven't been tampered with. Key mods should not be modified between NewFrame() and EndFrame()
    // One possible reason leading to this assert is that your backends update inputs _AFTER_ NewFrame().
    // It is known that when some modal native windows called mid-frame takes focus away, some backends such as GLFW will
    // send key release events mid-frame. This would normally trigger this assertion and lead to sheared inputs.
    // We silently accommodate for this case by ignoring the case where all io.KeyXXX modifiers were released (aka key_mod_flags == 0),
    // while still correctly asserting on mid-frame key press events.
    ImGuiContext& g = *GImGui;
    const ImGuiKeyChord key_mods = GetMergedModsFromKeys();
    IM_UNUSED(g);
    IM_UNUSED(key_mods);
    IM_ASSERT((key_mods == 0 || g.IO.KeyMods == key_mods) && "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
    IM_UNUSED(key_mods);

    IM_ASSERT(g.CurrentWindowStack.Size == 1);
    IM_ASSERT(g.CurrentWindowStack[0].Window->IsFallbackWindow);
}

// Save current stack sizes. Called e.g. by NewFrame() and by Begin() but may be called for manual recovery.
void ImGui::ErrorRecoveryStoreState(ImGuiErrorRecoveryState* state_out)
{
    ImGuiContext& g = *GImGui;
    state_out->SizeOfWindowStack = (short)g.CurrentWindowStack.Size;
    state_out->SizeOfIDStack = (short)g.CurrentWindow->IDStack.Size;
    state_out->SizeOfTreeStack = (short)g.CurrentWindow->DC.TreeDepth; // NOT g.TreeNodeStack.Size which is a partial stack!
    state_out->SizeOfColorStack = (short)g.ColorStack.Size;
    state_out->SizeOfStyleVarStack = (short)g.StyleVarStack.Size;
    state_out->SizeOfFontStack = (short)g.FontStack.Size;
    state_out->SizeOfFocusScopeStack = (short)g.FocusScopeStack.Size;
    state_out->SizeOfGroupStack = (short)g.GroupStack.Size;
    state_out->SizeOfItemFlagsStack = (short)g.ItemFlagsStack.Size;
    state_out->SizeOfBeginPopupStack = (short)g.BeginPopupStack.Size;
    state_out->SizeOfDisabledStack = (short)g.DisabledStackSize;
}

// Chosen name "Try to recover" over e.g. "Restore" to suggest this is not a 100% guaranteed recovery.
// Called by e.g. EndFrame() but may be called for manual recovery.
// Attempt to recover full window stack.
void ImGui::ErrorRecoveryTryToRecoverState(const ImGuiErrorRecoveryState* state_in)
{
    // PVS-Studio V1044 is "Loop break conditions do not depend on the number of iterations"
    ImGuiContext& g = *GImGui;
    while (g.CurrentWindowStack.Size > state_in->SizeOfWindowStack) //-V1044
    {
        // Recap:
        // - Begin()/BeginChild() return false to indicate the window is collapsed or fully clipped.
        // - Always call a matching End() for each Begin() call, regardless of its return value!
        // - Begin/End and BeginChild/EndChild logic is KNOWN TO BE INCONSISTENT WITH ALL OTHER BEGIN/END FUNCTIONS.
        // - We will fix that in a future major update.
        ImGuiWindow* window = g.CurrentWindow;
        if (window->Flags & ImGuiWindowFlags_ChildWindow)
        {
            if (g.CurrentTable != NULL && g.CurrentTable->InnerWindow == g.CurrentWindow)
            {
                IM_ASSERT_USER_ERROR(0, "Missing EndTable()");
                EndTable();
            }
            else
            {
                IM_ASSERT_USER_ERROR(0, "Missing EndChild()");
                EndChild();
            }
        }
        else
        {
            IM_ASSERT_USER_ERROR(0, "Missing End()");
            End();
        }
    }
    if (g.CurrentWindowStack.Size == state_in->SizeOfWindowStack)
        ErrorRecoveryTryToRecoverWindowState(state_in);
}

// Called by e.g. End() but may be called for manual recovery.
// Read '// Error Handling [BETA]' block in imgui_internal.h for details.
// Attempt to recover from incorrect usage of BeginXXX/EndXXX/PushXXX/PopXXX calls.
void    ImGui::ErrorRecoveryTryToRecoverWindowState(const ImGuiErrorRecoveryState* state_in)
{
    ImGuiContext& g = *GImGui;

    while (g.CurrentTable != NULL && g.CurrentTable->InnerWindow == g.CurrentWindow) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndTable()");
        EndTable();
    }

    ImGuiWindow* window = g.CurrentWindow;

    // FIXME: Can't recover from inside BeginTabItem/EndTabItem yet.
    while (g.CurrentTabBar != NULL && g.CurrentTabBar->Window == window) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndTabBar()");
        EndTabBar();
    }
    while (g.CurrentMultiSelect != NULL && g.CurrentMultiSelect->Storage->Window == window) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndMultiSelect()");
        EndMultiSelect();
    }
    if (window->DC.MenuBarAppending) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndMenuBar()");
        EndMenuBar();
    }
    while (window->DC.TreeDepth > state_in->SizeOfTreeStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing TreePop()");
        TreePop();
    }
    while (g.GroupStack.Size > state_in->SizeOfGroupStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndGroup()");
        EndGroup();
    }
    IM_ASSERT(g.GroupStack.Size == state_in->SizeOfGroupStack);
    while (window->IDStack.Size > state_in->SizeOfIDStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopID()");
        PopID();
    }
    while (g.DisabledStackSize > state_in->SizeOfDisabledStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndDisabled()");
        if (g.CurrentItemFlags & ImGuiItemFlags_Disabled)
            EndDisabled();
        else
        {
            EndDisabledOverrideReenable();
            g.CurrentWindowStack.back().DisabledOverrideReenable = false;
        }
    }
    IM_ASSERT(g.DisabledStackSize == state_in->SizeOfDisabledStack);
    while (g.ColorStack.Size > state_in->SizeOfColorStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopStyleColor()");
        PopStyleColor();
    }
    while (g.ItemFlagsStack.Size > state_in->SizeOfItemFlagsStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopItemFlag()");
        PopItemFlag();
    }
    while (g.StyleVarStack.Size > state_in->SizeOfStyleVarStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopStyleVar()");
        PopStyleVar();
    }
    while (g.FontStack.Size > state_in->SizeOfFontStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopFont()");
        PopFont();
    }
    while (g.FocusScopeStack.Size > state_in->SizeOfFocusScopeStack) //-V1044
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopFocusScope()");
        PopFocusScope();
    }
    //IM_ASSERT(g.FocusScopeStack.Size == state_in->SizeOfFocusScopeStack);
}

bool    ImGui::ErrorLog(const char* msg)
{
    ImGuiContext& g = *GImGui;

    // Output to debug log
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiWindow* window = g.CurrentWindow;

    if (g.IO.ConfigErrorRecoveryEnableDebugLog)
    {
        if (g.ErrorFirst)
            IMGUI_DEBUG_LOG_ERROR("[imgui-error] (current settings: Assert=%d, Log=%d, Tooltip=%d)\n",
                g.IO.ConfigErrorRecoveryEnableAssert, g.IO.ConfigErrorRecoveryEnableDebugLog, g.IO.ConfigErrorRecoveryEnableTooltip);
        IMGUI_DEBUG_LOG_ERROR("[imgui-error] In window '%s': %s\n", window ? window->Name : "NULL", msg);
    }
    g.ErrorFirst = false;

    // Output to tooltip
    if (g.IO.ConfigErrorRecoveryEnableTooltip)
    {
        if (g.WithinFrameScope && BeginErrorTooltip())
        {
            if (g.ErrorCountCurrentFrame < 20)
            {
                Text("In window '%s': %s", window ? window->Name : "NULL", msg);
                if (window && (!window->IsFallbackWindow || window->WasActive))
                    GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 0, 0, 255));
            }
            if (g.ErrorCountCurrentFrame == 20)
                Text("(and more errors)");
            // EndFrame() will amend debug buttons to this window, after all errors have been submitted.
            EndErrorTooltip();
        }
        g.ErrorCountCurrentFrame++;
    }
#endif

    // Output to callback
    if (g.ErrorCallback != NULL)
        g.ErrorCallback(&g, g.ErrorCallbackUserData, msg);

    // Return whether we should assert
    return g.IO.ConfigErrorRecoveryEnableAssert;
}

// Display an error tooltip when same ID as HoveredId was submitted multiple times.
// See code in ItemHoverable() for an explanation of why we associate this error to HoveredId + code drawing of rectangles over individual items instances.
void ImGui::ErrorCheckEndFrameFinalizeErrorTooltip()
{
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugDrawIdConflictsId != 0 && g.IO.KeyCtrl == false)
        g.DebugDrawIdConflictsCount = g.HoveredIdPreviousFrameItemCount;
    if (g.DebugDrawIdConflictsId != 0 && g.DebugItemPickerActive == false && BeginErrorTooltip())
    {
        Text("Programmer error: %d visible items with conflicting ID!", g.DebugDrawIdConflictsCount);
        BulletText("Code should use PushID()/PopID() in loops, or append \"##xx\" to same-label identifiers!");
        BulletText("Empty label e.g. Button(\"\") == same ID as parent widget/node. Use Button(\"##xx\") instead!");
        //BulletText("Code intending to use duplicate ID may use e.g. PushItemFlag(ImGuiItemFlags_AllowDuplicateId, true); ... PopItemFlag()"); // Not making this too visible for fear of it being abused.
        BulletText("Set io.ConfigDebugHighlightIdConflicts=false to disable this warning in non-programmers builds.");
        Separator();
        if (g.IO.ConfigDebugHighlightIdConflictsShowItemPicker)
        {
            Text("(Hold Ctrl to: use ");
            SameLine(0.0f, 0.0f);
            if (SmallButton("Item Picker"))
                DebugStartItemPicker();
            SameLine(0.0f, 0.0f);
            Text(" to break in item call-stack, or ");
        }
        else
        {
            Text("(Hold Ctrl to: ");
        }
        SameLine(0.0f, 0.0f);
        TextLinkOpenURL("read FAQ \"About ID Stack System\"", "https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#qa-usage");
        SameLine(0.0f, 0.0f);
        Text(")");
        EndErrorTooltip();
    }

    if (g.ErrorCountCurrentFrame > 0 && BeginErrorTooltip()) // Amend at end of frame
    {
        Separator();
        Text("(Hold Ctrl to: ");
        SameLine(0.0f, 0.0f);
        if (SmallButton("Enable Asserts"))
            g.IO.ConfigErrorRecoveryEnableAssert = true;
        //SameLine();
        //if (SmallButton("Hide Error Tooltips"))
        //    g.IO.ConfigErrorRecoveryEnableTooltip = false; // Too dangerous
        SameLine(0, 0);
        Text(")");
        EndErrorTooltip();
    }
#endif
}

// Pseudo-tooltip. Follow mouse until Ctrl is held. When Ctrl is held we lock position, allowing to click it.
bool ImGui::BeginErrorTooltip()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = FindWindowByName("##Tooltip_Error");
    const bool use_locked_pos = (g.IO.KeyCtrl && window && window->WasActive);
    PushStyleColor(ImGuiCol_PopupBg, ImLerp(g.Style.Colors[ImGuiCol_PopupBg], ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0.15f));
    if (use_locked_pos)
        SetNextWindowPos(g.ErrorTooltipLockedPos);
    bool is_visible = Begin("##Tooltip_Error", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    PopStyleColor();
    if (is_visible && g.CurrentWindow->BeginCount == 1)
    {
        SeparatorText("MESSAGE FROM DEAR IMGUI");
        BringWindowToDisplayFront(g.CurrentWindow);
        BringWindowToFocusFront(g.CurrentWindow);
        g.ErrorTooltipLockedPos = GetWindowPos();
    }
    else if (!is_visible)
    {
        End();
    }
    return is_visible;
}

void ImGui::EndErrorTooltip()
{
    End();
}

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
static const char* GetInputSourceName(ImGuiInputSource source)
{
    const char* input_source_names[] = { "None", "Mouse", "Keyboard", "Gamepad" };
    IM_ASSERT(IM_COUNTOF(input_source_names) == ImGuiInputSource_COUNT);
    if (source < 0 || source >= ImGuiInputSource_COUNT)
        return "Unknown";
    return input_source_names[source];
}
static const char* GetMouseSourceName(ImGuiMouseSource source)
{
    const char* mouse_source_names[] = { "Mouse", "TouchScreen", "Pen" };
    IM_ASSERT(IM_COUNTOF(mouse_source_names) == ImGuiMouseSource_COUNT);
    if (source < 0 || source >= ImGuiMouseSource_COUNT)
        return "Unknown";
    return mouse_source_names[source];
}
void ImGui::DebugPrintInputEvent(const char* prefix, const ImGuiInputEvent* e)
{
    ImGuiContext& g = *GImGui;
    char buf[5];
    if (e->Type == ImGuiInputEventType_MousePos)    { if (e->MousePos.PosX == -FLT_MAX && e->MousePos.PosY == -FLT_MAX) IMGUI_DEBUG_LOG_IO("[io] %s: MousePos (-FLT_MAX, -FLT_MAX)\n", prefix); else IMGUI_DEBUG_LOG_IO("[io] %s: MousePos (%.1f, %.1f) (%s)\n", prefix, e->MousePos.PosX, e->MousePos.PosY, GetMouseSourceName(e->MousePos.MouseSource)); return; }
    if (e->Type == ImGuiInputEventType_MouseButton) { IMGUI_DEBUG_LOG_IO("[io] %s: MouseButton %d %s (%s)\n", prefix, e->MouseButton.Button, e->MouseButton.Down ? "Down" : "Up", GetMouseSourceName(e->MouseButton.MouseSource)); return; }
    if (e->Type == ImGuiInputEventType_MouseWheel)  { IMGUI_DEBUG_LOG_IO("[io] %s: MouseWheel (%.3f, %.3f) (%s)\n", prefix, e->MouseWheel.WheelX, e->MouseWheel.WheelY, GetMouseSourceName(e->MouseWheel.MouseSource)); return; }
    if (e->Type == ImGuiInputEventType_MouseViewport){IMGUI_DEBUG_LOG_IO("[io] %s: MouseViewport (0x%08X)\n", prefix, e->MouseViewport.HoveredViewportID); return; }
    if (e->Type == ImGuiInputEventType_Key)         { IMGUI_DEBUG_LOG_IO("[io] %s: Key \"%s\" %s\n", prefix, ImGui::GetKeyName(e->Key.Key), e->Key.Down ? "Down" : "Up"); return; }
    if (e->Type == ImGuiInputEventType_Text)        { ImTextCharToUtf8(buf, e->Text.Char); IMGUI_DEBUG_LOG_IO("[io] %s: Text: '%s' (U+%08X)\n", prefix, buf, e->Text.Char); return; }
    if (e->Type == ImGuiInputEventType_Focus)       { IMGUI_DEBUG_LOG_IO("[io] %s: AppFocused %d\n", prefix, e->AppFocused.Focused); return; }
}
#endif

//-----------------------------------------------------------------------------
// [SECTION] METRICS/DEBUGGER WINDOW
//-----------------------------------------------------------------------------
// - MetricsHelpMarker() [Internal]
// - DebugRenderViewportThumbnail() [Internal]
// - RenderViewportsThumbnails() [Internal]
// - DebugRenderKeyboardPreview() [Internal]
// - DebugTextEncoding()
// - DebugFlashStyleColorStop() [Internal]
// - DebugFlashStyleColor()
// - UpdateDebugToolFlashStyleColor() [Internal]
// - ShowFontAtlas() [Internal but called by Demo!]
// - DebugNodeTexture() [Internal]
// - ShowMetricsWindow()
// - DebugNodeColumns() [Internal]
// - DebugNodeDockNode() [Internal]
// - DebugNodeDrawList() [Internal]
// - DebugNodeDrawCmdShowMeshAndBoundingBox() [Internal]
// - DebugNodeFont() [Internal]
// - DebugNodeFontGlyph() [Internal]
// - DebugNodeStorage() [Internal]
// - DebugNodeTabBar() [Internal]
// - DebugNodeViewport() [Internal]
// - DebugNodeWindow() [Internal]
// - DebugNodeWindowSettings() [Internal]
// - DebugNodeWindowsList() [Internal]
// - DebugNodeWindowsListByBeginStackParent() [Internal]
// - ShowFontSelector()
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS) || !defined(IMGUI_DISABLE_DEBUG_TOOLS)
// Avoid naming collision with imgui_demo.cpp's HelpMarker() for unity builds.
static void MetricsHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
#endif

#ifndef IMGUI_DISABLE_DEBUG_TOOLS

void ImGui::DebugRenderViewportThumbnail(ImDrawList* draw_list, ImGuiViewportP* viewport, const ImRect& bb)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    ImVec2 scale = bb.GetSize() / viewport->Size;
    ImVec2 off = bb.Min - viewport->Pos * scale;
    float alpha_mul = (viewport->Flags & ImGuiViewportFlags_IsMinimized) ? 0.30f : 1.00f;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border, alpha_mul * 0.40f));
    for (ImGuiWindow* thumb_window : g.Windows)
    {
        if (!thumb_window->WasActive || (thumb_window->Flags & ImGuiWindowFlags_ChildWindow))
            continue;
        if (thumb_window->Viewport != viewport)
            continue;

        ImRect thumb_r = thumb_window->Rect();
        ImRect title_r = thumb_window->TitleBarRect();
        thumb_r = ImRect(ImTrunc(off + thumb_r.Min * scale), ImTrunc(off +  thumb_r.Max * scale));
        title_r = ImRect(ImTrunc(off + title_r.Min * scale), ImTrunc(off +  ImVec2(title_r.Max.x, title_r.Min.y + title_r.GetHeight() * 3.0f) * scale)); // Exaggerate title bar height
        thumb_r.ClipWithFull(bb);
        title_r.ClipWithFull(bb);
        const bool window_is_focused = (g.NavWindow && thumb_window->RootWindowForTitleBarHighlight == g.NavWindow->RootWindowForTitleBarHighlight);
        window->DrawList->AddRectFilled(thumb_r.Min, thumb_r.Max, GetColorU32(ImGuiCol_WindowBg, alpha_mul));
        window->DrawList->AddRectFilled(title_r.Min, title_r.Max, GetColorU32(window_is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg, alpha_mul));
        window->DrawList->AddRect(thumb_r.Min, thumb_r.Max, GetColorU32(ImGuiCol_Border, alpha_mul));
        window->DrawList->AddText(g.Font, g.FontSize * 1.0f, title_r.Min, GetColorU32(ImGuiCol_Text, alpha_mul), thumb_window->Name, FindRenderedTextEnd(thumb_window->Name));
    }
    draw_list->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border, alpha_mul));
    if (viewport->ID == g.DebugMetricsConfig.HighlightViewportID)
        window->DrawList->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 0, 255));
}

static void RenderViewportsThumbnails()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Draw monitor and calculate their boundaries
    float SCALE = 1.0f / 8.0f;
    ImRect bb_full(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (ImGuiPlatformMonitor& monitor : g.PlatformIO.Monitors)
        bb_full.Add(ImRect(monitor.MainPos, monitor.MainPos + monitor.MainSize));
    ImVec2 p = window->DC.CursorPos;
    ImVec2 off = p - bb_full.Min * SCALE;
    for (ImGuiPlatformMonitor& monitor : g.PlatformIO.Monitors)
    {
        ImRect monitor_draw_bb(off + (monitor.MainPos) * SCALE, off + (monitor.MainPos + monitor.MainSize) * SCALE);
        window->DrawList->AddRect(monitor_draw_bb.Min, monitor_draw_bb.Max, (g.DebugMetricsConfig.HighlightMonitorIdx == g.PlatformIO.Monitors.index_from_ptr(&monitor)) ? IM_COL32(255, 255, 0, 255) : ImGui::GetColorU32(ImGuiCol_Border), 4.0f);
        window->DrawList->AddRectFilled(monitor_draw_bb.Min, monitor_draw_bb.Max, ImGui::GetColorU32(ImGuiCol_Border, 0.10f), 4.0f);
    }

    // Draw viewports
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        ImRect viewport_draw_bb(off + (viewport->Pos) * SCALE, off + (viewport->Pos + viewport->Size) * SCALE);
        ImGui::DebugRenderViewportThumbnail(window->DrawList, viewport, viewport_draw_bb);
    }
    ImGui::Dummy(bb_full.GetSize() * SCALE);
}

static int IMGUI_CDECL ViewportComparerByLastFocusedStampCount(const void* lhs, const void* rhs)
{
    const ImGuiViewportP* a = *(const ImGuiViewportP* const*)lhs;
    const ImGuiViewportP* b = *(const ImGuiViewportP* const*)rhs;
    return b->LastFocusedStampCount - a->LastFocusedStampCount;
}

// Draw an arbitrary US keyboard layout to visualize translated keys
void ImGui::DebugRenderKeyboardPreview(ImDrawList* draw_list)
{
    const float scale = ImGui::GetFontSize() / 13.0f;
    const ImVec2 key_size = ImVec2(35.0f, 35.0f) * scale;
    const float  key_rounding = 3.0f * scale;
    const ImVec2 key_face_size = ImVec2(25.0f, 25.0f) * scale;
    const ImVec2 key_face_pos = ImVec2(5.0f, 3.0f) * scale;
    const float  key_face_rounding = 2.0f * scale;
    const ImVec2 key_label_pos = ImVec2(7.0f, 4.0f) * scale;
    const ImVec2 key_step = ImVec2(key_size.x - 1.0f, key_size.y - 1.0f);
    const float  key_row_offset = 9.0f * scale;

    ImVec2 board_min = GetCursorScreenPos();
    ImVec2 board_max = ImVec2(board_min.x + 3 * key_step.x + 2 * key_row_offset + 10.0f, board_min.y + 3 * key_step.y + 10.0f);
    ImVec2 start_pos = ImVec2(board_min.x + 5.0f - key_step.x, board_min.y);

    struct KeyLayoutData { int Row, Col; const char* Label; ImGuiKey Key; };
    const KeyLayoutData keys_to_display[] =
    {
        { 0, 0, "", ImGuiKey_Tab },      { 0, 1, "Q", ImGuiKey_Q }, { 0, 2, "W", ImGuiKey_W }, { 0, 3, "E", ImGuiKey_E }, { 0, 4, "R", ImGuiKey_R },
        { 1, 0, "", ImGuiKey_CapsLock }, { 1, 1, "A", ImGuiKey_A }, { 1, 2, "S", ImGuiKey_S }, { 1, 3, "D", ImGuiKey_D }, { 1, 4, "F", ImGuiKey_F },
        { 2, 0, "", ImGuiKey_LeftShift },{ 2, 1, "Z", ImGuiKey_Z }, { 2, 2, "X", ImGuiKey_X }, { 2, 3, "C", ImGuiKey_C }, { 2, 4, "V", ImGuiKey_V }
    };

    // Elements rendered manually via ImDrawList API are not clipped automatically.
    // While not strictly necessary, here IsItemVisible() is used to avoid rendering these shapes when they are out of view.
    Dummy(board_max - board_min);
    if (!IsItemVisible())
        return;
    draw_list->PushClipRect(board_min, board_max, true);
    for (int n = 0; n < IM_COUNTOF(keys_to_display); n++)
    {
        const KeyLayoutData* key_data = &keys_to_display[n];
        ImVec2 key_min = ImVec2(start_pos.x + key_data->Col * key_step.x + key_data->Row * key_row_offset, start_pos.y + key_data->Row * key_step.y);
        ImVec2 key_max = key_min + key_size;
        draw_list->AddRectFilled(key_min, key_max, IM_COL32(204, 204, 204, 255), key_rounding);
        draw_list->AddRect(key_min, key_max, IM_COL32(24, 24, 24, 255), key_rounding);
        ImVec2 face_min = ImVec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
        ImVec2 face_max = ImVec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
        draw_list->AddRect(face_min, face_max, IM_COL32(193, 193, 193, 255), key_face_rounding, ImDrawFlags_None, 2.0f);
        draw_list->AddRectFilled(face_min, face_max, IM_COL32(252, 252, 252, 255), key_face_rounding);
        ImVec2 label_min = ImVec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
        draw_list->AddText(label_min, IM_COL32(64, 64, 64, 255), key_data->Label);
        if (IsKeyDown(key_data->Key))
            draw_list->AddRectFilled(key_min, key_max, IM_COL32(255, 0, 0, 128), key_rounding);
    }
    draw_list->PopClipRect();
}

// Helper tool to diagnose between text encoding issues and font loading issues. Pass your UTF-8 string and verify that there are correct.
void ImGui::DebugTextEncoding(const char* str)
{
    Text("Text: \"%s\"", str);
    if (!BeginTable("##DebugTextEncoding", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable))
        return;
    TableSetupColumn("Offset");
    TableSetupColumn("UTF-8");
    TableSetupColumn("Glyph");
    TableSetupColumn("Codepoint");
    TableHeadersRow();
    const char* str_end = str + strlen(str); // As we may receive malformed UTF-8, pass an explicit end instead of relying on ImTextCharFromUtf8() assuming enough space.
    for (const char* p = str; *p != 0; )
    {
        unsigned int c;
        const int c_utf8_len = ImTextCharFromUtf8(&c, p, str_end);
        TableNextColumn();
        Text("%d", (int)(p - str));
        TableNextColumn();
        for (int byte_index = 0; byte_index < c_utf8_len; byte_index++)
        {
            if (byte_index > 0)
                SameLine();
            Text("0x%02X", (int)(unsigned char)p[byte_index]);
        }
        TableNextColumn();
        TextUnformatted(p, p + c_utf8_len);
        if (!GetFont()->IsGlyphInFont((ImWchar)c))
        {
            SameLine();
            TextUnformatted("[missing]");
        }
        TableNextColumn();
        Text("U+%04X", (int)c);
        p += c_utf8_len;
    }
    EndTable();
}

static void DebugFlashStyleColorStop()
{
    ImGuiContext& g = *GImGui;
    if (g.DebugFlashStyleColorIdx != ImGuiCol_COUNT)
        g.Style.Colors[g.DebugFlashStyleColorIdx] = g.DebugFlashStyleColorBackup;
    g.DebugFlashStyleColorIdx = ImGuiCol_COUNT;
}

// Flash a given style color for some + inhibit modifications of this color via PushStyleColor() calls.
void ImGui::DebugFlashStyleColor(ImGuiCol idx)
{
    ImGuiContext& g = *GImGui;
    DebugFlashStyleColorStop();
    g.DebugFlashStyleColorTime = 0.5f;
    g.DebugFlashStyleColorIdx = idx;
    g.DebugFlashStyleColorBackup = g.Style.Colors[idx];
}

void ImGui::UpdateDebugToolFlashStyleColor()
{
    ImGuiContext& g = *GImGui;
    if (g.DebugFlashStyleColorTime <= 0.0f)
        return;
    ColorConvertHSVtoRGB(ImCos(g.DebugFlashStyleColorTime * 6.0f) * 0.5f + 0.5f, 0.5f, 0.5f, g.Style.Colors[g.DebugFlashStyleColorIdx].x, g.Style.Colors[g.DebugFlashStyleColorIdx].y, g.Style.Colors[g.DebugFlashStyleColorIdx].z);
    g.Style.Colors[g.DebugFlashStyleColorIdx].w = 1.0f;
    if ((g.DebugFlashStyleColorTime -= g.IO.DeltaTime) <= 0.0f)
        DebugFlashStyleColorStop();
}

ImU64 ImGui::DebugTextureIDToU64(ImTextureID tex_id)
{
    ImU64 v = 0;
    memcpy(&v, &tex_id, ImMin(sizeof(ImU64), sizeof(ImTextureID)));
    return v;
}

static const char* FormatTextureRefForDebugDisplay(char* buf, int buf_size, ImTextureRef tex_ref)
{
    char* buf_p = buf;
    char* buf_end = buf + buf_size;
    if (tex_ref._TexData != NULL)
        buf_p += ImFormatString(buf_p, buf_end - buf_p, "#%03d: ", tex_ref._TexData->UniqueID);
    ImFormatString(buf_p, buf_end - buf_p, "0x%X", ImGui::DebugTextureIDToU64(tex_ref.GetTexID()));
    return buf;
}

#ifdef IMGUI_ENABLE_FREETYPE
namespace ImGuiFreeType { IMGUI_API const ImFontLoader* GetFontLoader(); IMGUI_API bool DebugEditFontLoaderFlags(unsigned int* p_font_builder_flags); }
#endif

// [DEBUG] List fonts in a font atlas and display its texture
void ImGui::ShowFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiStyle& style = g.Style;

    BeginDisabled();
    CheckboxFlags("io.BackendFlags: RendererHasTextures", &io.BackendFlags, ImGuiBackendFlags_RendererHasTextures);
    EndDisabled();
    ShowFontSelector("Font");
    //BeginDisabled((io.BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0);
    if (DragFloat("FontSizeBase", &style.FontSizeBase, 0.20f, 5.0f, 100.0f, "%.0f"))
        style._NextFrameFontSizeBase = style.FontSizeBase; // FIXME: Temporary hack until we finish remaining work.
    SameLine(0.0f, 0.0f); Text(" (out %.2f)", GetFontSize());
    SameLine(); MetricsHelpMarker("- This is scaling font only. General scaling will come later.");
    DragFloat("FontScaleMain", &style.FontScaleMain, 0.02f, 0.5f, 4.0f);
    //BeginDisabled(io.ConfigDpiScaleFonts);
    DragFloat("FontScaleDpi", &style.FontScaleDpi, 0.02f, 0.5f, 4.0f);
    //SetItemTooltip("When io.ConfigDpiScaleFonts is set, this value is automatically overwritten.");
    //EndDisabled();
    if ((io.BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0)
    {
        BulletText("Warning: Font scaling will NOT be smooth, because\nImGuiBackendFlags_RendererHasTextures is not set!");
        BulletText("For instructions, see:");
        SameLine();
        TextLinkOpenURL("docs/BACKENDS.md", "https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md");
    }
    BulletText("Load a nice font for better results!");
    BulletText("Please submit feedback:");
    SameLine(); TextLinkOpenURL("#8465", "https://github.com/ocornut/imgui/issues/8465");
    BulletText("Read FAQ for more details:");
    SameLine(); TextLinkOpenURL("dearimgui.com/faq", "https://www.dearimgui.com/faq/");
    //EndDisabled();

    SeparatorText("Font List");

    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    Checkbox("Show font preview", &cfg->ShowFontPreview);

    // Font loaders
    if (TreeNode("Loader", "Loader: \'%s\'", atlas->FontLoaderName ? atlas->FontLoaderName : "NULL"))
    {
        const ImFontLoader* loader_current = atlas->FontLoader;
        BeginDisabled(!atlas->RendererHasTextures);
#ifdef IMGUI_ENABLE_STB_TRUETYPE
        const ImFontLoader* loader_stbtruetype = ImFontAtlasGetFontLoaderForStbTruetype();
        if (RadioButton("stb_truetype", loader_current == loader_stbtruetype))
            atlas->SetFontLoader(loader_stbtruetype);
#else
        BeginDisabled();
        RadioButton("stb_truetype", false);
        SetItemTooltip("Requires #define IMGUI_ENABLE_STB_TRUETYPE");
        EndDisabled();
#endif
        SameLine();
#ifdef IMGUI_ENABLE_FREETYPE
        const ImFontLoader* loader_freetype = ImGuiFreeType::GetFontLoader();
        if (RadioButton("FreeType", loader_current == loader_freetype))
            atlas->SetFontLoader(loader_freetype);
        if (loader_current == loader_freetype)
        {
            unsigned int loader_flags = atlas->FontLoaderFlags;
            Text("Shared FreeType Loader Flags:  0x%08X", loader_flags);
            if (ImGuiFreeType::DebugEditFontLoaderFlags(&loader_flags))
            {
                for (ImFont* font : atlas->Fonts)
                    ImFontAtlasFontDestroyOutput(atlas, font);
                atlas->FontLoaderFlags = loader_flags;
                for (ImFont* font : atlas->Fonts)
                    ImFontAtlasFontInitOutput(atlas, font);
            }
        }
#else
        BeginDisabled();
        RadioButton("FreeType", false);
        SetItemTooltip("Requires #define IMGUI_ENABLE_FREETYPE + imgui_freetype.cpp.");
        EndDisabled();
#endif
        EndDisabled();
        TreePop();
    }

    // Font list
    for (ImFont* font : atlas->Fonts)
    {
        PushID(font);
        DebugNodeFont(font);
        PopID();
    }

    SeparatorText("Font Atlas");
    if (Button("Compact"))
        atlas->CompactCache();
    SameLine();
    if (Button("Grow"))
        ImFontAtlasTextureGrow(atlas);
    SameLine();
    if (Button("Clear All"))
        ImFontAtlasBuildClear(atlas);
    SetItemTooltip("Destroy cache and custom rectangles.");

    for (int tex_n = 0; tex_n < atlas->TexList.Size; tex_n++)
    {
        ImTextureData* tex = atlas->TexList[tex_n];
        if (tex_n > 0)
            SameLine();
        Text("Tex: %dx%d", tex->Width, tex->Height);
    }
    const int packed_surface_sqrt = (int)sqrtf((float)atlas->Builder->RectsPackedSurface);
    const int discarded_surface_sqrt = (int)sqrtf((float)atlas->Builder->RectsDiscardedSurface);
    Text("Packed rects: %d, area: about %d px ~%dx%d px", atlas->Builder->RectsPackedCount, atlas->Builder->RectsPackedSurface, packed_surface_sqrt, packed_surface_sqrt);
    Text("incl. Discarded rects: %d, area: about %d px ~%dx%d px", atlas->Builder->RectsDiscardedCount, atlas->Builder->RectsDiscardedSurface, discarded_surface_sqrt, discarded_surface_sqrt);

    ImFontAtlasRectId highlight_r_id = ImFontAtlasRectId_Invalid;
    if (TreeNode("Rects Index", "Rects Index (%d)", atlas->Builder->RectsPackedCount)) // <-- Use count of used rectangles
    {
        PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
        if (BeginTable("##table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0.0f, GetTextLineHeightWithSpacing() * 12)))
        {
            for (const ImFontAtlasRectEntry& entry : atlas->Builder->RectsIndex)
                if (entry.IsUsed)
                {
                    ImFontAtlasRectId id = ImFontAtlasRectId_Make(atlas->Builder->RectsIndex.index_from_ptr(&entry), entry.Generation);
                    ImFontAtlasRect r = {};
                    atlas->GetCustomRect(id, &r);
                    const char* buf;
                    ImFormatStringToTempBuffer(&buf, NULL, "ID:%08X, used:%d, { w:%3d, h:%3d } { x:%4d, y:%4d }", id, entry.IsUsed, r.w, r.h, r.x, r.y);
                    TableNextColumn();
                    Selectable(buf);
                    if (IsItemHovered())
                        highlight_r_id = id;
                    TableNextColumn();
                    Image(atlas->TexRef, ImVec2(r.w, r.h), r.uv0, r.uv1);
                }
            EndTable();
        }
        PopStyleVar();
        TreePop();
    }

    // Texture list
    // (ensure the last texture always use the same ID, so we can keep it open neatly)
    ImFontAtlasRect highlight_r;
    if (highlight_r_id != ImFontAtlasRectId_Invalid)
        atlas->GetCustomRect(highlight_r_id, &highlight_r);
    for (int tex_n = 0; tex_n < atlas->TexList.Size; tex_n++)
    {
        if (tex_n == atlas->TexList.Size - 1)
            SetNextItemOpen(true, ImGuiCond_Once);
        DebugNodeTexture(atlas->TexList[tex_n], atlas->TexList.Size - 1 - tex_n, (highlight_r_id != ImFontAtlasRectId_Invalid) ? &highlight_r : NULL);
    }
}

void ImGui::DebugNodeTexture(ImTextureData* tex, int int_id, const ImFontAtlasRect* highlight_rect)
{
    ImGuiContext& g = *GImGui;
    PushID(int_id);
    if (TreeNode("", "Texture #%03d (%dx%d pixels)", tex->UniqueID, tex->Width, tex->Height))
    {
        ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
        Checkbox("Show used rect", &cfg->ShowTextureUsedRect);
        PushStyleVar(ImGuiStyleVar_ImageBorderSize, ImMax(1.0f, g.Style.ImageBorderSize));
        ImVec2 p = GetCursorScreenPos();
        if (tex->WantDestroyNextFrame)
            Dummy(ImVec2((float)tex->Width, (float)tex->Height));
        else
            ImageWithBg(tex->GetTexRef(), ImVec2((float)tex->Width, (float)tex->Height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (cfg->ShowTextureUsedRect)
            GetWindowDrawList()->AddRect(ImVec2(p.x + tex->UsedRect.x, p.y + tex->UsedRect.y), ImVec2(p.x + tex->UsedRect.x + tex->UsedRect.w, p.y + tex->UsedRect.y + tex->UsedRect.h), IM_COL32(255, 0, 255, 255));
        if (highlight_rect != NULL)
        {
            ImRect r_outer(p.x, p.y, p.x + tex->Width, p.y + tex->Height);
            ImRect r_inner(p.x + highlight_rect->x, p.y + highlight_rect->y, p.x + highlight_rect->x + highlight_rect->w, p.y + highlight_rect->y + highlight_rect->h);
            RenderRectFilledWithHole(GetWindowDrawList(), r_outer, r_inner, IM_COL32(0, 0, 0, 100), 0.0f);
            GetWindowDrawList()->AddRect(r_inner.Min - ImVec2(1, 1), r_inner.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255));
        }
        PopStyleVar();

        char texref_desc[30];
        Text("Status = %s (%d), Format = %s (%d), UseColors = %d", ImTextureDataGetStatusName(tex->Status), tex->Status, ImTextureDataGetFormatName(tex->Format), tex->Format, tex->UseColors);
        Text("TexRef = %s, BackendUserData = %p", FormatTextureRefForDebugDisplay(texref_desc, IM_COUNTOF(texref_desc), tex->GetTexRef()), tex->BackendUserData);
        TreePop();
    }
    PopID();
}

void ImGui::ShowMetricsWindow(bool* p_open)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    if (cfg->ShowDebugLog)
        ShowDebugLogWindow(&cfg->ShowDebugLog);
    if (cfg->ShowIDStackTool)
        ShowIDStackToolWindow(&cfg->ShowIDStackTool);

    if (!Begin("Dear ImGui Metrics/Debugger", p_open) || GetCurrentWindow()->BeginCount > 1)
    {
        End();
        return;
    }

    // [DEBUG] Clear debug breaks hooks after exactly one cycle.
    DebugBreakClearData();

    // Basic info
    Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    if (g.ContextName[0] != 0)
    {
        SameLine();
        Text("(Context Name: \"%s\")", g.ContextName);
    }
    Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices, io.MetricsRenderIndices / 3);
    Text("%d visible windows, %d current allocations", io.MetricsRenderWindows, g.DebugAllocInfo.TotalAllocCount - g.DebugAllocInfo.TotalFreeCount);
    //SameLine(); if (SmallButton("GC")) { g.GcCompactAll = true; }

    Separator();

    // Debugging enums
    enum { WRT_OuterRect, WRT_OuterRectClipped, WRT_InnerRect, WRT_InnerClipRect, WRT_WorkRect, WRT_Content, WRT_ContentIdeal, WRT_ContentRegionRect, WRT_Count }; // Windows Rect Type
    const char* wrt_rects_names[WRT_Count] = { "OuterRect", "OuterRectClipped", "InnerRect", "InnerClipRect", "WorkRect", "Content", "ContentIdeal", "ContentRegionRect" };
    enum { TRT_OuterRect, TRT_InnerRect, TRT_WorkRect, TRT_HostClipRect, TRT_InnerClipRect, TRT_BackgroundClipRect, TRT_ColumnsRect, TRT_ColumnsWorkRect, TRT_ColumnsClipRect, TRT_ColumnsContentHeadersUsed, TRT_ColumnsContentHeadersIdeal, TRT_ColumnsContentFrozen, TRT_ColumnsContentUnfrozen, TRT_Count }; // Tables Rect Type
    const char* trt_rects_names[TRT_Count] = { "OuterRect", "InnerRect", "WorkRect", "HostClipRect", "InnerClipRect", "BackgroundClipRect", "ColumnsRect", "ColumnsWorkRect", "ColumnsClipRect", "ColumnsContentHeadersUsed", "ColumnsContentHeadersIdeal", "ColumnsContentFrozen", "ColumnsContentUnfrozen" };
    if (cfg->ShowWindowsRectsType < 0)
        cfg->ShowWindowsRectsType = WRT_WorkRect;
    if (cfg->ShowTablesRectsType < 0)
        cfg->ShowTablesRectsType = TRT_WorkRect;

    struct Funcs
    {
        static ImRect GetTableRect(ImGuiTable* table, int rect_type, int n)
        {
            ImGuiTableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent); // Always using last submitted instance
            if (rect_type == TRT_OuterRect)                     { return table->OuterRect; }
            else if (rect_type == TRT_InnerRect)                { return table->InnerRect; }
            else if (rect_type == TRT_WorkRect)                 { return table->WorkRect; }
            else if (rect_type == TRT_HostClipRect)             { return table->HostClipRect; }
            else if (rect_type == TRT_InnerClipRect)            { return table->InnerClipRect; }
            else if (rect_type == TRT_BackgroundClipRect)       { return table->BgClipRect; }
            else if (rect_type == TRT_ColumnsRect)              { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->MinX, table->InnerClipRect.Min.y, c->MaxX, table->InnerClipRect.Min.y + table_instance->LastOuterHeight); }
            else if (rect_type == TRT_ColumnsWorkRect)          { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->WorkRect.Min.y, c->WorkMaxX, table->WorkRect.Max.y); }
            else if (rect_type == TRT_ColumnsClipRect)          { ImGuiTableColumn* c = &table->Columns[n]; return c->ClipRect; }
            else if (rect_type == TRT_ColumnsContentHeadersUsed){ ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXHeadersUsed, table->InnerClipRect.Min.y + table_instance->LastTopHeadersRowHeight); } // Note: y1/y2 not always accurate
            else if (rect_type == TRT_ColumnsContentHeadersIdeal){ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXHeadersIdeal, table->InnerClipRect.Min.y + table_instance->LastTopHeadersRowHeight); }
            else if (rect_type == TRT_ColumnsContentFrozen)     { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXFrozen, table->InnerClipRect.Min.y + table_instance->LastFrozenHeight); }
            else if (rect_type == TRT_ColumnsContentUnfrozen)   { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y + table_instance->LastFrozenHeight, c->ContentMaxXUnfrozen, table->InnerClipRect.Max.y); }
            IM_ASSERT(0);
            return ImRect();
        }

        static ImRect GetWindowRect(ImGuiWindow* window, int rect_type)
        {
            if (rect_type == WRT_OuterRect)                 { return window->Rect(); }
            else if (rect_type == WRT_OuterRectClipped)     { return window->OuterRectClipped; }
            else if (rect_type == WRT_InnerRect)            { return window->InnerRect; }
            else if (rect_type == WRT_InnerClipRect)        { return window->InnerClipRect; }
            else if (rect_type == WRT_WorkRect)             { return window->WorkRect; }
            else if (rect_type == WRT_Content)              { ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding; return ImRect(min, min + window->ContentSize); }
            else if (rect_type == WRT_ContentIdeal)         { ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding; return ImRect(min, min + window->ContentSizeIdeal); }
            else if (rect_type == WRT_ContentRegionRect)    { return window->ContentRegionRect; }
            IM_ASSERT(0);
            return ImRect();
        }
    };

#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
    TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS is enabled.\nMust disable after use! Otherwise Dear ImGui will run slower.\n");
#endif

    // Tools
    if (TreeNode("Tools"))
    {
        // Debug Break features
        // The Item Picker tool is super useful to visually select an item and break into the call-stack of where it was submitted.
        SeparatorTextEx(0, "Debug breaks", NULL, CalcTextSize("(?)").x + g.Style.SeparatorTextPadding.x);
        SameLine();
        MetricsHelpMarker("Will call the IM_DEBUG_BREAK() macro to break in debugger.\nWarning: If you don't have a debugger attached, this will probably crash.");
        if (Checkbox("Show Item Picker", &g.DebugItemPickerActive) && g.DebugItemPickerActive)
            DebugStartItemPicker();
        Checkbox("Show \"Debug Break\" buttons in other sections (io.ConfigDebugIsDebuggerPresent)", &g.IO.ConfigDebugIsDebuggerPresent);

        SeparatorText("Visualize");

        Checkbox("Show Debug Log", &cfg->ShowDebugLog);
        SameLine();
        MetricsHelpMarker("You can also call ImGui::ShowDebugLogWindow() from your code.");

        Checkbox("Show ID Stack Tool", &cfg->ShowIDStackTool);
        SameLine();
        MetricsHelpMarker("You can also call ImGui::ShowIDStackToolWindow() from your code.");

        Checkbox("Show windows begin order", &cfg->ShowWindowsBeginOrder);
        Checkbox("Show windows rectangles", &cfg->ShowWindowsRects);
        SameLine();
        SetNextItemWidth(GetFontSize() * 12);
        cfg->ShowWindowsRects |= Combo("##show_windows_rect_type", &cfg->ShowWindowsRectsType, wrt_rects_names, WRT_Count, WRT_Count);
        if (cfg->ShowWindowsRects && g.NavWindow != NULL)
        {
            BulletText("'%s':", g.NavWindow->Name);
            Indent();
            for (int rect_n = 0; rect_n < WRT_Count; rect_n++)
            {
                ImRect r = Funcs::GetWindowRect(g.NavWindow, rect_n);
                Text("(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), wrt_rects_names[rect_n]);
            }
            Unindent();
        }

        Checkbox("Show tables rectangles", &cfg->ShowTablesRects);
        SameLine();
        SetNextItemWidth(GetFontSize() * 12);
        cfg->ShowTablesRects |= Combo("##show_table_rects_type", &cfg->ShowTablesRectsType, trt_rects_names, TRT_Count, TRT_Count);
        if (cfg->ShowTablesRects && g.NavWindow != NULL)
        {
            for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++)
            {
                ImGuiTable* table = g.Tables.TryGetMapData(table_n);
                if (table == NULL || table->LastFrameActive < g.FrameCount - 1 || (table->OuterWindow != g.NavWindow && table->InnerWindow != g.NavWindow))
                    continue;

                BulletText("Table 0x%08X (%d columns, in '%s')", table->ID, table->ColumnsCount, table->OuterWindow->Name);
                if (IsItemHovered())
                    GetForegroundDrawList(table->OuterWindow)->AddRect(table->OuterRect.Min - ImVec2(1, 1), table->OuterRect.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                Indent();
                char buf[128];
                for (int rect_n = 0; rect_n < TRT_Count; rect_n++)
                {
                    if (rect_n >= TRT_ColumnsRect)
                    {
                        if (rect_n != TRT_ColumnsRect && rect_n != TRT_ColumnsClipRect)
                            continue;
                        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                        {
                            ImRect r = Funcs::GetTableRect(table, rect_n, column_n);
                            ImFormatString(buf, IM_COUNTOF(buf), "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) Col %d %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), column_n, trt_rects_names[rect_n]);
                            Selectable(buf);
                            if (IsItemHovered())
                                GetForegroundDrawList(table->OuterWindow)->AddRect(r.Min - ImVec2(1, 1), r.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                        }
                    }
                    else
                    {
                        ImRect r = Funcs::GetTableRect(table, rect_n, -1);
                        ImFormatString(buf, IM_COUNTOF(buf), "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), trt_rects_names[rect_n]);
                        Selectable(buf);
                        if (IsItemHovered())
                            GetForegroundDrawList(table->OuterWindow)->AddRect(r.Min - ImVec2(1, 1), r.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                    }
                }
                Unindent();
            }
        }
        Checkbox("Show groups rectangles", &g.DebugShowGroupRects); // Storing in context as this is used by group code and prefers to be in hot-data

        SeparatorText("Validate");

        Checkbox("Debug Begin/BeginChild return value", &io.ConfigDebugBeginReturnValueLoop);
        SameLine();
        MetricsHelpMarker("Some calls to Begin()/BeginChild() will return false.\n\nWill cycle through window depths then repeat. Windows should be flickering while running.");

        Checkbox("UTF-8 Encoding viewer", &cfg->ShowTextEncodingViewer);
        SameLine();
        MetricsHelpMarker("You can also call ImGui::DebugTextEncoding() from your code with a given string to test that your UTF-8 encoding settings are correct.");
        if (cfg->ShowTextEncodingViewer)
        {
            static char buf[64] = "";
            SetNextItemWidth(-FLT_MIN);
            InputText("##DebugTextEncodingBuf", buf, IM_COUNTOF(buf));
            if (buf[0] != 0)
                DebugTextEncoding(buf);
        }

        TreePop();
    }

    // Windows
    if (TreeNode("Windows", "Windows (%d)", g.Windows.Size))
    {
        //SetNextItemOpen(true, ImGuiCond_Once);
        DebugNodeWindowsList(&g.Windows, "By display order");
        DebugNodeWindowsList(&g.WindowsFocusOrder, "By focus order (root windows)");
        if (TreeNode("By submission order (begin stack)"))
        {
            // Here we display windows in their submitted order/hierarchy, however note that the Begin stack doesn't constitute a Parent<>Child relationship!
            ImVector<ImGuiWindow*>& temp_buffer = g.WindowsTempSortBuffer;
            temp_buffer.resize(0);
            for (ImGuiWindow* window : g.Windows)
                if (window->LastFrameActive + 1 >= g.FrameCount)
                    temp_buffer.push_back(window);
            struct Func { static int IMGUI_CDECL WindowComparerByBeginOrder(const void* lhs, const void* rhs) { return ((int)(*(const ImGuiWindow* const *)lhs)->BeginOrderWithinContext - (*(const ImGuiWindow* const*)rhs)->BeginOrderWithinContext); } };
            ImQsort(temp_buffer.Data, (size_t)temp_buffer.Size, sizeof(ImGuiWindow*), Func::WindowComparerByBeginOrder);
            DebugNodeWindowsListByBeginStackParent(temp_buffer.Data, temp_buffer.Size, NULL);
            TreePop();
        }

        TreePop();
    }

    // DrawLists
    int drawlist_count = 0;
    for (ImGuiViewportP* viewport : g.Viewports)
        drawlist_count += viewport->DrawDataP.CmdLists.Size;
    if (TreeNode("DrawLists", "DrawLists (%d)", drawlist_count))
    {
        Checkbox("Show ImDrawCmd mesh when hovering", &cfg->ShowDrawCmdMesh);
        Checkbox("Show ImDrawCmd bounding boxes when hovering", &cfg->ShowDrawCmdBoundingBoxes);
        for (ImGuiViewportP* viewport : g.Viewports)
        {
            bool viewport_has_drawlist = false;
            for (ImDrawList* draw_list : viewport->DrawDataP.CmdLists)
            {
                if (!viewport_has_drawlist)
                    Text("Active DrawLists in Viewport #%d, ID: 0x%08X", viewport->Idx, viewport->ID);
                viewport_has_drawlist = true;
                DebugNodeDrawList(NULL, viewport, draw_list, "DrawList");
            }
        }
        TreePop();
    }

    // Viewports
    if (TreeNode("Viewports", "Viewports (%d)", g.Viewports.Size))
    {
        cfg->HighlightMonitorIdx = -1;
        bool open = TreeNode("Monitors", "Monitors (%d)", g.PlatformIO.Monitors.Size);
        SameLine();
        MetricsHelpMarker("Dear ImGui uses monitor data:\n- to query DPI settings on a per monitor basis\n- to position popup/tooltips so they don't straddle monitors.");
        if (open)
        {
            for (int i = 0; i < g.PlatformIO.Monitors.Size; i++)
            {
                DebugNodePlatformMonitor(&g.PlatformIO.Monitors[i], "Monitor", i);
                if (IsItemHovered())
                    cfg->HighlightMonitorIdx = i;
            }
            DebugNodePlatformMonitor(&g.FallbackMonitor, "Fallback", 0);
            TreePop();
        }

        SetNextItemOpen(true, ImGuiCond_Once);
        if (TreeNode("Windows Minimap"))
        {
            RenderViewportsThumbnails();
            TreePop();
        }
        cfg->HighlightViewportID = 0;

        BulletText("MouseViewport: 0x%08X (UserHovered 0x%08X, LastHovered 0x%08X)", g.MouseViewport ? g.MouseViewport->ID : 0, g.IO.MouseHoveredViewport, g.MouseLastHoveredViewport ? g.MouseLastHoveredViewport->ID : 0);
        if (TreeNode("Inferred Z order (front-to-back)"))
        {
            static ImVector<ImGuiViewportP*> viewports;
            viewports.resize(g.Viewports.Size);
            memcpy(viewports.Data, g.Viewports.Data, g.Viewports.size_in_bytes());
            if (viewports.Size > 1)
                ImQsort(viewports.Data, viewports.Size, sizeof(ImGuiViewport*), ViewportComparerByLastFocusedStampCount);
            for (ImGuiViewportP* viewport : viewports)
            {
                BulletText("Viewport #%d, ID: 0x%08X, LastFocused = %08d, PlatformFocused = %s, Window: \"%s\"",
                    viewport->Idx, viewport->ID, viewport->LastFocusedStampCount,
                    (g.PlatformIO.Platform_GetWindowFocus && viewport->PlatformWindowCreated) ? (g.PlatformIO.Platform_GetWindowFocus(viewport) ? "1" : "0") : "N/A",
                    viewport->Window ? viewport->Window->Name : "N/A");
                if (IsItemHovered())
                    cfg->HighlightViewportID = viewport->ID;
            }
            TreePop();
        }

        for (ImGuiViewportP* viewport : g.Viewports)
            DebugNodeViewport(viewport);
        TreePop();
    }

    // Details for Fonts
    for (ImFontAtlas* atlas : g.FontAtlases)
        if (TreeNode((void*)atlas, "Fonts (%d), Textures (%d)", atlas->Fonts.Size, atlas->TexList.Size))
        {
            ShowFontAtlas(atlas);
            TreePop();
        }

    // Details for Popups
    if (TreeNode("Popups", "Popups (%d)", g.OpenPopupStack.Size))
    {
        for (const ImGuiPopupData& popup_data : g.OpenPopupStack)
        {
            // As it's difficult to interact with tree nodes while popups are open, we display everything inline.
            ImGuiWindow* window = popup_data.Window;
            BulletText("PopupID: %08x, Window: '%s' (%s%s), RestoreNavWindow '%s', ParentWindow '%s'",
                popup_data.PopupId, window ? window->Name : "NULL", window && (window->Flags & ImGuiWindowFlags_ChildWindow) ? "Child;" : "", window && (window->Flags & ImGuiWindowFlags_ChildMenu) ? "Menu;" : "",
                popup_data.RestoreNavWindow ? popup_data.RestoreNavWindow->Name : "NULL", window && window->ParentWindow ? window->ParentWindow->Name : "NULL");
        }
        TreePop();
    }

    // Details for TabBars
    if (TreeNode("TabBars", "Tab Bars (%d)", g.TabBars.GetAliveCount()))
    {
        for (int n = 0; n < g.TabBars.GetMapSize(); n++)
            if (ImGuiTabBar* tab_bar = g.TabBars.TryGetMapData(n))
            {
                PushID(tab_bar);
                DebugNodeTabBar(tab_bar, "TabBar");
                PopID();
            }
        TreePop();
    }

    // Details for Tables
    if (TreeNode("Tables", "Tables (%d)", g.Tables.GetAliveCount()))
    {
        for (int n = 0; n < g.Tables.GetMapSize(); n++)
            if (ImGuiTable* table = g.Tables.TryGetMapData(n))
                DebugNodeTable(table);
        TreePop();
    }

    // Details for InputText
    if (TreeNode("InputText"))
    {
        DebugNodeInputTextState(&g.InputTextState);
        TreePop();
    }

    // Details for TypingSelect
    if (TreeNode("TypingSelect", "TypingSelect (%d)", g.TypingSelectState.SearchBuffer[0] != 0 ? 1 : 0))
    {
        DebugNodeTypingSelectState(&g.TypingSelectState);
        TreePop();
    }

    // Details for MultiSelect
    if (TreeNode("MultiSelect", "MultiSelect (%d)", g.MultiSelectStorage.GetAliveCount()))
    {
        ImGuiBoxSelectState* bs = &g.BoxSelectState;
        BulletText("BoxSelect ID=0x%08X, Starting = %d, Active %d", bs->ID, bs->IsStarting, bs->IsActive);
        for (int n = 0; n < g.MultiSelectStorage.GetMapSize(); n++)
            if (ImGuiMultiSelectState* state = g.MultiSelectStorage.TryGetMapData(n))
                DebugNodeMultiSelectState(state);
        TreePop();
    }

    // Details for Docking
#ifdef IMGUI_HAS_DOCK
    if (TreeNode("Docking"))
    {
        static bool root_nodes_only = true;
        ImGuiDockContext* dc = &g.DockContext;
        Checkbox("List root nodes", &root_nodes_only);
        Checkbox("Ctrl shows window dock info", &cfg->ShowDockingNodes);
        if (SmallButton("Clear nodes")) { DockContextClearNodes(&g, 0, true); }
        SameLine();
        if (SmallButton("Rebuild all")) { dc->WantFullRebuild = true; }
        for (int n = 0; n < dc->Nodes.Data.Size; n++)
            if (ImGuiDockNode* node = (ImGuiDockNode*)dc->Nodes.Data[n].val_p)
                if (!root_nodes_only || node->IsRootNode())
                    DebugNodeDockNode(node, "Node");
        TreePop();
    }
#endif // #ifdef IMGUI_HAS_DOCK

    // Settings
    if (TreeNode("Settings"))
    {
        if (SmallButton("Clear"))
            ClearIniSettings();
        SameLine();
        if (SmallButton("Save to memory"))
            SaveIniSettingsToMemory();
        SameLine();
        if (SmallButton("Save to disk"))
            SaveIniSettingsToDisk(g.IO.IniFilename);
        SameLine();
        if (g.IO.IniFilename)
            Text("\"%s\"", g.IO.IniFilename);
        else
            TextUnformatted("<NULL>");
        Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
        Text("SettingsDirtyTimer %.2f", g.SettingsDirtyTimer);
        if (TreeNode("SettingsHandlers", "Settings handlers: (%d)", g.SettingsHandlers.Size))
        {
            for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
                BulletText("\"%s\"", handler.TypeName);
            TreePop();
        }
        if (TreeNode("SettingsWindows", "Settings packed data: Windows: %d bytes", g.SettingsWindows.size()))
        {
            for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
                DebugNodeWindowSettings(settings);
            TreePop();
        }

        if (TreeNode("SettingsTables", "Settings packed data: Tables: %d bytes", g.SettingsTables.size()))
        {
            for (ImGuiTableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
                DebugNodeTableSettings(settings);
            TreePop();
        }

#ifdef IMGUI_HAS_DOCK
        if (TreeNode("SettingsDocking", "Settings packed data: Docking"))
        {
            ImGuiDockContext* dc = &g.DockContext;
            Text("In SettingsWindows:");
            for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
                if (settings->DockId != 0)
                    BulletText("Window '%s' -> DockId %08X DockOrder=%d", settings->GetName(), settings->DockId, settings->DockOrder);
            Text("In SettingsNodes:");
            for (int n = 0; n < dc->NodesSettings.Size; n++)
            {
                ImGuiDockNodeSettings* settings = &dc->NodesSettings[n];
                const char* selected_tab_name = NULL;
                if (settings->SelectedTabId)
                {
                    if (ImGuiWindow* window = FindWindowByID(settings->SelectedTabId))
                        selected_tab_name = window->Name;
                    else if (ImGuiWindowSettings* window_settings = FindWindowSettingsByID(settings->SelectedTabId))
                        selected_tab_name = window_settings->GetName();
                }
                BulletText("Node %08X, Parent %08X, SelectedTab %08X ('%s')", settings->ID, settings->ParentNodeId, settings->SelectedTabId, selected_tab_name ? selected_tab_name : settings->SelectedTabId ? "N/A" : "");
            }
            TreePop();
        }
#endif // #ifdef IMGUI_HAS_DOCK

        if (TreeNode("SettingsIniData", "Settings unpacked data (.ini): %d bytes", g.SettingsIniData.size()))
        {
            InputTextMultiline("##Ini", (char*)(void*)g.SettingsIniData.c_str(), g.SettingsIniData.Buf.Size, ImVec2(-FLT_MIN, GetTextLineHeight() * 20), ImGuiInputTextFlags_ReadOnly);
            TreePop();
        }
        TreePop();
    }

    // Settings
    if (TreeNode("Memory allocations"))
    {
        ImGuiDebugAllocInfo* info = &g.DebugAllocInfo;
        Text("%d current allocations", info->TotalAllocCount - info->TotalFreeCount);
        Text("Releasing selected unused buffers after: %.2f secs", g.IO.ConfigMemoryCompactTimer);
        if (SmallButton("GC now")) { g.GcCompactAll = true; }
        Text("Recent frames with allocations:");
        int buf_size = IM_COUNTOF(info->LastEntriesBuf);
        for (int n = buf_size - 1; n >= 0; n--)
        {
            ImGuiDebugAllocEntry* entry = &info->LastEntriesBuf[(info->LastEntriesIdx - n + buf_size) % buf_size];
            BulletText("Frame %06d: %+3d ( %2d alloc, %2d free )", entry->FrameCount, entry->AllocCount - entry->FreeCount, entry->AllocCount, entry->FreeCount);
            if (n == 0)
            {
                SameLine();
                Text("<- %d frames ago", g.FrameCount - entry->FrameCount);
            }
        }
        TreePop();
    }

    if (TreeNode("Inputs"))
    {
        Text("KEYBOARD/GAMEPAD/MOUSE KEYS");
        {
            // User code should never have to go through such hoops! You can generally iterate between ImGuiKey_NamedKey_BEGIN and ImGuiKey_NamedKey_END.
            Indent();
            Text("Keys down:");         for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (!IsKeyDown(key)) continue;     SameLine(); Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); SameLine(); Text("(%.02f)", GetKeyData(key)->DownDuration); }
            Text("Keys pressed:");      for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (!IsKeyPressed(key)) continue;  SameLine(); Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); }
            Text("Keys released:");     for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (!IsKeyReleased(key)) continue; SameLine(); Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); }
            Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "Ctrl " : "", io.KeyShift ? "Shift " : "", io.KeyAlt ? "Alt " : "", io.KeySuper ? "Super " : "");
            Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; SameLine(); Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
            DebugRenderKeyboardPreview(GetWindowDrawList());
            Unindent();
        }

        Text("MOUSE STATE");
        {
            Indent();
            if (IsMousePosValid())
                Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                Text("Mouse pos: <INVALID>");
            Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            int count = IM_COUNTOF(io.MouseDown);
            Text("Mouse down:");     for (int i = 0; i < count; i++) if (IsMouseDown(i)) { SameLine(); Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
            Text("Mouse clicked:");  for (int i = 0; i < count; i++) if (IsMouseClicked(i)) { SameLine(); Text("b%d (%d)", i, io.MouseClickedCount[i]); }
            Text("Mouse released:"); for (int i = 0; i < count; i++) if (IsMouseReleased(i)) { SameLine(); Text("b%d", i); }
            Text("Mouse wheel: %.1f", io.MouseWheel);
            Text("MouseStationaryTimer: %.2f", g.MouseStationaryTimer);
            Text("Mouse source: %s", GetMouseSourceName(io.MouseSource));
            Text("Pen Pressure: %.1f", io.PenPressure); // Note: currently unused
            Unindent();
        }

        Text("MOUSE WHEELING");
        {
            Indent();
            Text("WheelingWindow: '%s'", g.WheelingWindow ? g.WheelingWindow->Name : "NULL");
            Text("WheelingWindowReleaseTimer: %.2f", g.WheelingWindowReleaseTimer);
            Text("WheelingAxisAvg[] = { %.3f, %.3f }, Main Axis: %s", g.WheelingAxisAvg.x, g.WheelingAxisAvg.y, (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? "X" : (g.WheelingAxisAvg.x < g.WheelingAxisAvg.y) ? "Y" : "<none>");
            Unindent();
        }

        Text("KEY OWNERS");
        {
            Indent();
            if (BeginChild("##owners", ImVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoSavedSettings))
                for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
                {
                    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
                    if (owner_data->OwnerCurr == ImGuiKeyOwner_NoOwner)
                        continue;
                    Text("%s: 0x%08X%s", GetKeyName(key), owner_data->OwnerCurr,
                        owner_data->LockUntilRelease ? " LockUntilRelease" : owner_data->LockThisFrame ? " LockThisFrame" : "");
                    DebugLocateItemOnHover(owner_data->OwnerCurr);
                }
            EndChild();
            Unindent();
        }
        Text("SHORTCUT ROUTING");
        SameLine();
        MetricsHelpMarker("Declared shortcut routes automatically set key owner when mods matches.");
        {
            Indent();
            if (BeginChild("##routes", ImVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoSavedSettings))
                for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
                {
                    ImGuiKeyRoutingTable* rt = &g.KeysRoutingTable;
                    for (ImGuiKeyRoutingIndex idx = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; idx != -1; )
                    {
                        ImGuiKeyRoutingData* routing_data = &rt->Entries[idx];
                        ImGuiKeyChord key_chord = key | routing_data->Mods;
                        Text("%s: 0x%08X (scored %d)", GetKeyChordName(key_chord), routing_data->RoutingCurr, routing_data->RoutingCurrScore);
                        DebugLocateItemOnHover(routing_data->RoutingCurr);
                        if (g.IO.ConfigDebugIsDebuggerPresent)
                        {
                            SameLine();
                            if (DebugBreakButton("**DebugBreak**", "in SetShortcutRouting() for this KeyChord"))
                                g.DebugBreakInShortcutRouting = key_chord;
                        }
                        idx = routing_data->NextEntryIndex;
                    }
                }
            EndChild();
            Text("(ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: 0x%X)", g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
            Unindent();
        }
        TreePop();
    }

    if (TreeNode("Internal state"))
    {
        Text("WINDOWING");
        Indent();
        Text("HoveredWindow: '%s'", g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
        Text("HoveredWindow->Root: '%s'", g.HoveredWindow ? g.HoveredWindow->RootWindowDockTree->Name : "NULL");
        Text("HoveredWindowUnderMovingWindow: '%s'", g.HoveredWindowUnderMovingWindow ? g.HoveredWindowUnderMovingWindow->Name : "NULL");
        Text("HoveredDockNode: 0x%08X", g.DebugHoveredDockNode ? g.DebugHoveredDockNode->ID : 0);
        Text("MovingWindow: '%s'", g.MovingWindow ? g.MovingWindow->Name : "NULL");
        Text("MouseViewport: 0x%08X (UserHovered 0x%08X, LastHovered 0x%08X)", g.MouseViewport->ID, g.IO.MouseHoveredViewport, g.MouseLastHoveredViewport ? g.MouseLastHoveredViewport->ID : 0);
        Unindent();

        Text("ITEMS");
        Indent();
        Text("ActiveId: 0x%08X/0x%08X (%.2f sec), AllowOverlap: %d, Source: %s", g.ActiveId, g.ActiveIdPreviousFrame, g.ActiveIdTimer, g.ActiveIdAllowOverlap, GetInputSourceName(g.ActiveIdSource));
        DebugLocateItemOnHover(g.ActiveId);
        Text("ActiveIdWindow: '%s'", g.ActiveIdWindow ? g.ActiveIdWindow->Name : "NULL");
        Text("ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: %X", g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
        Text("HoveredId: 0x%08X (%.2f sec), AllowOverlap: %d", g.HoveredIdPreviousFrame, g.HoveredIdTimer, g.HoveredIdAllowOverlap); // Not displaying g.HoveredId as it is update mid-frame
        Text("HoverItemDelayId: 0x%08X, Timer: %.2f, ClearTimer: %.2f", g.HoverItemDelayId, g.HoverItemDelayTimer, g.HoverItemDelayClearTimer);
        Text("DragDrop: %d, SourceId = 0x%08X, Payload \"%s\" (%d bytes)", g.DragDropActive, g.DragDropPayload.SourceId, g.DragDropPayload.DataType, g.DragDropPayload.DataSize);
        DebugLocateItemOnHover(g.DragDropPayload.SourceId);
        Unindent();

        Text("NAV,FOCUS");
        Indent();
        Text("NavWindow: '%s'", g.NavWindow ? g.NavWindow->Name : "NULL");
        Text("NavId: 0x%08X, NavLayer: %d", g.NavId, g.NavLayer);
        DebugLocateItemOnHover(g.NavId);
        Text("NavInputSource: %s", GetInputSourceName(g.NavInputSource));
        Text("NavLastValidSelectionUserData = %" IM_PRId64 " (0x%" IM_PRIX64 ")", g.NavLastValidSelectionUserData, g.NavLastValidSelectionUserData);
        Text("NavActive: %d, NavVisible: %d", g.IO.NavActive, g.IO.NavVisible);
        Text("NavActivateId/DownId/PressedId: %08X/%08X/%08X", g.NavActivateId, g.NavActivateDownId, g.NavActivatePressedId);
        Text("NavActivateFlags: %04X", g.NavActivateFlags);
        Text("NavCursorVisible: %d, NavHighlightItemUnderNav: %d", g.NavCursorVisible, g.NavHighlightItemUnderNav);
        Text("NavFocusScopeId = 0x%08X", g.NavFocusScopeId);
        Text("NavFocusRoute[] = ");
        for (int path_n = g.NavFocusRoute.Size - 1; path_n >= 0; path_n--)
        {
            const ImGuiFocusScopeData& focus_scope = g.NavFocusRoute[path_n];
            SameLine(0.0f, 0.0f);
            Text("0x%08X/", focus_scope.ID);
            SetItemTooltip("In window \"%s\"", FindWindowByID(focus_scope.WindowID)->Name);
        }
        Text("NavWindowingTarget: '%s'", g.NavWindowingTarget ? g.NavWindowingTarget->Name : "NULL");
        Unindent();

        TreePop();
    }

    // Overlay: Display windows Rectangles and Begin Order
    if (cfg->ShowWindowsRects || cfg->ShowWindowsBeginOrder)
    {
        for (ImGuiWindow* window : g.Windows)
        {
            if (!window->WasActive)
                continue;
            ImDrawList* draw_list = GetForegroundDrawList(window);
            if (cfg->ShowWindowsRects)
            {
                ImRect r = Funcs::GetWindowRect(window, cfg->ShowWindowsRectsType);
                draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 0, 128, 255));
            }
            if (cfg->ShowWindowsBeginOrder && !(window->Flags & ImGuiWindowFlags_ChildWindow))
            {
                char buf[32];
                ImFormatString(buf, IM_COUNTOF(buf), "%d", window->BeginOrderWithinContext);
                float font_size = GetFontSize();
                draw_list->AddRectFilled(window->Pos, window->Pos + ImVec2(font_size, font_size), IM_COL32(200, 100, 100, 255));
                draw_list->AddText(window->Pos, IM_COL32(255, 255, 255, 255), buf);
            }
        }
    }

    // Overlay: Display Tables Rectangles
    if (cfg->ShowTablesRects)
    {
        for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++)
        {
            ImGuiTable* table = g.Tables.TryGetMapData(table_n);
            if (table == NULL || table->LastFrameActive < g.FrameCount - 1)
                continue;
            ImDrawList* draw_list = GetForegroundDrawList(table->OuterWindow);
            if (cfg->ShowTablesRectsType >= TRT_ColumnsRect)
            {
                for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                {
                    ImRect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, column_n);
                    ImU32 col = (table->HoveredColumnBody == column_n) ? IM_COL32(255, 255, 128, 255) : IM_COL32(255, 0, 128, 255);
                    float thickness = (table->HoveredColumnBody == column_n) ? 3.0f : 1.0f;
                    draw_list->AddRect(r.Min, r.Max, col, 0.0f, 0, thickness);
                }
            }
            else
            {
                ImRect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, -1);
                draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 0, 128, 255));
            }
        }
    }

#ifdef IMGUI_HAS_DOCK
    // Overlay: Display Docking info
    if (cfg->ShowDockingNodes && g.IO.KeyCtrl && g.DebugHoveredDockNode)
    {
        char buf[64] = "";
        char* p = buf;
        ImGuiDockNode* node = g.DebugHoveredDockNode;
        ImDrawList* overlay_draw_list = node->HostWindow ? GetForegroundDrawList(node->HostWindow) : GetForegroundDrawList(GetMainViewport());
        p += ImFormatString(p, buf + IM_COUNTOF(buf) - p, "DockId: %X%s\n", node->ID, node->IsCentralNode() ? " *CentralNode*" : "");
        p += ImFormatString(p, buf + IM_COUNTOF(buf) - p, "WindowClass: %08X\n", node->WindowClass.ClassId);
        p += ImFormatString(p, buf + IM_COUNTOF(buf) - p, "Size: (%.0f, %.0f)\n", node->Size.x, node->Size.y);
        p += ImFormatString(p, buf + IM_COUNTOF(buf) - p, "SizeRef: (%.0f, %.0f)\n", node->SizeRef.x, node->SizeRef.y);
        int depth = DockNodeGetDepth(node);
        overlay_draw_list->AddRect(node->Pos + ImVec2(3, 3) * (float)depth, node->Pos + node->Size - ImVec2(3, 3) * (float)depth, IM_COL32(200, 100, 100, 255));
        ImVec2 pos = node->Pos + ImVec2(3, 3) * (float)depth;
        overlay_draw_list->AddRectFilled(pos - ImVec2(1, 1), pos + CalcTextSize(buf) + ImVec2(1, 1), IM_COL32(200, 100, 100, 255));
        overlay_draw_list->AddText(NULL, 0.0f, pos, IM_COL32(255, 255, 255, 255), buf);
    }
#endif // #ifdef IMGUI_HAS_DOCK

    End();
}

void ImGui::DebugBreakClearData()
{
    // Those fields are scattered in their respective subsystem to stay in hot-data locations
    ImGuiContext& g = *GImGui;
    g.DebugBreakInWindow = 0;
    g.DebugBreakInTable = 0;
    g.DebugBreakInShortcutRouting = ImGuiKey_None;
}

void ImGui::DebugBreakButtonTooltip(bool keyboard_only, const char* description_of_location)
{
    if (!BeginItemTooltip())
        return;
    Text("To call IM_DEBUG_BREAK() %s:", description_of_location);
    Separator();
    TextUnformatted(keyboard_only ? "- Press 'Pause/Break' on keyboard." : "- Press 'Pause/Break' on keyboard.\n- or Click (may alter focus/active id).\n- or navigate using keyboard and press space.");
    Separator();
    TextUnformatted("Choose one way that doesn't interfere with what you are trying to debug!\nYou need a debugger attached or this will crash!");
    EndTooltip();
}

// Special button that doesn't take focus, doesn't take input owner, and can be activated without a click etc.
// In order to reduce interferences with the contents we are trying to debug into.
bool ImGui::DebugBreakButton(const char* label, const char* description_of_location)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos + ImVec2(0.0f, window->DC.CurrLineTextBaseOffset);
    ImVec2 size = ImVec2(label_size.x + g.Style.FramePadding.x * 2.0f, label_size.y);

    const ImRect bb(pos, pos + size);
    ItemSize(size, 0.0f);
    if (!ItemAdd(bb, id))
        return false;

    // WE DO NOT USE ButtonEx() or ButtonBehavior() in order to reduce our side-effects.
    bool hovered = ItemHoverable(bb, id, g.CurrentItemFlags);
    bool pressed = hovered && (IsKeyChordPressed(g.DebugBreakKeyChord) || IsMouseClicked(0) || g.NavActivateId == id);
    DebugBreakButtonTooltip(false, description_of_location);

    ImVec4 col4f = GetStyleColorVec4(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImVec4 hsv;
    ColorConvertRGBtoHSV(col4f.x, col4f.y, col4f.z, hsv.x, hsv.y, hsv.z);
    ColorConvertHSVtoRGB(hsv.x + 0.20f, hsv.y, hsv.z, col4f.x, col4f.y, col4f.z);

    RenderNavCursor(bb, id);
    RenderFrame(bb.Min, bb.Max, GetColorU32(col4f), true, g.Style.FrameRounding);
    RenderTextClipped(bb.Min, bb.Max, label, NULL, &label_size, g.Style.ButtonTextAlign, &bb);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

// [DEBUG] Display contents of Columns
void ImGui::DebugNodeColumns(ImGuiOldColumns* columns)
{
    if (!TreeNode((void*)(uintptr_t)columns->ID, "Columns Id: 0x%08X, Count: %d, Flags: 0x%04X", columns->ID, columns->Count, columns->Flags))
        return;
    BulletText("Width: %.1f (MinX: %.1f, MaxX: %.1f)", columns->OffMaxX - columns->OffMinX, columns->OffMinX, columns->OffMaxX);
    for (ImGuiOldColumnData& column : columns->Columns)
        BulletText("Column %02d: OffsetNorm %.3f (= %.1f px)", (int)columns->Columns.index_from_ptr(&column), column.OffsetNorm, GetColumnOffsetFromNorm(columns, column.OffsetNorm));
    TreePop();
}

static void DebugNodeDockNodeFlags(ImGuiDockNodeFlags* p_flags, const char* label, bool enabled)
{
    using namespace ImGui;
    PushID(label);
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    Text("%s:", label);
    if (!enabled)
        BeginDisabled();
    CheckboxFlags("NoResize", p_flags, ImGuiDockNodeFlags_NoResize);
    CheckboxFlags("NoResizeX", p_flags, ImGuiDockNodeFlags_NoResizeX);
    CheckboxFlags("NoResizeY",p_flags, ImGuiDockNodeFlags_NoResizeY);
    CheckboxFlags("NoTabBar", p_flags, ImGuiDockNodeFlags_NoTabBar);
    CheckboxFlags("HiddenTabBar", p_flags, ImGuiDockNodeFlags_HiddenTabBar);
    CheckboxFlags("NoWindowMenuButton", p_flags, ImGuiDockNodeFlags_NoWindowMenuButton);
    CheckboxFlags("NoCloseButton", p_flags, ImGuiDockNodeFlags_NoCloseButton);
    CheckboxFlags("DockedWindowsInFocusRoute", p_flags, ImGuiDockNodeFlags_DockedWindowsInFocusRoute);
    CheckboxFlags("NoDocking", p_flags, ImGuiDockNodeFlags_NoDocking); // Multiple flags
    CheckboxFlags("NoDockingSplit", p_flags, ImGuiDockNodeFlags_NoDockingSplit);
    CheckboxFlags("NoDockingSplitOther", p_flags, ImGuiDockNodeFlags_NoDockingSplitOther);
    CheckboxFlags("NoDockingOver", p_flags, ImGuiDockNodeFlags_NoDockingOverMe);
    CheckboxFlags("NoDockingOverOther", p_flags, ImGuiDockNodeFlags_NoDockingOverOther);
    CheckboxFlags("NoDockingOverEmpty", p_flags, ImGuiDockNodeFlags_NoDockingOverEmpty);
    CheckboxFlags("NoUndocking", p_flags, ImGuiDockNodeFlags_NoUndocking);
    if (!enabled)
        EndDisabled();
    PopStyleVar();
    PopID();
}

// [DEBUG] Display contents of ImDockNode
void ImGui::DebugNodeDockNode(ImGuiDockNode* node, const char* label)
{
    ImGuiContext& g = *GImGui;
    const bool is_alive = (g.FrameCount - node->LastFrameAlive < 2);    // Submitted with ImGuiDockNodeFlags_KeepAliveOnly
    const bool is_active = (g.FrameCount - node->LastFrameActive < 2);  // Submitted
    if (!is_alive) { PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled)); }
    bool open;
    ImGuiTreeNodeFlags tree_node_flags = node->IsFocused ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
    if (node->Windows.Size > 0)
        open = TreeNodeEx((void*)(intptr_t)node->ID, tree_node_flags, "%s 0x%04X%s: %d windows (vis: '%s')", label, node->ID, node->IsVisible ? "" : " (hidden)", node->Windows.Size, node->VisibleWindow ? node->VisibleWindow->Name : "NULL");
    else
        open = TreeNodeEx((void*)(intptr_t)node->ID, tree_node_flags, "%s 0x%04X%s: %s (vis: '%s')", label, node->ID, node->IsVisible ? "" : " (hidden)", (node->SplitAxis == ImGuiAxis_X) ? "horizontal split" : (node->SplitAxis == ImGuiAxis_Y) ? "vertical split" : "empty", node->VisibleWindow ? node->VisibleWindow->Name : "NULL");
    if (!is_alive) { PopStyleColor(); }
    if (is_active && IsItemHovered())
        if (ImGuiWindow* window = node->HostWindow ? node->HostWindow : node->VisibleWindow)
            GetForegroundDrawList(window)->AddRect(node->Pos, node->Pos + node->Size, IM_COL32(255, 255, 0, 255));
    if (open)
    {
        IM_ASSERT(node->ChildNodes[0] == NULL || node->ChildNodes[0]->ParentNode == node);
        IM_ASSERT(node->ChildNodes[1] == NULL || node->ChildNodes[1]->ParentNode == node);
        BulletText("Pos (%.0f,%.0f), Size (%.0f, %.0f) Ref (%.0f, %.0f)",
            node->Pos.x, node->Pos.y, node->Size.x, node->Size.y, node->SizeRef.x, node->SizeRef.y);
        DebugNodeWindow(node->HostWindow, "HostWindow");
        DebugNodeWindow(node->VisibleWindow, "VisibleWindow");
        BulletText("SelectedTabID: 0x%08X, LastFocusedNodeID: 0x%08X", node->SelectedTabId, node->LastFocusedNodeId);
        BulletText("Misc:%s%s%s%s%s%s%s",
            node->IsDockSpace() ? " IsDockSpace" : "",
            node->IsCentralNode() ? " IsCentralNode" : "",
            is_alive ? " IsAlive" : "", is_active ? " IsActive" : "", node->IsFocused ? " IsFocused" : "",
            node->WantLockSizeOnce ? " WantLockSizeOnce" : "",
            node->HasCentralNodeChild ? " HasCentralNodeChild" : "");
        if (TreeNode("flags", "Flags Merged: 0x%04X, Local: 0x%04X, InWindows: 0x%04X, Shared: 0x%04X", node->MergedFlags, node->LocalFlags, node->LocalFlagsInWindows, node->SharedFlags))
        {
            if (BeginTable("flags", 4))
            {
                TableNextColumn(); DebugNodeDockNodeFlags(&node->MergedFlags, "MergedFlags", false);
                TableNextColumn(); DebugNodeDockNodeFlags(&node->LocalFlags, "LocalFlags", true);
                TableNextColumn(); DebugNodeDockNodeFlags(&node->LocalFlagsInWindows, "LocalFlagsInWindows", false);
                TableNextColumn(); DebugNodeDockNodeFlags(&node->SharedFlags, "SharedFlags", true);
                EndTable();
            }
            TreePop();
        }
        if (node->ParentNode)
            DebugNodeDockNode(node->ParentNode, "ParentNode");
        if (node->ChildNodes[0])
            DebugNodeDockNode(node->ChildNodes[0], "Child[0]");
        if (node->ChildNodes[1])
            DebugNodeDockNode(node->ChildNodes[1], "Child[1]");
        if (node->TabBar)
            DebugNodeTabBar(node->TabBar, "TabBar");
        DebugNodeWindowsList(&node->Windows, "Windows");

        TreePop();
    }
}

// [DEBUG] Display contents of ImDrawList
// Note that both 'window' and 'viewport' may be NULL here. Viewport is generally null of destroyed popups which previously owned a viewport.
void ImGui::DebugNodeDrawList(ImGuiWindow* window, ImGuiViewportP* viewport, const ImDrawList* draw_list, const char* label)
{
    ImGuiContext& g = *GImGui;
    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    int cmd_count = draw_list->CmdBuffer.Size;
    if (cmd_count > 0 && draw_list->CmdBuffer.back().ElemCount == 0 && draw_list->CmdBuffer.back().UserCallback == NULL)
        cmd_count--;
    bool node_open = TreeNode(draw_list, "%s: '%s' %d vtx, %d indices, %d cmds", label, draw_list->_OwnerName ? draw_list->_OwnerName : "", draw_list->VtxBuffer.Size, draw_list->IdxBuffer.Size, cmd_count);
    if (draw_list == GetWindowDrawList())
    {
        SameLine();
        TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "CURRENTLY APPENDING"); // Can't display stats for active draw list! (we don't have the data double-buffered)
        if (node_open)
            TreePop();
        return;
    }

    ImDrawList* fg_draw_list = viewport ? GetForegroundDrawList(viewport) : NULL; // Render additional visuals into the top-most draw list
    if (window && IsItemHovered() && fg_draw_list)
        fg_draw_list->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 255, 0, 255));
    if (!node_open)
        return;

    if (window && !window->WasActive)
        TextDisabled("Warning: owning Window is inactive. This DrawList is not being rendered!");

    for (const ImDrawCmd* pcmd = draw_list->CmdBuffer.Data; pcmd < draw_list->CmdBuffer.Data + cmd_count; pcmd++)
    {
        if (pcmd->UserCallback)
        {
            BulletText("Callback %p, user_data %p", pcmd->UserCallback, pcmd->UserCallbackData);
            continue;
        }

        char texid_desc[30];
        FormatTextureRefForDebugDisplay(texid_desc, IM_COUNTOF(texid_desc), pcmd->TexRef);
        char buf[300];
        ImFormatString(buf, IM_COUNTOF(buf), "DrawCmd:%5d tris, Tex %s, ClipRect (%4.0f,%4.0f)-(%4.0f,%4.0f)",
            pcmd->ElemCount / 3, texid_desc, pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
        bool pcmd_node_open = TreeNode((void*)(pcmd - draw_list->CmdBuffer.begin()), "%s", buf);
        if (IsItemHovered() && (cfg->ShowDrawCmdMesh || cfg->ShowDrawCmdBoundingBoxes) && fg_draw_list)
            DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd, cfg->ShowDrawCmdMesh, cfg->ShowDrawCmdBoundingBoxes);
        if (!pcmd_node_open)
            continue;

        // Calculate approximate coverage area (touched pixel count)
        // This will be in pixels squared as long there's no post-scaling happening to the renderer output.
        const ImDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
        const ImDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + pcmd->VtxOffset;
        float total_area = 0.0f;
        for (unsigned int idx_n = pcmd->IdxOffset; idx_n < pcmd->IdxOffset + pcmd->ElemCount; )
        {
            ImVec2 triangle[3];
            for (int n = 0; n < 3; n++, idx_n++)
                triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos;
            total_area += ImTriangleArea(triangle[0], triangle[1], triangle[2]);
        }

        // Display vertex information summary. Hover to get all triangles drawn in wire-frame
        ImFormatString(buf, IM_COUNTOF(buf), "Mesh: ElemCount: %d, VtxOffset: +%d, IdxOffset: +%d, Area: ~%0.f px", pcmd->ElemCount, pcmd->VtxOffset, pcmd->IdxOffset, total_area);
        Selectable(buf);
        if (IsItemHovered() && fg_draw_list)
            DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd, true, false);

        // Display individual triangles/vertices. Hover on to get the corresponding triangle highlighted.
        ImGuiListClipper clipper;
        clipper.Begin(pcmd->ElemCount / 3); // Manually coarse clip our print out of individual vertices to save CPU, only items that may be visible.
        while (clipper.Step())
            for (int prim = clipper.DisplayStart, idx_i = pcmd->IdxOffset + clipper.DisplayStart * 3; prim < clipper.DisplayEnd; prim++)
            {
                char* buf_p = buf, * buf_end = buf + IM_COUNTOF(buf);
                ImVec2 triangle[3];
                for (int n = 0; n < 3; n++, idx_i++)
                {
                    const ImDrawVert& v = vtx_buffer[idx_buffer ? idx_buffer[idx_i] : idx_i];
                    triangle[n] = v.pos;
                    buf_p += ImFormatString(buf_p, buf_end - buf_p, "%s %04d: pos (%8.2f,%8.2f), uv (%.6f,%.6f), col %08X\n",
                        (n == 0) ? "Vert:" : "     ", idx_i, v.pos.x, v.pos.y, v.uv.x, v.uv.y, v.col);
                }

                Selectable(buf, false);
                if (fg_draw_list && IsItemHovered())
                {
                    ImDrawListFlags backup_flags = fg_draw_list->Flags;
                    fg_draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines; // Disable AA on triangle outlines is more readable for very large and thin triangles.
                    fg_draw_list->AddPolyline(triangle, 3, IM_COL32(255, 255, 0, 255), ImDrawFlags_Closed, 1.0f);
                    fg_draw_list->Flags = backup_flags;
                }
            }
        TreePop();
    }
    TreePop();
}

// [DEBUG] Display mesh/aabb of a ImDrawCmd
void ImGui::DebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList* out_draw_list, const ImDrawList* draw_list, const ImDrawCmd* draw_cmd, bool show_mesh, bool show_aabb)
{
    IM_ASSERT(show_mesh || show_aabb);

    // Draw wire-frame version of all triangles
    ImRect clip_rect = draw_cmd->ClipRect;
    ImRect vtxs_rect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    ImDrawListFlags backup_flags = out_draw_list->Flags;
    out_draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines; // Disable AA on triangle outlines is more readable for very large and thin triangles.
    for (unsigned int idx_n = draw_cmd->IdxOffset, idx_end = draw_cmd->IdxOffset + draw_cmd->ElemCount; idx_n < idx_end; )
    {
        ImDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL; // We don't hold on those pointers past iterations as ->AddPolyline() may invalidate them if out_draw_list==draw_list
        ImDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + draw_cmd->VtxOffset;

        ImVec2 triangle[3];
        for (int n = 0; n < 3; n++, idx_n++)
            vtxs_rect.Add((triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos));
        if (show_mesh)
            out_draw_list->AddPolyline(triangle, 3, IM_COL32(255, 255, 0, 255), ImDrawFlags_Closed, 1.0f); // In yellow: mesh triangles
    }
    // Draw bounding boxes
    if (show_aabb)
    {
        out_draw_list->AddRect(ImTrunc(clip_rect.Min), ImTrunc(clip_rect.Max), IM_COL32(255, 0, 255, 255)); // In pink: clipping rectangle submitted to GPU
        out_draw_list->AddRect(ImTrunc(vtxs_rect.Min), ImTrunc(vtxs_rect.Max), IM_COL32(0, 255, 255, 255)); // In cyan: bounding box of triangles
    }
    out_draw_list->Flags = backup_flags;
}

// [DEBUG] Compute mask of inputs with the same codepoint.
static int CalcFontGlyphSrcOverlapMask(ImFontAtlas* atlas, ImFont* font, unsigned int codepoint)
{
    int mask = 0, count = 0;
    for (int src_n = 0; src_n < font->Sources.Size; src_n++)
    {
        ImFontConfig* src = font->Sources[src_n];
        if (!(src->FontLoader ? src->FontLoader : atlas->FontLoader)->FontSrcContainsGlyph(atlas, src, (ImWchar)codepoint))
            continue;
        mask |= (1 << src_n);
        count++;
    }
    return count > 1 ? mask : 0;
}

// [DEBUG] Display details for a single font, called by ShowStyleEditor().
void ImGui::DebugNodeFont(ImFont* font)
{
    ImGuiContext& g = *GImGui;
    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    ImFontAtlas* atlas = font->OwnerAtlas;
    bool opened = TreeNode(font, "Font: \"%s\": %d sources(s)", font->GetDebugName(), font->Sources.Size);

    // Display preview text
    if (!opened)
        Indent();
    Indent();
    if (cfg->ShowFontPreview)
    {
        PushFont(font, 0.0f);
        Text("The quick brown fox jumps over the lazy dog");
        PopFont();
    }
    if (!opened)
    {
        Unindent();
        Unindent();
        return;
    }
    if (SmallButton("Set as default"))
        GetIO().FontDefault = font;
    SameLine();
    BeginDisabled(atlas->Fonts.Size <= 1 || atlas->Locked);
    if (SmallButton("Remove"))
        atlas->RemoveFont(font);
    EndDisabled();
    SameLine();
    if (SmallButton("Clear bakes"))
        ImFontAtlasFontDiscardBakes(atlas, font, 0);
    SameLine();
    if (SmallButton("Clear unused"))
        ImFontAtlasFontDiscardBakes(atlas, font, 2);

    // Display details
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    SetNextItemWidth(GetFontSize() * 8);
    DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");
    /*SameLine(); MetricsHelpMarker(
        "Note that the default embedded font is NOT meant to be scaled.\n\n"
        "Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
        "You may oversample them to get some flexibility with scaling. "
        "You can also render at multiple sizes and select which one to use at runtime.\n\n"
        "(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");*/
#endif

    char c_str[5];
    ImTextCharToUtf8(c_str, font->FallbackChar);
    Text("Fallback character: '%s' (U+%04X)", c_str, font->FallbackChar);
    ImTextCharToUtf8(c_str, font->EllipsisChar);
    Text("Ellipsis character: '%s' (U+%04X)", c_str, font->EllipsisChar);

    for (int src_n = 0; src_n < font->Sources.Size; src_n++)
    {
        ImFontConfig* src = font->Sources[src_n];
        if (TreeNode(src, "Input %d: \'%s\' [%d], Oversample: %d,%d, PixelSnapH: %d, Offset: (%.1f,%.1f)",
            src_n, src->Name, src->FontNo, src->OversampleH, src->OversampleV, src->PixelSnapH, src->GlyphOffset.x, src->GlyphOffset.y))
        {
            const ImFontLoader* loader = src->FontLoader ? src->FontLoader : atlas->FontLoader;
            Text("Loader: '%s'", loader->Name ? loader->Name : "N/A");

            //if (DragFloat("ExtraSizeScale", &src->ExtraSizeScale, 0.01f, 0.10f, 2.0f))
            //    ImFontAtlasFontRebuildOutput(atlas, font);
#ifdef IMGUI_ENABLE_FREETYPE
            if (loader->Name != NULL && strcmp(loader->Name, "FreeType") == 0)
            {
                unsigned int loader_flags = src->FontLoaderFlags;
                Text("FreeType Loader Flags: 0x%08X", loader_flags);
                if (ImGuiFreeType::DebugEditFontLoaderFlags(&loader_flags))
                {
                    ImFontAtlasFontDestroyOutput(atlas, font);
                    src->FontLoaderFlags = loader_flags;
                    ImFontAtlasFontInitOutput(atlas, font);
                }
            }
#endif
            TreePop();
        }
    }
    if (font->Sources.Size > 1 && TreeNode("Input Glyphs Overlap Detection Tool"))
    {
        TextWrapped("- First Input that contains the glyph is used.\n"
            "- Use ImFontConfig::GlyphExcludeRanges[] to specify ranges to ignore glyph in given Input.\n- Prefer using a small number of ranges as the list is scanned every time a new glyph is loaded,\n  - e.g. GlyphExcludeRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };\n- This tool doesn't cache results and is slow, don't keep it open!");
        if (BeginTable("table", 2))
        {
            for (unsigned int c = 0; c < 0x10000; c++)
                if (int overlap_mask = CalcFontGlyphSrcOverlapMask(atlas, font, c))
                {
                    unsigned int c_end = c + 1;
                    while (c_end < 0x10000 && CalcFontGlyphSrcOverlapMask(atlas, font, c_end) == overlap_mask)
                        c_end++;
                    if (TableNextColumn() && TreeNode((void*)(intptr_t)c, "U+%04X-U+%04X: %d codepoints in %d inputs", c, c_end - 1, c_end - c, ImCountSetBits(overlap_mask)))
                    {
                        char utf8_buf[5];
                        for (unsigned int n = c; n < c_end; n++)
                        {
                            ImTextCharToUtf8(utf8_buf, n);
                            BulletText("Codepoint U+%04X (%s)", n, utf8_buf);
                        }
                        TreePop();
                    }
                    TableNextColumn();
                    for (int src_n = 0; src_n < font->Sources.Size; src_n++)
                        if (overlap_mask & (1 << src_n))
                        {
                            Text("%d ", src_n);
                            SameLine();
                        }
                    c = c_end - 1;
                }
            EndTable();
        }
        TreePop();
    }

    // Display all glyphs of the fonts in separate pages of 256 characters
    for (int baked_n = 0; baked_n < atlas->Builder->BakedPool.Size; baked_n++)
    {
        ImFontBaked* baked = &atlas->Builder->BakedPool[baked_n];
        if (baked->OwnerFont != font)
            continue;
        PushID(baked->BakedId);
        if (TreeNode("Glyphs", "Baked at { %.2fpx, d.%.2f }: %d glyphs%s", baked->Size, baked->RasterizerDensity, baked->Glyphs.Size, (baked->LastUsedFrame < atlas->Builder->FrameCount - 1) ? " *Unused*" : ""))
        {
            if (SmallButton("Load all"))
                for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base++)
                    baked->FindGlyph((ImWchar)base);

            const int surface_sqrt = (int)ImSqrt((float)baked->MetricsTotalSurface);
            Text("Ascent: %f, Descent: %f, Ascent-Descent: %f", baked->Ascent, baked->Descent, baked->Ascent - baked->Descent);
            Text("Texture Area: about %d px ~%dx%d px", baked->MetricsTotalSurface, surface_sqrt, surface_sqrt);
            for (int src_n = 0; src_n < font->Sources.Size; src_n++)
            {
                ImFontConfig* src = font->Sources[src_n];
                int oversample_h, oversample_v;
                ImFontAtlasBuildGetOversampleFactors(src, baked, &oversample_h, &oversample_v);
                BulletText("Input %d: \'%s\', Oversample: (%d=>%d,%d=>%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
                    src_n, src->Name, src->OversampleH, oversample_h, src->OversampleV, oversample_v, src->PixelSnapH, src->GlyphOffset.x, src->GlyphOffset.y);
            }

            DebugNodeFontGlyphsForSrcMask(font, baked, ~0);
            TreePop();
        }
        PopID();
    }
    TreePop();
    Unindent();
}

void ImGui::DebugNodeFontGlyphsForSrcMask(ImFont* font, ImFontBaked* baked, int src_mask)
{
    ImDrawList* draw_list = GetWindowDrawList();
    const ImU32 glyph_col = GetColorU32(ImGuiCol_Text);
    const float cell_size = baked->Size * 1;
    const float cell_spacing = GetStyle().ItemSpacing.y;
    for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
    {
        // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
        // This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
        // is large // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
        if (!(base & 8191) && font->IsGlyphRangeUnused(base, base + 8191))
        {
            base += 8192 - 256;
            continue;
        }

        int count = 0;
        for (unsigned int n = 0; n < 256; n++)
            if (const ImFontGlyph* glyph = baked->IsGlyphLoaded((ImWchar)(base + n)) ? baked->FindGlyph((ImWchar)(base + n)) : NULL)
                if (src_mask & (1 << glyph->SourceIdx))
                    count++;
        if (count <= 0)
            continue;
        if (!TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
            continue;

        // Draw a 16x16 grid of glyphs
        ImVec2 base_pos = GetCursorScreenPos();
        for (unsigned int n = 0; n < 256; n++)
        {
            // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions
            // available here and thus cannot easily generate a zero-terminated UTF-8 encoded string.
            ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
            ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
            const ImFontGlyph* glyph = baked->IsGlyphLoaded((ImWchar)(base + n)) ? baked->FindGlyph((ImWchar)(base + n)) : NULL;
            draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
            if (!glyph || (src_mask & (1 << glyph->SourceIdx)) == 0)
                continue;
            font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (ImWchar)(base + n));
            if (IsMouseHoveringRect(cell_p1, cell_p2) && BeginTooltip())
            {
                DebugNodeFontGlyph(font, glyph);
                EndTooltip();
            }
        }
        Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
        TreePop();
    }
}

void ImGui::DebugNodeFontGlyph(ImFont* font, const ImFontGlyph* glyph)
{
    Text("Codepoint: U+%04X", glyph->Codepoint);
    Separator();
    Text("Visible: %d", glyph->Visible);
    Text("AdvanceX: %.1f", glyph->AdvanceX);
    Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
    Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
    if (glyph->PackId >= 0)
    {
        ImTextureRect* r = ImFontAtlasPackGetRect(font->OwnerAtlas, glyph->PackId);
        Text("PackId: 0x%X (%dx%d rect at %d,%d)", glyph->PackId, r->w, r->h, r->x, r->y);
    }
    Text("SourceIdx: %d", glyph->SourceIdx);
}

// [DEBUG] Display contents of ImGuiStorage
void ImGui::DebugNodeStorage(ImGuiStorage* storage, const char* label)
{
    if (!TreeNode(label, "%s: %d entries, %d bytes", label, storage->Data.Size, storage->Data.size_in_bytes()))
        return;
    for (const ImGuiStoragePair& p : storage->Data)
    {
        BulletText("Key 0x%08X Value { i: %d }", p.key, p.val_i); // Important: we currently don't store a type, real value may not be integer.
        DebugLocateItemOnHover(p.key);
    }
    TreePop();
}

// [DEBUG] Display contents of ImGuiTabBar
void ImGui::DebugNodeTabBar(ImGuiTabBar* tab_bar, const char* label)
{
    // Standalone tab bars (not associated to docking/windows functionality) currently hold no discernible strings.
    char buf[256];
    char* p = buf;
    const char* buf_end = buf + IM_COUNTOF(buf);
    const bool is_active = (tab_bar->PrevFrameVisible >= GetFrameCount() - 2);
    p += ImFormatString(p, buf_end - p, "%s 0x%08X (%d tabs)%s  {", label, tab_bar->ID, tab_bar->Tabs.Size, is_active ? "" : " *Inactive*");
    for (int tab_n = 0; tab_n < ImMin(tab_bar->Tabs.Size, 3); tab_n++)
    {
        ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
        p += ImFormatString(p, buf_end - p, "%s'%s'", tab_n > 0 ? ", " : "", TabBarGetTabName(tab_bar, tab));
    }
    p += ImFormatString(p, buf_end - p, (tab_bar->Tabs.Size > 3) ? " ... }" : " } ");
    if (!is_active) { PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled)); }
    bool open = TreeNode(label, "%s", buf);
    if (!is_active) { PopStyleColor(); }
    if (is_active && IsItemHovered())
    {
        ImDrawList* draw_list = GetForegroundDrawList(tab_bar->Window);
        draw_list->AddRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max, IM_COL32(255, 255, 0, 255));
        draw_list->AddLine(ImVec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Min.y), ImVec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Max.y), IM_COL32(0, 255, 0, 255));
        draw_list->AddLine(ImVec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Min.y), ImVec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Max.y), IM_COL32(0, 255, 0, 255));
    }
    if (open)
    {
        for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
        {
            ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
            PushID(tab);
            if (SmallButton("<")) { TabBarQueueReorder(tab_bar, tab, -1); } SameLine(0, 2);
            if (SmallButton(">")) { TabBarQueueReorder(tab_bar, tab, +1); } SameLine();
            Text("%02d%c Tab 0x%08X '%s' Offset: %.2f, Width: %.2f/%.2f",
                tab_n, (tab->ID == tab_bar->SelectedTabId) ? '*' : ' ', tab->ID, TabBarGetTabName(tab_bar, tab), tab->Offset, tab->Width, tab->ContentWidth);
            PopID();
        }
        TreePop();
    }
}

void ImGui::DebugNodeViewport(ImGuiViewportP* viewport)
{
    ImGuiContext& g = *GImGui;
    SetNextItemOpen(true, ImGuiCond_Once);
    bool open = TreeNode((void*)(intptr_t)viewport->ID, "Viewport #%d, ID: 0x%08X, Parent: 0x%08X, Window: \"%s\"", viewport->Idx, viewport->ID, viewport->ParentViewportId, viewport->Window ? viewport->Window->Name : "N/A");
    if (IsItemHovered())
        g.DebugMetricsConfig.HighlightViewportID = viewport->ID;
    if (open)
    {
        ImGuiWindowFlags flags = viewport->Flags;
        BulletText("Main Pos: (%.0f,%.0f), Size: (%.0f,%.0f)\nFrameBufferScale: (%.2f,%.2f)\nWorkArea Inset Left: %.0f Top: %.0f, Right: %.0f, Bottom: %.0f\nMonitor: %d, DpiScale: %.0f%%",
            viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y,
            viewport->FramebufferScale.x, viewport->FramebufferScale.y,
            viewport->WorkInsetMin.x, viewport->WorkInsetMin.y, viewport->WorkInsetMax.x, viewport->WorkInsetMax.y,
            viewport->PlatformMonitor, viewport->DpiScale * 100.0f);
        if (viewport->Idx > 0) { SameLine(); if (SmallButton("Reset Pos")) { viewport->Pos = ImVec2(200, 200); viewport->UpdateWorkRect(); if (viewport->Window) viewport->Window->Pos = viewport->Pos; } }
        BulletText("Flags: 0x%04X =%s%s%s%s%s%s%s%s%s%s%s%s%s", viewport->Flags,
            //(flags & ImGuiViewportFlags_IsPlatformWindow) ? " IsPlatformWindow" : "", // Omitting because it is the standard
            (flags & ImGuiViewportFlags_IsPlatformMonitor) ? " IsPlatformMonitor" : "",
            (flags & ImGuiViewportFlags_IsMinimized) ? " IsMinimized" : "",
            (flags & ImGuiViewportFlags_IsFocused) ? " IsFocused" : "",
            (flags & ImGuiViewportFlags_OwnedByApp) ? " OwnedByApp" : "",
            (flags & ImGuiViewportFlags_NoDecoration) ? " NoDecoration" : "",
            (flags & ImGuiViewportFlags_NoTaskBarIcon) ? " NoTaskBarIcon" : "",
            (flags & ImGuiViewportFlags_NoFocusOnAppearing) ? " NoFocusOnAppearing" : "",
            (flags & ImGuiViewportFlags_NoFocusOnClick) ? " NoFocusOnClick" : "",
            (flags & ImGuiViewportFlags_NoInputs) ? " NoInputs" : "",
            (flags & ImGuiViewportFlags_NoRendererClear) ? " NoRendererClear" : "",
            (flags & ImGuiViewportFlags_NoAutoMerge) ? " NoAutoMerge" : "",
            (flags & ImGuiViewportFlags_TopMost) ? " TopMost" : "",
            (flags & ImGuiViewportFlags_CanHostOtherWindows) ? " CanHostOtherWindows" : "");
        for (ImDrawList* draw_list : viewport->DrawDataP.CmdLists)
            DebugNodeDrawList(NULL, viewport, draw_list, "DrawList");
        TreePop();
    }
}

void ImGui::DebugNodePlatformMonitor(ImGuiPlatformMonitor* monitor, const char* label, int idx)
{
    BulletText("%s %d: DPI %.0f%%\n MainMin (%.0f,%.0f), MainMax (%.0f,%.0f), MainSize (%.0f,%.0f)\n WorkMin (%.0f,%.0f), WorkMax (%.0f,%.0f), WorkSize (%.0f,%.0f)",
        label, idx, monitor->DpiScale * 100.0f,
        monitor->MainPos.x, monitor->MainPos.y, monitor->MainPos.x + monitor->MainSize.x, monitor->MainPos.y + monitor->MainSize.y, monitor->MainSize.x, monitor->MainSize.y,
        monitor->WorkPos.x, monitor->WorkPos.y, monitor->WorkPos.x + monitor->WorkSize.x, monitor->WorkPos.y + monitor->WorkSize.y, monitor->WorkSize.x, monitor->WorkSize.y);
}

void ImGui::DebugNodeWindow(ImGuiWindow* window, const char* label)
{
    if (window == NULL)
    {
        BulletText("%s: NULL", label);
        return;
    }

    ImGuiContext& g = *GImGui;
    const bool is_active = window->WasActive;
    ImGuiTreeNodeFlags tree_node_flags = (window == g.NavWindow) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
    if (!is_active) { PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled)); }
    const bool open = TreeNodeEx(label, tree_node_flags, "%s '%s'%s", label, window->Name, is_active ? "" : " *Inactive*");
    if (!is_active) { PopStyleColor(); }
    if (IsItemHovered() && is_active)
        GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 255, 0, 255));
    if (!open)
        return;

    if (window->MemoryCompacted)
        TextDisabled("Note: some memory buffers have been compacted/freed.");

    if (g.IO.ConfigDebugIsDebuggerPresent && DebugBreakButton("**DebugBreak**", "in Begin()"))
        g.DebugBreakInWindow = window->ID;

    ImGuiWindowFlags flags = window->Flags;
    DebugNodeDrawList(window, window->Viewport, window->DrawList, "DrawList");
    BulletText("Pos: (%.1f,%.1f), Size: (%.1f,%.1f), ContentSize (%.1f,%.1f) Ideal (%.1f,%.1f)", window->Pos.x, window->Pos.y, window->Size.x, window->Size.y, window->ContentSize.x, window->ContentSize.y, window->ContentSizeIdeal.x, window->ContentSizeIdeal.y);
    BulletText("Flags: 0x%08X (%s%s%s%s%s%s%s%s%s..)", flags,
        (flags & ImGuiWindowFlags_ChildWindow)  ? "Child " : "",      (flags & ImGuiWindowFlags_Tooltip)     ? "Tooltip "   : "",  (flags & ImGuiWindowFlags_Popup) ? "Popup " : "",
        (flags & ImGuiWindowFlags_Modal)        ? "Modal " : "",      (flags & ImGuiWindowFlags_ChildMenu)   ? "ChildMenu " : "",  (flags & ImGuiWindowFlags_NoSavedSettings) ? "NoSavedSettings " : "",
        (flags & ImGuiWindowFlags_NoMouseInputs)? "NoMouseInputs":"", (flags & ImGuiWindowFlags_NoNavInputs) ? "NoNavInputs" : "", (flags & ImGuiWindowFlags_AlwaysAutoResize) ? "AlwaysAutoResize" : "");
    if (flags & ImGuiWindowFlags_ChildWindow)
        BulletText("ChildFlags: 0x%08X (%s%s%s%s..)", window->ChildFlags,
            (window->ChildFlags & ImGuiChildFlags_Borders) ? "Borders " : "",
            (window->ChildFlags & ImGuiChildFlags_ResizeX) ? "ResizeX " : "",
            (window->ChildFlags & ImGuiChildFlags_ResizeY) ? "ResizeY " : "",
            (window->ChildFlags & ImGuiChildFlags_NavFlattened) ? "NavFlattened " : "");
    BulletText("WindowClassId: 0x%08X", window->WindowClass.ClassId);
    BulletText("Scroll: (%.2f/%.2f,%.2f/%.2f) Scrollbar:%s%s", window->Scroll.x, window->ScrollMax.x, window->Scroll.y, window->ScrollMax.y, window->ScrollbarX ? "X" : "", window->ScrollbarY ? "Y" : "");
    BulletText("Active: %d/%d, WriteAccessed: %d, BeginOrderWithinContext: %d", window->Active, window->WasActive, window->WriteAccessed, (window->Active || window->WasActive) ? window->BeginOrderWithinContext : -1);
    BulletText("Appearing: %d, Hidden: %d (CanSkip %d Cannot %d), SkipItems: %d", window->Appearing, window->Hidden, window->HiddenFramesCanSkipItems, window->HiddenFramesCannotSkipItems, window->SkipItems);
    for (int layer = 0; layer < ImGuiNavLayer_COUNT; layer++)
    {
        ImRect r = window->NavRectRel[layer];
        if (r.Min.x >= r.Max.x && r.Min.y >= r.Max.y)
            BulletText("NavLastIds[%d]: 0x%08X", layer, window->NavLastIds[layer]);
        else
            BulletText("NavLastIds[%d]: 0x%08X at +(%.1f,%.1f)(%.1f,%.1f)", layer, window->NavLastIds[layer], r.Min.x, r.Min.y, r.Max.x, r.Max.y);
        DebugLocateItemOnHover(window->NavLastIds[layer]);
    }
    const ImVec2* pr = window->NavPreferredScoringPosRel;
    for (int layer = 0; layer < ImGuiNavLayer_COUNT; layer++)
        BulletText("NavPreferredScoringPosRel[%d] = (%.1f,%.1f)", layer, (pr[layer].x == FLT_MAX ? -99999.0f : pr[layer].x), (pr[layer].y == FLT_MAX ? -99999.0f : pr[layer].y)); // Display as 99999.0f so it looks neater.
    BulletText("NavLayersActiveMask: %X, NavLastChildNavWindow: %s", window->DC.NavLayersActiveMask, window->NavLastChildNavWindow ? window->NavLastChildNavWindow->Name : "NULL");

    BulletText("Viewport: %d%s, ViewportId: 0x%08X, ViewportPos: (%.1f,%.1f)", window->Viewport ? window->Viewport->Idx : -1, window->ViewportOwned ? " (Owned)" : "", window->ViewportId, window->ViewportPos.x, window->ViewportPos.y);
    BulletText("ViewportMonitor: %d", window->Viewport ? window->Viewport->PlatformMonitor : -1);
    BulletText("DockId: 0x%04X, DockOrder: %d, Act: %d, Vis: %d", window->DockId, window->DockOrder, window->DockIsActive, window->DockTabIsVisible);
    if (window->DockNode || window->DockNodeAsHost)
        DebugNodeDockNode(window->DockNodeAsHost ? window->DockNodeAsHost : window->DockNode, window->DockNodeAsHost ? "DockNodeAsHost" : "DockNode");

    if (window->RootWindow != window)               { DebugNodeWindow(window->RootWindow, "RootWindow"); }
    if (window->RootWindowDockTree != window->RootWindow) { DebugNodeWindow(window->RootWindowDockTree, "RootWindowDockTree"); }
    if (window->ParentWindow != NULL)               { DebugNodeWindow(window->ParentWindow, "ParentWindow"); }
    if (window->ParentWindowForFocusRoute != NULL)  { DebugNodeWindow(window->ParentWindowForFocusRoute, "ParentWindowForFocusRoute"); }
    if (window->DC.ChildWindows.Size > 0)           { DebugNodeWindowsList(&window->DC.ChildWindows, "ChildWindows"); }
    if (window->ColumnsStorage.Size > 0 && TreeNode("Columns", "Columns sets (%d)", window->ColumnsStorage.Size))
    {
        for (ImGuiOldColumns& columns : window->ColumnsStorage)
            DebugNodeColumns(&columns);
        TreePop();
    }
    DebugNodeStorage(&window->StateStorage, "Storage");
    TreePop();
}

void ImGui::DebugNodeWindowSettings(ImGuiWindowSettings* settings)
{
    if (settings->WantDelete)
        BeginDisabled();
    Text("0x%08X \"%s\" Pos (%d,%d) Size (%d,%d) Collapsed=%d",
        settings->ID, settings->GetName(), settings->Pos.x, settings->Pos.y, settings->Size.x, settings->Size.y, settings->Collapsed);
    if (settings->WantDelete)
        EndDisabled();
}

void ImGui::DebugNodeWindowsList(ImVector<ImGuiWindow*>* windows, const char* label)
{
    if (!TreeNode(label, "%s (%d)", label, windows->Size))
        return;
    for (int i = windows->Size - 1; i >= 0; i--) // Iterate front to back
    {
        PushID((*windows)[i]);
        DebugNodeWindow((*windows)[i], "Window");
        PopID();
    }
    TreePop();
}

// FIXME-OPT: This is technically suboptimal, but it is simpler this way.
void ImGui::DebugNodeWindowsListByBeginStackParent(ImGuiWindow** windows, int windows_size, ImGuiWindow* parent_in_begin_stack)
{
    for (int i = 0; i < windows_size; i++)
    {
        ImGuiWindow* window = windows[i];
        if (window->ParentWindowInBeginStack != parent_in_begin_stack)
            continue;
        char buf[20];
        ImFormatString(buf, IM_COUNTOF(buf), "[%04d] Window", window->BeginOrderWithinContext);
        //BulletText("[%04d] Window '%s'", window->BeginOrderWithinContext, window->Name);
        DebugNodeWindow(window, buf);
        TreePush(buf);
        DebugNodeWindowsListByBeginStackParent(windows + i + 1, windows_size - i - 1, window);
        TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DEBUG LOG WINDOW
//-----------------------------------------------------------------------------

void ImGui::DebugLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    DebugLogV(fmt, args);
    va_end(args);
}

void ImGui::DebugLogV(const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    const int old_size = g.DebugLogBuf.size();
    if (g.ContextName[0] != 0)
        g.DebugLogBuf.appendf("[%s] [%05d] ", g.ContextName, g.FrameCount);
    else
        g.DebugLogBuf.appendf("[%05d] ", g.FrameCount);
    g.DebugLogBuf.appendfv(fmt, args);
    g.DebugLogIndex.append(g.DebugLogBuf.c_str(), old_size, g.DebugLogBuf.size());

    const char* str = g.DebugLogBuf.begin() + old_size;
    if (g.DebugLogFlags & ImGuiDebugLogFlags_OutputToTTY)
        IMGUI_DEBUG_PRINTF("%s", str);
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
    if (g.DebugLogFlags & ImGuiDebugLogFlags_OutputToDebugger)
    {
        ::OutputDebugStringA("[imgui] ");
        ::OutputDebugStringA(str);
    }
#endif
#ifdef IMGUI_ENABLE_TEST_ENGINE
    // IMGUI_TEST_ENGINE_LOG() adds a trailing \n automatically
    const int new_size = g.DebugLogBuf.size();
    const bool trailing_carriage_return = (g.DebugLogBuf[new_size - 1] == '\n');
    if (g.DebugLogFlags & ImGuiDebugLogFlags_OutputToTestEngine)
        IMGUI_TEST_ENGINE_LOG("%.*s", new_size - old_size - (trailing_carriage_return ? 1 : 0), str);
#endif
}

// FIXME-LAYOUT: To be done automatically via layout mode once we rework ItemSize/ItemAdd into ItemLayout.
static void SameLineOrWrap(const ImVec2& size)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 pos(window->DC.CursorPosPrevLine.x + g.Style.ItemSpacing.x, window->DC.CursorPosPrevLine.y);
    if (window->WorkRect.Contains(ImRect(pos, pos + size)))
        ImGui::SameLine();
}

static void ShowDebugLogFlag(const char* name, ImGuiDebugLogFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImVec2 size(ImGui::GetFrameHeight() + g.Style.ItemInnerSpacing.x + ImGui::CalcTextSize(name).x, ImGui::GetFrameHeight());
    SameLineOrWrap(size); // FIXME-LAYOUT: To be done automatically once we rework ItemSize/ItemAdd into ItemLayout.

    bool highlight_errors = (flags == ImGuiDebugLogFlags_EventError && g.DebugLogSkippedErrors > 0);
    if (highlight_errors)
        ImGui::PushStyleColor(ImGuiCol_Text, ImLerp(g.Style.Colors[ImGuiCol_Text], ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0.30f));
    if (ImGui::CheckboxFlags(name, &g.DebugLogFlags, flags) && g.IO.KeyShift && (g.DebugLogFlags & flags) != 0)
    {
        g.DebugLogAutoDisableFrames = 2;
        g.DebugLogAutoDisableFlags |= flags;
    }
    if (highlight_errors)
    {
        ImGui::PopStyleColor();
        ImGui::SetItemTooltip("%d past errors skipped.", g.DebugLogSkippedErrors);
    }
    else
    {
        ImGui::SetItemTooltip("Hold Shift when clicking to enable for 2 frames only (useful for spammy log entries)");
    }
}

void ImGui::ShowDebugLogWindow(bool* p_open)
{
    ImGuiContext& g = *GImGui;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0)
        SetNextWindowSize(ImVec2(0.0f, GetFontSize() * 12.0f), ImGuiCond_FirstUseEver);
    if (!Begin("Dear ImGui Debug Log", p_open) || GetCurrentWindow()->BeginCount > 1)
    {
        End();
        return;
    }

    ImGuiDebugLogFlags all_enable_flags = ImGuiDebugLogFlags_EventMask_ & ~ImGuiDebugLogFlags_EventInputRouting;
    CheckboxFlags("All", &g.DebugLogFlags, all_enable_flags);
    SetItemTooltip("(except InputRouting which is spammy)");

    ShowDebugLogFlag("Errors", ImGuiDebugLogFlags_EventError);
    ShowDebugLogFlag("ActiveId", ImGuiDebugLogFlags_EventActiveId);
    ShowDebugLogFlag("Clipper", ImGuiDebugLogFlags_EventClipper);
    ShowDebugLogFlag("Docking", ImGuiDebugLogFlags_EventDocking);
    ShowDebugLogFlag("Focus", ImGuiDebugLogFlags_EventFocus);
    ShowDebugLogFlag("IO", ImGuiDebugLogFlags_EventIO);
    ShowDebugLogFlag("Font", ImGuiDebugLogFlags_EventFont);
    ShowDebugLogFlag("Nav", ImGuiDebugLogFlags_EventNav);
    ShowDebugLogFlag("Popup", ImGuiDebugLogFlags_EventPopup);
    ShowDebugLogFlag("Selection", ImGuiDebugLogFlags_EventSelection);
    ShowDebugLogFlag("Viewport", ImGuiDebugLogFlags_EventViewport);
    ShowDebugLogFlag("InputRouting", ImGuiDebugLogFlags_EventInputRouting);

    if (SmallButton("Clear"))
    {
        g.DebugLogBuf.clear();
        g.DebugLogIndex.clear();
        g.DebugLogSkippedErrors = 0;
    }
    SameLine();
    if (SmallButton("Copy"))
        SetClipboardText(g.DebugLogBuf.c_str());
    SameLine();
    if (SmallButton("Configure Outputs.."))
        OpenPopup("Outputs");
    if (BeginPopup("Outputs"))
    {
        CheckboxFlags("OutputToTTY", &g.DebugLogFlags, ImGuiDebugLogFlags_OutputToTTY);
        CheckboxFlags("OutputToDebugger", &g.DebugLogFlags, ImGuiDebugLogFlags_OutputToDebugger);
#ifndef IMGUI_ENABLE_TEST_ENGINE
        BeginDisabled();
#endif
        CheckboxFlags("OutputToTestEngine", &g.DebugLogFlags, ImGuiDebugLogFlags_OutputToTestEngine);
#ifndef IMGUI_ENABLE_TEST_ENGINE
        EndDisabled();
#endif
        EndPopup();
    }

    BeginChild("##log", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

    const ImGuiDebugLogFlags backup_log_flags = g.DebugLogFlags;
    g.DebugLogFlags &= ~ImGuiDebugLogFlags_EventClipper;

    ImGuiListClipper clipper;
    clipper.Begin(g.DebugLogIndex.size());
    while (clipper.Step())
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            DebugTextUnformattedWithLocateItem(g.DebugLogIndex.get_line_begin(g.DebugLogBuf.c_str(), line_no), g.DebugLogIndex.get_line_end(g.DebugLogBuf.c_str(), line_no));
    g.DebugLogFlags = backup_log_flags;
    if (GetScrollY() >= GetScrollMaxY())
        SetScrollHereY(1.0f);
    EndChild();

    End();
}

// Display line, search for 0xXXXXXXXX identifiers and call DebugLocateItemOnHover() when hovered.
void ImGui::DebugTextUnformattedWithLocateItem(const char* line_begin, const char* line_end)
{
    TextUnformatted(line_begin, line_end);
    if (!IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        return;
    ImGuiContext& g = *GImGui;
    ImRect text_rect = g.LastItemData.Rect;
    for (const char* p = line_begin; p <= line_end - 10; p++)
    {
        ImGuiID id = 0;
        if (p[0] != '0' || (p[1] != 'x' && p[1] != 'X') || sscanf(p + 2, "%X", &id) != 1 || ImCharIsXdigitA(p[10]))
            continue;
        ImVec2 p0 = CalcTextSize(line_begin, p);
        ImVec2 p1 = CalcTextSize(p, p + 10);
        g.LastItemData.Rect = ImRect(text_rect.Min + ImVec2(p0.x, 0.0f), text_rect.Min + ImVec2(p0.x + p1.x, p1.y));
        if (IsMouseHoveringRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, true))
            DebugLocateItemOnHover(id);
        p += 10;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] OTHER DEBUG TOOLS (ITEM PICKER, ID STACK TOOL)
//-----------------------------------------------------------------------------

// Draw a small cross at current CursorPos in current window's DrawList
void ImGui::DebugDrawCursorPos(ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 pos = window->DC.CursorPos;
    window->DrawList->AddLine(ImVec2(pos.x, pos.y - 3.0f), ImVec2(pos.x, pos.y + 4.0f), col, 1.0f);
    window->DrawList->AddLine(ImVec2(pos.x - 3.0f, pos.y), ImVec2(pos.x + 4.0f, pos.y), col, 1.0f);
}

// Draw a 10px wide rectangle around CurposPos.x using Line Y1/Y2 in current window's DrawList
void ImGui::DebugDrawLineExtents(ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float curr_x = window->DC.CursorPos.x;
    float line_y1 = (window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y : window->DC.CursorPos.y);
    float line_y2 = line_y1 + (window->DC.IsSameLine ? window->DC.PrevLineSize.y : window->DC.CurrLineSize.y);
    window->DrawList->AddLine(ImVec2(curr_x - 5.0f, line_y1), ImVec2(curr_x + 5.0f, line_y1), col, 1.0f);
    window->DrawList->AddLine(ImVec2(curr_x - 0.5f, line_y1), ImVec2(curr_x - 0.5f, line_y2), col, 1.0f);
    window->DrawList->AddLine(ImVec2(curr_x - 5.0f, line_y2), ImVec2(curr_x + 5.0f, line_y2), col, 1.0f);
}

// Draw last item rect in ForegroundDrawList (so it is always visible)
void ImGui::DebugDrawItemRect(ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    GetForegroundDrawList(window)->AddRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, col);
}

// [DEBUG] Locate item position/rectangle given an ID.
static const ImU32 DEBUG_LOCATE_ITEM_COLOR = IM_COL32(0, 255, 0, 255);  // Green

void ImGui::DebugLocateItem(ImGuiID target_id)
{
    ImGuiContext& g = *GImGui;
    g.DebugLocateId = target_id;
    g.DebugLocateFrames = 2;
    g.DebugBreakInLocateId = false;
}

// FIXME: Doesn't work over through a modal window, because they clear HoveredWindow.
void ImGui::DebugLocateItemOnHover(ImGuiID target_id)
{
    if (target_id == 0 || !IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        return;
    ImGuiContext& g = *GImGui;
    DebugLocateItem(target_id);
    GetForegroundDrawList(g.CurrentWindow)->AddRect(g.LastItemData.Rect.Min - ImVec2(3.0f, 3.0f), g.LastItemData.Rect.Max + ImVec2(3.0f, 3.0f), DEBUG_LOCATE_ITEM_COLOR);

    // Can't easily use a context menu here because it will mess with focus, active id etc.
    if (g.IO.ConfigDebugIsDebuggerPresent && g.MouseStationaryTimer > 1.0f)
    {
        DebugBreakButtonTooltip(false, "in ItemAdd()");
        if (IsKeyChordPressed(g.DebugBreakKeyChord))
            g.DebugBreakInLocateId = true;
    }
}

void ImGui::DebugLocateItemResolveWithLastItem()
{
    ImGuiContext& g = *GImGui;

    // [DEBUG] Debug break requested by user
    if (g.DebugBreakInLocateId)
        IM_DEBUG_BREAK();

    ImGuiLastItemData item_data = g.LastItemData;
    g.DebugLocateId = 0;
    ImDrawList* draw_list = GetForegroundDrawList(g.CurrentWindow);
    ImRect r = item_data.Rect;
    r.Expand(3.0f);
    ImVec2 p1 = g.IO.MousePos;
    ImVec2 p2 = ImVec2((p1.x < r.Min.x) ? r.Min.x : (p1.x > r.Max.x) ? r.Max.x : p1.x, (p1.y < r.Min.y) ? r.Min.y : (p1.y > r.Max.y) ? r.Max.y : p1.y);
    draw_list->AddRect(r.Min, r.Max, DEBUG_LOCATE_ITEM_COLOR);
    draw_list->AddLine(p1, p2, DEBUG_LOCATE_ITEM_COLOR);
}

void ImGui::DebugStartItemPicker()
{
    ImGuiContext& g = *GImGui;
    g.DebugItemPickerActive = true;
}

// [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to visually select an item and break into its call-stack.
void ImGui::UpdateDebugToolItemPicker()
{
    ImGuiContext& g = *GImGui;
    g.DebugItemPickerBreakId = 0;
    if (!g.DebugItemPickerActive)
        return;

    const ImGuiID hovered_id = g.HoveredIdPreviousFrame;
    SetMouseCursor(ImGuiMouseCursor_Hand);
    if (IsKeyPressed(ImGuiKey_Escape))
        g.DebugItemPickerActive = false;
    const bool change_mapping = g.IO.KeyMods == (ImGuiMod_Ctrl | ImGuiMod_Shift);
    if (!change_mapping && IsMouseClicked(g.DebugItemPickerMouseButton) && hovered_id)
    {
        g.DebugItemPickerBreakId = hovered_id;
        g.DebugItemPickerActive = false;
    }
    for (int mouse_button = 0; mouse_button < 3; mouse_button++)
        if (change_mapping && IsMouseClicked(mouse_button))
            g.DebugItemPickerMouseButton = (ImU8)mouse_button;
    SetNextWindowBgAlpha(0.70f);
    if (!BeginTooltip())
        return;
    Text("HoveredId: 0x%08X", hovered_id);
    Text("Press ESC to abort picking.");
    const char* mouse_button_names[] = { "Left", "Right", "Middle" };
    if (change_mapping)
        Text("Remap w/ Ctrl+Shift: click anywhere to select new mouse button.");
    else
        TextColored(GetStyleColorVec4(hovered_id ? ImGuiCol_Text : ImGuiCol_TextDisabled), "Click %s Button to break in debugger! (remap w/ Ctrl+Shift)", mouse_button_names[g.DebugItemPickerMouseButton]);
    EndTooltip();
}

// Update queries. The steps are: -1: query Stack, >= 0: query each stack item
// We can only perform 1 ID Info query every frame. This is designed so the GetID() tests are cheap and constant-time
static ImGuiID DebugItemPathQuery_UpdateAndGetHookId(ImGuiDebugItemPathQuery* query, ImGuiID id)
{
    // Update query. Clear hook when no active query
    if (query->MainID != id)
    {
        query->MainID = id;
        query->Step = -1;
        query->Complete = false;
        query->Results.resize(0);
        query->ResultsDescBuf.resize(0);
    }
    query->Active = false;
    if (id == 0)
        return 0;

    // Advance to next stack level when we got our result, or after 2 frames (in case we never get a result)
    if (query->Step >= 0 && query->Step < query->Results.Size)
        if (query->Results[query->Step].QuerySuccess || query->Results[query->Step].QueryFrameCount > 2)
            query->Step++;

    // Update status and hook
    query->Complete = (query->Step == query->Results.Size);
    if (query->Step == -1)
    {
        query->Active = true;
        return id;
    }
    else if (query->Step >= 0 && query->Step < query->Results.Size)
    {
        query->Results[query->Step].QueryFrameCount++;
        query->Active = true;
        return query->Results[query->Step].ID;
    }
    return 0;
}

// [DEBUG] ID Stack Tool: update query. Called by NewFrame()
void ImGui::UpdateDebugToolItemPathQuery()
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = 0;
    if (g.DebugIDStackTool.LastActiveFrame + 1 == g.FrameCount)
        id = g.HoveredIdPreviousFrame ? g.HoveredIdPreviousFrame : g.ActiveId;
    g.DebugHookIdInfoId = DebugItemPathQuery_UpdateAndGetHookId(&g.DebugItemPathQuery, id);
}

// [DEBUG] ID Stack tool: hooks called by GetID() family functions
void ImGui::DebugHookIdInfo(ImGuiID id, ImGuiDataType data_type, const void* data_id, const void* data_id_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiDebugItemPathQuery* query = &g.DebugItemPathQuery;
    if (query->Active == false)
    {
        IM_ASSERT(id == 0);
        return;
    }
    ImGuiWindow* window = g.CurrentWindow;

    // Step -1: stack query
    // This assumes that the ID was computed with the current ID stack, which tends to be the case for our widget.
    if (query->Step == -1)
    {
        IM_ASSERT(query->Results.Size == 0);
        query->Step++;
        query->Results.resize(window->IDStack.Size + 1, ImGuiStackLevelInfo());
        for (int n = 0; n < window->IDStack.Size + 1; n++)
            query->Results[n].ID = (n < window->IDStack.Size) ? window->IDStack[n] : id;
        return;
    }

    // Step 0+: query for individual level
    IM_ASSERT(query->Step >= 0);
    if (query->Step != window->IDStack.Size)
        return;
    ImGuiStackLevelInfo* info = &query->Results[query->Step];
    IM_ASSERT(info->ID == id && info->QueryFrameCount > 0);

    if (info->DescOffset == -1)
    {
        const char* result = NULL;
        const char* result_end = NULL;
        switch (data_type)
        {
        case ImGuiDataType_S32:
            ImFormatStringToTempBuffer(&result, &result_end, "%d", (int)(intptr_t)data_id);
            break;
        case ImGuiDataType_String:
            ImFormatStringToTempBuffer(&result, &result_end, "%.*s", data_id_end ? (int)((const char*)data_id_end - (const char*)data_id) : (int)ImStrlen((const char*)data_id), (const char*)data_id);
            break;
        case ImGuiDataType_Pointer:
            ImFormatStringToTempBuffer(&result, &result_end, "(void*)0x%p", data_id);
            break;
        case ImGuiDataType_ID:
            // PushOverrideID() is often used to avoid hashing twice, which would lead to 2 calls to DebugHookIdInfo(). We prioritize the first one.
            ImFormatStringToTempBuffer(&result, &result_end, "0x%08X [override]", id);
            break;
        default:
            IM_ASSERT(0);
        }
        info->DescOffset = query->ResultsDescBuf.size();
        query->ResultsDescBuf.append(result, result_end + 1); // Include zero terminator
    }
    info->QuerySuccess = true;
    if (info->DataType == -1)
        info->DataType = (ImS8)data_type;
}

static int DebugItemPathQuery_FormatLevelInfo(ImGuiDebugItemPathQuery* query, int n, bool format_for_ui, char* buf, size_t buf_size)
{
    ImGuiStackLevelInfo* info = &query->Results[n];
    ImGuiWindow* window = (info->DescOffset == -1 && n == 0) ? ImGui::FindWindowByID(info->ID) : NULL;
    if (window)                                 // Source: window name (because the root ID don't call GetID() and so doesn't get hooked)
        return ImFormatString(buf, buf_size, format_for_ui ? "\"%s\" [window]" : "%s", ImHashSkipUncontributingPrefix(window->Name));
    if (info->QuerySuccess)                     // Source: GetID() hooks (prioritize over ItemInfo() because we frequently use patterns like: PushID(str), Button("") where they both have same id)
        return ImFormatString(buf, buf_size, (format_for_ui && info->DataType == ImGuiDataType_String) ? "\"%s\"" : "%s", ImHashSkipUncontributingPrefix(&query->ResultsDescBuf.Buf[info->DescOffset]));
    if (query->Step < query->Results.Size)      // Only start using fallback below when all queries are done, so during queries we don't flickering ??? markers.
        return (*buf = 0);
#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (const char* label = ImGuiTestEngine_FindItemDebugLabel(GImGui, info->ID)) // Source: ImGuiTestEngine's ItemInfo()
        return ImFormatString(buf, buf_size, format_for_ui ? "??? \"%s\"" : "%s", ImHashSkipUncontributingPrefix(label));
#endif
    return ImFormatString(buf, buf_size, "???");
}

static const char* DebugItemPathQuery_GetResultAsPath(ImGuiDebugItemPathQuery* query, bool hex_encode_non_ascii_chars)
{
    ImGuiTextBuffer* buf = &query->ResultPathBuf;
    buf->resize(0);
    for (int stack_n = 0; stack_n < query->Results.Size; stack_n++)
    {
        char level_desc[256];
        DebugItemPathQuery_FormatLevelInfo(query, stack_n, false, level_desc, IM_COUNTOF(level_desc));
        buf->append(stack_n == 0 ? "//" : "/");
        for (const char* p = level_desc; *p != 0; )
        {
            unsigned int c;
            const char* p_next = p + ImTextCharFromUtf8(&c, p, NULL);
            if (c == '/')
                buf->append("\\");
            if (c < 256 || !hex_encode_non_ascii_chars)
                buf->append(p, p_next);
            else for (; p < p_next; p++)
                buf->appendf("\\x%02x", (unsigned char)*p);
            p = p_next;
        }
    }
    return buf->c_str();
}

// ID Stack Tool: Display UI
void ImGui::ShowIDStackToolWindow(bool* p_open)
{
    ImGuiContext& g = *GImGui;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0)
        SetNextWindowSize(ImVec2(0.0f, GetFontSize() * 8.0f), ImGuiCond_FirstUseEver);
    if (!Begin("Dear ImGui ID Stack Tool", p_open) || GetCurrentWindow()->BeginCount > 1)
    {
        End();
        return;
    }

    ImGuiDebugItemPathQuery* query = &g.DebugItemPathQuery;
    ImGuiIDStackTool* tool = &g.DebugIDStackTool;
    tool->LastActiveFrame = g.FrameCount;
    const char* result_path = DebugItemPathQuery_GetResultAsPath(query, tool->OptHexEncodeNonAsciiChars);
    Text("0x%08X", query->MainID);
    SameLine();
    MetricsHelpMarker("Hover an item with the mouse to display elements of the ID Stack leading to the item's final ID.\nEach level of the stack correspond to a PushID() call.\nAll levels of the stack are hashed together to make the final ID of a widget (ID displayed at the bottom level of the stack).\nRead FAQ entry about the ID stack for details.");

    // Ctrl+C to copy path
    const float time_since_copy = (float)g.Time - tool->CopyToClipboardLastTime;
    PushStyleVarY(ImGuiStyleVar_FramePadding, 0.0f);
    Checkbox("Hex-encode non-ASCII", &tool->OptHexEncodeNonAsciiChars);
    SameLine();
    Checkbox("Ctrl+C: copy path", &tool->OptCopyToClipboardOnCtrlC);
    PopStyleVar();
    SameLine();
    TextColored((time_since_copy >= 0.0f && time_since_copy < 0.75f && ImFmod(time_since_copy, 0.25f) < 0.25f * 0.5f) ? ImVec4(1.f, 1.f, 0.3f, 1.f) : ImVec4(), "*COPIED*");
    if (tool->OptCopyToClipboardOnCtrlC && Shortcut(ImGuiMod_Ctrl | ImGuiKey_C, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverFocused))
    {
        tool->CopyToClipboardLastTime = (float)g.Time;
        SetClipboardText(result_path);
    }

    Text("- Path \"%s\"", query->Complete ? result_path : "");
#ifdef IMGUI_ENABLE_TEST_ENGINE
    Text("- Label \"%s\"", query->MainID ? ImGuiTestEngine_FindItemDebugLabel(&g, query->MainID) : "");
#endif
    Separator();

    // Display decorated stack
    if (query->Results.Size > 0 && BeginTable("##table", 3, ImGuiTableFlags_Borders))
    {
        const float id_width = CalcTextSize("0xDDDDDDDD").x;
        TableSetupColumn("Seed", ImGuiTableColumnFlags_WidthFixed, id_width);
        TableSetupColumn("PushID", ImGuiTableColumnFlags_WidthStretch);
        TableSetupColumn("Result", ImGuiTableColumnFlags_WidthFixed, id_width);
        TableHeadersRow();
        for (int n = 0; n < query->Results.Size; n++)
        {
            ImGuiStackLevelInfo* info = &query->Results[n];
            TableNextColumn();
            Text("0x%08X", (n > 0) ? query->Results[n - 1].ID : 0);
            TableNextColumn();
            DebugItemPathQuery_FormatLevelInfo(query, n, true, g.TempBuffer.Data, g.TempBuffer.Size);
            TextUnformatted(g.TempBuffer.Data);
            TableNextColumn();
            Text("0x%08X", info->ID);
            if (n == query->Results.Size - 1)
                TableSetBgColor(ImGuiTableBgTarget_CellBg, GetColorU32(ImGuiCol_Header));
        }
        EndTable();
    }
    End();
}

#else

void ImGui::ShowMetricsWindow(bool*) {}
void ImGui::ShowFontAtlas(ImFontAtlas*) {}
void ImGui::DebugNodeColumns(ImGuiOldColumns*) {}
void ImGui::DebugNodeDrawList(ImGuiWindow*, ImGuiViewportP*, const ImDrawList*, const char*) {}
void ImGui::DebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList*, const ImDrawList*, const ImDrawCmd*, bool, bool) {}
void ImGui::DebugNodeFont(ImFont*) {}
void ImGui::DebugNodeFontGlyphsForSrcMask(ImFont*, ImFontBaked*, int) {}
void ImGui::DebugNodeStorage(ImGuiStorage*, const char*) {}
void ImGui::DebugNodeTabBar(ImGuiTabBar*, const char*) {}
void ImGui::DebugNodeWindow(ImGuiWindow*, const char*) {}
void ImGui::DebugNodeWindowSettings(ImGuiWindowSettings*) {}
void ImGui::DebugNodeWindowsList(ImVector<ImGuiWindow*>*, const char*) {}
void ImGui::DebugNodeViewport(ImGuiViewportP*) {}

void ImGui::ShowDebugLogWindow(bool*) {}
void ImGui::ShowIDStackToolWindow(bool*) {}
void ImGui::DebugStartItemPicker() {}
void ImGui::DebugHookIdInfo(ImGuiID, ImGuiDataType, const void*, const void*) {}

#endif // #ifndef IMGUI_DISABLE_DEBUG_TOOLS

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS) || !defined(IMGUI_DISABLE_DEBUG_TOOLS)
// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is the more flexible one.
void ImGui::ShowFontSelector(const char* label)
{
    ImGuiIO& io = GetIO();
    ImFont* font_current = GetFont();
    if (BeginCombo(label, font_current->GetDebugName()))
    {
        for (ImFont* font : io.Fonts->Fonts)
        {
            PushID((void*)font);
            if (Selectable(font->GetDebugName(), font == font_current, ImGuiSelectableFlags_SelectOnNav))
                io.FontDefault = font;
            if (font == font_current)
                SetItemDefaultFocus();
            PopID();
        }
        EndCombo();
    }
    SameLine();
    if (io.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        MetricsHelpMarker(
            "- Load additional fonts with io.Fonts->AddFontXXX() functions.\n"
            "- Read FAQ and docs/FONTS.md for more details.");
    else
        MetricsHelpMarker(
            "- Load additional fonts with io.Fonts->AddFontXXX() functions.\n"
            "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
            "- Read FAQ and docs/FONTS.md for more details.\n"
            "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}
#endif // #if !defined(IMGUI_DISABLE_DEMO_WINDOWS) || !defined(IMGUI_DISABLE_DEBUG_TOOLS)

#endif // #ifndef IMGUI_DISABLE
