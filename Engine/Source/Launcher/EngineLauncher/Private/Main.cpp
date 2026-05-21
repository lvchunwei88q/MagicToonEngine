
#include "Main.h"
#include <Logo/resource.h>
#include <Window/WindowAppointment.hpp>
#include <Subsystem/Subsystem.h>
#include <Win32Application.h>

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

    EngineLauncher::WindowsContext::Get().hWnd = CreateWindowW(CLASS, L"Engine Launcher",
        WS_POPUP,
        x, y, winW, winH,
        nullptr, nullptr, hInstance, nullptr);

    HWND hwnd = EngineLauncher::WindowsContext::Get().hWnd;
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    if (hIcon)
    {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);   // 大图标（任务栏）
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);   // 小图标（标题栏）
    }
    
    Core::SubsystemError error = Core::SubsystemControl::Init();// 初始化子系统
    if (!error.error) {
        //每个进程有独立的虚拟地址空间，泄漏只影响自己的进程，不会污染其他程序或系统 所以不需要释放了
        std::stringstream ss;
        ss << "There was a problem during engine initialization!\n";
        ss << "Problem Subsystem: " << error.target << "\n";
        OutputDebugStringA(ss.str().c_str());
        return 0;
    }

    EngineLauncher::Win32Application::Get().InitializeUI(); // UI初始化
    ShowWindow(hwnd, SW_SHOW);


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

	EngineLauncher::Win32Application::Get().EndUI(); // 结束UI
	Core::SubsystemControl::Uninstall(); // 卸载子系统
    CoUninitialize();
    return 0;
}

namespace EngineLauncher {
    void Tick()
    {
		Win32Application::Get().Tick();
    }
}