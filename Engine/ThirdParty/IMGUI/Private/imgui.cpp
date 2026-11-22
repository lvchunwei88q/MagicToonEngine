// dear imgui, v1.92.8 WIP
// (main code)
//
// Copyright (c) 2014-2026 Omar Cornut
// Developed by Omar Cornut and every direct or indirect contributors to the GitHub.
// See LICENSE.txt for copyright and licensing details (standard MIT License).
//
// -----------------------------------------------------------------------------
// DOCUMENTATION HAS MOVED OUT OF THIS FILE
// -----------------------------------------------------------------------------
// Upstream keeps ~1,200 lines of documentation in this header. It now lives here:
//   docs/imgui.cpp-notes.en.md     - English, upstream structure (source of truth)
//   docs/imgui.cpp-notes.zh-CN.md  - 中文版（正文译成中文，API 变更日志保留英文）
// Read those before asking "how do I integrate this" or "why did my code break on upgrade".
//
// Kept here because it is useful while reading the code below:
//   - Demo code ..... ImGui::ShowDemoWindow() in imgui_demo.cpp
//   - Integration ... https://github.com/ocornut/imgui/wiki/Getting-Started
//   - FAQ ........... https://dearimgui.com/faq
//   - API changes ... docs/imgui.cpp-notes.en.md, section "API BREAKING CHANGES"
//   - Releases ...... https://github.com/ocornut/imgui/releases
//   - Code map ...... the [SECTION] banners in this file
//
// Notes:
//   - Don't modify imgui.cpp: 'ImGui::' is a namespace, so you can add functions to it
//     from your own source files. imgui_internal.h exposes internals, but with no
//     forward-compatibility guarantee.
//   - Keep IM_ASSERT enabled (it maps to assert() by default, and can be redirected).
//     When an assert fires, read the comments around it.
// -----------------------------------------------------------------------------

//-------------------------------------------------------------------------
// [SECTION] INCLUDES
//-------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"

// System includes
#include <stdio.h>      // vsnprintf, sscanf, printf
#include <stdint.h>     // intptr_t

// io moudel
#include <IO.h> 

#include "imgui_platform_private.h"

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)             // condition expression is constant
#pragma warning (disable: 4996)             // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#if defined(_MSC_VER) && _MSC_VER >= 1922   // MSVC 2019 16.2 or later
#pragma warning (disable: 5054)             // operator '|': deprecated between enumerations of different types
#endif
#pragma warning (disable: 26451)            // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to an 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26495)            // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#pragma warning (disable: 26812)            // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat"                         // warning: format specifies type 'int' but the argument has type 'unsigned int'
#pragma clang diagnostic ignored "-Wformat-nonliteral"              // warning: format string is not a string literal            // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wformat-pedantic"                // warning: format specifies type 'void *' but the argument has type 'xxxx *' // unreasonable, would lead to casting every %p arg to void*. probably enabled by -pedantic.
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor     // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wglobal-constructors"            // warning: declaration requires a global destructor         // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type 'int'
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#pragma clang diagnostic ignored "-Wswitch-default"                 // warning: 'switch' missing 'default' label
#elif defined(__GNUC__)
// We disable -Wpragmas because GCC doesn't provide a has_warning equivalent and some forks/patches may not follow the warning/version association.
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"                  // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"              // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wformat"                           // warning: format '%p' expects argument of type 'int'/'void*', but argument X has type 'unsigned int'/'ImGuiWindow*'
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wformat-nonliteral"                // warning: format not a string literal, format string not checked
#pragma GCC diagnostic ignored "-Wstrict-overflow"                  // warning: assuming signed overflow does not occur when assuming that (X - c) > X is always false
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#pragma GCC diagnostic ignored "-Wcast-qual"                        // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#pragma GCC diagnostic ignored "-Wsign-conversion"                  // warning: conversion to 'xxxx' from 'xxxx' may change the sign of the result
#endif

// Debug options

// Default font size if unspecified in both style.FontSizeBase and AddFontXXX() calls.
static const float FONT_DEFAULT_SIZE_BASE = 20.0f;

// When using Ctrl+Tab (or Gamepad Square+L/R) we delay the visual a little in order to reduce visual noise doing a fast switch.
// Tooltip offset
//-------------------------------------------------------------------------
// [SECTION] FORWARD DECLARATIONS
//-------------------------------------------------------------------------


static void             AddWindowToSortBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window);

// Settings
static void             WindowSettingsHandler_ClearAll(ImGuiContext*, ImGuiSettingsHandler*);
static void*            WindowSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
static void             WindowSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
static void             WindowSettingsHandler_ApplyAll(ImGuiContext*, ImGuiSettingsHandler*);
static void             WindowSettingsHandler_WriteAll(ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* buf);

namespace ImGui
{
// Item

// Window Focus

// Navigation

// Misc
static void             UpdateFontsNewFrame();
static void             UpdateFontsEndFrame();
static void             UpdateTexturesNewFrame();
static void             UpdateTexturesEndFrame();
static void             UpdateSettings();
static void             RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col);
static void             RenderDimmedBackgrounds();

// Viewports

}

//-----------------------------------------------------------------------------
// [SECTION] CONTEXT AND MEMORY ALLOCATORS
//-----------------------------------------------------------------------------

// DLL users:
// - Heaps and globals are not shared across DLL boundaries!
// - You will need to call SetCurrentContext() + SetAllocatorFunctions() for each static/DLL boundary you are calling from.
// - Same applies for hot-reloading mechanisms that are reliant on reloading DLL (note that many hot-reloading mechanisms work without DLL).
// - Using Dear ImGui via a shared library is not recommended, because of function call overhead and because we don't guarantee backward nor forward ABI compatibility.
// - Confused? In a debugger: add GImGui to your watch window and notice how its value changes depending on your current location (which DLL boundary you are in).

// Current context pointer. Implicitly used by all Dear ImGui functions. Always assumed to be != NULL.
// - ImGui::CreateContext() will automatically set this pointer if it is NULL.
//   Change to a different context by calling ImGui::SetCurrentContext().
// - Important: Dear ImGui functions are not thread-safe because of this pointer.
//   If you want thread-safety to allow N threads to access N different contexts:
//   - Change this variable to use thread local storage so each thread can refer to a different context, in your imconfig.h:
//         struct ImGuiContext;
//         extern thread_local ImGuiContext* MyImGuiTLS;
//         #define GImGui MyImGuiTLS
//     And then define MyImGuiTLS in one of your cpp files. Note that thread_local is a C++11 keyword, earlier C++ uses compiler-specific keyword.
//   - Future development aims to make this context pointer explicit to all calls. Also read https://github.com/ocornut/imgui/issues/586
//   - If you need a finite number of contexts, you may compile and use multiple instances of the ImGui code from a different namespace.
// - DLL users: read comments above.
#ifndef GImGui
ImGuiContext*   GImGui = NULL;
#endif

// Memory Allocator functions. Use SetAllocatorFunctions() to change them.
// - You probably don't want to modify that mid-program, and if you use global/static e.g. ImVector<> instances you may need to keep them accessible during program destruction.
// - DLL users: read comments above.
#ifndef IMGUI_DISABLE_DEFAULT_ALLOCATORS
static void*   MallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); return malloc(size); }
static void    FreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); free(ptr); }
#else
static void*   MallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); IM_UNUSED(size); IM_ASSERT(0); return NULL; }
static void    FreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); IM_UNUSED(ptr); IM_ASSERT(0); }
#endif
static ImGuiMemAllocFunc    GImAllocatorAllocFunc = MallocWrapper;
static ImGuiMemFreeFunc     GImAllocatorFreeFunc = FreeWrapper;
static void*                GImAllocatorUserData = NULL;

//-----------------------------------------------------------------------------
// [SECTION] USER FACING STRUCTURES (ImGuiStyle, ImGuiIO, ImGuiPlatformIO)
//-----------------------------------------------------------------------------

ImGuiStyle::ImGuiStyle()
{
    FontSizeBase                = 0.0f;             // Will default to io.Fonts->Fonts[0] on first frame.
    FontScaleMain               = 1.0f;             // Main scale factor. May be set by application once, or exposed to end-user.
    FontScaleDpi                = 1.0f;             // Additional scale factor from viewport/monitor contents scale. When io.ConfigDpiScaleFonts is enabled, this is automatically overwritten when changing monitor DPI.

    Alpha                       = 1.0f;             // Global alpha applies to everything in Dear ImGui.
    DisabledAlpha               = 0.60f;            // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    WindowPadding               = ImVec2(8,8);      // Padding within a window
    WindowRounding              = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    WindowBorderSize            = 1.0f;             // Thickness of border around windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    WindowBorderHoverPadding    = 4.0f;             // Hit-testing extent outside/inside resizing border. Also extend determination of hovered window. Generally meaningfully larger than WindowBorderSize to make it easy to reach borders.
    WindowMinSize               = ImVec2(32,32);    // Minimum window size
    WindowTitleAlign            = ImVec2(0.0f,0.5f);// Alignment for title bar text
    WindowMenuButtonPosition    = ImGuiDir_Left;    // Position of the collapsing/docking button in the title bar (left/right). Defaults to ImGuiDir_Left.
    ChildRounding               = 0.0f;             // Radius of child window corners rounding. Set to 0.0f to have rectangular child windows
    ChildBorderSize             = 1.0f;             // Thickness of border around child windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    PopupRounding               = 0.0f;             // Radius of popup window corners rounding. Set to 0.0f to have rectangular child windows
    PopupBorderSize             = 1.0f;             // Thickness of border around popup or tooltip windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    FramePadding                = ImVec2(4,3);      // Padding within a framed rectangle (used by most widgets)
    FrameRounding               = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
    FrameBorderSize             = 0.0f;             // Thickness of border around frames. Generally set to 0.0f or 1.0f. Other values not well tested.
    ItemSpacing                 = ImVec2(8,4);      // Horizontal and vertical spacing between widgets/lines
    ItemInnerSpacing            = ImVec2(4,4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
    CellPadding                 = ImVec2(4,2);      // Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows.
    TouchExtraPadding           = ImVec2(0,0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    IndentSpacing               = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    ColumnsMinSpacing           = 6.0f;             // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    ScrollbarSize               = 14.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
    ScrollbarRounding           = 9.0f;             // Radius of grab corners rounding for scrollbar
    ScrollbarPadding            = 2.0f;             // Padding of scrollbar grab within its frame (same for both axes)
    GrabMinSize                 = 12.0f;            // Minimum width/height of a grab box for slider/scrollbar
    GrabRounding                = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    LogSliderDeadzone           = 4.0f;             // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    ImageRounding               = 0.0f;             // Rounding of Image() calls.
    ImageBorderSize             = 0.0f;             // Thickness of border around tabs.
    TabRounding                 = 5.0f;             // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    TabBorderSize               = 0.0f;             // Thickness of border around tabs.
    TabMinWidthBase             = 1.0f;             // Minimum tab width, to make tabs larger than their contents. TabBar buttons are not affected.
    TabMinWidthShrink           = 80.0f;            // Minimum tab width after shrinking, when using ImGuiTabBarFlags_FittingPolicyMixed policy. FLT_MAX: never shrink, will behave like ImGuiTabBarFlags_FittingPolicyScroll.
    TabCloseButtonMinWidthSelected   = -1.0f;       // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width.
    TabCloseButtonMinWidthUnselected = 0.0f;        // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never show close button when unselected.
    TabBarBorderSize            = 1.0f;             // Thickness of tab-bar separator, which takes on the tab active color to denote focus.
    TabBarOverlineSize          = 1.0f;             // Thickness of tab-bar overline, which highlights the selected tab-bar.
    TableAngledHeadersAngle     = 35.0f * (IM_PI / 180.0f); // Angle of angled headers (supported values range from -50 degrees to +50 degrees).
    TableAngledHeadersTextAlign = ImVec2(0.5f,0.0f);// Alignment of angled headers within the cell
    TreeLinesFlags              = ImGuiTreeNodeFlags_DrawLinesNone;
    TreeLinesSize               = 1.0f;             // Thickness of outlines when using ImGuiTreeNodeFlags_DrawLines.
    TreeLinesRounding           = 0.0f;             // Radius of lines connecting child nodes to the vertical line.
    DragDropTargetRounding      = 0.0f;             // Radius of the drag and drop target frame.
    DragDropTargetBorderSize    = 2.0f;             // Thickness of the drag and drop target border.
    DragDropTargetPadding       = 3.0f;             // Size to expand the drag and drop target from actual target item size.
    ColorMarkerSize             = 3.0f;             // Size of R/G/B/A color markers for ColorEdit4() and for Drags/Sliders when using ImGuiSliderFlags_ColorMarkers.
    ColorButtonPosition         = ImGuiDir_Right;   // Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
    ButtonTextAlign             = ImVec2(0.5f,0.5f);// Alignment of button text when button is larger than text.
    SelectableTextAlign         = ImVec2(0.0f,0.0f);// Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    SeparatorSize               = 1.0f;             // Thickness of border in Separator().
    SeparatorTextBorderSize     = 3.0f;             // Thickness of border in SeparatorText().
    SeparatorTextAlign          = ImVec2(0.0f,0.5f);// Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center).
    SeparatorTextPadding        = ImVec2(20.0f,3.f);// Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y.
    DisplayWindowPadding        = ImVec2(19,19);    // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
    DisplaySafeAreaPadding      = ImVec2(3,3);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
    DockingNodeHasCloseButton   = true;             // Docking nodes have their own CloseButton() to close all docked windows.
    DockingSeparatorSize        = 2.0f;             // Thickness of resizing border between docked windows
    MouseCursorScale            = 1.0f;             // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
    AntiAliasedLines            = true;             // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU.
    AntiAliasedLinesUseTex      = true;             // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    AntiAliasedFill             = true;             // Enable anti-aliased filled shapes (rounded rectangles, circles, etc.).
    CurveTessellationTol        = 1.25f;            // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    CircleTessellationMaxError  = 0.30f;            // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

    // Behaviors
    HoverStationaryDelay        = 0.15f;            // Delay for IsItemHovered(ImGuiHoveredFlags_Stationary). Time required to consider mouse stationary.
    HoverDelayShort             = 0.15f;            // Delay for IsItemHovered(ImGuiHoveredFlags_DelayShort). Usually used along with HoverStationaryDelay.
    HoverDelayNormal            = 0.40f;            // Delay for IsItemHovered(ImGuiHoveredFlags_DelayNormal). "
    HoverFlagsForTooltipMouse   = ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled;    // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using mouse.
    HoverFlagsForTooltipNav     = ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled;  // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.

    // [Internal]
    _MainScale                  = 1.0f;
    _NextFrameFontSizeBase      = 0.0f;

    // Default theme
    ImGui::StyleColorsDark(this);
}


// Scale all spacing/padding/thickness values. Do not scale fonts.
// Consider not calling this if your initial scale factor if <1.0.
// Important: This operation is lossy because we round all sizes to integer. If you need to change your scale multiples, call this over a freshly initialized ImGuiStyle structure rather than scaling multiple times.
void ImGuiStyle::ScaleAllSizes(float scale_factor)
{
    _MainScale *= scale_factor;
    WindowPadding = ImTrunc(WindowPadding * scale_factor);
    WindowRounding = ImTrunc(WindowRounding * scale_factor);
    WindowBorderSize = ImTrunc(WindowBorderSize * scale_factor);
    WindowMinSize = ImTrunc(WindowMinSize * scale_factor);
    WindowBorderHoverPadding = ImTrunc(WindowBorderHoverPadding * scale_factor);
    ChildRounding = ImTrunc(ChildRounding * scale_factor);
    ChildBorderSize = ImTrunc(ChildBorderSize * scale_factor);
    PopupRounding = ImTrunc(PopupRounding * scale_factor);
    PopupBorderSize = ImTrunc(PopupBorderSize * scale_factor);
    FramePadding = ImTrunc(FramePadding * scale_factor);
    FrameBorderSize = ImTrunc(FrameBorderSize * scale_factor);
    FrameRounding = ImTrunc(FrameRounding * scale_factor);
    ItemSpacing = ImTrunc(ItemSpacing * scale_factor);
    ItemInnerSpacing = ImTrunc(ItemInnerSpacing * scale_factor);
    CellPadding = ImTrunc(CellPadding * scale_factor);
    TouchExtraPadding = ImTrunc(TouchExtraPadding * scale_factor);
    IndentSpacing = ImTrunc(IndentSpacing * scale_factor);
    ColumnsMinSpacing = ImTrunc(ColumnsMinSpacing * scale_factor);
    ScrollbarSize = ImTrunc(ScrollbarSize * scale_factor);
    ScrollbarRounding = ImTrunc(ScrollbarRounding * scale_factor);
    ScrollbarPadding = ImTrunc(ScrollbarPadding * scale_factor);
    GrabMinSize = ImTrunc(GrabMinSize * scale_factor);
    GrabRounding = ImTrunc(GrabRounding * scale_factor);
    LogSliderDeadzone = ImTrunc(LogSliderDeadzone * scale_factor);
    ImageRounding = ImTrunc(ImageRounding * scale_factor);
    ImageBorderSize = ImTrunc(ImageBorderSize * scale_factor);
    TabRounding = ImTrunc(TabRounding * scale_factor);
    TabBorderSize = ImTrunc(TabBorderSize * scale_factor);
    TabMinWidthBase = ImTrunc(TabMinWidthBase * scale_factor);
    TabMinWidthShrink = ImTrunc(TabMinWidthShrink * scale_factor);
    TabCloseButtonMinWidthSelected = (TabCloseButtonMinWidthSelected > 0.0f && TabCloseButtonMinWidthSelected != FLT_MAX) ? ImTrunc(TabCloseButtonMinWidthSelected * scale_factor) : TabCloseButtonMinWidthSelected;
    TabCloseButtonMinWidthUnselected = (TabCloseButtonMinWidthUnselected > 0.0f && TabCloseButtonMinWidthUnselected != FLT_MAX) ? ImTrunc(TabCloseButtonMinWidthUnselected * scale_factor) : TabCloseButtonMinWidthUnselected;
    TabBarBorderSize = ImTrunc(TabBarBorderSize * scale_factor);
    TabBarOverlineSize = ImTrunc(TabBarOverlineSize * scale_factor);
    TreeLinesSize = ImTrunc(TreeLinesSize * scale_factor);
    TreeLinesRounding = ImTrunc(TreeLinesRounding * scale_factor);
    DragDropTargetRounding = ImTrunc(DragDropTargetRounding * scale_factor);
    DragDropTargetBorderSize = ImTrunc(DragDropTargetBorderSize * scale_factor);
    DragDropTargetPadding = ImTrunc(DragDropTargetPadding * scale_factor);
    ColorMarkerSize = ImTrunc(ColorMarkerSize * scale_factor);
    SeparatorSize = ImTrunc(SeparatorSize * scale_factor);
    SeparatorTextBorderSize = ImTrunc(SeparatorTextBorderSize * scale_factor);
    SeparatorTextPadding = ImTrunc(SeparatorTextPadding * scale_factor);
    DockingSeparatorSize = ImTrunc(DockingSeparatorSize * scale_factor);
    DisplayWindowPadding = ImTrunc(DisplayWindowPadding * scale_factor);
    DisplaySafeAreaPadding = ImTrunc(DisplaySafeAreaPadding * scale_factor);
    MouseCursorScale = ImTrunc(MouseCursorScale * scale_factor);
}

ImGuiIO::ImGuiIO()
{
    // Most fields are initialized with zero
    memset((void*)this, 0, sizeof(*this));
    IM_STATIC_ASSERT(IM_COUNTOF(ImGuiIO::MouseDown) == ImGuiMouseButton_COUNT && IM_COUNTOF(ImGuiIO::MouseClicked) == ImGuiMouseButton_COUNT);

    // Settings
    ConfigFlags = ImGuiConfigFlags_None;
    BackendFlags = ImGuiBackendFlags_None;
    DisplaySize = ImVec2(-1.0f, -1.0f);
    DeltaTime = 1.0f / 60.0f;
    IniSavingRate = 5.0f;
    IniFilename = "imgui.ini"; // Important: "imgui.ini" is relative to current working dir, most apps will want to lock this to an absolute path (e.g. same path as executables).
    LogFilename = "imgui_log.txt";
    UserData = NULL;

    Fonts = NULL;
    FontDefault = NULL;
    FontAllowUserScaling = false;
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    FontGlobalScale = 1.0f; // Use style.FontScaleMain instead!
#endif
    DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Keyboard/Gamepad Navigation options
    ConfigNavSwapGamepadButtons = false;
    ConfigNavMoveSetMousePos = false;
    ConfigNavCaptureKeyboard = true;
    ConfigNavEscapeClearFocusItem = true;
    ConfigNavEscapeClearFocusWindow = false;
    ConfigNavCursorVisibleAuto = true;
    ConfigNavCursorVisibleAlways = false;

    // Docking options (when ImGuiConfigFlags_DockingEnable is set)
    ConfigDockingNoSplit = false;
    ConfigDockingNoDockingOver = false;
    ConfigDockingWithShift = false;
    ConfigDockingAlwaysTabBar = false;
    ConfigDockingTransparentPayload = false;

    // Viewport options (when ImGuiConfigFlags_ViewportsEnable is set)
    ConfigViewportsNoAutoMerge = false;
    ConfigViewportsNoTaskBarIcon = false;
    ConfigViewportsNoDecoration = true;
    ConfigViewportsNoDefaultParent = true;
    ConfigViewportsPlatformFocusSetsImGuiFocus = true;

    // Miscellaneous options
    MouseDrawCursor = false;
#ifdef __APPLE__
    ConfigMacOSXBehaviors = true;  // Set Mac OS X style defaults based on __APPLE__ compile time flag
#else
    ConfigMacOSXBehaviors = false;
#endif
    ConfigInputTrickleEventQueue = true;
    ConfigInputTextCursorBlink = true;
    ConfigInputTextEnterKeepActive = false;
    ConfigDragClickToInputText = false;
    ConfigWindowsResizeFromEdges = true;
    ConfigWindowsMoveFromTitleBarOnly = false;
    ConfigWindowsCopyContentsWithCtrlC = false;
    ConfigScrollbarScrollByPage = true;
    ConfigMemoryCompactTimer = 60.0f;
    ConfigDebugIsDebuggerPresent = false;
    ConfigDebugHighlightIdConflicts = true;
    ConfigDebugHighlightIdConflictsShowItemPicker = true;
    ConfigDebugBeginReturnValueOnce = false;
    ConfigDebugBeginReturnValueLoop = false;

    ConfigErrorRecovery = true;
    ConfigErrorRecoveryEnableAssert = true;
    ConfigErrorRecoveryEnableDebugLog = true;
    ConfigErrorRecoveryEnableTooltip = true;

    // Inputs Behaviors
    MouseDoubleClickTime = 0.30f;
    MouseDoubleClickMaxDist = 6.0f;
    MouseDragThreshold = 6.0f;
    KeyRepeatDelay = 0.275f;
    KeyRepeatRate = 0.050f;

    // Platform Functions
    // Note: Initialize() will setup default clipboard/ime handlers.
    BackendPlatformName = BackendRendererName = NULL;
    BackendPlatformUserData = BackendRendererUserData = BackendLanguageUserData = NULL;

    // Input (NB: we already have memset zero the entire structure!)
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    MousePosPrev = ImVec2(-FLT_MAX, -FLT_MAX);
    MouseSource = ImGuiMouseSource_Mouse;
    for (int i = 0; i < IM_COUNTOF(MouseDownDuration); i++) MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
    for (int i = 0; i < IM_COUNTOF(KeysData); i++) { KeysData[i].DownDuration = KeysData[i].DownDurationPrev = -1.0f; }
    AppAcceptingEvents = true;
}

ImGuiPlatformIO::ImGuiPlatformIO()
{
    // Most fields are initialized with zero
    memset((void*)this, 0, sizeof(*this));
    Platform_LocaleDecimalPoint = '.';
}

//-----------------------------------------------------------------------------
// [SECTION] STYLING
//-----------------------------------------------------------------------------

ImGuiStyle& ImGui::GetStyle()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->Style;
}

ImU32 ImGui::GetColorU32(ImGuiCol idx, float alpha_mul)
{
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = style.Colors[idx];
    c.w *= style.Alpha * alpha_mul;
    return ColorConvertFloat4ToU32(c);
}

ImU32 ImGui::GetColorU32(const ImVec4& col)
{
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = col;
    c.w *= style.Alpha;
    return ColorConvertFloat4ToU32(c);
}

const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol idx)
{
    ImGuiStyle& style = GImGui->Style;
    return style.Colors[idx];
}

