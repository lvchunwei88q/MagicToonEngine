#include "Main.h"
#include <Logo/resource.h>

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
    int x = (screenW - winW) / 2;
    int y = (screenH - winH) / 2;

    HWND hwnd = CreateWindowW(CLASS, L"Project Launcher",
        WS_POPUP,
        x, y, winW, winH,
        nullptr, nullptr, hInstance, nullptr);
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    if (hIcon)
    {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);   // 大图标（任务栏）
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);   // 小图标（标题栏）
    }
    ShowWindow(hwnd, SW_SHOWMINIMIZED);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return 0;
}