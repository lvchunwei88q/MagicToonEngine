#pragma once 
#include <Common/EDITOR_API.h>
#include <Windows.h>

#include <Subsystem/Subsystem.h>
#include <Tools/Singleton.h>
#include <functional>

#include <Subsystem/SubsystemTemplate.h>

namespace Editor
{
	class WindowsContext : public Singleton<WindowsContext>
	{
	public:
		HWND hWnd = nullptr; // windows id
	};

	class EDITOR_API Windows : public Core::Subsystem, public Singleton<Windows>
	{
	private:
		using WindowUpdateRenderingFunc = std::function<void()>;
	public:
		Windows();

		virtual bool Init();
		virtual void Uninstall();

		WindowsContext* GetWindowsContext() { return &WindowsContext::Get(); };
		void RegisterWindowUpdateRenderCallbackFunction(WindowUpdateRenderingFunc func);
		WindowUpdateRenderingFunc GetWindowUpdateRenderFunction();

	private:
		void CreateWindows();
		void CreateDeviceD3D();
		void CleanupWindows();

	public: // 这个函数需要在窗口销毁时调用，以确保D3D设备被正确清理
		void CleanupDeviceD3D();

	private:
		WindowUpdateRenderingFunc WUR;
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(Windows)
}