ImU32 ImGui::GetColorU32(ImU32 col, float alpha_mul)
{
    ImGuiStyle& style = GImGui->Style;
    alpha_mul *= style.Alpha;
    if (alpha_mul >= 1.0f)
        return col;
    ImU32 a = (col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;
    a = (ImU32)(a * alpha_mul); // We don't need to clamp 0..255 because alpha is in 0..1 range.
    return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}

// FIXME: This may incur a round-trip (if the end user got their data from a float4) but eventually we aim to store the in-flight colors as ImU32
void ImGui::PushStyleColor(ImGuiCol idx, ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorStack.push_back(backup);
    if (g.DebugFlashStyleColorIdx != idx)
        g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void ImGui::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorStack.push_back(backup);
    if (g.DebugFlashStyleColorIdx != idx)
        g.Style.Colors[idx] = col;
}

void ImGui::PopStyleColor(int count)
{
    ImGuiContext& g = *GImGui;
    if (g.ColorStack.Size < count)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopStyleColor() too many times!");
        count = g.ColorStack.Size;
    }
    while (count > 0)
    {
        ImGuiColorMod& backup = g.ColorStack.back();
        g.Style.Colors[backup.Col] = backup.BackupValue;
        g.ColorStack.pop_back();
        count--;
    }
}

static const ImGuiStyleVarInfo GStyleVarsInfo[] =
{
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, Alpha) },                     // ImGuiStyleVar_Alpha
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DisabledAlpha) },             // ImGuiStyleVar_DisabledAlpha
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowPadding) },             // ImGuiStyleVar_WindowPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowRounding) },            // ImGuiStyleVar_WindowRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowBorderSize) },          // ImGuiStyleVar_WindowBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowMinSize) },             // ImGuiStyleVar_WindowMinSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowTitleAlign) },          // ImGuiStyleVar_WindowTitleAlign
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ChildRounding) },             // ImGuiStyleVar_ChildRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ChildBorderSize) },           // ImGuiStyleVar_ChildBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, PopupRounding) },             // ImGuiStyleVar_PopupRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, PopupBorderSize) },           // ImGuiStyleVar_PopupBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FramePadding) },              // ImGuiStyleVar_FramePadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FrameRounding) },             // ImGuiStyleVar_FrameRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FrameBorderSize) },           // ImGuiStyleVar_FrameBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ItemSpacing) },               // ImGuiStyleVar_ItemSpacing
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ItemInnerSpacing) },          // ImGuiStyleVar_ItemInnerSpacing
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, IndentSpacing) },             // ImGuiStyleVar_IndentSpacing
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, CellPadding) },               // ImGuiStyleVar_CellPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarSize) },             // ImGuiStyleVar_ScrollbarSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarRounding) },         // ImGuiStyleVar_ScrollbarRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarPadding) },          // ImGuiStyleVar_ScrollbarPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, GrabMinSize) },               // ImGuiStyleVar_GrabMinSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, GrabRounding) },              // ImGuiStyleVar_GrabRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ImageRounding) },             // ImGuiStyleVar_ImageRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ImageBorderSize) },           // ImGuiStyleVar_ImageBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabRounding) },               // ImGuiStyleVar_TabRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBorderSize) },             // ImGuiStyleVar_TabBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabMinWidthBase) },           // ImGuiStyleVar_TabMinWidthBase
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabMinWidthShrink) },         // ImGuiStyleVar_TabMinWidthShrink
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBarBorderSize) },          // ImGuiStyleVar_TabBarBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBarOverlineSize) },        // ImGuiStyleVar_TabBarOverlineSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TableAngledHeadersAngle)},    // ImGuiStyleVar_TableAngledHeadersAngle
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TableAngledHeadersTextAlign)},// ImGuiStyleVar_TableAngledHeadersTextAlign
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TreeLinesSize)},              // ImGuiStyleVar_TreeLinesSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TreeLinesRounding)},          // ImGuiStyleVar_TreeLinesRounding
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ButtonTextAlign) },           // ImGuiStyleVar_ButtonTextAlign
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SelectableTextAlign) },       // ImGuiStyleVar_SelectableTextAlign
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorSize)},              // ImGuiStyleVar_SeparatorSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextBorderSize)},    // ImGuiStyleVar_SeparatorTextBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextAlign) },        // ImGuiStyleVar_SeparatorTextAlign
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextPadding) },      // ImGuiStyleVar_SeparatorTextPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DockingSeparatorSize) },      // ImGuiStyleVar_DockingSeparatorSize
};

const ImGuiStyleVarInfo* ImGui::GetStyleVarInfo(ImGuiStyleVar idx)
{
    IM_ASSERT(idx >= 0 && idx < ImGuiStyleVar_COUNT);
    IM_STATIC_ASSERT(IM_COUNTOF(GStyleVarsInfo) == ImGuiStyleVar_COUNT);
    return &GStyleVarsInfo[idx];
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, float val)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 1, "Calling PushStyleVar() variant with wrong type!");
    float* pvar = (float*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    *pvar = val;
}

void ImGui::PushStyleVarX(ImGuiStyleVar idx, float val_x)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    pvar->x = val_x;
}

void ImGui::PushStyleVarY(ImGuiStyleVar idx, float val_y)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    pvar->y = val_y;
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    *pvar = val;
}

void ImGui::PopStyleVar(int count)
{
    ImGuiContext& g = *GImGui;
    if (g.StyleVarStack.Size < count)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopStyleVar() too many times!");
        count = g.StyleVarStack.Size;
    }
    while (count > 0)
    {
        // We avoid a generic memcpy(data, &backup.Backup.., GDataTypeSize[info->Type] * info->Count), the overhead in Debug is not worth it.
        ImGuiStyleMod& backup = g.StyleVarStack.back();
        const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(backup.VarIdx);
        void* data = var_info->GetVarPtr(&g.Style);
        if (var_info->DataType == ImGuiDataType_Float && var_info->Count == 1)      { ((float*)data)[0] = backup.BackupFloat[0]; }
        else if (var_info->DataType == ImGuiDataType_Float && var_info->Count == 2) { ((float*)data)[0] = backup.BackupFloat[0]; ((float*)data)[1] = backup.BackupFloat[1]; }
        g.StyleVarStack.pop_back();
        count--;
    }
}

const char* ImGui::GetStyleColorName(ImGuiCol idx)
{
    // Create switch-case from enum with regexp: ImGuiCol_{.*}, --> case ImGuiCol_\1: return "\1";
    switch (idx)
    {
    case ImGuiCol_Text: return "Text";
    case ImGuiCol_TextDisabled: return "TextDisabled";
    case ImGuiCol_WindowBg: return "WindowBg";
    case ImGuiCol_ChildBg: return "ChildBg";
    case ImGuiCol_PopupBg: return "PopupBg";
    case ImGuiCol_Border: return "Border";
    case ImGuiCol_BorderShadow: return "BorderShadow";
    case ImGuiCol_FrameBg: return "FrameBg";
    case ImGuiCol_FrameBgHovered: return "FrameBgHovered";
    case ImGuiCol_FrameBgActive: return "FrameBgActive";
    case ImGuiCol_TitleBg: return "TitleBg";
    case ImGuiCol_TitleBgActive: return "TitleBgActive";
    case ImGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
    case ImGuiCol_MenuBarBg: return "MenuBarBg";
    case ImGuiCol_ScrollbarBg: return "ScrollbarBg";
    case ImGuiCol_ScrollbarGrab: return "ScrollbarGrab";
    case ImGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
    case ImGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
    case ImGuiCol_CheckMark: return "CheckMark";
    case ImGuiCol_SliderGrab: return "SliderGrab";
    case ImGuiCol_SliderGrabActive: return "SliderGrabActive";
    case ImGuiCol_Button: return "Button";
    case ImGuiCol_ButtonHovered: return "ButtonHovered";
    case ImGuiCol_ButtonActive: return "ButtonActive";
    case ImGuiCol_Header: return "Header";
    case ImGuiCol_HeaderHovered: return "HeaderHovered";
    case ImGuiCol_HeaderActive: return "HeaderActive";
    case ImGuiCol_Separator: return "Separator";
    case ImGuiCol_SeparatorHovered: return "SeparatorHovered";
    case ImGuiCol_SeparatorActive: return "SeparatorActive";
    case ImGuiCol_ResizeGrip: return "ResizeGrip";
    case ImGuiCol_ResizeGripHovered: return "ResizeGripHovered";
    case ImGuiCol_ResizeGripActive: return "ResizeGripActive";
    case ImGuiCol_InputTextCursor: return "InputTextCursor";
    case ImGuiCol_TabHovered: return "TabHovered";
    case ImGuiCol_Tab: return "Tab";
    case ImGuiCol_TabSelected: return "TabSelected";
    case ImGuiCol_TabSelectedOverline: return "TabSelectedOverline";
    case ImGuiCol_TabDimmed: return "TabDimmed";
    case ImGuiCol_TabDimmedSelected: return "TabDimmedSelected";
    case ImGuiCol_TabDimmedSelectedOverline: return "TabDimmedSelectedOverline";
    case ImGuiCol_DockingPreview: return "DockingPreview";
    case ImGuiCol_DockingEmptyBg: return "DockingEmptyBg";
    case ImGuiCol_PlotLines: return "PlotLines";
    case ImGuiCol_PlotLinesHovered: return "PlotLinesHovered";
    case ImGuiCol_PlotHistogram: return "PlotHistogram";
    case ImGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
    case ImGuiCol_TableHeaderBg: return "TableHeaderBg";
    case ImGuiCol_TableBorderStrong: return "TableBorderStrong";
    case ImGuiCol_TableBorderLight: return "TableBorderLight";
    case ImGuiCol_TableRowBg: return "TableRowBg";
    case ImGuiCol_TableRowBgAlt: return "TableRowBgAlt";
    case ImGuiCol_TextLink: return "TextLink";
    case ImGuiCol_TextSelectedBg: return "TextSelectedBg";
    case ImGuiCol_TreeLines: return "TreeLines";
    case ImGuiCol_DragDropTarget: return "DragDropTarget";
    case ImGuiCol_DragDropTargetBg: return "DragDropTargetBg";
    case ImGuiCol_UnsavedMarker: return "UnsavedMarker";
    case ImGuiCol_NavCursor: return "NavCursor";
    case ImGuiCol_NavWindowingHighlight: return "NavWindowingHighlight";
    case ImGuiCol_NavWindowingDimBg: return "NavWindowingDimBg";
    case ImGuiCol_ModalWindowDimBg: return "ModalWindowDimBg";
    }
    IM_ASSERT(0);
    return "Unknown";
}

//-----------------------------------------------------------------------------
// [SECTION] RENDER HELPERS
// Some of those (internal) functions are currently quite a legacy mess - their signature and behavior will change,
// we need a nicer separation between low-level functions and high-level functions relying on the ImGui context.
// Also see imgui_draw.cpp for some more which have been reworked to not rely on ImGui:: context.
//-----------------------------------------------------------------------------

const char* ImGui::FindRenderedTextEnd(const char* text, const char* text_end)
{
    const char* text_display_end = text;
    if (!text_end)
        text_end = (const char*)-1;

    while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
        text_display_end++;
    return text_display_end;
}

// Internal ImGui functions to render text
// RenderText***() functions calls ImDrawList::AddText() calls ImBitmapFont::RenderText()
void ImGui::RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Hide anything after a '##' string
    const char* text_display_end;
    if (hide_text_after_hash)
    {
        text_display_end = FindRenderedTextEnd(text, text_end);
    }
    else
    {
        if (!text_end)
            text_end = text + ImStrlen(text); // FIXME-OPT (not reached by our internal calls)
        text_display_end = text_end;
    }

    if (text != text_display_end)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_display_end);
    }
}

void ImGui::RenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (!text_end)
        text_end = text + ImStrlen(text); // FIXME-OPT (not reached by our internal calls)

    if (text != text_end)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_end, wrap_width);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_end);
    }
}

// Default clip_rect uses (pos_min,pos_max)
// Handle clipping on CPU immediately (vs typically let the GPU clip the triangles that are overlapping the clipping rectangle edges)
// FIXME-OPT: Since we have or calculate text_size we could coarse clip whole block immediately, especially for text above draw_list->DrawList.
// Effectively as this is called from widget doing their own coarse clipping it's not very valuable presently. Next time function will take
// better advantage of the render function taking size into account for coarse clipping.
void ImGui::RenderTextClippedEx(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    // Perform CPU side clipping for single clipped element to avoid using scissor state
    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_display_end, false, 0.0f);

    const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
    const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
        need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    // Align whole block. We should defer that to the better rendering function when we'll have support for individual line alignment.
    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

    // Render
    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        draw_list->AddText(NULL, 0.0f, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect);
    }
    else
    {
        draw_list->AddText(NULL, 0.0f, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, NULL);
    }
}

void ImGui::RenderTextClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    // Hide anything after a '##' string
    const char* text_display_end = FindRenderedTextEnd(text, text_end);
    const int text_len = (int)(text_display_end - text);
    if (text_len == 0)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    RenderTextClippedEx(window->DrawList, pos_min, pos_max, text, text_display_end, text_size_if_known, align, clip_rect);
    if (g.LogEnabled)
        LogRenderedText(&pos_min, text, text_display_end);
}

// Another overly complex function until we reorganize everything into a nice all-in-one helper.
// This is made more complex because we have dissociated the layout rectangle (pos_min..pos_max) from 'ellipsis_max_x' which may be beyond it.
// This is because in the context of tabs we selectively hide part of the text when the Close Button appears, but we don't want the ellipsis to move.
// (BREAKING) On 2025/04/16 we removed the 'float clip_max_x' parameters which was preceding 'float ellipsis_max' and was the same value for 99% of users.
void ImGui::RenderTextEllipsis(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end_full, const ImVec2* text_size_if_known)
{
    ImGuiContext& g = *GImGui;
    if (text_end_full == NULL)
        text_end_full = FindRenderedTextEnd(text);
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_end_full, false, 0.0f);

    //draw_list->AddLine(ImVec2(pos_max.x, pos_min.y - 4), ImVec2(pos_max.x, pos_max.y + 6), IM_COL32(0, 0, 255, 255));
    //draw_list->AddLine(ImVec2(ellipsis_max_x, pos_min.y - 2), ImVec2(ellipsis_max_x, pos_max.y + 3), IM_COL32(0, 255, 0, 255));

    // FIXME: We could technically remove (last_glyph->AdvanceX - last_glyph->X1) from text_size.x here and save a few pixels.
    if (text_size.x > pos_max.x - pos_min.x)
    {
        // Hello wo...
        // |       |   |
        // min   max   ellipsis_max
        //          <-> this is generally some padding value

        ImFont* font = draw_list->_Data->Font;
        const float font_size = draw_list->_Data->FontSize;
        const float font_scale = draw_list->_Data->FontScale;
        const char* text_end_ellipsis = NULL;
        ImFontBaked* baked = font->GetFontBaked(font_size);
        const float ellipsis_width = baked->GetCharAdvance(font->EllipsisChar) * font_scale;

        // We can now claim the space between pos_max.x and ellipsis_max.x
        const float text_avail_width = ImMax((ImMax(pos_max.x, ellipsis_max_x) - ellipsis_width) - pos_min.x, 1.0f);
        const float text_size_clipped_x = font->CalcTextSizeA(font_size, text_avail_width, 0.0f, text, text_end_full, &text_end_ellipsis).x;

        // Render text, render ellipsis
        RenderTextClippedEx(draw_list, pos_min, pos_max, text, text_end_ellipsis, &text_size, ImVec2(0.0f, 0.0f));
        ImVec4 cpu_fine_clip_rect(pos_min.x, pos_min.y, pos_max.x, pos_max.y);
        ImVec2 ellipsis_pos = ImTrunc(ImVec2(pos_min.x + text_size_clipped_x, pos_min.y));
        font->RenderChar(draw_list, font_size, ellipsis_pos, GetColorU32(ImGuiCol_Text), font->EllipsisChar, &cpu_fine_clip_rect);
    }
    else
    {
        RenderTextClippedEx(draw_list, pos_min, pos_max, text, text_end_full, &text_size, ImVec2(0.0f, 0.0f));
    }

    if (g.LogEnabled)
        LogRenderedText(&pos_min, text, text_end_full);
}

// Render a rectangle shaped with optional rounding and borders
void ImGui::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool borders, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
    const float border_size = g.Style.FrameBorderSize;
    if (borders && border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
    }
}

void ImGui::RenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
    }
}

void ImGui::RenderColorComponentMarker(const ImRect& bb, ImU32 col, float rounding)
{
    if (bb.Min.x + 1 >= bb.Max.x)
        return;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    RenderRectFilledInRangeH(window->DrawList, bb, col, bb.Min.x, ImMin(bb.Min.x + g.Style.ColorMarkerSize, bb.Max.x), rounding);
}

void ImGui::RenderNavCursor(const ImRect& bb, ImGuiID id, ImGuiNavRenderCursorFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (id != g.NavId)
        return;
    if (!g.NavCursorVisible && !(flags & ImGuiNavRenderCursorFlags_AlwaysDraw))
        return;
    if (id == g.LastItemData.ID && (g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav))
        return;

    // We don't early out on 'window->Flags & ImGuiWindowFlags_NoNavInputs' because it would be inconsistent with
    // other code directly checking NavCursorVisible. Instead we aim for NavCursorVisible to always be false.
    ImGuiWindow* window = g.CurrentWindow;
    if (window->DC.NavHideHighlightOneFrame)
        return;

    float rounding = (flags & ImGuiNavRenderCursorFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
    ImRect display_rect = bb;
    display_rect.ClipWith(window->ClipRect);
    const float thickness = 2.0f;
    if (flags & ImGuiNavRenderCursorFlags_Compact)
    {
        window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavCursor), rounding, 0, thickness);
    }
    else
    {
        const float distance = 3.0f + thickness * 0.5f;
        display_rect.Expand(ImVec2(distance, distance));
        bool fully_visible = window->ClipRect.Contains(display_rect);
        if (!fully_visible)
            window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
        window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavCursor), rounding, 0, thickness);
        if (!fully_visible)
            window->DrawList->PopClipRect();
    }
}

void ImGui::RenderMouseCursor(ImVec2 base_pos, float base_scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow)
{
    ImGuiContext& g = *GImGui;
    if (mouse_cursor <= ImGuiMouseCursor_None || mouse_cursor >= ImGuiMouseCursor_COUNT) // We intentionally accept out of bound values.
        mouse_cursor = ImGuiMouseCursor_Arrow;
    ImFontAtlas* font_atlas = g.DrawListSharedData.FontAtlas;
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        // We scale cursor with current viewport/monitor, however Windows 10 for its own hardware cursor seems to be using a different scale factor.
        ImVec2 offset, size, uv[4];
        if (!ImFontAtlasGetMouseCursorTexData(font_atlas, mouse_cursor, &offset, &size, &uv[0], &uv[2]))
            continue;
        const ImVec2 pos = base_pos - offset;
        const float scale = base_scale * viewport->DpiScale;
        if (!viewport->GetMainRect().Overlaps(ImRect(pos, pos + ImVec2(size.x + 2, size.y + 2) * scale)))
            continue;
        ImDrawList* draw_list = GetForegroundDrawList(viewport);
        ImTextureRef tex_ref = font_atlas->TexRef;
        draw_list->PushTexture(tex_ref);
        draw_list->AddImage(tex_ref, pos + ImVec2(1, 0) * scale, pos + (ImVec2(1, 0) + size) * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_ref, pos + ImVec2(2, 0) * scale, pos + (ImVec2(2, 0) + size) * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_ref, pos,                        pos + size * scale,                  uv[2], uv[3], col_border);
        draw_list->AddImage(tex_ref, pos,                        pos + size * scale,                  uv[0], uv[1], col_fill);
        if (mouse_cursor == ImGuiMouseCursor_Wait || mouse_cursor == ImGuiMouseCursor_Progress)
        {
            float a_min = ImFmod((float)g.Time * 5.0f, 2.0f * IM_PI);
            float a_max = a_min + IM_PI * 1.65f;
            draw_list->PathArcTo(pos + ImVec2(14, -1) * scale, 6.0f * scale, a_min, a_max);
            draw_list->PathStroke(col_fill, ImDrawFlags_None, 3.0f * scale);
        }
        draw_list->PopTexture();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] INITIALIZATION, SHUTDOWN
//-----------------------------------------------------------------------------

// Internal state access - if you want to share Dear ImGui state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
ImGuiContext* ImGui::GetCurrentContext()
{
    return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext* ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
    GImGui = ctx;
#endif
}

void ImGui::SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void* user_data)
{
    GImAllocatorAllocFunc = alloc_func;
    GImAllocatorFreeFunc = free_func;
    GImAllocatorUserData = user_data;
}

// This is provided to facilitate copying allocators from one static/DLL boundary to another (e.g. retrieve default allocator of your executable address space)
void ImGui::GetAllocatorFunctions(ImGuiMemAllocFunc* p_alloc_func, ImGuiMemFreeFunc* p_free_func, void** p_user_data)
{
    *p_alloc_func = GImAllocatorAllocFunc;
    *p_free_func = GImAllocatorFreeFunc;
    *p_user_data = GImAllocatorUserData;
}

ImGuiContext* ImGui::CreateContext(ImFontAtlas* shared_font_atlas)
{
    ImGuiContext* prev_ctx = GetCurrentContext();
    ImGuiContext* ctx = IM_NEW(ImGuiContext)(shared_font_atlas);
    SetCurrentContext(ctx);
    Initialize();
    if (prev_ctx != NULL)
        SetCurrentContext(prev_ctx); // Restore previous context if any, else keep new one.
    return ctx;
}

void ImGui::DestroyContext(ImGuiContext* ctx)
{
    ImGuiContext* prev_ctx = GetCurrentContext();
    if (ctx == NULL) //-V1051
        ctx = prev_ctx;
    SetCurrentContext(ctx);
    Shutdown();
    SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
    IM_DELETE(ctx);
}

// IMPORTANT: interactive elements requires a fixed ###xxx suffix, it must be same in ALL languages to allow for automation.
static const ImGuiLocEntry GLocalizationEntriesEnUS[] =
{
    { ImGuiLocKey_VersionStr,           "Dear ImGui " IMGUI_VERSION " (" IM_STRINGIFY(IMGUI_VERSION_NUM) ")" },
    { ImGuiLocKey_TableSizeOne,         "Size column to fit###SizeOne"          },
    { ImGuiLocKey_TableSizeAllFit,      "Size all columns to fit###SizeAll"     },
    { ImGuiLocKey_TableSizeAllDefault,  "Size all columns to default###SizeAll" },
    { ImGuiLocKey_TableResetOrder,      "Reset order###ResetOrder"              },
    { ImGuiLocKey_WindowingMainMenuBar, "(Main menu bar)"                       },
    { ImGuiLocKey_WindowingPopup,       "(Popup)"                               },
    { ImGuiLocKey_WindowingUntitled,    "(Untitled)"                            },
    { ImGuiLocKey_OpenLink_s,           "Open '%s'"                             },
    { ImGuiLocKey_CopyLink,             "Copy Link###CopyLink"                  },
    { ImGuiLocKey_DockingHideTabBar,            "Hide tab bar###HideTabBar"             },
    { ImGuiLocKey_DockingHoldShiftToDock,       "Hold SHIFT to enable Docking window."  },
    { ImGuiLocKey_DockingDragToUndockOrMoveNode,"Click and drag to move or undock whole node."    },
};

