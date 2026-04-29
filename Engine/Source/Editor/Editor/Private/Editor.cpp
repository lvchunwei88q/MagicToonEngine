#include <Editor.h>
#include <ErrorCapture/ErrorCapture.h>
#include <EditorRender.h>

#include <Log.h>

namespace Editor
{
	Editor::Editor()
	{
        InitCoreErrorCapture();
	}

    void Editor::InitCoreErrorCapture()
    {
        // 初始化核心错误捕获 在单例构造函数进行，但是这并不代表可以很早使用。
        Core::ErrorCapture::RegisterCaptureFunction([](const std::string& errorMessage){
            LOG_ERROR(errorMessage.c_str());
        });
    }

    void Editor::SetEngineState(EngineState State)
    {
        state = State;
    }

	void Editor::Run()
	{
		HWND Hwnd = Windows::Get().GetWindowsContext()->hWnd; // 确保窗口上下文已创建
		ShowWindow(Hwnd, SW_SHOW);

        EditorRender::Get().Init(); // 渲染初始化

        Windows::Get().RegisterWindowUpdateRenderCallbackFunction([this]() {
            if (state == EngineState::Run) Tick(); // 每帧更新逻辑
        });

        // 游戏循环 + 消息处理
        while (state == EngineState::Run)
        {
            MSG msg = {};

            // 处理所有窗口消息
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    state = EngineState::Stop;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

			if(state == EngineState::Run) Tick(); // 每帧更新逻辑
        }

        EditorRender::Get().End();
	}

    void EditorRender::Init()
    {
        renderui.Init(Windows::Get().GetWindowsContext()->hWnd); // 初始化UI系统
    }
    void EditorRender::End()
    {
        renderui.Shutdown();
    }
}