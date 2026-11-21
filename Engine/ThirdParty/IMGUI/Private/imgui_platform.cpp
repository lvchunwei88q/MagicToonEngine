// Dear ImGui implementation - see LICENSE.txt.
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_platform_private.h"

//-----------------------------------------------------------------------------
// [SECTION] PLATFORM DEPENDENT HELPERS
//-----------------------------------------------------------------------------
// - Default clipboard handlers
// - Default shell function handlers
// - Default IME handlers
//-----------------------------------------------------------------------------

#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS)

#ifdef _MSC_VER
#pragma comment(lib, "user32")
#pragma comment(lib, "kernel32")
#endif

// Win32 clipboard implementation
// We use g.ClipboardHandlerData for temporary storage to ensure it is freed on Shutdown()
const char* ImGui::Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx)
{
    ImGuiContext& g = *ctx;
    g.ClipboardHandlerData.clear();
    if (!::OpenClipboard(NULL))
        return NULL;
    HANDLE wbuf_handle = ::GetClipboardData(CF_UNICODETEXT);
    if (wbuf_handle == NULL)
    {
        ::CloseClipboard();
        return NULL;
    }
    if (const WCHAR* wbuf_global = (const WCHAR*)::GlobalLock(wbuf_handle))
    {
        int buf_len = ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, NULL, 0, NULL, NULL);
        g.ClipboardHandlerData.resize(buf_len);
        ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, g.ClipboardHandlerData.Data, buf_len, NULL, NULL);
    }
    ::GlobalUnlock(wbuf_handle);
    ::CloseClipboard();
    return g.ClipboardHandlerData.Data;
}

void ImGui::Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext*, const char* text)
{
    if (!::OpenClipboard(NULL))
        return;
    const int wbuf_length = ::MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    HGLOBAL wbuf_handle = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(WCHAR));
    if (wbuf_handle == NULL)
    {
        ::CloseClipboard();
        return;
    }
    WCHAR* wbuf_global = (WCHAR*)::GlobalLock(wbuf_handle);
    ::MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf_global, wbuf_length);
    ::GlobalUnlock(wbuf_handle);
    ::EmptyClipboard();
    if (::SetClipboardData(CF_UNICODETEXT, wbuf_handle) == NULL)
        ::GlobalFree(wbuf_handle);
    ::CloseClipboard();
}

#elif defined(__APPLE__) && TARGET_OS_OSX && defined(IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS)

#include <Carbon/Carbon.h>  // Use old API to avoid need for separate .mm file
static PasteboardRef main_clipboard = 0;

// OSX clipboard implementation
// If you enable this you will need to add '-framework ApplicationServices' to your linker command-line!
void ImGui::Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext*, const char* text)
{
    if (!main_clipboard)
        PasteboardCreate(kPasteboardClipboard, &main_clipboard);
    PasteboardClear(main_clipboard);
    CFDataRef cf_data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)text, ImStrlen(text));
    if (cf_data)
    {
        PasteboardPutItemFlavor(main_clipboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), cf_data, 0);
        CFRelease(cf_data);
    }
}

const char* ImGui::Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx)
{
    ImGuiContext& g = *ctx;
    if (!main_clipboard)
        PasteboardCreate(kPasteboardClipboard, &main_clipboard);
    PasteboardSynchronize(main_clipboard);

    ItemCount item_count = 0;
    PasteboardGetItemCount(main_clipboard, &item_count);
    for (ItemCount i = 0; i < item_count; i++)
    {
        PasteboardItemID item_id = 0;
        PasteboardGetItemIdentifier(main_clipboard, i + 1, &item_id);
        CFArrayRef flavor_type_array = 0;
        PasteboardCopyItemFlavors(main_clipboard, item_id, &flavor_type_array);
        for (CFIndex j = 0, nj = CFArrayGetCount(flavor_type_array); j < nj; j++)
        {
            CFDataRef cf_data;
            if (PasteboardCopyItemFlavorData(main_clipboard, item_id, CFSTR("public.utf8-plain-text"), &cf_data) == noErr)
            {
                g.ClipboardHandlerData.clear();
                int length = (int)CFDataGetLength(cf_data);
                g.ClipboardHandlerData.resize(length + 1);
                CFDataGetBytes(cf_data, CFRangeMake(0, length), (UInt8*)g.ClipboardHandlerData.Data);
                g.ClipboardHandlerData[length] = 0;
                CFRelease(cf_data);
                return g.ClipboardHandlerData.Data;
            }
        }
    }
    return NULL;
}

