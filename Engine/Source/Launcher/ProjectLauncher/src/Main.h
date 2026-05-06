#pragma once

#include <windows.h>

// windows event
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void init_app(HWND hwnd);
void Showindow();