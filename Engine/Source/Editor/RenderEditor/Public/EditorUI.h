#pragma once

#include "Common/RENDEREDITOR_API.h"
#include <windows.h>

namespace RenderEditor {
	class RENDEREDITOR_API EditorUI
	{
	public:
		void Tick();

		void Init();
		void Shutdown();
	};


	RENDEREDITOR_API LRESULT EditorUI_IMGUI_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}