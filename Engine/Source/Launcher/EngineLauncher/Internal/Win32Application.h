#pragma once
#include <windows.h>

#include <Subsystem/SubsystemTemplate.h>

#include <IRender.h> // 呈现
#include <LauncherUI.h>

using namespace RenderLauncher;

namespace EngineLauncher {
	struct WindowsContext : public Singleton<WindowsContext>
	{
	public:
		HWND hWnd = nullptr; // windows id
	};

	class Win32Application final : public SubsystemTemplate<Win32Application,Core::Priority::High>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();
		virtual void Notification(Core::NotificationContext context); // Notification

		// UI初始化一般是在所有系统创建完成后进行的，所以我们在这里提供一个单独的函数来初始化UI。
		void InitializeUI();
		void Tick(); // windows tick
		void EndUI();

	private:
		LauncherUI launcherUI;
	};
}