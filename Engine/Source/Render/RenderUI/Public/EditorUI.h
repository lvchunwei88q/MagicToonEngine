#pragma once

#include "Common/RENDERUI_API.h"

namespace RenderUI {
	class RENDERUI_API EditorUI
	{
	public:
		void Tick();
		void Init(HWND hwnd);
		void Shutdown();
	};

	RENDERUI_API LRESULT EditorUI_Windows_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}