ImGuiContext::ImGuiContext(ImFontAtlas* shared_font_atlas)
{
    IO.Ctx = this;
    InputTextState.Ctx = this;

    Initialized = false;
    WithinFrameScope = WithinFrameScopeWithImplicitWindow = false;
    TestEngineHookItems = false;
    FrameCount = 0;
    FrameCountEnded = FrameCountPlatformEnded = FrameCountRendered = -1;
    Time = 0.0f;
    memset(ContextName, 0, sizeof(ContextName));
    ConfigFlagsCurrFrame = ConfigFlagsLastFrame = ImGuiConfigFlags_None;

    Font = NULL;
    FontBaked = NULL;
    FontSize = FontSizeBase = FontBakedScale = CurrentDpiScale = 0.0f;
    FontRasterizerDensity = 1.0f;
    IO.Fonts = shared_font_atlas ? shared_font_atlas : IM_NEW(ImFontAtlas)();
    if (shared_font_atlas == NULL)
        IO.Fonts->OwnerContext = this;
    WithinEndChildID = WithinEndPopupID = 0;
    TestEngine = NULL;

    InputEventsNextMouseSource = ImGuiMouseSource_Mouse;
    InputEventsNextEventId = 1;

    WindowsActiveCount = 0;
    WindowsBorderHoverPadding = 0.0f;
    CurrentWindow = NULL;
    HoveredWindow = NULL;
    HoveredWindowUnderMovingWindow = NULL;
    HoveredWindowBeforeClear = NULL;
    MovingWindow = NULL;
    WheelingWindow = NULL;
    WheelingWindowStartFrame = WheelingWindowScrolledFrame = -1;
    WheelingWindowReleaseTimer = 0.0f;

    DebugDrawIdConflictsId = 0;
    DebugHookIdInfoId = 0;
    HoveredId = HoveredIdPreviousFrame = 0;
    HoveredIdPreviousFrameItemCount = 0;
    HoveredIdAllowOverlap = false;
    HoveredIdIsDisabled = false;
    HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
    ItemUnclipByLog = false;
    ActiveId = 0;
    ActiveIdIsAlive = 0;
    ActiveIdTimer = 0.0f;
    ActiveIdIsJustActivated = false;
    ActiveIdAllowOverlap = false;
    ActiveIdNoClearOnFocusLoss = false;
    ActiveIdHasBeenPressedBefore = false;
    ActiveIdHasBeenEditedBefore = false;
    ActiveIdHasBeenEditedThisFrame = false;
    ActiveIdFromShortcut = false;
    ActiveIdClickOffset = ImVec2(-1, -1);
    ActiveIdSource = ImGuiInputSource_None;
    ActiveIdWindow = NULL;
    ActiveIdMouseButton = -1;
    ActiveIdDisabledId = 0;
    ActiveIdPreviousFrame = 0;
    memset(&DeactivatedItemData, 0, sizeof(DeactivatedItemData));
    memset(&ActiveIdValueOnActivation, 0, sizeof(ActiveIdValueOnActivation));
    LastActiveId = 0;
    LastActiveIdTimer = 0.0f;

    LastKeyboardKeyPressTime = LastKeyModsChangeTime = LastKeyModsChangeFromNoneTime = -1.0;

    ActiveIdUsingNavDirMask = 0x00;
    ActiveIdUsingAllKeyboardKeys = false;

    CurrentFocusScopeId = 0;
    CurrentItemFlags = ImGuiItemFlags_None;
    DebugShowGroupRects = false;
    GcCompactAll = false;

    CurrentViewport = NULL;
    MouseViewport = MouseLastHoveredViewport = NULL;
    PlatformLastFocusedViewportId = 0;
    ViewportCreatedCount = PlatformWindowsCreatedCount = 0;
    ViewportFocusedStampCount = 0;

    NavCursorVisible = false;
    NavHighlightItemUnderNav = false;
    NavMousePosDirty = false;
    NavIdIsAlive = false;
    NavId = 0;
    NavWindow = NULL;
    NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId = 0;
    NavLayer = ImGuiNavLayer_Main;
    NavIdItemFlags = ImGuiItemFlags_None;
    NavOpenContextMenuItemId = NavOpenContextMenuWindowId = 0;
    NavNextActivateId = 0;
    NavActivateFlags = NavNextActivateFlags = ImGuiActivateFlags_None;
    NavHighlightActivatedId = 0;
    NavHighlightActivatedTimer = 0.0f;
    NavInputSource = ImGuiInputSource_Keyboard;
    NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;
    NavCursorHideFrames = 0;

    NavAnyRequest = false;
    NavInitRequest = false;
    NavInitRequestFromMove = false;
    NavMoveSubmitted = false;
    NavMoveScoringItems = false;
    NavMoveForwardToNextFrame = false;
    NavMoveFlags = ImGuiNavMoveFlags_None;
    NavMoveScrollFlags = ImGuiScrollFlags_None;
    NavMoveKeyMods = ImGuiMod_None;
    NavMoveDir = NavMoveDirForDebug = NavMoveClipDir = ImGuiDir_None;
    NavScoringDebugCount = 0;
    NavTabbingDir = 0;
    NavTabbingCounter = 0;

    NavJustMovedFromFocusScopeId = NavJustMovedToId = NavJustMovedToFocusScopeId = 0;
    NavJustMovedToKeyMods = ImGuiMod_None;
    NavJustMovedToIsTabbing = false;
    NavJustMovedToHasSelectionData = false;

    // All platforms use Ctrl+Tab but Ctrl<>Super are swapped on Mac...
    // FIXME: Because this value is stored, it annoyingly interfere with toggling io.ConfigMacOSXBehaviors updating this..
    ConfigNavEnableTabbing = true;
    ConfigNavWindowingWithGamepad = true;
    ConfigNavWindowingKeyNext = IO.ConfigMacOSXBehaviors ? (ImGuiMod_Super | ImGuiKey_Tab) : (ImGuiMod_Ctrl | ImGuiKey_Tab);
    ConfigNavWindowingKeyPrev = IO.ConfigMacOSXBehaviors ? (ImGuiMod_Super | ImGuiMod_Shift | ImGuiKey_Tab) : (ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Tab);
    NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
    NavWindowingInputSource = ImGuiInputSource_None;
    NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
    NavWindowingToggleLayer = false;
    NavWindowingToggleKey = ImGuiKey_None;

    DimBgRatio = 0.0f;

    DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
    DragDropSourceFlags = ImGuiDragDropFlags_None;
    DragDropSourceFrameCount = -1;
    DragDropMouseButton = -1;
    DragDropTargetId = 0;
    DragDropTargetFullViewport = 0;
    DragDropAcceptFlagsCurr = DragDropAcceptFlagsPrev = ImGuiDragDropFlags_None;
    DragDropAcceptIdCurrRectSurface = 0.0f;
    DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
    DragDropAcceptFrameCount = -1;
    DragDropHoldJustPressedId = 0;
    memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

    ClipperTempDataStacked = 0;

    CurrentTable = NULL;
    TablesTempDataStacked = 0;
    CurrentTabBar = NULL;
    CurrentMultiSelect = NULL;
    MultiSelectTempDataStacked = 0;

    HoverItemDelayId = HoverItemDelayIdPreviousFrame = HoverItemUnlockedStationaryId = HoverWindowUnlockedStationaryId = 0;
    HoverItemDelayTimer = HoverItemDelayClearTimer = 0.0f;

    MouseCursor = ImGuiMouseCursor_Arrow;
    MouseStationaryTimer = 0.0f;

    InputTextPasswordFontBackupFlags = ImFontFlags_None;
    InputTextReactivateId = 0;
    TempInputId = 0;
    memset(&DataTypeZeroValue, 0, sizeof(DataTypeZeroValue));
    BeginMenuDepth = BeginComboDepth = 0;
    ColorEditOptions = ImGuiColorEditFlags_DefaultOptions_;
    ColorEditCurrentID = ColorEditSavedID = 0;
    ColorEditSavedHue = ColorEditSavedSat = 0.0f;
    ColorEditSavedColor = 0;
    WindowResizeRelativeMode = false;
    ScrollbarSeekMode = 0;
    ScrollbarClickDeltaToGrabCenter = 0.0f;
    SliderGrabClickOffset = 0.0f;
    SliderCurrentAccum = 0.0f;
    SliderCurrentAccumDirty = false;
    DragCurrentAccumDirty = false;
    DragCurrentAccum = 0.0f;
    DragSpeedDefaultRatio = 1.0f / 100.0f;
    DisabledAlphaBackup = 0.0f;
    DisabledStackSize = 0;
    TooltipOverrideCount = 0;
    TooltipPreviousWindow = NULL;

    PlatformImeData.InputPos = ImVec2(0.0f, 0.0f);
    PlatformImeDataPrev.InputPos = ImVec2(-1.0f, -1.0f); // Different to ensure initial submission

    DockNodeWindowMenuHandler = NULL;

    SettingsLoaded = false;
    SettingsDirtyTimer = 0.0f;
    HookIdNext = 0;
    DemoMarkerCallback = NULL;

    memset(LocalizationTable, 0, sizeof(LocalizationTable));

    LogEnabled = false;
    LogLineFirstItem = false;
    LogFlags = ImGuiLogFlags_None;
    LogWindow = NULL;
    LogNextPrefix = LogNextSuffix = NULL;
    LogFile = NULL;
    LogLinePosY = FLT_MAX;
    LogDepthRef = 0;
    LogDepthToExpand = LogDepthToExpandDefault = 2;

    ErrorCallback = NULL;
    ErrorCallbackUserData = NULL;
    ErrorFirst = true;
    ErrorCountCurrentFrame = 0;
    StackSizesInBeginForCurrentWindow = NULL;

    DebugDrawIdConflictsCount = 0;
    DebugLogFlags = ImGuiDebugLogFlags_EventError | ImGuiDebugLogFlags_OutputToTTY;
    DebugLocateId = 0;
    DebugLogSkippedErrors = 0;
    DebugLogAutoDisableFlags = ImGuiDebugLogFlags_None;
    DebugLogAutoDisableFrames = 0;
    DebugLocateFrames = 0;
    DebugBeginReturnValueCullDepth = -1;
    DebugItemPickerActive = false;
    DebugItemPickerMouseButton = ImGuiMouseButton_Left;
    DebugItemPickerBreakId = 0;
    DebugFlashStyleColorTime = 0.0f;
    DebugFlashStyleColorIdx = ImGuiCol_COUNT;
    DebugHoveredDockNode = NULL;

    // Same as DebugBreakClearData(). Those fields are scattered in their respective subsystem to stay in hot-data locations
    DebugBreakInWindow = 0;
    DebugBreakInTable = 0;
    DebugBreakInLocateId = false;
    DebugBreakKeyChord = ImGuiKey_Pause;
    DebugBreakInShortcutRouting = ImGuiKey_None;

    memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
    FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
    FramerateSecPerFrameAccum = 0.0f;
    WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
    memset(TempKeychordName, 0, sizeof(TempKeychordName));
}

ImGuiContext::~ImGuiContext()
{
    IM_ASSERT(Initialized == false && "Forgot to call DestroyContext()?");
}

void ImGui::Initialize()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(!g.Initialized && !g.SettingsLoaded);

    // Add .ini handle for ImGuiWindow and ImGuiTable types
    {
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "Window";
        ini_handler.TypeHash = ImHashStr("Window");
        ini_handler.ClearAllFn = WindowSettingsHandler_ClearAll;
        ini_handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
        ini_handler.ReadLineFn = WindowSettingsHandler_ReadLine;
        ini_handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
        ini_handler.WriteAllFn = WindowSettingsHandler_WriteAll;
        AddSettingsHandler(&ini_handler);
    }
    TableSettingsAddSettingsHandler();

    // Setup default localization table
    LocalizeRegisterEntries(GLocalizationEntriesEnUS, IM_COUNTOF(GLocalizationEntriesEnUS));

    // Setup default ImGuiPlatformIO clipboard/IME handlers.
    g.PlatformIO.Platform_GetClipboardTextFn = Platform_GetClipboardTextFn_DefaultImpl;    // Platform dependent default implementations
    g.PlatformIO.Platform_SetClipboardTextFn = Platform_SetClipboardTextFn_DefaultImpl;
    g.PlatformIO.Platform_OpenInShellFn = Platform_OpenInShellFn_DefaultImpl;
    g.PlatformIO.Platform_SetImeDataFn = Platform_SetImeDataFn_DefaultImpl;

    // Create default viewport
    ImGuiViewportP* viewport = IM_NEW(ImGuiViewportP)();
    viewport->ID = IMGUI_VIEWPORT_DEFAULT_ID;
    viewport->Idx = 0;
    viewport->PlatformWindowCreated = true;
    viewport->Flags = ImGuiViewportFlags_OwnedByApp;
    g.Viewports.push_back(viewport);
    g.TempBuffer.resize(1024 * 3 + 1, 0);
    g.ViewportCreatedCount++;
    g.PlatformIO.Viewports.push_back(g.Viewports[0]);

    // Build KeysMayBeCharInput[] lookup table (1 bit per named key)
    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
        if ((key >= ImGuiKey_0 && key <= ImGuiKey_9) || (key >= ImGuiKey_A && key <= ImGuiKey_Z) || (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9)
            || key == ImGuiKey_Tab || key == ImGuiKey_Space || key == ImGuiKey_Apostrophe || key == ImGuiKey_Comma || key == ImGuiKey_Minus || key == ImGuiKey_Period
            || key == ImGuiKey_Slash || key == ImGuiKey_Semicolon || key == ImGuiKey_Equal || key == ImGuiKey_LeftBracket || key == ImGuiKey_RightBracket || key == ImGuiKey_GraveAccent
            || key == ImGuiKey_KeypadDecimal || key == ImGuiKey_KeypadDivide || key == ImGuiKey_KeypadMultiply || key == ImGuiKey_KeypadSubtract || key == ImGuiKey_KeypadAdd || key == ImGuiKey_KeypadEqual)
            g.KeysMayBeCharInput.SetBit(key);

#ifdef IMGUI_HAS_DOCK
    // Initialize Docking
    DockContextInitialize(&g);
#endif

    // Print a debug message when running with debug feature IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS because it is very slow.
    // DO NOT COMMENT OUT THIS MESSAGE. IT IS DESIGNED TO REMIND YOU THAT IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS SHOULD ONLY BE TEMPORARILY ENABLED.
#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
    DebugLog("IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS is enabled.\nMust disable after use! Otherwise Dear ImGui will run slower.\n");
#endif

    // ImDrawList/ImFontAtlas are designed to function without ImGui, and 99% of it works without an ImGui context.
    // But this link allows us to facilitate/handle a few edge cases better.
    ImFontAtlas* atlas = g.IO.Fonts;
    g.DrawListSharedData.Context = &g;
    RegisterFontAtlas(atlas);

    g.Initialized = true;
}

// This function is merely here to free heap allocations.
void ImGui::Shutdown()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR(g.IO.BackendPlatformUserData == NULL, "Forgot to shutdown Platform backend?");
    IM_ASSERT_USER_ERROR(g.IO.BackendRendererUserData == NULL, "Forgot to shutdown Renderer backend?");
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        IM_UNUSED(viewport);
        IM_ASSERT_USER_ERROR(viewport->RendererUserData == NULL && viewport->PlatformUserData == NULL && viewport->PlatformHandle == NULL, "Backend or app forgot to call DestroyPlatformWindows()?");
    }

    // The fonts atlas can be used prior to calling NewFrame(), so we clear it even if g.Initialized is FALSE (which would happen if we never called NewFrame)
    for (ImFontAtlas* atlas : g.FontAtlases)
    {
        UnregisterFontAtlas(atlas);
        if (atlas->RefCount == 0)
        {
            atlas->Locked = false;
            IM_DELETE(atlas);
        }
    }
    g.DrawListSharedData.TempBuffer.clear();

    // Cleanup of other data are conditional on actually having initialized Dear ImGui.
    if (!g.Initialized)
        return;

    // Save settings (unless we haven't attempted to load them: CreateContext/DestroyContext without a call to NewFrame shouldn't save an empty file)
    if (g.SettingsLoaded && g.IO.IniFilename != NULL)
        SaveIniSettingsToDisk(g.IO.IniFilename);

    // Shutdown extensions
    DockContextShutdown(&g);

    CallContextHooks(&g, ImGuiContextHookType_Shutdown);

    // Clear everything else
    g.Windows.clear_delete();
    g.WindowsFocusOrder.clear();
    g.WindowsTempSortBuffer.clear();
    g.CurrentWindow = NULL;
    g.CurrentWindowStack.clear();
    g.WindowsById.Clear();
    g.NavWindow = NULL;
    g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;
    g.ActiveIdWindow = NULL;
    g.MovingWindow = NULL;

    g.KeysRoutingTable.Clear();

    g.ColorStack.clear();
    g.StyleVarStack.clear();
    g.FontStack.clear();
    g.OpenPopupStack.clear();
    g.BeginPopupStack.clear();
    g.TreeNodeStack.clear();

    g.CurrentViewport = g.MouseViewport = g.MouseLastHoveredViewport = NULL;
    g.Viewports.clear_delete();

    g.TabBars.Clear();
    g.CurrentTabBarStack.clear();
    g.ShrinkWidthBuffer.clear();

    g.ClipperTempData.clear_destruct();

    g.Tables.Clear();
    g.TablesTempData.clear_destruct();
    g.DrawChannelsTempMergeBuffer.clear();

    g.MultiSelectStorage.Clear();
    g.MultiSelectTempData.clear_destruct();

    g.ClipboardHandlerData.clear();
    g.MenusIdSubmittedThisFrame.clear();
    g.InputTextState.ClearFreeMemory();
    g.InputTextLineIndex.clear();
    g.InputTextDeactivatedState.ClearFreeMemory();

    g.SettingsWindows.clear();
    g.SettingsHandlers.clear();

    if (g.LogFile)
    {
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
        if (g.LogFile != stdout)
#endif
            ImFileClose(g.LogFile);
        g.LogFile = NULL;
    }
    g.LogBuffer.clear();
    g.DebugLogBuf.clear();
    g.DebugLogIndex.clear();

    g.Initialized = false;
}

// When using multiple context it can be helpful to give name a name.
// (A) Will be visible in debugger, (B) Will be included in all IMGUI_DEBUG_LOG() calls, (C) Should be <= 15 characters long.
void ImGui::SetContextName(ImGuiContext* ctx, const char* name)
{
    ImStrncpy(ctx->ContextName, name, IM_COUNTOF(ctx->ContextName));
}

// No specific ordering/dependency support, will see as needed
ImGuiID ImGui::AddContextHook(ImGuiContext* ctx, const ImGuiContextHook* hook)
{
    ImGuiContext& g = *ctx;
    IM_ASSERT(hook->Callback != NULL && hook->HookId == 0 && hook->Type != ImGuiContextHookType_PendingRemoval_);
    g.Hooks.push_back(*hook);
    g.Hooks.back().HookId = ++g.HookIdNext;
    return g.HookIdNext;
}

// Deferred removal, avoiding issue with changing vector while iterating it
void ImGui::RemoveContextHook(ImGuiContext* ctx, ImGuiID hook_id)
{
    ImGuiContext& g = *ctx;
    IM_ASSERT(hook_id != 0);
    for (ImGuiContextHook& hook : g.Hooks)
        if (hook.HookId == hook_id)
            hook.Type = ImGuiContextHookType_PendingRemoval_;
}

// Call context hooks (used by e.g. test engine)
// We assume a small number of hooks so all stored in same array
void ImGui::CallContextHooks(ImGuiContext* ctx, ImGuiContextHookType hook_type)
{
    ImGuiContext& g = *ctx;
    for (ImGuiContextHook& hook : g.Hooks)
        if (hook.Type == hook_type)
            hook.Callback(&g, &hook);
}

//-----------------------------------------------------------------------------
// [SECTION] MAIN CODE (most of the code! lots of stuff, needs tidying up!)
//-----------------------------------------------------------------------------

void ImGui::GcCompactTransientMiscBuffers()
{
    ImGuiContext& g = *GImGui;
    g.ItemFlagsStack.clear();
    g.GroupStack.clear();
    g.InputTextLineIndex.clear();
    g.MultiSelectTempDataStacked = 0;
    g.MultiSelectTempData.clear_destruct();
    TableGcCompactSettings();
    for (ImFontAtlas* atlas : g.FontAtlases)
        atlas->CompactCache();
}

// Free up/compact internal window buffers, we can use this when a window becomes unused.
// Not freed:
// - ImGuiWindow, ImGuiWindowSettings, Name, StateStorage, ColumnsStorage (may hold useful data)
// This should have no noticeable visual effect. When the window reappear however, expect new allocation/buffer growth/copy cost.
// FIXME: Consider exposing of elaborating GC policy, e.g. being able to trim excessive ImDrawList gaps. (#9303)
void ImGui::GcCompactTransientWindowBuffers(ImGuiWindow* window)
{
    window->MemoryCompacted = true;
    window->MemoryDrawListIdxCapacity = ImMin((int)(window->DrawList->IdxBuffer.Size * 1.05f), window->DrawList->IdxBuffer.Capacity);
    window->MemoryDrawListVtxCapacity = ImMin((int)(window->DrawList->VtxBuffer.Size * 1.05f), window->DrawList->VtxBuffer.Capacity);
    window->IDStack.clear();
    window->DrawList->_ClearFreeMemory();
    window->DC.ChildWindows.clear();
    window->DC.ItemWidthStack.clear();
    window->DC.TextWrapPosStack.clear();
}

void ImGui::GcAwakeTransientWindowBuffers(ImGuiWindow* window)
{
    // We stored capacity of the ImDrawList buffer to reduce growth-caused allocation/copy when awakening.
    // The other buffers tends to amortize much faster.
    window->MemoryCompacted = false;
    window->DrawList->IdxBuffer.reserve(window->MemoryDrawListIdxCapacity);
    window->DrawList->VtxBuffer.reserve(window->MemoryDrawListVtxCapacity);
    window->MemoryDrawListIdxCapacity = window->MemoryDrawListVtxCapacity = 0;
}

