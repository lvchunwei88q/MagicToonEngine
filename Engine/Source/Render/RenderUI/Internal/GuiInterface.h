#pragma once
#include <Windows.h>

#include <imgui.h> // 支持停靠布局
#include <backends/imgui_impl_win32.h> // 适配windows
#include <backends/imgui_impl_dx11.h> // 使用DX11

// 序列化
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace RenderUI {

	struct LoggerSwitch {
		bool showDebug;
		bool showInfo;
		bool showWarning;
		bool showError;
		bool autoScroll;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(showDebug, showInfo, showWarning, showError, autoScroll);
		}
	};

	struct ComponentSwitch {
		// Component Switch
		LoggerSwitch loggerswitch;
		// Switch
		bool ExampleWindow;
		bool LoggerWindow;
		bool RenderViewWindow;
		bool ContentBrowserWindow;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(loggerswitch,LoggerWindow, ExampleWindow, RenderViewWindow, ContentBrowserWindow);
		}
	};

	extern ComponentSwitch Switch;

	class SetBackColor {
	public:
		SetBackColor(ImVec4 color) {
			ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
		}
		~SetBackColor() {
			ImGui::PopStyleColor();
		}
	};

	void BasicLayout();
	void LoggerWindow();
	void RenderViewWindow();
	void DrawContentBrowserWindow();
	void ExampleWindow();

	void DrawToolsWindows();
}