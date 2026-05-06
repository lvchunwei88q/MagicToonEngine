#include "Main.h"

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