void ImGui::SetActiveID(ImGuiID id, ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;

    // Clear previous active id
    if (g.ActiveId != 0)
    {
        // Store deactivate data
        ImGuiDeactivatedItemData* deactivated_data = &g.DeactivatedItemData;
        deactivated_data->ID = g.ActiveId;
        deactivated_data->ElapseFrame = (g.LastItemData.ID == g.ActiveId) ? g.FrameCount : g.FrameCount + 1; // FIXME: OK to use LastItemData?
        deactivated_data->HasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
        deactivated_data->IsAlive = (g.ActiveIdIsAlive == g.ActiveId);

        // This could be written in a more general way (e.g associate a hook to ActiveId),
        // but since this is currently quite an exception we'll leave it as is.
        // One common scenario leading to this is: pressing Key ->NavMoveRequestApplyResult() -> ClearActiveID()
        if (g.InputTextState.ID == g.ActiveId)
            InputTextDeactivateHook(g.ActiveId);

        // While most behaved code would make an effort to not steal active id during window move/drag operations,
        // we at least need to be resilient to it. Canceling the move is rather aggressive and users of 'master' branch
        // may prefer the weird ill-defined half working situation ('docking' did assert), so may need to rework that.
        if (g.MovingWindow != NULL && g.ActiveId == g.MovingWindow->MoveId)
        {
            IMGUI_DEBUG_LOG_ACTIVEID("SetActiveID() cancel MovingWindow\n");
            StopMouseMovingWindow();
        }
    }

    // Set active id
    g.ActiveIdIsJustActivated = (g.ActiveId != id);
    if (g.ActiveIdIsJustActivated)
    {
        IMGUI_DEBUG_LOG_ACTIVEID("SetActiveID() old:0x%08X (window \"%s\") -> new:0x%08X (window \"%s\")\n", g.ActiveId, g.ActiveIdWindow ? g.ActiveIdWindow->Name : "", id, window ? window->Name : "");
        g.ActiveIdTimer = 0.0f;
        g.ActiveIdHasBeenPressedBefore = false;
        g.ActiveIdHasBeenEditedBefore = false;
        g.ActiveIdMouseButton = -1;
        if (id != 0)
        {
            g.LastActiveId = id;
            g.LastActiveIdTimer = 0.0f;
        }
    }
    g.ActiveId = id;
    g.ActiveIdAllowOverlap = false;
    g.ActiveIdNoClearOnFocusLoss = false;
    g.ActiveIdWindow = window;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdFromShortcut = false;
    g.ActiveIdDisabledId = 0;
    if (id)
    {
        g.ActiveIdIsAlive = id;
        g.ActiveIdSource = (g.NavActivateId == id || g.NavJustMovedToId == id) ? g.NavInputSource : ImGuiInputSource_Mouse;
        IM_ASSERT(g.ActiveIdSource != ImGuiInputSource_None);
    }

    // Clear declaration of inputs claimed by the widget
    // (Please note that this is WIP and not all keys/inputs are thoroughly declared by all widgets yet)
    g.ActiveIdUsingNavDirMask = 0x00;
    g.ActiveIdUsingAllKeyboardKeys = false;
}

void ImGui::ClearActiveID()
{
    SetActiveID(0, NULL); // g.ActiveId = 0;
}

void ImGui::SetHoveredID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.HoveredId = id;
    g.HoveredIdAllowOverlap = false;
    if (id != 0 && g.HoveredIdPreviousFrame != id)
        g.HoveredIdTimer = g.HoveredIdNotActiveTimer = 0.0f;
}

ImGuiID ImGui::GetHoveredID()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredId ? g.HoveredId : g.HoveredIdPreviousFrame;
}

void ImGui::MarkItemEdited(ImGuiID id)
{
    // This marking is to be able to provide info for IsItemDeactivatedAfterEdit().
    // ActiveId might have been released by the time we call this (as in the typical press/release button behavior) but still need to fill the data.
    ImGuiContext& g = *GImGui;

    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_EditedInternal;
    if (g.LastItemData.ItemFlags & ImGuiItemFlags_NoMarkEdited)
        return;
    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;

    if (g.ActiveId == id || g.ActiveId == 0)
    {
        // FIXME: Can't we fully rely on LastItemData yet?
        g.ActiveIdHasBeenEditedThisFrame = true;
        g.ActiveIdHasBeenEditedBefore = true;
        if (g.DeactivatedItemData.ID == id)
            g.DeactivatedItemData.HasBeenEditedBefore = true;
    }

    // We accept a MarkItemEdited() on drag and drop targets (see https://github.com/ocornut/imgui/issues/1875#issuecomment-978243343)
    // We accept 'ActiveIdPreviousFrame == id' for InputText() returning an edit after it has been taken ActiveId away (#4714)
    // FIXME: This assert is getting a bit meaningless over time. It helped detect some unusual use cases but eventually it is becoming an unnecessary restriction.
    IM_ASSERT(g.DragDropActive || g.ActiveId == id || g.ActiveId == 0 || g.ActiveIdPreviousFrame == id || g.NavJustMovedToId || (g.CurrentMultiSelect != NULL && g.BoxSelectState.IsActive));
}

bool ImGui::IsWindowContentHoverable(ImGuiWindow* window, ImGuiHoveredFlags flags)
{
    // An active popup disable hovering on other windows (apart from its own children)
    // FIXME-OPT: This could be cached/stored within the window.
    ImGuiContext& g = *GImGui;
    if (g.NavWindow)
        if (ImGuiWindow* focused_root_window = g.NavWindow->RootWindowDockTree)
            if (focused_root_window->WasActive && focused_root_window != window->RootWindowDockTree)
            {
                // For the purpose of those flags we differentiate "standard popup" from "modal popup"
                // NB: The 'else' is important because Modal windows are also Popups.
                bool want_inhibit = false;
                if (focused_root_window->Flags & ImGuiWindowFlags_Modal)
                    want_inhibit = true;
                else if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiHoveredFlags_AllowWhenBlockedByPopup))
                    want_inhibit = true;

                // Inhibit hover unless the window is within the stack of our modal/popup
                if (want_inhibit)
                    if (!IsWindowWithinBeginStackOf(window->RootWindow, focused_root_window))
                        return false;
            }

    // Filter by viewport
    if (window->Viewport != g.MouseViewport)
        if (g.MovingWindow == NULL || window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree)
            return false;

    return true;
}

static inline float CalcDelayFromHoveredFlags(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (flags & ImGuiHoveredFlags_DelayNormal)
        return g.Style.HoverDelayNormal;
    if (flags & ImGuiHoveredFlags_DelayShort)
        return g.Style.HoverDelayShort;
    return 0.0f;
}

// This is roughly matching the behavior of internal-facing ItemHoverable()
// - we allow hovering to be true when ActiveId==window->MoveID, so that clicking on non-interactive items such as a Text() item still returns true with IsItemHovered()
// - this should work even for non-interactive items that have no ID, so we cannot use LastItemId
bool ImGui::IsItemHovered(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR((flags & ~ImGuiHoveredFlags_AllowedMaskForIsItemHovered) == 0, "Invalid flags for IsItemHovered()!");

    if (g.NavHighlightItemUnderNav && g.NavCursorVisible && !(flags & ImGuiHoveredFlags_NoNavOverride))
    {
        if (!IsItemFocused())
            return false;
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled) && !(flags & ImGuiHoveredFlags_AllowWhenDisabled))
            return false;

        if (flags & ImGuiHoveredFlags_ForTooltip)
            flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipNav);
    }
    else
    {
        // Test for bounding box overlap, as updated as ItemAdd()
        ImGuiItemStatusFlags status_flags = g.LastItemData.StatusFlags;
        if (!(status_flags & ImGuiItemStatusFlags_HoveredRect))
            return false;

        if (flags & ImGuiHoveredFlags_ForTooltip)
            flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);

        // Done with rectangle culling so we can perform heavier checks now
        // Test if we are hovering the right window (our window could be behind another window)
        // [2021/03/02] Reworked / reverted the revert, finally. Note we want e.g. BeginGroup/ItemAdd/EndGroup to work as well. (#3851)
        // [2017/10/16] Reverted commit 344d48be3 and testing RootWindow instead. I believe it is correct to NOT test for RootWindow but this leaves us unable
        // to use IsItemHovered() after EndChild() itself. Until a solution is found I believe reverting to the test from 2017/09/27 is safe since this was
        // the test that has been running for a long while.
        if (g.HoveredWindow != window && (status_flags & ImGuiItemStatusFlags_HoveredWindow) == 0)
            if ((flags & ImGuiHoveredFlags_AllowWhenOverlappedByWindow) == 0)
                return false;

        // Test if another item is active (e.g. being dragged)
        const ImGuiID id = g.LastItemData.ID;
        if ((flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) == 0)
            if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap && !g.ActiveIdFromShortcut)
            {
                // When ActiveId == MoveId it means that either:
                // - (1) user clicked on void _or_ an item with no id, which triggers moving window (ActiveId is set even when window has _NoMove flag)
                //   - the (id == 0) test handles it, however, IsItemHovered() will leak between id==0 items (mostly visible when using _NoMove). // FIXME: May be fixed.
                // - (2) user clicked a disabled item. UpdateMouseMovingWindowEndFrame() uses ActiveId == MoveId to avoid interference with item logic + sets ActiveIdDisabledId.
                bool cancel_is_hovered = true;
                if (g.ActiveId == window->MoveId && (id == 0 || g.ActiveIdDisabledId == id))
                    cancel_is_hovered = false;
                // When ActiveId == TabId it means user clicked docking tab for the window.
                if (g.ActiveId == window->TabId)
                    cancel_is_hovered = false;
                if (cancel_is_hovered)
                    return false;
            }

        // Test if interactions on this window are blocked by an active popup or modal.
        // The ImGuiHoveredFlags_AllowWhenBlockedByPopup flag will be tested here.
        if (!IsWindowContentHoverable(window, flags) && !(g.LastItemData.ItemFlags & ImGuiItemFlags_NoWindowHoverableCheck))
            return false;

        // Test if the item is disabled
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled) && !(flags & ImGuiHoveredFlags_AllowWhenDisabled))
            return false;

        // Special handling for calling after Begin() which represent the title bar or tab.
        // When the window is skipped/collapsed (SkipItems==true) that last item (always ->MoveId submitted by Begin)
        // will never be overwritten so we need to detect the case.
        if (id == window->MoveId && window->WriteAccessed)
            return false;

        // Test if using AllowOverlap and overlapped
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap) && id != 0)
            if ((flags & ImGuiHoveredFlags_AllowWhenOverlappedByItem) == 0)
                if (g.HoveredIdPreviousFrame != g.LastItemData.ID)
                    return false;
    }

    // Handle hover delay
    // (some ideas: https://www.nngroup.com/articles/timing-exposing-content)
    const float delay = CalcDelayFromHoveredFlags(flags);
    if (delay > 0.0f || (flags & ImGuiHoveredFlags_Stationary))
    {
        ImGuiID hover_delay_id = (g.LastItemData.ID != 0) ? g.LastItemData.ID : window->GetIDFromPos(g.LastItemData.Rect.Min);
        if ((flags & ImGuiHoveredFlags_NoSharedDelay) && (g.HoverItemDelayIdPreviousFrame != hover_delay_id))
            g.HoverItemDelayTimer = 0.0f;
        g.HoverItemDelayId = hover_delay_id;

        // When changing hovered item we requires a bit of stationary delay before activating hover timer,
        // but once unlocked on a given item we also moving.
        //if (g.HoverDelayTimer >= delay && (g.HoverDelayTimer - g.IO.DeltaTime < delay || g.MouseStationaryTimer - g.IO.DeltaTime < g.Style.HoverStationaryDelay)) { IMGUI_DEBUG_LOG("HoverDelayTimer = %f/%f, MouseStationaryTimer = %f\n", g.HoverDelayTimer, delay, g.MouseStationaryTimer); }
        if ((flags & ImGuiHoveredFlags_Stationary) != 0 && g.HoverItemUnlockedStationaryId != hover_delay_id)
            return false;

        if (g.HoverItemDelayTimer < delay)
            return false;
    }

    return true;
}

// Internal facing ItemHoverable() used when submitting widgets. THIS IS A SUBMISSION NOT A HOVER CHECK.
// Returns whether the item was hovered, logic differs slightly from IsItemHovered().
// (this does not rely on LastItemData it can be called from a ButtonBehavior() call not following an ItemAdd() call)
// FIXME-LEGACY: the 'ImGuiItemFlags item_flags' parameter was added on 2023-06-28.
// If you used this in your legacy/custom widgets code:
// - Commonly: if your ItemHoverable() call comes after an ItemAdd() call: pass 'item_flags = g.LastItemData.ItemFlags'.
// - Rare: otherwise you may pass 'item_flags = 0' (ImGuiItemFlags_None) unless you want to benefit from special behavior handled by ItemHoverable.
bool ImGui::ItemHoverable(const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Detect ID conflicts
    // (this is specifically done here by comparing on hover because it allows us a detection of duplicates that is algorithmically extra cheap, 1 u32 compare per item. No O(log N) lookup whatsoever)
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0 && g.HoveredIdPreviousFrame == id && (item_flags & ImGuiItemFlags_AllowDuplicateId) == 0)
    {
        g.HoveredIdPreviousFrameItemCount++;
        if (g.DebugDrawIdConflictsId == id)
            window->DrawList->AddRect(bb.Min - ImVec2(1,1), bb.Max + ImVec2(1,1), IM_COL32(255, 0, 0, 255), 0.0f, ImDrawFlags_None, 2.0f);
    }
#endif

    if (g.HoveredWindow != window)
        return false;
    if (!IsMouseHoveringRect(bb.Min, bb.Max))
        return false;

    if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
        return false;
    if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
        if (!g.ActiveIdFromShortcut)
            return false;

    // We are done with rectangle culling so we can perform heavier checks now.
    if (!(item_flags & ImGuiItemFlags_NoWindowHoverableCheck) && !IsWindowContentHoverable(window, ImGuiHoveredFlags_None))
    {
        g.HoveredIdIsDisabled = true;
        return false;
    }

    // We exceptionally allow this function to be called with id==0 to allow using it for easy high-level
    // hover test in widgets code. We could also decide to split this function is two.
    if (id != 0)
    {
        // Drag source doesn't report as hovered
        if (g.DragDropActive && g.DragDropPayload.SourceId == id && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoDisableHover))
            return false;

        SetHoveredID(id);

        // AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match.
        // This allows using patterns where a later submitted widget overlaps a previous one. Generally perceived as a front-to-back hit-test.
        if (item_flags & ImGuiItemFlags_AllowOverlap)
        {
            g.HoveredIdAllowOverlap = true;
            if (g.HoveredIdPreviousFrame != id)
                return false;
        }

        // Display shortcut (only works with mouse)
        // (ImGuiItemStatusFlags_HasShortcut in LastItemData denotes we want a tooltip)
        if (id == g.LastItemData.ID && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasShortcut) && g.ActiveId != id)
            if (IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal))
                SetTooltip("%s", GetKeyChordName(g.LastItemData.Shortcut));
    }

    // When disabled we'll return false but still set HoveredId
    if (item_flags & ImGuiItemFlags_Disabled)
    {
        // Release active id if turning disabled
        if (g.ActiveId == id && id != 0)
            ClearActiveID();
        g.HoveredIdIsDisabled = true;
        return false;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0)
    {
        // [DEBUG] Item Picker tool!
        // We perform the check here because reaching is path is rare (1~ time a frame),
        // making the cost of this tool near-zero! We could get better call-stack and support picking non-hovered
        // items if we performed the test in ItemAdd(), but that would incur a bigger runtime cost.
        if (g.DebugItemPickerActive && g.HoveredIdPreviousFrame == id)
            GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 0, 255));
        if (g.DebugItemPickerBreakId == id)
            IM_DEBUG_BREAK();
    }
#endif

    if (g.NavHighlightItemUnderNav && (item_flags & ImGuiItemFlags_NoNavDisableMouseHover) == 0)
        return false;

    return true;
}

// FIXME: This is inlined/duplicated in ItemAdd()
// FIXME: The id != 0 path is not used by our codebase, may get rid of it?
bool ImGui::IsClippedEx(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!bb.Overlaps(window->ClipRect))
        if (id == 0 || (id != g.ActiveId && id != g.ActiveIdPreviousFrame && id != g.NavId && id != g.NavActivateId))
            if (!g.ItemUnclipByLog)
                return true;
    return false;
}

// This is also inlined in ItemAdd()
// Note: if ImGuiItemStatusFlags_HasDisplayRect is set, user needs to set g.LastItemData.DisplayRect.
void ImGui::SetLastItemData(ImGuiID item_id, ImGuiItemFlags item_flags, ImGuiItemStatusFlags status_flags, const ImRect& item_rect)
{
    ImGuiContext& g = *GImGui;
    g.LastItemData.ID = item_id;
    g.LastItemData.ItemFlags = item_flags;
    g.LastItemData.StatusFlags = status_flags;
    g.LastItemData.Rect = g.LastItemData.NavRect = item_rect;
}

float ImGui::CalcWrapWidthForPos(const ImVec2& pos, float wrap_pos_x)
{
    if (wrap_pos_x < 0.0f)
        return 0.0f;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (wrap_pos_x == 0.0f)
    {
        // We could decide to setup a default wrapping max point for auto-resizing windows,
        // or have auto-wrap (with unspecified wrapping pos) behave as a ContentSize extending function?
        //if (window->Hidden && (window->Flags & ImGuiWindowFlags_AlwaysAutoResize))
        //    wrap_pos_x = ImMax(window->WorkRect.Min.x + g.FontSize * 10.0f, window->WorkRect.Max.x);
        //else
        wrap_pos_x = window->WorkRect.Max.x;
    }
    else if (wrap_pos_x > 0.0f)
    {
        wrap_pos_x += window->Pos.x - window->Scroll.x; // wrap_pos_x is provided is window local space
    }

    return ImMax(wrap_pos_x - pos.x, 1.0f);
}

// IM_ALLOC() == ImGui::MemAlloc()
void* ImGui::MemAlloc(size_t size)
{
    void* ptr = (*GImAllocatorAllocFunc)(size, GImAllocatorUserData);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (ImGuiContext* ctx = GImGui)
        DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, size);
#endif
    return ptr;
}

// IM_FREE() == ImGui::MemFree()
void ImGui::MemFree(void* ptr)
{
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (ptr != NULL)
        if (ImGuiContext* ctx = GImGui)
            DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, (size_t)-1);
#endif
    return (*GImAllocatorFreeFunc)(ptr, GImAllocatorUserData);
}

void ImGui::DemoMarker(const char* file, int line, const char* section)
{
    ImGuiContext& g = *GImGui;
    if (g.DemoMarkerCallback != NULL)
        g.DemoMarkerCallback(file, line, section);
}

// We record the number of allocation in recent frames, as a way to audit/sanitize our guiding principles of "no allocations on idle/repeating frames"
void ImGui::DebugAllocHook(ImGuiDebugAllocInfo* info, int frame_count, void* ptr, size_t size)
{
    ImGuiDebugAllocEntry* entry = &info->LastEntriesBuf[info->LastEntriesIdx];
    IM_UNUSED(ptr);
    if (entry->FrameCount != frame_count)
    {
        info->LastEntriesIdx = (info->LastEntriesIdx + 1) % IM_COUNTOF(info->LastEntriesBuf);
        entry = &info->LastEntriesBuf[info->LastEntriesIdx];
        entry->FrameCount = frame_count;
        entry->AllocCount = entry->FreeCount = 0;
    }
    if (size != (size_t)-1)
    {
        //printf("[%05d] MemAlloc(%d) -> 0x%p\n", frame_count, (int)size, ptr);
        entry->AllocCount++;
        info->TotalAllocCount++;
    }
    else
    {
        //printf("[%05d] MemFree(0x%p)\n", frame_count, ptr);
        entry->FreeCount++;
        info->TotalFreeCount++;
    }
}

// A conformant backend should return NULL on failure (e.g. clipboard data is not text).
const char* ImGui::GetClipboardText()
{
    ImGuiContext& g = *GImGui;
    return g.PlatformIO.Platform_GetClipboardTextFn ? g.PlatformIO.Platform_GetClipboardTextFn(&g) : NULL;
}

void ImGui::SetClipboardText(const char* text)
{
    ImGuiContext& g = *GImGui;
    if (g.PlatformIO.Platform_SetClipboardTextFn != NULL)
        g.PlatformIO.Platform_SetClipboardTextFn(&g, text);
}

const char* ImGui::GetVersion()
{
    return IMGUI_VERSION;
}

ImGuiIO& ImGui::GetIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->IO;
}

// This variant exists to facilitate backends experimenting with multi-threaded parallel context. (#8069, #6293, #5856)
ImGuiIO& ImGui::GetIO(ImGuiContext* ctx)
{
    IM_ASSERT(ctx != NULL);
    return ctx->IO;
}

ImGuiPlatformIO& ImGui::GetPlatformIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext()?");
    return GImGui->PlatformIO;
}

// This variant exists to facilitate backends experimenting with multi-threaded parallel context. (#8069, #6293, #5856)
ImGuiPlatformIO& ImGui::GetPlatformIO(ImGuiContext* ctx)
{
    IM_ASSERT(ctx != NULL);
    return ctx->PlatformIO;
}

// Pass this to your backend rendering function! Valid after Render() and until the next call to NewFrame()
ImDrawData* ImGui::GetDrawData()
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = g.Viewports[0];
    return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

double ImGui::GetTime()
{
    return GImGui->Time;
}

int ImGui::GetFrameCount()
{
    return GImGui->FrameCount;
}

static ImDrawList* GetViewportBgFgDrawList(ImGuiViewportP* viewport, size_t drawlist_no, const char* drawlist_name)
{
    // Create the draw list on demand, because they are not frequently used for all viewports
    ImGuiContext& g = *GImGui;
    IM_ASSERT(drawlist_no < IM_COUNTOF(viewport->BgFgDrawLists));
    ImDrawList* draw_list = viewport->BgFgDrawLists[drawlist_no];
    if (draw_list == NULL)
    {
        draw_list = IM_NEW(ImDrawList)(&g.DrawListSharedData);
        draw_list->_OwnerName = drawlist_name;
        viewport->BgFgDrawLists[drawlist_no] = draw_list;
    }

    // Our ImDrawList system requires that there is always a command
    if (viewport->BgFgDrawListsLastTimeActive[drawlist_no] != (float)g.Time)
    {
        draw_list->_ResetForNewFrame();
        draw_list->PushTexture(g.IO.Fonts->TexRef);
        draw_list->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size, false);
        viewport->BgFgDrawListsLastTimeActive[drawlist_no] = (float)g.Time;
    }
    return draw_list;
}

ImDrawList* ImGui::GetBackgroundDrawList(ImGuiViewport* viewport)
{
    if (viewport == NULL)
        viewport = GImGui->CurrentWindow->Viewport;
    return GetViewportBgFgDrawList((ImGuiViewportP*)viewport, 0, "##Background");
}

ImDrawList* ImGui::GetForegroundDrawList(ImGuiViewport* viewport)
{
    if (viewport == NULL)
        viewport = GImGui->CurrentWindow->Viewport;
    return GetViewportBgFgDrawList((ImGuiViewportP*)viewport, 1, "##Foreground");
}

ImDrawListSharedData* ImGui::GetDrawListSharedData()
{
    return &GImGui->DrawListSharedData;
}

