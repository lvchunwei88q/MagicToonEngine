#include <EditorWindows.h>
#include <dwmapi.h>

#include <WindowsConfig.h>
#include <RenderInterface.h>

#include <AbsolutePath.h>
#include <FileManager.h>
#include <fstream>
#include <Serialize/SerializeMacro.h>

namespace EditorWindows
{
	void EditorWindows::CreateWindows()
	{
		WindowsConfig& config = WindowsConfig::Get(); // Create a default configuration. You can load this from a file if needed.

		FILE_SERIALIZATION_LOADING(config, CACHE "Editor\\Windows\\", L"WindowsConfig.mtdata")

		// init windows
		WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WindowsProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
			L"MagicEditorClass", nullptr };
		wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		::RegisterClassEx(&wc);

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
	}

	void EditorWindows::CreateDeviceD3D()
	{
		RenderCore::GetRenderInterface()->CreateDeviceD3D(WindowsContext::Get().hWnd);
	}

	void EditorWindows::CleanupWindows()
	{
		WindowsConfig& config = WindowsConfig::Get(); // Create a default configuration. You can load this from a file if needed.
		FILE_SERIALIZATION_SAVE(config, CACHE "Editor\\Windows\\", L"WindowsConfig.mtdata")
	}

	void EditorWindows::CleanupDeviceD3D()
	{
		RenderCore::GetRenderInterface()->CleanupDeviceD3D();
	}
}