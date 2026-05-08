#pragma once

#include <windows.h>
#include <string>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#define winW 960
#define winH 540

// windows event
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

using CallbackFunc = void(*)();

void init_app(HWND hwnd);
void SHOW_WINDOW();

extern CallbackFunc WebViewSuccess;
extern HWND g_hwnd;

// tools function
void Operation(JSON json);
void NewProJect(std::string path, std::string name);