// The reason this is exposed in imgui_internal.h is: on touch-based system that don't have hovering, we want to dispatch inputs to the right target (imgui vs imgui+app)
void ImGui::UpdateHoveredWindowAndCaptureFlags(const ImVec2& mouse_pos)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    // FIXME-DPI: This storage was added on 2021/03/31 for test engine, but if we want to multiply WINDOWS_HOVER_PADDING
    // by DpiScale, we need to make this window-agnostic anyhow, maybe need storing inside ImGuiWindow.
    g.WindowsBorderHoverPadding = ImMax(ImMax(g.Style.TouchExtraPadding.x, g.Style.TouchExtraPadding.y), g.Style.WindowBorderHoverPadding);

    // Find the window hovered by mouse:
    // - Child windows can extend beyond the limit of their parent so we need to derive HoveredRootWindow from HoveredWindow.
    // - When moving a window we can skip the search, which also conveniently bypasses the fact that window->WindowRectClipped is lagging as this point of the frame.
    // - We also support the moved window toggling the NoInputs flag after moving has started in order to be able to detect windows below it, which is useful for e.g. docking mechanisms.
    bool clear_hovered_windows = false;
    FindHoveredWindowEx(mouse_pos, false, &g.HoveredWindow, &g.HoveredWindowUnderMovingWindow);
    IM_ASSERT(g.HoveredWindow == NULL || g.HoveredWindow == g.MovingWindow || g.HoveredWindow->Viewport == g.MouseViewport);
    g.HoveredWindowBeforeClear = g.HoveredWindow;

    // Modal windows prevents mouse from hovering behind them.
    ImGuiWindow* modal_window = GetTopMostPopupModal();
    if (modal_window && g.HoveredWindow && !IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, modal_window)) // FIXME-MERGE: RootWindowDockTree ?
        clear_hovered_windows = true;

    // Disabled mouse hovering (we don't currently clear MousePos, we could)
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
        clear_hovered_windows = true;

    // We track click ownership. When clicked outside of a window the click is owned by the application and
    // won't report hovering nor request capture even while dragging over our windows afterward.
    const bool has_open_popup = (g.OpenPopupStack.Size > 0);
    const bool has_open_modal = (modal_window != NULL);
    int mouse_earliest_down = -1;
    bool mouse_any_down = false;
    for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++)
    {
        if (io.MouseClicked[i])
        {
            io.MouseDownOwned[i] = (g.HoveredWindow != NULL) || has_open_popup;
            io.MouseDownOwnedUnlessPopupClose[i] = (g.HoveredWindow != NULL) || has_open_modal;
        }
        mouse_any_down |= io.MouseDown[i];
        if (io.MouseDown[i] || io.MouseReleased[i]) // Increase release frame for our evaluation of earliest button (#1392)
            if (mouse_earliest_down == -1 || io.MouseClickedTime[i] < io.MouseClickedTime[mouse_earliest_down])
                mouse_earliest_down = i;
    }
    const bool mouse_avail = (mouse_earliest_down == -1) || io.MouseDownOwned[mouse_earliest_down];
    const bool mouse_avail_unless_popup_close = (mouse_earliest_down == -1) || io.MouseDownOwnedUnlessPopupClose[mouse_earliest_down];

    // If mouse was first clicked outside of ImGui bounds we also cancel out hovering.
    // FIXME: For patterns of drag and drop across OS windows, we may need to rework/remove this test (first committed 311c0ca9 on 2015/02)
    const bool mouse_dragging_extern_payload = g.DragDropActive && (g.DragDropSourceFlags & ImGuiDragDropFlags_SourceExtern) != 0;
    if (!mouse_avail && !mouse_dragging_extern_payload)
        clear_hovered_windows = true;

    if (clear_hovered_windows)
        g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;

    // Update io.WantCaptureMouse for the user application (true = dispatch mouse info to Dear ImGui only, false = dispatch mouse to Dear ImGui + underlying app)
    // Update io.WantCaptureMouseAllowPopupClose (experimental) to give a chance for app to react to popup closure with a drag
    if (g.WantCaptureMouseNextFrame != -1)
    {
        io.WantCaptureMouse = io.WantCaptureMouseUnlessPopupClose = (g.WantCaptureMouseNextFrame != 0);
    }
    else
    {
        io.WantCaptureMouse = (mouse_avail && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_popup;
        io.WantCaptureMouseUnlessPopupClose = (mouse_avail_unless_popup_close && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_modal;
    }

    // Update io.WantCaptureKeyboard for the user application (true = dispatch keyboard info to Dear ImGui only, false = dispatch keyboard info to Dear ImGui + underlying app)
    io.WantCaptureKeyboard = false;
    if ((io.ConfigFlags & ImGuiConfigFlags_NoKeyboard) == 0)
    {
        if ((g.ActiveId != 0) || (modal_window != NULL))
            io.WantCaptureKeyboard = true;
        else if (io.NavActive && (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) && io.ConfigNavCaptureKeyboard)
            io.WantCaptureKeyboard = true;
    }
    if (g.WantCaptureKeyboardNextFrame != -1) // Manual override
        io.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != 0);

    // Update io.WantTextInput flag, this is to allow systems without a keyboard (e.g. mobile, hand-held) to show a software keyboard if possible
    io.WantTextInput = (g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : false;
}

// Called once a frame. Followed by SetCurrentFont() which sets up the remaining data.
// FIXME-VIEWPORT: the concept of a single ClipRectFullscreen is not ideal!
static void SetupDrawListSharedData()
{
    ImGuiContext& g = *GImGui;
    ImRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (ImGuiViewportP* viewport : g.Viewports)
        virtual_space.Add(viewport->GetMainRect());
    g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
    g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
    g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
    g.DrawListSharedData.InitialFlags = ImDrawListFlags_None;
    if (g.Style.AntiAliasedLines)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLines;
    if (g.Style.AntiAliasedLinesUseTex && !(g.IO.Fonts->Flags & ImFontAtlasFlags_NoBakedLines))
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
    if (g.Style.AntiAliasedFill)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedFill;
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AllowVtxOffset;
    g.DrawListSharedData.InitialFringeScale = 1.0f; // FIXME-DPI: Change this for some DPI scaling experiments.
}

void ImGui::NewFrame()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    ImGuiContext& g = *GImGui;

    // Remove pending delete hooks before frame start.
    // This deferred removal avoid issues of removal while iterating the hook vector
    for (int n = g.Hooks.Size - 1; n >= 0; n--)
        if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
            g.Hooks.erase(&g.Hooks[n]);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

    // Check and assert for various common IO and Configuration mistakes
    g.ConfigFlagsLastFrame = g.ConfigFlagsCurrFrame;
    ErrorCheckNewFrameSanityChecks();
    g.ConfigFlagsCurrFrame = g.IO.ConfigFlags;

    // Load settings on first frame, save settings when modified (after a delay)
    UpdateSettings();

    g.Time += g.IO.DeltaTime;
    g.FrameCount += 1;
    g.TooltipOverrideCount = 0;
    g.WindowsActiveCount = 0;
    g.MenusIdSubmittedThisFrame.resize(0);

    // Calculate frame-rate for the user, as a purely luxurious feature
    g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
    g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
    g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_COUNTOF(g.FramerateSecPerFrame);
    g.FramerateSecPerFrameCount = ImMin(g.FramerateSecPerFrameCount + 1, IM_COUNTOF(g.FramerateSecPerFrame));
    g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f) ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount)) : FLT_MAX;

    // Process input queue (trickle as many events as possible), turn events into writes to IO structure
    g.InputEventsTrail.resize(0);
    UpdateInputEvents(g.IO.ConfigInputTrickleEventQueue);

    // Update viewports (after processing input queue, so io.MouseHoveredViewport is set)
    UpdateViewportsNewFrame();

    // Update texture list (collect destroyed textures, etc.)
    UpdateTexturesNewFrame();

    // Setup current font and draw list shared data
    SetupDrawListSharedData();
    UpdateFontsNewFrame();

    g.WithinFrameScope = true;

    // Mark rendering data as invalid to prevent user who may have a handle on it to use it.
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        viewport->DrawData = NULL;
        viewport->DrawDataP.Valid = false;
    }

    // Drag and drop keep the source ID alive so even if the source disappear our state is consistent
    if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
        KeepAliveID(g.DragDropPayload.SourceId);

    // [DEBUG]
    if (!g.IO.ConfigDebugHighlightIdConflicts || !g.IO.KeyCtrl) // Count is locked while holding Ctrl
        g.DebugDrawIdConflictsId = 0;
    if (g.IO.ConfigDebugHighlightIdConflicts && g.HoveredIdPreviousFrameItemCount > 1)
        g.DebugDrawIdConflictsId = g.HoveredIdPreviousFrame;

    // Update HoveredId data
    if (!g.HoveredIdPreviousFrame)
        g.HoveredIdTimer = 0.0f;
    if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
        g.HoveredIdNotActiveTimer = 0.0f;
    if (g.HoveredId)
        g.HoveredIdTimer += g.IO.DeltaTime;
    if (g.HoveredId && g.ActiveId != g.HoveredId)
        g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
    g.HoveredIdPreviousFrame = g.HoveredId;
    g.HoveredIdPreviousFrameItemCount = 0;
    g.HoveredId = 0;
    g.HoveredIdAllowOverlap = false;
    g.HoveredIdIsDisabled = false;

    // Clear ActiveID if the item is not alive anymore.
    // In 1.87, the common most call to KeepAliveID() was moved from GetID() to ItemAdd().
    // As a result, custom widget using ButtonBehavior() _without_ ItemAdd() need to call KeepAliveID() themselves.
    if (g.ActiveId != 0 && g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId)
    {
        IMGUI_DEBUG_LOG_ACTIVEID("NewFrame(): ClearActiveID() 0x%08X because it isn't marked alive anymore!\n", g.ActiveId);
        ClearActiveID();
    }

    // Update ActiveId data (clear reference to active widget if the widget isn't alive anymore)
    if (g.ActiveId)
        g.ActiveIdTimer += g.IO.DeltaTime;
    g.LastActiveIdTimer += g.IO.DeltaTime;
    g.ActiveIdPreviousFrame = g.ActiveId;
    g.ActiveIdIsAlive = 0;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdIsJustActivated = false;
    if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
        g.TempInputId = 0;
    if (g.InputTextReactivateId != 0 && g.InputTextReactivateId != g.DeactivatedItemData.ID)
        g.InputTextReactivateId = 0;
    if (g.ActiveId == 0)
    {
        g.ActiveIdUsingNavDirMask = 0x00;
        g.ActiveIdUsingAllKeyboardKeys = false;
    }
    if (g.DeactivatedItemData.ElapseFrame < g.FrameCount)
        g.DeactivatedItemData.ID = 0;
    g.DeactivatedItemData.IsAlive = false;

    // Record when we have been stationary as this state is preserved while over same item.
    // FIXME: The way this is expressed means user cannot alter HoverStationaryDelay during the frame to use varying values.
    // To allow this we should store HoverItemMaxStationaryTime+ID and perform the >= check in IsItemHovered() function.
    if (g.HoverItemDelayId != 0 && g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
        g.HoverItemUnlockedStationaryId = g.HoverItemDelayId;
    else if (g.HoverItemDelayId == 0)
        g.HoverItemUnlockedStationaryId = 0;
    if (g.HoveredWindow != NULL && g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
        g.HoverWindowUnlockedStationaryId = g.HoveredWindow->ID;
    else if (g.HoveredWindow == NULL)
        g.HoverWindowUnlockedStationaryId = 0;

    // Update hover delay for IsItemHovered() with delays and tooltips
    g.HoverItemDelayIdPreviousFrame = g.HoverItemDelayId;
    if (g.HoverItemDelayId != 0)
    {
        g.HoverItemDelayTimer += g.IO.DeltaTime;
        g.HoverItemDelayClearTimer = 0.0f;
        g.HoverItemDelayId = 0;
    }
    else if (g.HoverItemDelayTimer > 0.0f)
    {
        // This gives a little bit of leeway before clearing the hover timer, allowing mouse to cross gaps
        // We could expose 0.25f as style.HoverClearDelay but I am not sure of the logic yet, this is particularly subtle.
        g.HoverItemDelayClearTimer += g.IO.DeltaTime;
        if (g.HoverItemDelayClearTimer >= ImMax(0.25f, g.IO.DeltaTime * 2.0f)) // ~7 frames at 30 Hz + allow for low framerate
            g.HoverItemDelayTimer = g.HoverItemDelayClearTimer = 0.0f; // May want a decaying timer, in which case need to clamp at max first, based on max of caller last requested timer.
    }

    // Close popups on focus lost (currently wip/opt-in)
    //if (g.IO.AppFocusLost)
    //    ClosePopupsExceptModals();

    // Update keyboard input state
    UpdateKeyboardInputs();

    //IM_ASSERT(g.IO.KeyCtrl == IsKeyDown(ImGuiKey_LeftCtrl) || IsKeyDown(ImGuiKey_RightCtrl));
    //IM_ASSERT(g.IO.KeyShift == IsKeyDown(ImGuiKey_LeftShift) || IsKeyDown(ImGuiKey_RightShift));
    //IM_ASSERT(g.IO.KeyAlt == IsKeyDown(ImGuiKey_LeftAlt) || IsKeyDown(ImGuiKey_RightAlt));
    //IM_ASSERT(g.IO.KeySuper == IsKeyDown(ImGuiKey_LeftSuper) || IsKeyDown(ImGuiKey_RightSuper));

    // Drag and drop
    g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
    g.DragDropAcceptIdCurr = 0;
    g.DragDropAcceptFlagsPrev = g.DragDropAcceptFlagsCurr;
    g.DragDropAcceptFlagsCurr = ImGuiDragDropFlags_None;
    g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    g.DragDropWithinSource = false;
    g.DragDropWithinTarget = false;
    g.DragDropHoldJustPressedId = 0;
    if (g.DragDropActive)
    {
        // Also works when g.ActiveId==0 (aka leftover payload in progress, no active id)
        // You may disable this externally by hijacking the input route:
        //  'if (GetDragDropPayload() != NULL) { Shortcut(ImGuiKey_Escape, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverActive); }
        // but you will not get a return value from Shortcut() due to ActiveIdUsingAllKeyboardKeys logic. You can however poll IsKeyPressed(ImGuiKey_Escape) afterwards.
        ImGuiID owner_id = g.ActiveId ? g.ActiveId : ImHashStr("##DragDropCancelHandler");
        if (Shortcut(ImGuiKey_Escape, ImGuiInputFlags_RouteGlobal, owner_id))
        {
            ClearActiveID();
            ClearDragDrop();
        }
    }
    g.TooltipPreviousWindow = NULL;

    // Update keyboard/gamepad navigation
    NavUpdate();

    // Update mouse input state
    UpdateMouseInputs();

    // Undocking
    // (needs to be before UpdateMouseMovingWindowNewFrame so the window is already offset and following the mouse on the detaching frame)
    DockContextNewFrameUpdateUndocking(&g);

    // Mark all windows as not visible and compact unused memory.
    IM_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
    const bool gc_all = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f);
    const float memory_compact_start_time = gc_all ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
    for (ImGuiWindow* window : g.Windows)
    {
        window->WasActive = window->Active;
        window->Active = false;
        window->WriteAccessed = false;
        window->BeginCountPreviousFrame = window->BeginCount;
        window->BeginCount = 0;

        // Garbage collect transient buffers of recently unused windows
        if ((!window->WasActive || gc_all) && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
            GcCompactTransientWindowBuffers(window);
    }

    // Find hovered window
    // (needs to be before UpdateMouseMovingWindowNewFrame so we fill g.HoveredWindowUnderMovingWindow on the mouse release frame)
    // (currently needs to be done after the WasActive=Active loop and FindHoveredWindowEx uses ->Active)
    UpdateHoveredWindowAndCaptureFlags(g.IO.MousePos);

    // Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering)
    UpdateMouseMovingWindowNewFrame();

    // Background darkening/whitening
    if (GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
        g.DimBgRatio = ImMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
    else
        g.DimBgRatio = ImMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

    g.MouseCursor = ImGuiMouseCursor_Arrow;
    g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;

    // Platform IME data: reset for the frame
    g.PlatformImeDataPrev = g.PlatformImeData;
    g.PlatformImeData.WantVisible = g.PlatformImeData.WantTextInput = false;

    // Mouse wheel scrolling, scale
    UpdateMouseWheel();

    // Garbage collect transient buffers of recently unused tables
    for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
        if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
            TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
    for (ImGuiTableTempData& table_temp_data : g.TablesTempData)
        if (table_temp_data.LastTimeActive >= 0.0f && table_temp_data.LastTimeActive < memory_compact_start_time)
            TableGcCompactTransientBuffers(&table_temp_data);
    if (g.GcCompactAll)
        GcCompactTransientMiscBuffers();
    g.GcCompactAll = false;

    // Closing the focused window restore focus to the first active root window in descending z-order
    if (g.NavWindow && !g.NavWindow->WasActive)
        FocusTopMostWindowUnderOne(NULL, NULL, NULL, ImGuiFocusRequestFlags_RestoreFocusedChild);

    // No window should be open at the beginning of the frame.
    // But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
    g.CurrentWindowStack.resize(0);
    g.BeginPopupStack.resize(0);
    g.ItemFlagsStack.resize(0);
    g.ItemFlagsStack.push_back(ImGuiItemFlags_AutoClosePopups); // Default flags
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    g.GroupStack.resize(0);

    // Docking
    DockContextNewFrameUpdateDocking(&g);

    // [DEBUG] Update debug features
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    UpdateDebugToolItemPicker();
    UpdateDebugToolItemPathQuery();
    UpdateDebugToolFlashStyleColor();
    if (g.DebugLocateFrames > 0 && --g.DebugLocateFrames == 0)
    {
        g.DebugLocateId = 0;
        g.DebugBreakInLocateId = false;
    }
    if (g.DebugLogAutoDisableFrames > 0 && --g.DebugLogAutoDisableFrames == 0)
    {
        DebugLog("(Debug Log: Auto-disabled some ImGuiDebugLogFlags after 2 frames)\n");
        g.DebugLogFlags &= ~g.DebugLogAutoDisableFlags;
        g.DebugLogAutoDisableFlags = ImGuiDebugLogFlags_None;
    }
#endif

    // Create implicit/fallback window - which we will only render it if the user has added something to it.
    // We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
    // This fallback is particularly important as it prevents ImGui:: calls from crashing.
    g.WithinFrameScopeWithImplicitWindow = true;
    SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    Begin("Debug##Default");
    IM_ASSERT(g.CurrentWindow->IsFallbackWindow == true);

    // Store stack sizes
    g.ErrorCountCurrentFrame = 0;
    ErrorRecoveryStoreState(&g.StackSizesInNewFrame);

    // [DEBUG] When io.ConfigDebugBeginReturnValue is set, we make Begin()/BeginChild() return false at different level of the window-stack,
    // allowing to validate correct Begin/End behavior in user code.
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.IO.ConfigDebugBeginReturnValueLoop)
        g.DebugBeginReturnValueCullDepth = (g.DebugBeginReturnValueCullDepth == -1) ? 0 : ((g.DebugBeginReturnValueCullDepth + ((g.FrameCount % 4) == 0 ? 1 : 0)) % 10);
    else
        g.DebugBeginReturnValueCullDepth = -1;
#endif

    CallContextHooks(&g, ImGuiContextHookType_NewFramePost);
}

// FIXME: Add a more explicit sort order in the window structure.
static int IMGUI_CDECL ChildWindowComparer(const void* lhs, const void* rhs)
{
    const ImGuiWindow* const a = *(const ImGuiWindow* const *)lhs;
    const ImGuiWindow* const b = *(const ImGuiWindow* const *)rhs;
    if (int d = (a->Flags & ImGuiWindowFlags_Popup) - (b->Flags & ImGuiWindowFlags_Popup))
        return d;
    if (int d = (a->Flags & ImGuiWindowFlags_Tooltip) - (b->Flags & ImGuiWindowFlags_Tooltip))
        return d;
    return a->BeginOrderWithinParent - b->BeginOrderWithinParent;
}

static void AddWindowToSortBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window)
{
    out_sorted_windows->push_back(window);
    if (window->Active)
    {
        int count = window->DC.ChildWindows.Size;
        ImQsort(window->DC.ChildWindows.Data, (size_t)count, sizeof(ImGuiWindow*), ChildWindowComparer);
        for (int i = 0; i < count; i++)
        {
            ImGuiWindow* child = window->DC.ChildWindows[i];
            if (child->Active)
                AddWindowToSortBuffer(out_sorted_windows, child);
        }
    }
}

static void AddWindowToDrawData(ImGuiWindow* window, int layer)
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = window->Viewport;
    IM_ASSERT(viewport != NULL);
    g.IO.MetricsRenderWindows++;
    if (window->DrawList->_Splitter._Count > 1)
        window->DrawList->ChannelsMerge(); // Merge if user forgot to merge back. Also required in Docking branch for ImGuiWindowFlags_DockNodeHost windows.
    ImGui::AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[layer], window->DrawList);
    for (ImGuiWindow* child : window->DC.ChildWindows)
        if (IsWindowActiveAndVisible(child)) // Clipped children may have been marked not active
            AddWindowToDrawData(child, layer);
}

// Layer is locked for the root window, however child windows may use a different viewport (e.g. extruding menu)
static inline void AddRootWindowToDrawData(ImGuiWindow* window)
{
    AddWindowToDrawData(window, GetWindowDisplayLayer(window));
}

static void FlattenDrawDataIntoSingleLayer(ImDrawDataBuilder* builder)
{
    int n = builder->Layers[0]->Size;
    int full_size = n;
    for (int i = 1; i < IM_COUNTOF(builder->Layers); i++)
        full_size += builder->Layers[i]->Size;
    builder->Layers[0]->resize(full_size);
    for (int layer_n = 1; layer_n < IM_COUNTOF(builder->Layers); layer_n++)
    {
        ImVector<ImDrawList*>* layer = builder->Layers[layer_n];
        if (layer->empty())
            continue;
        memcpy(builder->Layers[0]->Data + n, layer->Data, layer->Size * sizeof(ImDrawList*));
        n += layer->Size;
        layer->resize(0);
    }
}

static void InitViewportDrawData(ImGuiViewportP* viewport)
{
    ImGuiIO& io = ImGui::GetIO();
    ImDrawData* draw_data = &viewport->DrawDataP;

    viewport->DrawData = draw_data; // Make publicly accessible
    viewport->DrawDataBuilder.Layers[0] = &draw_data->CmdLists;
    viewport->DrawDataBuilder.Layers[1] = &viewport->DrawDataBuilder.LayerData1;
    viewport->DrawDataBuilder.Layers[0]->resize(0);
    viewport->DrawDataBuilder.Layers[1]->resize(0);

    // When minimized, we report draw_data->DisplaySize as zero to be consistent with non-viewport mode,
    // and to allow applications/backends to easily skip rendering.
    // FIXME: Note that we however do NOT attempt to report "zero drawlist / vertices" into the ImDrawData structure.
    // This is because the work has been done already, and its wasted! We should fix that and add optimizations for
    // it earlier in the pipeline, rather than pretend to hide the data at the end of the pipeline.
    const bool is_minimized = (viewport->Flags & ImGuiViewportFlags_IsMinimized) != 0;

    draw_data->Valid = true;
    draw_data->CmdListsCount = 0;
    draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
    draw_data->DisplayPos = viewport->Pos;
    draw_data->DisplaySize = is_minimized ? ImVec2(0.0f, 0.0f) : viewport->Size;
    draw_data->FramebufferScale = (viewport->FramebufferScale.x != 0.0f) ? viewport->FramebufferScale : io.DisplayFramebufferScale;
    draw_data->OwnerViewport = viewport;
    draw_data->Textures = &ImGui::GetPlatformIO().Textures;
}

// Push a clipping rectangle for both ImGui logic (hit-testing etc.) and low-level ImDrawList rendering.
// - When using this function it is sane to ensure that float are perfectly rounded to integer values,
//   so that e.g. (int)(max.x-min.x) in user's render produce correct result.
// - If the code here changes, may need to update code of functions like NextColumn() and PushColumnClipRect():
//   some frequently called functions which to modify both channels and clipping simultaneously tend to use the
//   more specialized SetWindowClipRectBeforeSetChannel() to avoid extraneous updates of underlying ImDrawCmds.
// - This is analogous to PushFont()/PopFont() in the sense that are a mixing a global stack and a window stack,
//   which in the case of ClipRect is not so problematic but tends to be more restrictive for fonts.
void ImGui::PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void ImGui::PopClipRect()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PopClipRect();
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

