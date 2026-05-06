#include "Main.h"
#include <AbsolutePath.h>

#include <WebView2/WebView2.h>

#include <wrl/client.h> 
#include <wrl/event.h>
#include <string>

using namespace Microsoft::WRL;

HWND g_hwnd = nullptr;
ComPtr<ICoreWebView2Controller> g_controller;
ComPtr<ICoreWebView2> g_webview;
CallbackFunc WebViewSuccess = SHOW_WINDOW;

void init_app(HWND hwnd) {
    g_hwnd = hwnd;
    std::wstring userDataFolder = IO::AbsolutePath::Get().GetExecutableDirectory() + L"\\" + CACHE + L"WebViewCache";

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, userDataFolder.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result)) return S_OK;

                env->CreateCoreWebView2Controller(
                    g_hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(result)) return S_OK;

                            g_controller = controller;
                            g_controller->get_CoreWebView2(&g_webview);

                            // 设置 WebView 大小
                            RECT bounds = { 0, 0, winW, winH };
                            g_controller->put_Bounds(bounds);

                            std::wstring src = IO::AbsolutePath::Get().GetContentPath() + L"\\ProjectLauncher\\index.html";
                            g_webview->Navigate(src.c_str());

#ifndef _DEBUG
                            // 禁用右键菜单和开发者工具
                            ComPtr<ICoreWebView2Settings> settings;
                            g_webview->get_Settings(&settings);
                            if (settings) {
                                settings->put_AreDefaultContextMenusEnabled(FALSE);
                                settings->put_AreDevToolsEnabled(FALSE);
                                settings->put_IsZoomControlEnabled(FALSE);
                            }
#endif

                            g_webview->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        WebViewSuccess();
                                        return S_OK;
                                    }).Get(),nullptr);

                            g_webview->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        LPWSTR rawMsg = nullptr;
                                        args->TryGetWebMessageAsString(&rawMsg);
                                        if (!rawMsg) return S_OK;

                                        std::wstring msg(rawMsg);
                                        CoTaskMemFree(rawMsg);

                                        JSON json = JSON::parse(msg);
                                        Operation(json);

                                        return S_OK;
                                    }).Get(),nullptr);

                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

void SHOW_WINDOW()
{
    ShowWindow(g_hwnd, SW_SHOWNORMAL);  // 激活并显示，恢复原有大小
    SetForegroundWindow(g_hwnd);
    UpdateWindow(g_hwnd);
}