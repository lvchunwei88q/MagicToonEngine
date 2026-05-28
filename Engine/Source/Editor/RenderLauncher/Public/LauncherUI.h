#pragma once

#include "Common/RENDERLAUNCHER_API.h"
#include <windows.h>

namespace RenderLauncher {
	class RENDERLAUNCHER_API LauncherUI
	{
	public:
		void Tick();

		void Init();
		void Shutdown();
	};


	RENDERLAUNCHER_API LRESULT LauncherUI_IMGUI_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}