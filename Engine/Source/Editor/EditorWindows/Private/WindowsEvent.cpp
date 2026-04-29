#include <WindowsConfig.h>

#include <EditorWindows.h>
#include <RenderUI.h>
#include <RenderInterface.h>
#include <RenderRTInterface.h>
#include <BufferManagerInterface.h>

namespace EditorWindows {
	LRESULT CALLBACK WindowsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (RenderUI::RenderUI_Windows_Event(hWnd, msg, wParam, lParam))
            return true;

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

                RenderCore::ViewContext context;
                context.ScreenSize.x = newWidth;
                context.ScreenSize.y = newHeight;
                RenderCore::GetBufferManagerAdminInterface()->UpdateBuffers(context);
                RenderRT::GetRenderRTInterface()->UpdateRenderTargetView(newWidth, newHeight);
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

            // WndProc 里
        case WM_ENTERSIZEMOVE:
            SetTimer(hWnd, 1, 16, nullptr);  // 每 16ms 触发一次
            return 0;

        case WM_EXITSIZEMOVE:
            KillTimer(hWnd, 1);
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                // 处理一帧
                if (EditorWindows::Get().GetWindowUpdateRenderFunction()) {
                    EditorWindows::Get().GetWindowUpdateRenderFunction()();
                }
            }
            return 0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return 0;
	}
}