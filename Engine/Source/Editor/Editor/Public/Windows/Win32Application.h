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

	class EDITOR_API Win32Application : public Core::Subsystem, public Singleton<Win32Application>
	{
	private:
		using WindowUpdateRenderingFunc = std::function<void()>;
	public:
		Win32Application();

		virtual bool Init();
		virtual void Uninstall();

		WindowsContext* GetWindowsContext() { return &WindowsContext::Get(); };
		void RegisterWindowUpdateRenderCallbackFunction(WindowUpdateRenderingFunc func);
		WindowUpdateRenderingFunc GetWindowUpdateRenderFunction();

	public:
		void CreateWindows();
		void CreateDeviceD3D();
		void CleanupWindows();
		void CleanupDeviceD3D();

		// Forward declaration of the window procedure
		static LRESULT CALLBACK WindowsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		WindowUpdateRenderingFunc WUR;
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(Win32Application)
}