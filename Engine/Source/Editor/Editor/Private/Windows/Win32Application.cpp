#include "Win32Application.h"
// windows配置
#include "WindowsConfig.h"
#include <ILog.h>

#include <IBufferManager.h> // 内存管理器
#include <IRender.h> // 渲染接口
#include <IRenderRT.h>

// 序列化文件相关
#include <AbsolutePath.h>
#include <FileManager.h>
#include <fstream>
#include <Serialize/SerializeMacro.h>

#include <Window/WindowAppointment.hpp> // 约定

// 主题文件
#include <dwmapi.h>

namespace Editor
{
	AUTO_REGISTER_SINGLETON(Win32Application, High);

	Win32Application::Win32Application()
	{
		// not
	}

	bool Win32Application::Init()
	{
		LOG_INFO("Editor subsystem created.");

		WindowsConfig& config = WindowsConfig::Get(); // Create a default configuration. You can load this from a file if needed.
		FILE_SERIALIZATION_LOADING(config, CONFIG "Editor\\Windows\\", L"WindowsConfig.mtdata")

		// Initialize the editor subsystem, e.g. create windows, initialize resources, etc.
		LOG_INFO("Editor Create Windows.");
		CreateWindows();
		LOG_INFO("Editor Create DeviceD3D.");
		CreateDeviceD3D();

		// init buffer Manager
		LOG_INFO("Init Buffer Manager.");
		RenderCore::IBufferManagerAdmin* IBMAdmin = RenderCore::GetBufferManagerAdminInterface();
		RenderCore::ViewContext context;
		context.ScreenSize = XMINT2(config.width, config.height);
		IBMAdmin->Initialize(context);

		return true;
	}

	void Win32Application::Uninstall()
	{
		CleanupWindows();
	}

	void Win32Application::CreateWindows()
	{
		WindowsConfig& config = WindowsConfig::Get();

		// init windows
		WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WindowsProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
				ENGINE_CLASS, nullptr }; // 使用约定名称
		wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		::RegisterClassEx(&wc);

		Core::SubsystemControl::NotificationSubsystem("IMGUI", { 3,nullptr }); // Set Imgui Dpi Scale
		HWND hwnd = CreateWindowEx(
			0, wc.lpszClassName, L"Magic Editor",
			WS_OVERLAPPEDWINDOW,
			config.windowsX, config.windowsY, config.width, config.height,
			nullptr, nullptr,
			wc.hInstance,
			nullptr
		);
		BOOL useDarkMode = TRUE;
		DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));

		// 强制重绘标题栏
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		WindowsContext::Get().hWnd = hwnd; // Store the window handle in the WindowsContext singleton

		char buffer[sizeof(HWND)];
		memcpy(buffer, &hwnd, sizeof(HWND));
		Core::SubsystemControl::NotificationSubsystem("IMGUI", { 2,buffer });
	}

	void Win32Application::CreateDeviceD3D()
	{
		RenderCore::GetRenderInterface()->CreateDeviceD3D(WindowsContext::Get().hWnd);
	}

	void Win32Application::CleanupWindows()
	{
		WindowsConfig& config = WindowsConfig::Get(); // Create a default configuration. You can load this from a file if needed.

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		if (config.windowsX > screenWidth || config.windowsY > screenHeight ||
			config.windowsX < 0 || config.windowsY < 0)
		{
			// 窗口位置不正常
			LOG_WARNING("The window has been minimized and will use the default position");
			config.windowsX = 100;
			config.windowsY = 100;
		}
		FILE_SERIALIZATION_SAVE(config, CONFIG "Editor\\Windows\\", L"WindowsConfig.mtdata")
	}

	void Win32Application::CleanupDeviceD3D()
	{
		RenderCore::GetRenderInterface()->CleanupDeviceD3D();
	}

	void Win32Application::InitializeUI()
	{
		editorUI.Init(); // 编辑器UI设置 - 同步Imgui上下文
	}

	void Win32Application::Tick() {
		RenderRT::GetRenderRTInterface()->SetRenderTarget();

		editorUI.Tick(); // Tick Engine UI

		// Present
		RenderRT::GetRenderRTInterface()->BindRenderTarget();
		//RenderCore::GetRenderInterface()->DebugD3D11State();
		RenderCore::GetRenderInterface()->Present();
	}

	void Win32Application::EndUI()
	{
		editorUI.Shutdown();
	}

	// 注册窗口改变时渲染回调函数
	void Win32Application::RegisterWindowUpdateRenderCallbackFunction(WindowUpdateRenderingFunc func) {
		this->WUR = func;
	}

	Win32Application::WindowUpdateRenderingFunc Win32Application::GetWindowUpdateRenderFunction()
	{
		return this->WUR;
	}
}