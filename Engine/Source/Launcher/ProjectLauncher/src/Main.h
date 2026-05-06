#pragma once

#include <windows.h>

#define winW 960
#define winH 540

// windows event
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

using CallbackFunc = void(*)();

void init_app(HWND hwnd);
void SHOW_WINDOW();

extern CallbackFunc WebViewSuccess;