static void ImGui::RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    ImGuiViewportP* viewport = window->Viewport;
    ImRect viewport_rect = viewport->GetMainRect();

    // Draw behind window by moving the draw command at the FRONT of the draw list
    {
        // Draw list have been trimmed already, hence the explicit recreation of a draw command if missing.
        // FIXME: This is a little bit complicated, solely to avoid creating/injecting an extra drawlist in drawdata.
        ImDrawList* draw_list = window->RootWindowDockTree->DrawList;
        draw_list->ChannelsMerge();
        if (draw_list->CmdBuffer.Size == 0)
            draw_list->AddDrawCmd();
        draw_list->PushClipRect(viewport_rect.Min - ImVec2(1, 1), viewport_rect.Max + ImVec2(1, 1), false); // FIXME: Need to strictly ensure ImDrawCmd are not merged (ElemCount==6 checks below will verify that)
        ImDrawCmd cmd = draw_list->CmdBuffer.back();
        IM_ASSERT(cmd.ElemCount == 0);
        draw_list->AddRectFilled(viewport_rect.Min, viewport_rect.Max, col);
        cmd = draw_list->CmdBuffer.back();
        draw_list->CmdBuffer.pop_back();
        draw_list->CmdBuffer.push_front(cmd);
        draw_list->AddDrawCmd(); // We need to create a command as CmdBuffer.back().IdxOffset won't be correct if we append to same command.
        draw_list->PopClipRect();
    }

    // Draw over sibling docking nodes in a same docking tree
    if (window->RootWindow->DockIsActive)
    {
        ImDrawList* draw_list = FindFrontMostVisibleChildWindow(window->RootWindowDockTree)->DrawList;
        draw_list->ChannelsMerge();
        if (draw_list->CmdBuffer.Size == 0)
            draw_list->AddDrawCmd();
        draw_list->PushClipRect(viewport_rect.Min, viewport_rect.Max, false);
        RenderRectFilledWithHole(draw_list, window->RootWindowDockTree->Rect(), window->RootWindow->Rect(), col, 0.0f);// window->RootWindowDockTree->WindowRounding);
        draw_list->PopClipRect();
    }
}

ImGuiWindow* ImGui::FindBottomMostVisibleWindowWithinBeginStack(ImGuiWindow* parent_window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* bottom_most_visible_window = parent_window;
    for (int i = FindWindowDisplayIndex(parent_window); i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        if (window->Flags & ImGuiWindowFlags_ChildWindow)
            continue;
        if (!IsWindowWithinBeginStackOf(window, parent_window))
            break;
        if (IsWindowActiveAndVisible(window) && GetWindowDisplayLayer(window) <= GetWindowDisplayLayer(parent_window))
            bottom_most_visible_window = window;
    }
    return bottom_most_visible_window;
}

// Important: AddWindowToDrawData() has not been called yet, meaning DockNodeHost windows needs a DrawList->ChannelsMerge() before usage.
// We call ChannelsMerge() lazily here at it is faster that doing a full iteration of g.Windows[] prior to calling RenderDimmedBackgrounds().
static void ImGui::RenderDimmedBackgrounds()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* modal_window = GetTopMostAndVisiblePopupModal();
    if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
        return;
    const bool dim_bg_for_modal = (modal_window != NULL);
    const bool dim_bg_for_window_list = (g.NavWindowingTargetAnim != NULL && g.NavWindowingTargetAnim->Active);
    if (!dim_bg_for_modal && !dim_bg_for_window_list)
        return;

    ImGuiViewport* viewports_already_dimmed[2] = { NULL, NULL };
    if (dim_bg_for_modal)
    {
        // Draw dimming behind modal or a begin stack child, whichever comes first in draw order.
        ImGuiWindow* dim_behind_window = FindBottomMostVisibleWindowWithinBeginStack(modal_window);
        RenderDimmedBackgroundBehindWindow(dim_behind_window, GetColorU32(modal_window->DC.ModalDimBgColor, g.DimBgRatio));
        viewports_already_dimmed[0] = modal_window->Viewport;
    }
    else if (dim_bg_for_window_list)
    {
        // Draw dimming behind Ctrl+Tab target window and behind Ctrl+Tab UI window
        RenderDimmedBackgroundBehindWindow(g.NavWindowingTargetAnim, GetColorU32(ImGuiCol_NavWindowingDimBg, g.DimBgRatio));
        viewports_already_dimmed[0] = g.NavWindowingTargetAnim->Viewport;
        if (g.NavWindowingListWindow != NULL && g.NavWindowingListWindow->Active && g.NavWindowingListWindow->Viewport && g.NavWindowingListWindow->Viewport != g.NavWindowingTargetAnim->Viewport)
        {
            RenderDimmedBackgroundBehindWindow(g.NavWindowingListWindow, GetColorU32(ImGuiCol_NavWindowingDimBg, g.DimBgRatio));
            viewports_already_dimmed[1] = g.NavWindowingListWindow->Viewport;
        }

        // Draw border around Ctrl+Tab target window
        ImGuiWindow* window = g.NavWindowingTargetAnim;
        ImGuiViewport* viewport = window->Viewport;
        float distance = g.FontSize;
        ImRect bb = window->Rect();
        bb.Expand(distance);
        if (bb.GetWidth() >= viewport->Size.x && bb.GetHeight() >= viewport->Size.y)
            bb.Expand(-distance - 1.0f); // If a window fits the entire viewport, adjust its highlight inward
        window->DrawList->ChannelsMerge();
        if (window->DrawList->CmdBuffer.Size == 0)
            window->DrawList->AddDrawCmd();
        window->DrawList->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size);
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha), window->WindowRounding, 0, 3.0f); // FIXME-DPI
        window->DrawList->PopClipRect();
    }

    // Draw dimming background on _other_ viewports than the ones our windows are in
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        if (viewport == viewports_already_dimmed[0] || viewport == viewports_already_dimmed[1])
            continue;
        if (modal_window && viewport->Window && IsWindowAbove(viewport->Window, modal_window))
            continue;
        ImDrawList* draw_list = GetForegroundDrawList(viewport);
        const ImU32 dim_bg_col = GetColorU32(dim_bg_for_modal ? ImGuiCol_ModalWindowDimBg : ImGuiCol_NavWindowingDimBg, g.DimBgRatio);
        draw_list->AddRectFilled(viewport->Pos, viewport->Pos + viewport->Size, dim_bg_col);
    }
}

// This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
void ImGui::EndFrame()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    // Don't process EndFrame() multiple times.
    if (g.FrameCountEnded == g.FrameCount)
        return;
    IM_ASSERT_USER_ERROR_RET(g.WithinFrameScope, "Forgot to call ImGui::NewFrame()?");

    CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

    // [EXPERIMENTAL] Recover from errors
    if (g.IO.ConfigErrorRecovery)
        ErrorRecoveryTryToRecoverState(&g.StackSizesInNewFrame);
    ErrorCheckEndFrameSanityChecks();
    ErrorCheckEndFrameFinalizeErrorTooltip();

    // Notify Platform/OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
    ImGuiPlatformImeData* ime_data = &g.PlatformImeData;
    if (g.PlatformIO.Platform_SetImeDataFn != NULL && memcmp(ime_data, &g.PlatformImeDataPrev, sizeof(ImGuiPlatformImeData)) != 0)
    {
        ImGuiViewport* viewport = FindViewportByID(ime_data->ViewportId);
        if (viewport == NULL)
            viewport = GetMainViewport();
        IMGUI_DEBUG_LOG_IO("[io] Calling Platform_SetImeDataFn(): WantVisible: %d, InputPos (%.2f,%.2f) for Viewport 0x%08X\n", ime_data->WantVisible, ime_data->InputPos.x, ime_data->InputPos.y, viewport->ID);
        g.PlatformIO.Platform_SetImeDataFn(&g, viewport, ime_data);
    }
    g.WantTextInputNextFrame = ime_data->WantTextInput ? 1 : 0;

    // Hide and unfocus implicit/fallback "Debug" window if it hasn't been used
    g.WithinFrameScopeWithImplicitWindow = false;
    if (g.CurrentWindow && g.CurrentWindow->IsFallbackWindow && g.CurrentWindow->WriteAccessed == false)
    {
        g.CurrentWindow->Active = false;
        if (g.NavWindow && g.NavWindow->RootWindow == g.CurrentWindow)
            FocusWindow(NULL);
    }
    End();

    // Update navigation: Ctrl+Tab, wrap-around requests
    NavEndFrame();

    // Update docking
    DockContextEndFrame(&g);

    SetCurrentViewport(NULL, NULL);

    // Drag and Drop: Elapse payload (if delivered, or if source stops being submitted)
    if (g.DragDropActive)
    {
        bool is_delivered = g.DragDropPayload.Delivery;
        bool is_elapsed = (g.DragDropSourceFrameCount + 1 < g.FrameCount) && ((g.DragDropSourceFlags & ImGuiDragDropFlags_PayloadAutoExpire) || g.DragDropMouseButton == -1 || !IsMouseDown(g.DragDropMouseButton));
        if (is_delivered || is_elapsed)
            ClearDragDrop();
    }

    // Drag and Drop: Fallback for missing source tooltip. This is not ideal but better than nothing.
    // If you want to handle source item disappearing: instead of submitting your description tooltip
    // in the BeginDragDropSource() block of the dragged item, you can submit them from a safe single spot
    // (e.g. end of your item loop, or before EndFrame) by reading payload data.
    // In the typical case, the contents of drag tooltip should be possible to infer solely from payload data.
    if (g.DragDropActive && g.DragDropSourceFrameCount + 1 < g.FrameCount && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    {
        g.DragDropWithinSource = true;
        SetTooltip("...");
        g.DragDropWithinSource = false;
    }

    // End frame
    g.WithinFrameScope = false;
    g.FrameCountEnded = g.FrameCount;
    UpdateFontsEndFrame();

    // Initiate moving window + handle left-click and right-click focus
    UpdateMouseMovingWindowEndFrame();

    // Update user-facing viewport list (g.Viewports -> g.PlatformIO.Viewports after filtering out some)
    UpdateViewportsEndFrame();

    // Sort the window list so that all child windows are after their parent
    // We cannot do that on FocusWindow() because children may not exist yet
    g.WindowsTempSortBuffer.resize(0);
    g.WindowsTempSortBuffer.reserve(g.Windows.Size);
    for (ImGuiWindow* window : g.Windows)
    {
        if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow))       // if a child is active its parent will add it
            continue;
        AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
    }

    // This usually assert if there is a mismatch between the ImGuiWindowFlags_ChildWindow / ParentWindow values and DC.ChildWindows[] in parents, aka we've done something wrong.
    IM_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
    g.Windows.swap(g.WindowsTempSortBuffer);
    g.IO.MetricsActiveWindows = g.WindowsActiveCount;

    UpdateTexturesEndFrame();

    // Unlock font atlas
    for (ImFontAtlas* atlas : g.FontAtlases)
        atlas->Locked = false;

    // Clear Input data for next frame
    g.IO.MousePosPrev = g.IO.MousePos;
    g.IO.AppFocusLost = false;
    g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
    g.IO.InputQueueCharacters.resize(0);

    CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
}

// Prepare the data for rendering so you can call GetDrawData()
// (As with anything within the ImGui:: namespace this doesn't touch your GPU or graphics API at all:
// it is the role of the ImGui_ImplXXXX_RenderDrawData() function provided by the renderer backend)
void ImGui::Render()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded != g.FrameCount)
        EndFrame();
    if (g.FrameCountRendered == g.FrameCount)
        return;
    g.FrameCountRendered = g.FrameCount;

    g.IO.MetricsRenderWindows = 0;
    CallContextHooks(&g, ImGuiContextHookType_RenderPre);

    // Add background ImDrawList (for each active viewport)
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        InitViewportDrawData(viewport);
        if (viewport->BgFgDrawLists[0] != NULL && viewport->BgFgDrawListsLastTimeActive[0] == (float)g.Time)
            AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
    }

    // Draw modal/window whitening backgrounds
    RenderDimmedBackgrounds();

    // Add ImDrawList to render
    ImGuiWindow* windows_to_render_top_most[2];
    windows_to_render_top_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindowDockTree : NULL;
    windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
    for (ImGuiWindow* window : g.Windows)
    {
        IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
        if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
            AddRootWindowToDrawData(window);
    }
    for (int n = 0; n < IM_COUNTOF(windows_to_render_top_most); n++)
        if (windows_to_render_top_most[n] && IsWindowActiveAndVisible(windows_to_render_top_most[n])) // NavWindowingTarget is always temporarily displayed as the top-most window
            AddRootWindowToDrawData(windows_to_render_top_most[n]);

    // Draw software mouse cursor if requested by io.MouseDrawCursor flag
    if (g.IO.MouseDrawCursor && g.MouseCursor != ImGuiMouseCursor_None)
        RenderMouseCursor(g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor, IM_COL32_WHITE, IM_COL32_BLACK, IM_COL32(0, 0, 0, 48));

    // Setup ImDrawData structures for end-user
    g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        FlattenDrawDataIntoSingleLayer(&viewport->DrawDataBuilder);

        // Add foreground ImDrawList (for each active viewport)
        if (viewport->BgFgDrawLists[1] != NULL && viewport->BgFgDrawListsLastTimeActive[1] == (float)g.Time)
            AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

        // We call _PopUnusedDrawCmd() last thing, as RenderDimmedBackgrounds() rely on a valid command being there (especially in docking branch).
        ImDrawData* draw_data = &viewport->DrawDataP;
        IM_ASSERT(draw_data->CmdLists.Size == draw_data->CmdListsCount);
        for (ImDrawList* draw_list : draw_data->CmdLists)
            draw_list->_PopUnusedDrawCmd();

        g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
        g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        for (ImFontAtlas* atlas : g.FontAtlases)
            ImFontAtlasDebugLogTextureRequests(atlas);
#endif

    CallContextHooks(&g, ImGuiContextHookType_RenderPost);
}

// Calculate text size. Text can be multi-line. Optionally ignore text after a ## marker.
// CalcTextSize("") should return ImVec2(0.0f, g.FontSize)
ImVec2 ImGui::CalcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImGuiContext& g = *GImGui;

    const char* text_display_end;
    if (hide_text_after_double_hash)
        text_display_end = FindRenderedTextEnd(text, text_end);      // Hide anything after a '##' string
    else
        text_display_end = text_end;

    ImFont* font = g.Font;
    const float font_size = g.FontSize;
    if (text == text_display_end)
        return ImVec2(0.0f, font_size);
    ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

    // Round
    // FIXME: This has been here since Dec 2015 (7b0bf230) but down the line we want this out.
    // FIXME: Investigate using ceilf or e.g.
    // - https://git.musl-libc.org/cgit/musl/tree/src/math/ceilf.c
    // - https://embarkstudios.github.io/rust-gpu/api/src/libm/math/ceilf.rs.html
    text_size.x = IM_TRUNC(text_size.x + 0.99999f);

    return text_size;
}

// Find window given position, search front-to-back
// - Typically write output back to g.HoveredWindow and g.HoveredWindowUnderMovingWindow.
// - FIXME: Note that we have an inconsequential lag here: OuterRectClipped is updated in Begin(), so windows moved programmatically
//   with SetWindowPos() and not SetNextWindowPos() will have that rectangle lagging by a frame at the time FindHoveredWindow() is
//   called, aka before the next Begin(). Moving window isn't affected.
// - The 'find_first_and_in_any_viewport = true' mode is only used by TestEngine. It is simpler to maintain here.
void ImGui::FindHoveredWindowEx(const ImVec2& pos, bool find_first_and_in_any_viewport, ImGuiWindow** out_hovered_window, ImGuiWindow** out_hovered_window_under_moving_window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* hovered_window = NULL;
    ImGuiWindow* hovered_window_under_moving_window = NULL;

    // Special handling for the window being moved: Ignore the mouse viewport check (because it may reset/lose its viewport during the undocking frame)
    ImGuiViewportP* backup_moving_window_viewport = NULL;
    if (find_first_and_in_any_viewport == false && g.MovingWindow)
    {
        backup_moving_window_viewport = g.MovingWindow->Viewport;
        g.MovingWindow->Viewport = g.MouseViewport;
        if (!(g.MovingWindow->Flags & ImGuiWindowFlags_NoMouseInputs))
            hovered_window = g.MovingWindow;
    }

    ImVec2 padding_regular = g.Style.TouchExtraPadding;
    ImVec2 padding_for_resize = ImMax(g.Style.TouchExtraPadding, ImVec2(g.Style.WindowBorderHoverPadding, g.Style.WindowBorderHoverPadding));
    for (int i = g.Windows.Size - 1; i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        IM_MSVC_WARNING_SUPPRESS(28182); // [Static Analyzer] Dereferencing NULL pointer.
        if (!window->WasActive || window->Hidden)
            continue;
        if (window->Flags & ImGuiWindowFlags_NoMouseInputs)
            continue;
        IM_ASSERT(window->Viewport);
        if (window->Viewport != g.MouseViewport)
            continue;

        // Using the clipped AABB, a child window will typically be clipped by its parent (not always)
        ImVec2 hit_padding = (window->Flags & (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) ? padding_regular : padding_for_resize;
        if (!window->OuterRectClipped.ContainsWithPad(pos, hit_padding))
            continue;

        // Support for one rectangular hole in any given window
        // FIXME: Consider generalizing hit-testing override (with more generic data, callback, etc.) (#1512)
        if (window->HitTestHoleSize.x != 0)
        {
            ImVec2 hole_pos(window->Pos.x + (float)window->HitTestHoleOffset.x, window->Pos.y + (float)window->HitTestHoleOffset.y);
            ImVec2 hole_size((float)window->HitTestHoleSize.x, (float)window->HitTestHoleSize.y);
            if (ImRect(hole_pos, hole_pos + hole_size).Contains(pos))
                continue;
        }

        if (find_first_and_in_any_viewport)
        {
            hovered_window = window;
            break;
        }
        else
        {
            if (hovered_window == NULL)
                hovered_window = window;
            IM_MSVC_WARNING_SUPPRESS(28182); // [Static Analyzer] Dereferencing NULL pointer.
            if (hovered_window_under_moving_window == NULL && (!g.MovingWindow || window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree))
                hovered_window_under_moving_window = window;
            if (hovered_window && hovered_window_under_moving_window)
                break;
        }
    }

    *out_hovered_window = hovered_window;
    if (out_hovered_window_under_moving_window != NULL)
        *out_hovered_window_under_moving_window = hovered_window_under_moving_window;
    if (find_first_and_in_any_viewport == false && g.MovingWindow)
        g.MovingWindow->Viewport = backup_moving_window_viewport;
}

bool ImGui::IsItemActive()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
        return g.ActiveId == g.LastItemData.ID;
    return false;
}

bool ImGui::IsItemActivated()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
        if (g.ActiveId == g.LastItemData.ID && g.ActiveIdPreviousFrame != g.LastItemData.ID)
            return true;
    return false;
}

bool ImGui::IsItemDeactivated()
{
    ImGuiContext& g = *GImGui;
    if (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDeactivated)
        return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Deactivated) != 0;
    return g.DeactivatedItemData.ID == g.LastItemData.ID && g.LastItemData.ID != 0 && g.DeactivatedItemData.ElapseFrame >= g.FrameCount;
}

bool ImGui::IsItemDeactivatedAfterEdit()
{
    ImGuiContext& g = *GImGui;
    return IsItemDeactivated() && g.DeactivatedItemData.HasBeenEditedBefore;
}

// == (GetItemID() == GetFocusID() && GetFocusID() != 0)
bool ImGui::IsItemFocused()
{
    ImGuiContext& g = *GImGui;
    if (g.NavId != g.LastItemData.ID || g.NavId == 0)
        return false;

    // Special handling for the dummy item after Begin() which represent the title bar or tab.
    // When the window is collapsed (SkipItems==true) that last item will never be overwritten so we need to detect the case.
    ImGuiWindow* window = g.CurrentWindow;
    if (g.LastItemData.ID == window->ID && window->WriteAccessed)
        return false;

    return true;
}

// Important: this can be useful but it is NOT equivalent to the behavior of e.g.Button()!
// Most widgets have specific reactions based on mouse-up/down state, mouse position etc.
bool ImGui::IsItemClicked(ImGuiMouseButton mouse_button)
{
    return IsMouseClicked(mouse_button) && IsItemHovered(ImGuiHoveredFlags_None);
}

bool ImGui::IsItemToggledOpen()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledOpen) ? true : false;
}

// Call after a Selectable() or TreeNode() involved in multi-selection.
// Useful if you need the per-item information before reaching EndMultiSelect(), e.g. for rendering purpose.
// This is only meant to be called inside a BeginMultiSelect()/EndMultiSelect() block.
// (Outside of multi-select, it would be misleading/ambiguous to report this signal, as widgets
// return e.g. a pressed event and user code is in charge of altering selection in ways we cannot predict.)
bool ImGui::IsItemToggledSelection()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentMultiSelect != NULL); // Can only be used inside a BeginMultiSelect()/EndMultiSelect()
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledSelection) ? true : false;
}

// IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear ImGui or to your underlying app,
// you should not use this function! Use the 'io.WantCaptureMouse' boolean for that!
// Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?" for details.
bool ImGui::IsAnyItemHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredId != 0 || g.HoveredIdPreviousFrame != 0;
}

bool ImGui::IsAnyItemActive()
{
    ImGuiContext& g = *GImGui;
    return g.ActiveId != 0;
}

bool ImGui::IsAnyItemFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavId != 0 && g.NavCursorVisible;
}

bool ImGui::IsItemVisible()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Visible) != 0;
}

bool ImGui::IsItemEdited()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Edited) != 0;
}

// Allow next item to be overlapped by subsequent items.
// This works by requiring HoveredId to match for two subsequent frames,
// so if a following items overwrite it our interactions will naturally be disabled.
void ImGui::SetNextItemAllowOverlap()
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.ItemFlags |= ImGuiItemFlags_AllowOverlap;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
// Allow last item to be overlapped by a subsequent item. Both may be activated during the same frame before the later one takes priority.
// Use SetNextItemAllowOverlap() *before* your item instead of calling this!
//void ImGui::SetItemAllowOverlap()
//{
//    ImGuiContext& g = *GImGui;
//    ImGuiID id = g.LastItemData.ID;
//    if (g.HoveredId == id)
//        g.HoveredIdAllowOverlap = true;
//    if (g.ActiveId == id) // Before we made this obsolete, most calls to SetItemAllowOverlap() used to avoid this path by testing g.ActiveId != id.
//        g.ActiveIdAllowOverlap = true;
//}
#endif

// This is a shortcut for not taking ownership of 100+ keys, frequently used by drag operations.
// FIXME: It might be undesirable that this will likely disable KeyOwner-aware shortcuts systems. Consider a more fine-tuned version if needed?
void ImGui::SetActiveIdUsingAllKeyboardKeys()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.ActiveId != 0);
    g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_COUNT) - 1;
    g.ActiveIdUsingAllKeyboardKeys = true;
    NavMoveRequestCancel();
}

ImGuiID ImGui::GetItemID()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.ID;
}

ImVec2 ImGui::GetItemRectMin()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.Min;
}

ImVec2 ImGui::GetItemRectMax()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.Max;
}

ImVec2 ImGui::GetItemRectSize()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.GetSize();
}

ImGuiItemFlags ImGui::GetItemFlags()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.ItemFlags;
}

void ImGui::PushItemFlag(ImGuiItemFlags option, bool enabled)
{
    ImGuiContext& g = *GImGui;
    ImGuiItemFlags item_flags = g.CurrentItemFlags;
    IM_ASSERT(item_flags == g.ItemFlagsStack.back());
    if (enabled)
        item_flags |= option;
    else
        item_flags &= ~option;
    g.CurrentItemFlags = item_flags;
    g.ItemFlagsStack.push_back(item_flags);
}

