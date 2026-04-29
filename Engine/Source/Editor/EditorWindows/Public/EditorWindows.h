#pragma once 
#include <Common/EDITORWINDOWS_API.h>
#include <Windows.h>

#include <Subsystem/Subsystem.h>
#include <Common/Singleton.h>
#include <functional>

#include <Subsystem/SubsystemTemplate.h>

namespace EditorWindows
{
	class WindowsContext : public Singleton<WindowsContext>
	{
	public:
		HWND hWnd = nullptr; // windows id
	};

	class EDITORWINDOWS_API EditorWindows : public Core::Subsystem, public Singleton<EditorWindows>
	{
	public:
		EditorWindows();

		using WindowUpdateRenderingFunc = std::function<void()>;

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

	AUTO_REGISTER_SINGLETON_INCLUDE(EditorWindows)
}