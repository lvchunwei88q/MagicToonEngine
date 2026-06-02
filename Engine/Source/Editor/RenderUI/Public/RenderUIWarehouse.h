#pragma once
#include "Common/RENDERUI_API.h"
#include <windows.h>

// imgui
#include <imgui.h> // 支持停靠布局
#include <Tools/ImGuiNotify.h> // 通知系统
#include <backends/imgui_impl_win32.h> // 适配windows
#include <backends/imgui_impl_dx11.h> // 使用DX11

#include <Editor/font_icon/IconsFontAwesome7.h>
#include <Editor/font_icon/IconsFontAwesome7Brands.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace RenderUI {
	RENDERUI_API LRESULT RenderUI_IMGUI_Windows_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}