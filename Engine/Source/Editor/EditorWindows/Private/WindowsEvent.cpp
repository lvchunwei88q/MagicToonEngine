#include <WindowsConfig.h>

#include <EditorWindows.h>

namespace EditorWindows {
	LRESULT CALLBACK WindowsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
        case WM_SIZE:
        {
            // 获取新的宽度和高度
            int newWidth = LOWORD(lParam);
            int newHeight = HIWORD(lParam);

            // 只在窗口不是最小化时更新
            if (wParam != SIZE_MINIMIZED)
            {
                WindowsConfig::Get().width = newWidth;
                WindowsConfig::Get().height = newHeight;
            }
            break;
        }

        case WM_MOVE:
        {
            // 获取新的位置
            int newX = LOWORD(lParam);
            int newY = HIWORD(lParam);

            WindowsConfig::Get().windowsX = newX;
            WindowsConfig::Get().windowsY = newY;
            break;
        }
		case WM_DESTROY:
            EditorWindows::Get().CleanupDeviceD3D(); // 确保在窗口销毁时清理D3D设备
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return 0;
	}
}