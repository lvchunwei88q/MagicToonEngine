#pragma once

#include <Common/RENDERUI_API.h>

namespace RenderUI {
	class RENDERUI_API RenderUI
	{
	public:
		void Tick();
		void Init(HWND hwnd);
		void Shutdown();
	};

	RENDERUI_API LRESULT RenderUI_Windows_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}