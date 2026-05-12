#pragma once
#include <Windows.h>

// imgui
#include <imgui.h> // 支持停靠布局
#include <backends/imgui_impl_win32.h> // 适配windows
#include <backends/imgui_impl_dx11.h> // 使用DX11

// 序列化
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);