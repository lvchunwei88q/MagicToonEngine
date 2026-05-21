#include "Main.h"
#include "Win32Application.h"
#include <IRenderRT.h> // Get RT interface for resizing

namespace EngineLauncher
{
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (LauncherUI_IMGUI_Event(hwnd, msg, wParam, lParam))
            return true;

		switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
        case WM_SIZE:
        {
            // 获取新的宽度和高度
            int newWidth = LOWORD(lParam);
            int newHeight = HIWORD(lParam);

            // 只在窗口不是最小化时更新
            if (wParam != SIZE_MINIMIZED)
            {
                RenderRT::GetRenderRTInterface()->UpdateRenderTargetView(newWidth, newHeight);
            }
            break;
        }
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}
}