#else

// Local Dear ImGui-only clipboard implementation, if user hasn't defined better clipboard handlers.
const char* ImGui::Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx)
{
    ImGuiContext& g = *ctx;
    return g.ClipboardHandlerData.empty() ? NULL : g.ClipboardHandlerData.begin();
}

void ImGui::Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext* ctx, const char* text)
{
    ImGuiContext& g = *ctx;
    g.ClipboardHandlerData.clear();
    const char* text_end = text + ImStrlen(text);
    g.ClipboardHandlerData.resize((int)(text_end - text) + 1);
    memcpy(&g.ClipboardHandlerData[0], text, (size_t)(text_end - text));
    g.ClipboardHandlerData[(int)(text_end - text)] = 0;
}

#endif // Default clipboard handlers

//-----------------------------------------------------------------------------

#ifndef IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#if defined(__APPLE__) && TARGET_OS_IPHONE
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#if defined(__3DS__)
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#if defined(_WIN32) && defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#endif // #ifndef IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS

#ifndef IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#ifdef _WIN32
#include <shellapi.h>   // ShellExecuteA()
#ifdef _MSC_VER
#pragma comment(lib, "shell32")
#endif
bool ImGui::Platform_OpenInShellFn_DefaultImpl(ImGuiContext*, const char* path)
{
    const int path_wsize = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    ImVector<wchar_t> path_wbuf;
    path_wbuf.resize(path_wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, path, -1, path_wbuf.Data, path_wsize);
    return (INT_PTR)::ShellExecuteW(NULL, L"open", path_wbuf.Data, NULL, NULL, SW_SHOWDEFAULT) > 32;
}
#else
#include <sys/wait.h>
#include <unistd.h>
bool ImGui::Platform_OpenInShellFn_DefaultImpl(ImGuiContext*, const char* path)
{
#if defined(__APPLE__)
    const char* args[] { "open", "--", path, NULL };
#else
    const char* args[] { "xdg-open", path, NULL };
#endif
    pid_t pid = fork();
    if (pid < 0)
        return false;
    if (!pid)
    {
        execvp(args[0], const_cast<char **>(args));
        exit(-1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status) == 0;
    }
}
#endif
#else
bool ImGui::Platform_OpenInShellFn_DefaultImpl(ImGuiContext*, const char*) { return false; }
#endif // Default shell handlers

//-----------------------------------------------------------------------------

// Win32 API IME support (for Asian languages, etc.)
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)

#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

void ImGui::Platform_SetImeDataFn_DefaultImpl(ImGuiContext*, ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{
    // Notify OS Input Method Editor of text input position
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;
    if (hwnd == 0)
        return;

    //::ImmAssociateContextEx(hwnd, NULL, data->WantVisible ? IACE_DEFAULT : 0);
    if (HIMC himc = ::ImmGetContext(hwnd))
    {
        COMPOSITIONFORM composition_form = {};
        composition_form.ptCurrentPos.x = (LONG)(data->InputPos.x - viewport->Pos.x);
        composition_form.ptCurrentPos.y = (LONG)(data->InputPos.y - viewport->Pos.y);
        composition_form.dwStyle = CFS_FORCE_POSITION;
        ::ImmSetCompositionWindow(himc, &composition_form);
        CANDIDATEFORM candidate_form = {};
        candidate_form.dwStyle = CFS_CANDIDATEPOS;
        candidate_form.ptCurrentPos.x = (LONG)(data->InputPos.x - viewport->Pos.x);
        candidate_form.ptCurrentPos.y = (LONG)(data->InputPos.y - viewport->Pos.y);
        ::ImmSetCandidateWindow(himc, &candidate_form);
        ::ImmReleaseContext(hwnd, himc);
    }
}

#else

void ImGui::Platform_SetImeDataFn_DefaultImpl(ImGuiContext*, ImGuiViewport*, ImGuiPlatformImeData*) {}

#endif // Default IME handlers

#endif // #ifndef IMGUI_DISABLE
