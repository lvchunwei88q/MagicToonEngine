#include <Editor.h>

namespace Editor
{
	Editor::Editor()
	{
	}

	void Editor::Run()
	{
		HWND Hwnd = EditorWindows::EditorWindows::Get().GetWindowsContext()->hWnd; // 确保窗口上下文已创建
		ShowWindow(Hwnd, SW_SHOW);

        // 游戏循环 + 消息处理
        bool running = true;
        while (running)
        {
            MSG msg = {};

            // 处理所有窗口消息
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    running = false;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

			Tick(); // 每帧更新逻辑
        }
	}
}