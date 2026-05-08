#include "Main.h"
#include <Logo/resource.h>
#include <Window/WindowAppointment.hpp>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t* CLASS = PROJECT_CLASS; // 使用约定名称
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // 初始化 COM
    ProjectList::Get().init();

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
    while (true) {
        // 有消息处理消息，没消息执行 Tick
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                goto exit_loop;  // 或 return
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 空闲时执行 Tick
        Tick();  // 你的逻辑
    }
    exit_loop:

    ProjectList::Get().close();
    CoUninitialize();
    return 0;
}
