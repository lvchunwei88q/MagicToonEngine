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
                            RECT bounds;
                            GetClientRect(g_hwnd, &bounds);
                            g_controller->put_Bounds(bounds);

                            g_webview->Navigate(L"https://cn.bing.com/");
                            Showindow();
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

void Showindow()
{
    ShowWindow(g_hwnd, SW_SHOW);
}
