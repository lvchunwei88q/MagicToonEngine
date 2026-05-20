
#include "Main.h"
#include <Logo/resource.h>
#include <Window/WindowAppointment.hpp>
#include <Subsystem/Subsystem.h>

HWND EngineLauncher::g_hwnd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t* CLASS = PROJECT_CLASS; // 使用约定名称
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // 初始化 COM

    WNDCLASSW wc = {};
    wc.lpfnWndProc = EngineLauncher::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - winW) / 2;
    int y = (screenH - winH) / 2;

    EngineLauncher::g_hwnd = CreateWindowW(CLASS, L"Engine Launcher",
        WS_POPUP,
        x, y, winW, winH,
        nullptr, nullptr, hInstance, nullptr);
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    if (hIcon)
    {
        SendMessage(EngineLauncher::g_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);   // 大图标（任务栏）
        SendMessage(EngineLauncher::g_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);   // 小图标（标题栏）
    }
    
    EngineLauncher::Init();
    Core::SubsystemControl::Init(); // 初始化子系统
    ShowWindow(EngineLauncher::g_hwnd, SW_SHOWMINIMIZED);

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

        EngineLauncher::Tick();
    }
    exit_loop:

	Core::SubsystemControl::Uninstall(); // 卸载子系统
    CoUninitialize();
    return 0;
}
