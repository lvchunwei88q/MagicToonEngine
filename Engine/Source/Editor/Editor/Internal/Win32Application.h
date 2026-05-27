#pragma once 
#include "Common/EDITOR_API.h"
#include <Windows.h>
#include <EditorUI.h>

#include <Subsystem/Subsystem.h>
#include <Tools/Singleton.h>
#include <functional>
#include <Subsystem/SubsystemTemplate.h>

using namespace RenderEditor;

namespace Editor
{
	struct WindowsContext : public Singleton<WindowsContext>
	{
	public:
		HWND hWnd = nullptr; // windows id
	};

	class Win32Application final : public Core::Subsystem, public Singleton<Win32Application>
	{
	private:
		using WindowUpdateRenderingFunc = std::function<void()>;
	public:
		Win32Application();

		virtual bool Init();
		virtual void Uninstall();

		// UI初始化一般是在所有系统创建完成后进行的，所以我们在这里提供一个单独的函数来初始化UI。
		void InitializeUI();
		void Tick(); // windows tick
		void EndUI();

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
		EditorUI editorUI;
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(Win32Application)
}