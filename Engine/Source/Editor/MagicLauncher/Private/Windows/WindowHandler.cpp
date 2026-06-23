#include "Main.h"
#include "Win32Application.h"
#include <IRenderRT.h> // Get RT interface for resizing
#include <Window/WindowAppointment.hpp>

namespace MagicLauncher
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

        case WM_COPYDATA: {
            PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;
            if (pcds->dwData == MessageTypeNumber_0 && pcds->cbData > 0) {
                const char* pData = (const char*)pcds->lpData;

                NotifContext Notif{ encodeToSizeT("LOADVALUE"),pData };
                Core::GetSubsystemContext()->Notification("LAUNCHERDATA", Notif);
                return TRUE;
            }
        }break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}

    void Win32Application::Notification(Core::NotificationContext context)
    {
        if (context.tags == encodeToSizeT("SETWINDOW")) {
			HWND hWnd = WindowsContext::Get().hWnd;

            int winWidth = 720; int winHeight = 430;

            MINMAXINFO mmi = { 0 };
            mmi.ptMinTrackSize.x = winWidth;
            mmi.ptMinTrackSize.y = winHeight;
            mmi.ptMaxTrackSize.x = winWidth;
            mmi.ptMaxTrackSize.y = winHeight;
            SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);
            int posX = (screenW - winWidth) / 2;
            int posY = (screenH - winHeight) / 2;

            SetWindowPos(
                hWnd,
                HWND_TOPMOST,   // 置顶
                posX, posY,     // 居中坐标
                winWidth, winHeight,
                SWP_SHOWWINDOW
            );
        }
    }
}