#include "Main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t* CLASS = L"Project Launcher";
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // 初始化 COM

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 960, winH = 540;
    int x = (screenW - winW) / 2;
    int y = (screenH - winH) / 2;

    HWND hwnd = CreateWindowW(CLASS, L"Project Launcher",
        WS_POPUP,
        x, y, winW, winH,
        nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOWMINIMIZED);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return 0;
}