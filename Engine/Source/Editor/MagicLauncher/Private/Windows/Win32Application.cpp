#include "Main.h"
#include "Win32Application.h"
#include <IRender.h>
#include <IRenderRT.h>

namespace MagicLauncher
{
	AUTO_REGISTER_NOTIFICATION(Win32Application,"Application");

	bool Win32Application::Init() {
		// 设置窗口句柄
		char buffer[sizeof(HWND)];
		memcpy(buffer, &WindowsContext::Get().hWnd, sizeof(HWND));

		NotifContext Notif{ encodeToSizeT("HWND"),buffer };
		Core::SubsystemControl::NotificationSubsystem("IMGUI", Notif);
		// 创建D3D设备
		RenderCore::GetRenderInterface()->CreateDeviceD3D(WindowsContext::Get().hWnd);

		return true;
	}

	void Win32Application::Uninstall() {
	}

	void Win32Application::InitializeUI() {
		launcherUI.Init();
	}

	void Win32Application::EndUI() {
		launcherUI.Shutdown();
		RenderCore::GetRenderInterface()->CleanupDeviceD3D();
	}

	void Win32Application::Tick()
	{
		RenderRT::GetRenderRTInterface()->SetRenderTarget();

		launcherUI.Tick(); // 更新UI逻辑

		// Present
		RenderRT::GetRenderRTInterface()->BindRenderTarget();
		//RenderCore::GetRenderInterface()->DebugD3D11State();
		RenderCore::GetRenderInterface()->Present();
	}
}