void ImGui::PopItemFlag()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.ItemFlagsStack.Size > 1, "Calling PopItemFlag() too many times!");
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
}

// BeginDisabled()/EndDisabled()
// - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
// - Visually this is currently altering alpha, but it is expected that in a future styling system this would work differently.
// - Feedback welcome at https://github.com/ocornut/imgui/issues/211
// - BeginDisabled(false)/EndDisabled() essentially does nothing but is provided to facilitate use of boolean expressions.
//   (as a micro-optimization: if you have tens of thousands of BeginDisabled(false)/EndDisabled() pairs, you might want to reformulate your code to avoid making those calls)
// - Note: mixing up BeginDisabled() and PushItemFlag(ImGuiItemFlags_Disabled) is currently NOT SUPPORTED.
void ImGui::BeginDisabled(bool disabled)
{
    ImGuiContext& g = *GImGui;
    bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (!was_disabled && disabled)
    {
        g.DisabledAlphaBackup = g.Style.Alpha;
        g.Style.Alpha *= g.Style.DisabledAlpha; // PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * g.Style.DisabledAlpha);
    }
    if (was_disabled || disabled)
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled;
    g.ItemFlagsStack.push_back(g.CurrentItemFlags); // FIXME-OPT: can we simply skip this and use DisabledStackSize?
    g.DisabledStackSize++;
}

void ImGui::EndDisabled()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.DisabledStackSize > 0, "Calling EndDisabled() too many times!");
    g.DisabledStackSize--;
    bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    //PopItemFlag();
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    if (was_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
        g.Style.Alpha = g.DisabledAlphaBackup; //PopStyleVar();
}

// Could have been called BeginDisabledDisable() but it didn't want to be award nominated for most awkward function name.
// Ideally we would use a shared e.g. BeginDisabled()->BeginDisabledEx() but earlier needs to be optimal.
// The whole code for this is awkward, will reevaluate if we find a way to implement SetNextItemDisabled().
void ImGui::BeginDisabledOverrideReenable()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentItemFlags & ImGuiItemFlags_Disabled);
    g.CurrentWindowStack.back().DisabledOverrideReenableAlphaBackup = g.Style.Alpha;
    g.Style.Alpha = g.DisabledAlphaBackup;
    g.CurrentItemFlags &= ~ImGuiItemFlags_Disabled;
    g.ItemFlagsStack.push_back(g.CurrentItemFlags);
    g.DisabledStackSize++;
}

void ImGui::EndDisabledOverrideReenable()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DisabledStackSize > 0);
    g.DisabledStackSize--;
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    g.Style.Alpha = g.CurrentWindowStack.back().DisabledOverrideReenableAlphaBackup;
}

// ATTENTION THIS IS IN LEGACY LOCAL SPACE.
void ImGui::PushTextWrapPos(float wrap_local_pos_x)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DC.TextWrapPosStack.push_back(window->DC.TextWrapPos);
    window->DC.TextWrapPos = wrap_local_pos_x;
}

void ImGui::PopTextWrapPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR_RET(window->DC.TextWrapPosStack.Size > 0, "Calling PopTextWrapPos() too many times!");
    window->DC.TextWrapPos = window->DC.TextWrapPosStack.back();
    window->DC.TextWrapPosStack.pop_back();
}

ImFont* ImGui::GetFont()
{
    return GImGui->Font;
}

ImFontBaked* ImGui::GetFontBaked()
{
    return GImGui->FontBaked;
}

// Get current font size (= height in pixels) of current font, with global scale factors applied.
// - Use style.FontSizeBase to get value before global scale factors.
// - recap: ImGui::GetFontSize() == style.FontSizeBase * (style.FontScaleMain * style.FontScaleDpi * other_scaling_factors)
float ImGui::GetFontSize()
{
    return GImGui->FontSize;
}

ImVec2 ImGui::GetFontTexUvWhitePixel()
{
    return GImGui->DrawListSharedData.TexUvWhitePixel;
}

// Prefer using PushFont(NULL, style.FontSizeBase * factor), or use style.FontScaleMain to scale all windows.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
void ImGui::SetWindowFontScale(float scale)
{
    IM_ASSERT(scale > 0.0f);
    ImGuiWindow* window = GetCurrentWindow();
    window->FontWindowScale = scale;
    UpdateCurrentFontSize(0.0f);
}
#endif

void ImGui::PushFocusScope(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiFocusScopeData data;
    data.ID = id;
    data.WindowID = g.CurrentWindow->ID;
    g.FocusScopeStack.push_back(data);
    g.CurrentFocusScopeId = id;
}

void ImGui::PopFocusScope()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.FocusScopeStack.Size > g.StackSizesInBeginForCurrentWindow->SizeOfFocusScopeStack, "Calling PopFocusScope() too many times!");
    g.FocusScopeStack.pop_back();
    g.CurrentFocusScopeId = g.FocusScopeStack.Size ? g.FocusScopeStack.back().ID : 0;
}

bool ImGui::IsInNavFocusRoute(ImGuiID focus_scope_id)
{
    ImGuiContext& g = *GImGui;
    if (g.NavFocusScopeId == focus_scope_id)
        return true;
    for (const ImGuiFocusScopeData& focus_scope : g.NavFocusRoute)
        if (focus_scope.ID == focus_scope_id)
            return true;
    return false;
}

void ImGui::SetNavFocusScope(ImGuiID focus_scope_id)
{
    ImGuiContext& g = *GImGui;
    g.NavFocusScopeId = focus_scope_id;
    g.NavFocusRoute.resize(0); // Invalidate
    if (focus_scope_id == 0)
        return;
    IM_ASSERT(g.NavWindow != NULL);

    // Store current path (in reverse order)
    if (focus_scope_id == g.CurrentFocusScopeId)
    {
        // Top of focus stack contains local focus scopes inside current window
        for (int n = g.FocusScopeStack.Size - 1; n >= 0 && g.FocusScopeStack.Data[n].WindowID == g.CurrentWindow->ID; n--)
            g.NavFocusRoute.push_back(g.FocusScopeStack.Data[n]);
    }
    else if (focus_scope_id == g.NavWindow->NavRootFocusScopeId)
        g.NavFocusRoute.push_back({ focus_scope_id, g.NavWindow->ID });
    else
        return;

    // Then follow on manually set ParentWindowForFocusRoute field (#6798)
    for (ImGuiWindow* window = g.NavWindow->ParentWindowForFocusRoute; window != NULL; window = window->ParentWindowForFocusRoute)
        g.NavFocusRoute.push_back({ window->NavRootFocusScopeId, window->ID });
    IM_ASSERT(g.NavFocusRoute.Size < 100); // Maximum depth is technically 251 as per CalcRoutingScore(): 254 - 3
}

// Focus = move navigation cursor, set scrolling, set focus window.
void ImGui::FocusItem()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IMGUI_DEBUG_LOG_FOCUS("FocusItem(0x%08x) in window \"%s\"\n", g.LastItemData.ID, window->Name);
    if (g.DragDropActive || g.MovingWindow != NULL) // FIXME: Opt-in flags for this?
    {
        IMGUI_DEBUG_LOG_FOCUS("FocusItem() ignored while DragDropActive!\n");
        return;
    }

    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi | ImGuiNavMoveFlags_NoSetNavCursorVisible | ImGuiNavMoveFlags_NoSelect;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    SetNavWindow(window);
    NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_Up, move_flags, scroll_flags);
    NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
}

void ImGui::ActivateItemByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.NavNextActivateId = id;
    g.NavNextActivateFlags = ImGuiActivateFlags_None;
}

// Note: this will likely be called ActivateItem() once we rework our Focus/Activation system!
// But ActivateItem() should function without altering scroll/focus?
void ImGui::SetKeyboardFocusHere(int offset)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(offset >= -1);    // -1 is allowed but not below
    IMGUI_DEBUG_LOG_FOCUS("SetKeyboardFocusHere(%d) in window \"%s\"\n", offset, window->Name);

    // It makes sense in the vast majority of cases to never interrupt a drag and drop.
    // When we refactor this function into ActivateItem() we may want to make this an option.
    // MovingWindow is protected from most user inputs using SetActiveIdUsingNavAndKeys(), but
    // is also automatically dropped in the event g.ActiveId is stolen.
    if (g.DragDropActive || g.MovingWindow != NULL)
    {
        IMGUI_DEBUG_LOG_FOCUS("SetKeyboardFocusHere() ignored while DragDropActive!\n");
        return;
    }

    SetNavWindow(window);

    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_Activate | ImGuiNavMoveFlags_FocusApi | ImGuiNavMoveFlags_NoSetNavCursorVisible;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    NavMoveRequestSubmit(ImGuiDir_None, offset < 0 ? ImGuiDir_Up : ImGuiDir_Down, move_flags, scroll_flags); // FIXME-NAV: Once we refactor tabbing, add LegacyApi flag to not activate non-inputable.
    if (offset == -1)
    {
        NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
    }
    else
    {
        g.NavTabbingDir = 1;
        g.NavTabbingCounter = offset + 1;
    }
}

void ImGui::SetItemDefaultFocus()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!window->Appearing)
        return;
    if (g.NavWindow != window->RootWindowForNav || (!g.NavInitRequest && g.NavInitResult.ID == 0) || g.NavLayer != window->DC.NavLayerCurrent)
        return;

    g.NavInitRequest = false;
    NavApplyItemToResult(&g.NavInitResult);
    NavUpdateAnyRequestFlag();

    // Scroll could be done in NavInitRequestApplyResult() via an opt-in flag (we however don't want regular init requests to scroll)
    if (!window->ClipRect.Contains(g.LastItemData.Rect))
        ScrollToRectEx(window, g.LastItemData.Rect, ImGuiScrollFlags_None);
}

void ImGui::SetStateStorage(ImGuiStorage* tree)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

ImGuiStorage* ImGui::GetStateStorage()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->DC.StateStorage;
}

bool ImGui::IsRectVisible(const ImVec2& size)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ClipRect.Overlaps(ImRect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool ImGui::IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ClipRect.Overlaps(ImRect(rect_min, rect_max));
}

//-----------------------------------------------------------------------------
// [SECTION] FONTS, TEXTURES
//-----------------------------------------------------------------------------
// Most of the relevant font logic is in imgui_draw.cpp.
// Those are high-level support functions.
//-----------------------------------------------------------------------------
// - UpdateTexturesNewFrame() [Internal]
// - UpdateTexturesEndFrame() [Internal]
// - UpdateFontsNewFrame() [Internal]
// - UpdateFontsEndFrame() [Internal]
// - GetDefaultFont() [Internal]
// - RegisterUserTexture() [Internal]
// - UnregisterUserTexture() [Internal]
// - RegisterFontAtlas() [Internal]
// - UnregisterFontAtlas() [Internal]
// - SetCurrentFont() [Internal]
// - UpdateCurrentFontSize() [Internal]
// - SetFontRasterizerDensity() [Internal]
// - PushFont()
// - PopFont()
//-----------------------------------------------------------------------------

static void ImGui::UpdateTexturesNewFrame()
{
    // Cannot update every atlases based on atlas's FrameCount < g.FrameCount, because an atlas may be shared by multiple contexts with different frame count.
    ImGuiContext& g = *GImGui;
    const bool has_textures = (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures) != 0;
    for (ImFontAtlas* atlas : g.FontAtlases)
    {
        if (atlas->OwnerContext == &g)
        {
            ImFontAtlasUpdateNewFrame(atlas, g.FrameCount, has_textures);
        }
        else
        {
            // (1) If you manage font atlases yourself, e.g. create a ImFontAtlas yourself you need to call ImFontAtlasUpdateNewFrame() on it.
            // Otherwise, calling ImGui::CreateContext() without parameter will create an atlas owned by the context.
            // (2) If you have multiple font atlases, make sure the 'atlas->RendererHasTextures' as specified in the ImFontAtlasUpdateNewFrame() call matches for that.
            // (3) If you have multiple imgui contexts, they also need to have a matching value for ImGuiBackendFlags_RendererHasTextures.
            IM_ASSERT(atlas->Builder != NULL && atlas->Builder->FrameCount != -1);
            IM_ASSERT(atlas->RendererHasTextures == has_textures);
        }
    }
}

// Build a single texture list
static void ImGui::UpdateTexturesEndFrame()
{
    ImGuiContext& g = *GImGui;
    g.PlatformIO.Textures.resize(0);
    for (ImFontAtlas* atlas : g.FontAtlases)
        for (ImTextureData* tex : atlas->TexList)
        {
            // We provide this information so backends can decide whether to destroy textures.
            // This means in practice that if N imgui contexts are created with a shared atlas, we assume all of them have a backend initialized.
            tex->RefCount = (unsigned short)atlas->RefCount;
            g.PlatformIO.Textures.push_back(tex);
        }
    for (ImTextureData* tex : g.UserTextures)
        g.PlatformIO.Textures.push_back(tex);
}

void ImGui::UpdateFontsNewFrame()
{
    ImGuiContext& g = *GImGui;
    if ((g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0)
        for (ImFontAtlas* atlas : g.FontAtlases)
            atlas->Locked = true;

    if (g.Style._NextFrameFontSizeBase != 0.0f)
    {
        g.Style.FontSizeBase = g.Style._NextFrameFontSizeBase;
        g.Style._NextFrameFontSizeBase = 0.0f;
    }

    // Apply default font size the first time
    ImFont* font = ImGui::GetDefaultFont();
    if (g.Style.FontSizeBase <= 0.0f)
        g.Style.FontSizeBase = (font->LegacySize > 0.0f ? font->LegacySize : FONT_DEFAULT_SIZE_BASE);

    // Set initial font
    g.Font = font;
    g.FontSizeBase = g.Style.FontSizeBase;
    g.FontSize = 0.0f;
    ImFontStackData font_stack_data = { font, g.Style.FontSizeBase, g.Style.FontSizeBase };           // <--- Will restore FontSize
    SetCurrentFont(font_stack_data.Font, font_stack_data.FontSizeBeforeScaling, 0.0f); // <--- but use 0.0f to enable scale
    g.FontStack.push_back(font_stack_data);
    IM_ASSERT(g.Font->IsLoaded());
}

void ImGui::UpdateFontsEndFrame()
{
    PopFont();
}

ImFont* ImGui::GetDefaultFont()
{
    ImGuiContext& g = *GImGui;
    ImFontAtlas* atlas = g.IO.Fonts;
    if (atlas->Builder == NULL || atlas->Fonts.Size == 0)
        ImFontAtlasBuildMain(atlas);
    return g.IO.FontDefault ? g.IO.FontDefault : atlas->Fonts[0];
}

// EXPERIMENTAL: DO NOT USE YET.
void ImGui::RegisterUserTexture(ImTextureData* tex)
{
    ImGuiContext& g = *GImGui;
    tex->RefCount++;
    g.UserTextures.push_back(tex);
}

void ImGui::UnregisterUserTexture(ImTextureData* tex)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(tex->RefCount > 0);
    tex->RefCount--;
    g.UserTextures.find_erase(tex);
}

void ImGui::RegisterFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    if (g.FontAtlases.Size == 0)
        IM_ASSERT(atlas == g.IO.Fonts);
    atlas->RefCount++;
    g.FontAtlases.push_back(atlas);
    ImFontAtlasAddDrawListSharedData(atlas, &g.DrawListSharedData);
    for (ImTextureData* tex : atlas->TexList)
        tex->RefCount = (unsigned short)atlas->RefCount;
}

void ImGui::UnregisterFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(atlas->RefCount > 0);
    ImFontAtlasRemoveDrawListSharedData(atlas, &g.DrawListSharedData);
    g.FontAtlases.find_erase(atlas);
    atlas->RefCount--;
    for (ImTextureData* tex : atlas->TexList)
        tex->RefCount = (unsigned short)atlas->RefCount;
}

// Use ImDrawList::_SetTexture(), making our shared g.FontStack[] authoritative against window-local ImDrawList.
// - Whereas ImDrawList::PushTexture()/PopTexture() is not to be used across Begin() calls.
// - Note that we don't propagate current texture id when e.g. Begin()-ing into a new window, we never really did...
//   - Some code paths never really fully worked with multiple atlas textures.
//   - The right-ish solution may be to remove _SetTexture() and make AddText/RenderText lazily call PushTexture()/PopTexture()
//     the same way AddImage() does, but then all other primitives would also need to? I don't think we should tackle this problem
//     because we have a concrete need and a test bed for multiple atlas textures.
// FIXME-NEWATLAS-V2: perhaps we can now leverage ImFontAtlasUpdateDrawListsTextures() ?
void ImGui::SetCurrentFont(ImFont* font, float font_size_before_scaling, float font_size_after_scaling)
{
    ImGuiContext& g = *GImGui;
    g.Font = font;
    g.FontSizeBase = font_size_before_scaling;
    UpdateCurrentFontSize(font_size_after_scaling);

    if (font != NULL)
    {
        IM_ASSERT(font && font->IsLoaded());    // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        IM_ASSERT(font->Scale > 0.0f);
#endif
        ImFontAtlas* atlas = font->OwnerAtlas;
        g.DrawListSharedData.FontAtlas = atlas;
        g.DrawListSharedData.Font = font;
        ImFontAtlasUpdateDrawListsSharedData(atlas);
        if (g.CurrentWindow != NULL)
            g.CurrentWindow->DrawList->_SetTexture(atlas->TexRef);
    }
}

void ImGui::UpdateCurrentFontSize(float restore_font_size_after_scaling)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    g.Style.FontSizeBase = g.FontSizeBase;

    // Restoring is pretty much only used by PopFont()
    float final_size = (restore_font_size_after_scaling > 0.0f) ? restore_font_size_after_scaling : 0.0f;
    if (final_size == 0.0f)
    {
        final_size = g.FontSizeBase;

        // Global scale factors
        final_size *= g.Style.FontScaleMain;    // Main global scale factor
        final_size *= g.Style.FontScaleDpi;     // Per-monitor/viewport DPI scale factor (in docking branch: automatically updated when io.ConfigDpiScaleFonts is enabled).

        // Window scale (mostly obsolete now)
        if (window != NULL)
            final_size *= window->FontWindowScale;

        // Legacy scale factors
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        final_size *= g.IO.FontGlobalScale; // Use style.FontScaleMain instead!
        if (g.Font != NULL)
            final_size *= g.Font->Scale;    // Was never really useful.
#endif
    }

    // Round font size
    // - We started rounding in 1.90 WIP (18991) as our layout system currently doesn't support non-rounded font size well yet.
    // - We may support it better later and remove this rounding.
    final_size = GetRoundedFontSize(final_size);
    final_size = ImClamp(final_size, 1.0f, IMGUI_FONT_SIZE_MAX);
    if (g.Font != NULL && (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures))
        g.Font->CurrentRasterizerDensity = g.FontRasterizerDensity;

    g.FontSize = final_size;
    g.DrawListSharedData.FontSize = g.FontSize;

    // Early out to avoid hidden window keeping bakes referenced and out of GC reach.
    // - However this leave a pretty subtle and damning error surface area if g.FontBaked was mismatching.
    //   Probably needs to be reevaluated into e.g. setting g.FontBaked = nullptr to mark it as dirty.
    // - Note that 'PushFont(); Begin(); End(); PopFont()' from within any collapsed window is not compromised, because Begin() calls SetCurrentWindow()->...->UpdateCurrentSize()
    if (window != NULL && window->SkipItems)
    {
        ImGuiTable* table = g.CurrentTable;
        const bool allow_early_out = table == NULL || (table->CurrentColumn != -1 && table->Columns[table->CurrentColumn].IsSkipItems == false); // See 8465#issuecomment-2951509561 and #8865. Ideally the SkipItems=true in tables would be amended with extra data.
        if (allow_early_out)
            return;
    }

    g.FontBaked = (g.Font != NULL && window != NULL) ? g.Font->GetFontBaked(final_size) : NULL;
    g.FontBakedScale = (g.FontBaked != NULL) ? (g.FontSize / g.FontBaked->Size) : 0.0f;
    g.DrawListSharedData.FontScale = g.FontBakedScale;
}

// Exposed in case user may want to override setting density.
// IMPORTANT: Begin()/End() is overriding density. Be considerate of this you change it.
void ImGui::SetFontRasterizerDensity(float rasterizer_density)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures);
    if (g.FontRasterizerDensity == rasterizer_density)
        return;
    g.FontRasterizerDensity = rasterizer_density;
    UpdateCurrentFontSize(0.0f);
}

// If you want to scale an existing font size! Read comments in imgui.h!
void ImGui::PushFont(ImFont* font, float font_size_base)
{
    ImGuiContext& g = *GImGui;
    if (font == NULL) // Before 1.92 (June 2025), PushFont(NULL) == PushFont(GetDefaultFont())
        font = g.Font;
    IM_ASSERT(font != NULL);
    IM_ASSERT(font_size_base >= 0.0f);

    g.FontStack.push_back({ g.Font, g.FontSizeBase, g.FontSize });
    if (font_size_base == 0.0f)
        font_size_base = g.FontSizeBase; // Keep current font size
    SetCurrentFont(font, font_size_base, 0.0f);
}

void  ImGui::PopFont()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.FontStack.Size > 0, "Calling PopFont() too many times!");
    ImFontStackData* font_stack_data = &g.FontStack.back();
    SetCurrentFont(font_stack_data->Font, font_stack_data->FontSizeBeforeScaling, font_stack_data->FontSizeAfterScaling);
    g.FontStack.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] ID STACK
//-----------------------------------------------------------------------------

// This is one of the very rare legacy case where we use ImGuiWindow methods,
// it should ideally be flattened at some point but it's been used a lots by widgets.
IM_MSVC_RUNTIME_CHECKS_OFF
ImGuiID ImGuiWindow::GetID(const char* str, const char* str_end)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashStr(str, str_end ? (str_end - str) : 0, seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_String, str, str_end);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetID(const void* ptr)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashData(&ptr, sizeof(void*), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_Pointer, ptr, NULL);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetID(int n)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashData(&n, sizeof(n), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_S32, (void*)(intptr_t)n, NULL);
#endif
    return id;
}

// This is only used in rare/specific situations to manufacture an ID out of nowhere.
// FIXME: Consider instead storing last non-zero ID + count of successive zero-ID, and combine those?
ImGuiID ImGuiWindow::GetIDFromPos(const ImVec2& p_abs)
{
    ImGuiID seed = IDStack.back();
    ImVec2 p_rel = ImGui::WindowPosAbsToRel(this, p_abs);
    ImGuiID id = ImHashData(&p_rel, sizeof(p_rel), seed);
    return id;
}

// "
ImGuiID ImGuiWindow::GetIDFromRectangle(const ImRect& r_abs)
{
    ImGuiID seed = IDStack.back();
    ImRect r_rel = ImGui::WindowRectAbsToRel(this, r_abs);
    ImGuiID id = ImHashData(&r_rel, sizeof(r_rel), seed);
    return id;
}

void ImGui::PushID(const char* str_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(str_id);
    window->IDStack.push_back(id);
}

void ImGui::PushID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(str_id_begin, str_id_end);
    window->IDStack.push_back(id);
}

void ImGui::PushID(const void* ptr_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(ptr_id);
    window->IDStack.push_back(id);
}

void ImGui::PushID(int int_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(int_id);
    window->IDStack.push_back(id);
}

// Push a given id value ignoring the ID stack as a seed.
void ImGui::PushOverrideID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_ID, NULL, NULL);
#endif
    window->IDStack.push_back(id);
}

