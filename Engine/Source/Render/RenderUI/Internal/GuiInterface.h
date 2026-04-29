#pragma once
#include <Windows.h>

#include <imgui.h> // 支持停靠布局
#include <backends/imgui_impl_win32.h> // 适配windows
#include <backends/imgui_impl_dx11.h> // 使用DX11

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace RenderUI {
	void BasicLayout();
	void ExampleWindow();
}