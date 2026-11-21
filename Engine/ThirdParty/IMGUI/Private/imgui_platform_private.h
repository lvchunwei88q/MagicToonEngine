#pragma once

// [Windows] On non-Visual Studio compilers, we default to IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS unless explicitly enabled
#if defined(_WIN32) && !defined(_MSC_VER) && !defined(IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif

// [Windows] OS specific includes (optional)
#if defined(_WIN32) && defined(IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS) && defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS) && defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS) && defined(IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#endif
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef __MINGW32__
#include <Windows.h>        // _wfopen, OpenClipboard
#else
#include <windows.h>
#endif
#if defined(WINAPI_FAMILY) && ((defined(WINAPI_FAMILY_APP) && WINAPI_FAMILY == WINAPI_FAMILY_APP) || (defined(WINAPI_FAMILY_GAMES) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES))
// The UWP and GDK Win32 API subsets don't support clipboard nor IME functions
#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#endif

// [Apple] OS specific includes
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace ImGui
{
// Platform Dependents default implementation for ImGuiPlatformIO functions
    const char* Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx);
    void Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext* ctx, const char* text);
    void Platform_SetImeDataFn_DefaultImpl(ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data);
    bool Platform_OpenInShellFn_DefaultImpl(ImGuiContext* ctx, const char* path);
}
