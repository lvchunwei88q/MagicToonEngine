#include "Main.h"
#include <IProjectController.h>

// 全局变量
bool g_isDragging = false;
POINT g_lastMousePos;
HHOOK g_mouseHook = nullptr;

// MouseProc
void StopDrag();
void StartDrag(HWND hwnd);

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && g_isDragging) {
		if (wParam == WM_LBUTTONUP) {  // 左键松开
			StopDrag();
		}
        if (wParam == WM_MOUSEMOVE) {
            MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

            int dx = pMouseStruct->pt.x - g_lastMousePos.x;
            int dy = pMouseStruct->pt.y - g_lastMousePos.y;

            HWND hwnd = GetForegroundWindow(); // 或者保存窗口句柄
            RECT rect;
            GetWindowRect(hwnd, &rect);
            SetWindowPos(hwnd, nullptr,
                rect.left + dx, rect.top + dy,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

            g_lastMousePos = pMouseStruct->pt;
        }
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}
void StartDrag(HWND hwnd) {
	g_isDragging = true;
	GetCursorPos(&g_lastMousePos);
	SetCapture(hwnd);
	g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc,
		GetModuleHandle(nullptr), 0);
}

void StopDrag() {
	g_isDragging = false;
	ReleaseCapture();

	if (g_mouseHook) {
		UnhookWindowsHookEx(g_mouseHook);
		g_mouseHook = nullptr;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CREATE: // 窗口创建
	{
		init_app(hwnd);
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

void Operation(JSON json)
{
	std::string action = json["action"];
	if (action == "create_projnew_projectct") {
		std::string name = json["name"];
		std::string path = json["path"];
		NewProJect(path,name);
	}
	else if(action == "web_ready") {
		WebViewSuccess(); // show
	}
	else if (action == "start_drag") {
		StartDrag(g_hwnd);
	}
}

using namespace EngineProject;

void NewProJect(std::string path, std::string name)
{
	IProjectController* controller = GetProjectControllerInterface();

	ProJectConfig config;
	config.name = name;
	config.path = path;
	controller->Create(config);
}