// Helper to avoid a common series of PushOverrideID -> GetID() -> PopID() call
// (note that when using this pattern, ID Stack Tool will tend to not display the intermediate stack level.
//  for that to work we would need to do PushOverrideID() -> ItemAdd() -> PopID() which would alter widget code a little more)
ImGuiID ImGui::GetIDWithSeed(const char* str, const char* str_end, ImGuiID seed)
{
    ImGuiID id = ImHashStr(str, str_end ? (str_end - str) : 0, seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_String, str, str_end);
#endif
    return id;
}

ImGuiID ImGui::GetIDWithSeed(int n, ImGuiID seed)
{
    ImGuiID id = ImHashData(&n, sizeof(n), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_S32, (void*)(intptr_t)n, NULL);
#endif
    return id;
}

void ImGui::PopID()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    IM_ASSERT_USER_ERROR_RET(window->IDStack.Size > 1, "Calling PopID() too many times!");
    window->IDStack.pop_back();
}

ImGuiID ImGui::GetID(const char* str_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(str_id);
}

ImGuiID ImGui::GetID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(str_id_begin, str_id_end);
}

ImGuiID ImGui::GetID(const void* ptr_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(ptr_id);
}

ImGuiID ImGui::GetID(int int_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(int_id);
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] LOGGING/CAPTURING
//-----------------------------------------------------------------------------
// All text output from the interface can be captured into tty/file/clipboard.
// By default, tree nodes are automatically opened during logging.
//-----------------------------------------------------------------------------

// Pass text data straight to log (without being displayed)
static inline void LogTextV(ImGuiContext& g, const char* fmt, va_list args)
{
    if (g.LogFile)
    {
        g.LogBuffer.Buf.resize(0);
        g.LogBuffer.appendfv(fmt, args);
        ImFileWrite(g.LogBuffer.c_str(), sizeof(char), (ImU64)g.LogBuffer.size(), g.LogFile);
    }
    else
    {
        g.LogBuffer.appendfv(fmt, args);
    }
}

void ImGui::LogText(const char* fmt, ...)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    va_list args;
    va_start(args, fmt);
    LogTextV(g, fmt, args);
    va_end(args);
}

void ImGui::LogTextV(const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogTextV(g, fmt, args);
}

// Internal version that takes a position to decide on newline placement and pad items according to their depth.
// We split text into individual lines to add current tree level padding
// FIXME: This code is a little complicated perhaps, considering simplifying the whole system.
void ImGui::LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const char* prefix = g.LogNextPrefix;
    const char* suffix = g.LogNextSuffix;
    g.LogNextPrefix = g.LogNextSuffix = NULL;

    if (!text_end)
        text_end = FindRenderedTextEnd(text, text_end);

    const bool log_new_line = ref_pos && (ref_pos->y > g.LogLinePosY + ImMax(g.Style.FramePadding.y, g.Style.ItemSpacing.y) + 1);
    if (ref_pos)
        g.LogLinePosY = ref_pos->y;
    if (log_new_line)
    {
        LogText(IM_NEWLINE);
        g.LogLineFirstItem = true;
    }

    if (prefix)
        LogRenderedText(ref_pos, prefix, prefix + ImStrlen(prefix)); // Calculate end ourself to ensure "##" are included here.

    // Re-adjust padding if we have popped out of our starting depth
    if (g.LogDepthRef > window->DC.TreeDepth)
        g.LogDepthRef = window->DC.TreeDepth;
    const int tree_depth = (window->DC.TreeDepth - g.LogDepthRef);

    const char* text_remaining = text;
    for (;;)
    {
        // Split the string. Each new line (after a '\n') is followed by indentation corresponding to the current depth of our log entry.
        // We don't add a trailing \n yet to allow a subsequent item on the same line to be captured.
        const char* line_start = text_remaining;
        const char* line_end = ImStreolRange(line_start, text_end);
        const bool is_last_line = (line_end == text_end);
        if (line_start != line_end || !is_last_line)
        {
            const int line_length = (int)(line_end - line_start);
            const int indentation = g.LogLineFirstItem ? tree_depth * 4 : 1;
            LogText("%*s%.*s", indentation, "", line_length, line_start);
            g.LogLineFirstItem = false;
            if (*line_end == '\n')
            {
                LogText(IM_NEWLINE);
                g.LogLineFirstItem = true;
            }
        }
        if (is_last_line)
            break;
        text_remaining = line_end + 1;
    }

    if (suffix)
        LogRenderedText(ref_pos, suffix, suffix + ImStrlen(suffix));
}

// Start logging/capturing text output
void ImGui::LogBegin(ImGuiLogFlags flags, int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(g.LogEnabled == false);
    IM_ASSERT(g.LogFile == NULL && g.LogBuffer.empty());
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiLogFlags_OutputMask_)); // Check that only 1 type flag is used

    g.LogEnabled = g.ItemUnclipByLog = true;
    g.LogFlags = flags;
    g.LogWindow = window;
    g.LogNextPrefix = g.LogNextSuffix = NULL;
    g.LogDepthRef = window->DC.TreeDepth;
    g.LogDepthToExpand = ((auto_open_depth >= 0) ? auto_open_depth : g.LogDepthToExpandDefault);
    g.LogLinePosY = FLT_MAX;
    g.LogLineFirstItem = true;
}

// Important: doesn't copy underlying data, use carefully (prefix/suffix must be in scope at the time of the next LogRenderedText)
void ImGui::LogSetNextTextDecoration(const char* prefix, const char* suffix)
{
    ImGuiContext& g = *GImGui;
    g.LogNextPrefix = prefix;
    g.LogNextSuffix = suffix;
}

void ImGui::LogToTTY(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    IM_UNUSED(auto_open_depth);
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    LogBegin(ImGuiLogFlags_OutputTTY, auto_open_depth);
    g.LogFile = stdout;
#endif
}

// Start logging/capturing text output to given file
void ImGui::LogToFile(int auto_open_depth, const char* filename)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;

    // FIXME: We could probably open the file in text mode "at", however note that clipboard/buffer logging will still
    // be subject to outputting OS-incompatible carriage return if within strings the user doesn't use IM_NEWLINE.
    // By opening the file in binary mode "ab" we have consistent output everywhere.
    if (!filename)
        filename = g.IO.LogFilename;
    if (!filename || !filename[0])
        return;
    ImFileHandle f = ImFileOpen(filename, "ab");
    if (!f)
    {
        IM_ASSERT(0);
        return;
    }

    LogBegin(ImGuiLogFlags_OutputFile, auto_open_depth);
    g.LogFile = f;
}

// Start logging/capturing text output to clipboard
void ImGui::LogToClipboard(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    LogBegin(ImGuiLogFlags_OutputClipboard, auto_open_depth);
}

void ImGui::LogToBuffer(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    LogBegin(ImGuiLogFlags_OutputBuffer, auto_open_depth);
}

void ImGui::LogFinish()
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogText(IM_NEWLINE);
    switch (g.LogFlags & ImGuiLogFlags_OutputMask_)
    {
    case ImGuiLogFlags_OutputTTY:
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
        fflush(g.LogFile);
#endif
        break;
    case ImGuiLogFlags_OutputFile:
        ImFileClose(g.LogFile);
        break;
    case ImGuiLogFlags_OutputBuffer:
        break;
    case ImGuiLogFlags_OutputClipboard:
        if (!g.LogBuffer.empty())
            SetClipboardText(g.LogBuffer.begin());
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    g.LogEnabled = g.ItemUnclipByLog = false;
    g.LogFlags = ImGuiLogFlags_None;
    g.LogFile = NULL;
    g.LogBuffer.clear();
}

// Helper to display logging buttons
// FIXME-OBSOLETE: We should probably obsolete this and let the user have their own helper (this is one of the oldest function alive!)
void ImGui::LogButtons()
{
    ImGuiContext& g = *GImGui;

    PushID("LogButtons");
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    const bool log_to_tty = Button("Log To TTY"); SameLine();
#else
    const bool log_to_tty = false;
#endif
    const bool log_to_file = Button("Log To File"); SameLine();
    const bool log_to_clipboard = Button("Log To Clipboard"); SameLine();
    PushItemFlag(ImGuiItemFlags_NoTabStop, true);
    SetNextItemWidth(CalcTextSize("999").x);
    SliderInt("Default Depth", &g.LogDepthToExpandDefault, 0, 9, NULL);
    PopItemFlag();
    PopID();

    // Start logging at the end of the function so that the buttons don't appear in the log
    if (log_to_tty)
        LogToTTY();
    if (log_to_file)
        LogToFile();
    if (log_to_clipboard)
        LogToClipboard();
}

//-----------------------------------------------------------------------------
// [SECTION] SETTINGS
//-----------------------------------------------------------------------------
// - UpdateSettings() [Internal]
// - MarkIniSettingsDirty() [Internal]
// - FindSettingsHandler() [Internal]
// - ClearIniSettings() [Internal]
// - LoadIniSettingsFromDisk()
// - LoadIniSettingsFromMemory()
// - SaveIniSettingsToDisk()
// - SaveIniSettingsToMemory()
//-----------------------------------------------------------------------------
// - CreateNewWindowSettings() [Internal]
// - FindWindowSettingsByID() [Internal]
// - FindWindowSettingsByWindow() [Internal]
// - ClearWindowSettings() [Internal]
// - WindowSettingsHandler_***() [Internal]
//-----------------------------------------------------------------------------

// Called by NewFrame()
void ImGui::UpdateSettings()
{
    // Load settings on first frame (if not explicitly loaded manually before)
    ImGuiContext& g = *GImGui;
    if (!g.SettingsLoaded)
    {
        IM_ASSERT(g.SettingsWindows.empty());
        if (g.IO.IniFilename)
            LoadIniSettingsFromDisk(g.IO.IniFilename);
        g.SettingsLoaded = true;
    }

    // Save settings (with a delay after the last modification, so we don't spam disk too much)
    if (g.SettingsDirtyTimer > 0.0f)
    {
        g.SettingsDirtyTimer -= g.IO.DeltaTime;
        if (g.SettingsDirtyTimer <= 0.0f)
        {
            if (g.IO.IniFilename != NULL)
                SaveIniSettingsToDisk(g.IO.IniFilename);
            else
                g.IO.WantSaveIniSettings = true;  // Let user know they can call SaveIniSettingsToMemory(). user will need to clear io.WantSaveIniSettings themselves.
            g.SettingsDirtyTimer = 0.0f;
        }
    }
}

void ImGui::MarkIniSettingsDirty()
{
    ImGuiContext& g = *GImGui;
    if (g.SettingsDirtyTimer <= 0.0f)
        g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void ImGui::MarkIniSettingsDirty(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
        if (g.SettingsDirtyTimer <= 0.0f)
            g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void ImGui::AddSettingsHandler(const ImGuiSettingsHandler* handler)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(FindSettingsHandler(handler->TypeName) == NULL);
    g.SettingsHandlers.push_back(*handler);
}

void ImGui::RemoveSettingsHandler(const char* type_name)
{
    ImGuiContext& g = *GImGui;
    if (ImGuiSettingsHandler* handler = FindSettingsHandler(type_name))
        g.SettingsHandlers.erase(handler);
}

ImGuiSettingsHandler* ImGui::FindSettingsHandler(const char* type_name)
{
    ImGuiContext& g = *GImGui;
    const ImGuiID type_hash = ImHashStr(type_name);
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.TypeHash == type_hash)
            return &handler;
    return NULL;
}

// Clear all settings (windows, tables, docking etc.)
void ImGui::ClearIniSettings()
{
    ImGuiContext& g = *GImGui;
    g.SettingsIniData.clear();
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ClearAllFn != NULL)
            handler.ClearAllFn(&g, &handler);
}

void ImGui::LoadIniSettingsFromDisk(const char* ini_filename)
{
    size_t file_data_size = 0;
    IO::path target = ENGINE_CACHE / ini_filename;
    char* file_data = (char*)ImFileLoadToMemory(target.string().c_str(), "rb", &file_data_size);
    if (!file_data)
        return;
    if (file_data_size > 0)
        LoadIniSettingsFromMemory(file_data, (size_t)file_data_size);
    IM_FREE(file_data);
}

// Zero-tolerance, no error reporting, cheap .ini parsing
// Set ini_size==0 to let us use strlen(ini_data). Do not call this function with a 0 if your buffer is actually empty!
void ImGui::LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);
    //IM_ASSERT(!g.WithinFrameScope && "Cannot be called between NewFrame() and EndFrame()");
    //IM_ASSERT(g.SettingsLoaded == false && g.FrameCount == 0);

    // For user convenience, we allow passing a non zero-terminated string (hence the ini_size parameter).
    // For our convenience and to make the code simpler, we'll also write zero-terminators within the buffer. So let's create a writable copy..
    if (ini_size == 0)
        ini_size = ImStrlen(ini_data);
    g.SettingsIniData.Buf.resize((int)ini_size + 1);
    char* const buf = g.SettingsIniData.Buf.Data;
    char* const buf_end = buf + ini_size;
    memcpy(buf, ini_data, ini_size);
    buf_end[0] = 0;

    // Call pre-read handlers
    // Some types will clear their data (e.g. dock information) some types will allow merge/override (window)
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ReadInitFn != NULL)
            handler.ReadInitFn(&g, &handler);

    void* entry_data = NULL;
    ImGuiSettingsHandler* entry_handler = NULL;

    char* line_end = NULL;
    for (char* line = buf; line < buf_end; line = line_end + 1)
    {
        // Skip new lines markers, then find end of the line
        while (*line == '\n' || *line == '\r')
            line++;
        line_end = line;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
            line_end++;
        line_end[0] = 0;
        if (line[0] == ';')
            continue;
        if (line[0] == '[' && line_end > line && line_end[-1] == ']')
        {
            // Parse "[Type][Name]". Note that 'Name' can itself contains [] characters, which is acceptable with the current format and parsing code.
            line_end[-1] = 0;
            const char* name_end = line_end - 1;
            const char* type_start = line + 1;
            char* type_end = (char*)(void*)ImStrchrRange(type_start, name_end, ']');
            const char* name_start = type_end ? ImStrchrRange(type_end + 1, name_end, '[') : NULL;
            if (!type_end || !name_start)
                continue;
            *type_end = 0; // Overwrite first ']'
            name_start++;  // Skip second '['
            entry_handler = FindSettingsHandler(type_start);
            entry_data = entry_handler ? entry_handler->ReadOpenFn(&g, entry_handler, name_start) : NULL;
        }
        else if (entry_handler != NULL && entry_data != NULL)
        {
            // Let type handler parse the line
            entry_handler->ReadLineFn(&g, entry_handler, entry_data, line);
        }
    }
    g.SettingsLoaded = true;

    // [DEBUG] Restore untouched copy so it can be browsed in Metrics (not strictly necessary)
    memcpy(buf, ini_data, ini_size);

    // Call post-read handlers
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ApplyAllFn != NULL)
            handler.ApplyAllFn(&g, &handler);
}

void ImGui::SaveIniSettingsToDisk(const char* ini_filename)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    if (!ini_filename)
        return;

    size_t ini_data_size = 0;
    const char* ini_data = SaveIniSettingsToMemory(&ini_data_size);
    IO::path target = ENGINE_CACHE / ini_filename;
    // use Program Path
    ImFileHandle f = ImFileOpen(target.string().c_str(), "wt");
    if (!f)
        return;
    ImFileWrite(ini_data, sizeof(char), ini_data_size, f);
    ImFileClose(f);
}

// Call registered handlers (e.g. SettingsHandlerWindow_WriteAll() + custom handlers) to write their stuff into a text buffer
const char* ImGui::SaveIniSettingsToMemory(size_t* out_size)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    g.SettingsIniData.Buf.resize(0);
    g.SettingsIniData.Buf.push_back(0);
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        handler.WriteAllFn(&g, &handler, &g.SettingsIniData);
    if (out_size)
        *out_size = (size_t)g.SettingsIniData.size();
    return g.SettingsIniData.c_str();
}

ImGuiWindowSettings* ImGui::CreateNewWindowSettings(const char* name)
{
    ImGuiContext& g = *GImGui;

    // Preserve the full string when ConfigDebugVerboseIniSettings is set to make .ini inspection easier.
    if (g.IO.ConfigDebugIniSettings == false)
        name = ImHashSkipUncontributingPrefix(name);
    const size_t name_len = ImStrlen(name);

    // Allocate chunk
    const size_t chunk_size = sizeof(ImGuiWindowSettings) + name_len + 1;
    ImGuiWindowSettings* settings = g.SettingsWindows.alloc_chunk(chunk_size);
    IM_PLACEMENT_NEW(settings) ImGuiWindowSettings();
    settings->ID = ImHashStr(name, name_len);
    memcpy(settings->GetName(), name, name_len + 1);   // Store with zero terminator

    return settings;
}

// We don't provide a FindWindowSettingsByName() because Docking system doesn't always hold on names.
// This is called once per window .ini entry + once per newly instantiated window.
ImGuiWindowSettings* ImGui::FindWindowSettingsByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->ID == id && !settings->WantDelete)
            return settings;
    return NULL;
}

// This is faster if you are holding on a Window already as we don't need to perform a search.
ImGuiWindowSettings* ImGui::FindWindowSettingsByWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (window->SettingsOffset != -1)
        return g.SettingsWindows.ptr_from_offset(window->SettingsOffset);
    return FindWindowSettingsByID(window->ID);
}

// This will revert window to its initial state, including enabling the ImGuiCond_FirstUseEver/ImGuiCond_Once conditions once more.
void ImGui::ClearWindowSettings(const char* name)
{
    //IMGUI_DEBUG_LOG("ClearWindowSettings('%s')\n", name);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = FindWindowByName(name);
    if (window != NULL)
    {
        window->Flags |= ImGuiWindowFlags_NoSavedSettings;
        InitOrLoadWindowSettings(window, NULL);
        if (window->DockId != 0)
            DockContextProcessUndockWindow(&g, window, true);
    }
    if (ImGuiWindowSettings* settings = window ? FindWindowSettingsByWindow(window) : FindWindowSettingsByID(ImHashStr(name)))
        settings->WantDelete = true;
}

static void WindowSettingsHandler_ClearAll(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
    ImGuiContext& g = *ctx;
    for (ImGuiWindow* window : g.Windows)
        window->SettingsOffset = -1;
    g.SettingsWindows.clear();
}

static void* WindowSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
{
    ImGuiID id = ImHashStr(name);
    ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByID(id);
    if (settings)
        *settings = ImGuiWindowSettings(); // Clear existing if recycling previous entry
    else
        settings = ImGui::CreateNewWindowSettings(name);
    settings->ID = id;
    settings->WantApply = true;
    return (void*)settings;
}

static void WindowSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
    ImGuiWindowSettings* settings = (ImGuiWindowSettings*)entry;
    int x, y;
    int i;
    ImU32 u1;
    if (sscanf(line, "Pos=%i,%i", &x, &y) == 2)             { settings->Pos = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "Size=%i,%i", &x, &y) == 2)       { settings->Size = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "ViewportId=0x%08X", &u1) == 1)   { settings->ViewportId = u1; }
    else if (sscanf(line, "ViewportPos=%i,%i", &x, &y) == 2){ settings->ViewportPos = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "Collapsed=%d", &i) == 1)         { settings->Collapsed = (i != 0); }
    else if (sscanf(line, "IsChild=%d", &i) == 1)           { settings->IsChild = (i != 0); }
    else if (sscanf(line, "DockId=0x%X,%d", &u1, &i) == 2)  { settings->DockId = u1; settings->DockOrder = (short)i; }
    else if (sscanf(line, "DockId=0x%X", &u1) == 1)         { settings->DockId = u1; settings->DockOrder = -1; }
    else if (sscanf(line, "ClassId=0x%X", &u1) == 1)        { settings->ClassId = u1; }
}

// Apply to existing windows (if any)
static void WindowSettingsHandler_ApplyAll(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
    ImGuiContext& g = *ctx;
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->WantApply)
        {
            if (ImGuiWindow* window = ImGui::FindWindowByID(settings->ID))
                ImGui::ApplyWindowSettings(window, settings);
            settings->WantApply = false;
        }
}

static void WindowSettingsHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
    // Gather data from windows that were active during this session
    // (if a window wasn't opened in this session we preserve its settings)
    ImGuiContext& g = *ctx;
    for (ImGuiWindow* window : g.Windows)
    {
        if (window->Flags & ImGuiWindowFlags_NoSavedSettings)
            continue;

        ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByWindow(window);
        if (!settings)
        {
            settings = ImGui::CreateNewWindowSettings(window->Name);
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
        }
        IM_ASSERT(settings->ID == window->ID);
        settings->Pos = ImVec2ih(window->Pos - window->ViewportPos);
        settings->Size = ImVec2ih(window->SizeFull);
        settings->ViewportId = window->ViewportId;
        settings->ViewportPos = ImVec2ih(window->ViewportPos);
        IM_ASSERT(window->DockNode == NULL || window->DockNode->ID == window->DockId);
        settings->DockId = window->DockId;
        settings->ClassId = window->WindowClass.ClassId;
        settings->DockOrder = window->DockOrder;
        settings->Collapsed = window->Collapsed;
        settings->IsChild = (window->RootWindow != window); // Cannot rely on ImGuiWindowFlags_ChildWindow here as docked windows have this set.
        settings->WantDelete = false;
    }

    // Write to text buffer
    buf->reserve(buf->size() + g.SettingsWindows.size() * 6); // ballpark reserve
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
    {
        if (settings->WantDelete)
            continue;
        const char* settings_name = settings->GetName();
        buf->appendf("[%s][%s]\n", handler->TypeName, settings_name);
        if (settings->IsChild)
        {
            buf->appendf("IsChild=1\n");
            buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
        }
        else
        {
            if (settings->ViewportId != 0 && settings->ViewportId != ImGui::IMGUI_VIEWPORT_DEFAULT_ID)
            {
                buf->appendf("ViewportPos=%d,%d\n", settings->ViewportPos.x, settings->ViewportPos.y);
                buf->appendf("ViewportId=0x%08X\n", settings->ViewportId);
            }
            if (settings->Pos.x != 0 || settings->Pos.y != 0 || settings->ViewportId == ImGui::IMGUI_VIEWPORT_DEFAULT_ID)
                buf->appendf("Pos=%d,%d\n", settings->Pos.x, settings->Pos.y);
            if (settings->Size.x != 0 || settings->Size.y != 0)
                buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
            buf->appendf("Collapsed=%d\n", settings->Collapsed);
            if (settings->DockId != 0)
            {
                //buf->appendf("TabId=0x%08X\n", ImHashStr("#TAB", 4, settings->ID)); // window->TabId: this is not read back but writing it makes "debugging" the .ini data easier.
                if (settings->DockOrder == -1)
                    buf->appendf("DockId=0x%08X\n", settings->DockId);
                else
                    buf->appendf("DockId=0x%08X,%d\n", settings->DockId, settings->DockOrder);
                if (settings->ClassId != 0)
                    buf->appendf("ClassId=0x%08X\n", settings->ClassId);
            }
        }
        buf->append("\n");
    }
}

//-----------------------------------------------------------------------------
// [SECTION] LOCALIZATION
//-----------------------------------------------------------------------------

void ImGui::LocalizeRegisterEntries(const ImGuiLocEntry* entries, int count)
{
    ImGuiContext& g = *GImGui;
    for (int n = 0; n < count; n++)
        g.LocalizationTable[entries[n].Key] = entries[n].Text;
}

//-----------------------------------------------------------------------------

// Include imgui_user.inl at the end of imgui.cpp to access private data/functions that aren't exposed.
// Prefer just including imgui_internal.h from your code rather than using this define. If a declaration is missing from imgui_internal.h add it or request it on the github.
#ifdef IMGUI_INCLUDE_IMGUI_USER_INL
#include "imgui_user.inl"
#endif

//-----------------------------------------------------------------------------

#endif // #ifndef IMGUI_DISABLE
