
#include <GuiInterface.h>
#include <RenderUI.h>
#include <RenderContext.h>

#include <ILog.h>

///////////////////////
// 序列化文件相关
#include <AbsolutePath.h>
#include <FileManager.h>
#include <fstream>
#include <Serialize/SerializeMacro.h>
// 编码转换
#include <Converter.h>
///////////////////////

namespace RenderUI {
	void RenderUI::Init(HWND hwnd)
	{
		// 启用DPI感知
		ImGui_ImplWin32_EnableDpiAwareness();
		float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// set imgui layout file path
		io.IniFilename = "Magic-WindowLayout.ini";

		// Setup Dear ImGui style
		ImGui::StyleColorsClassic();

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hwnd);

		ImGui_ImplDX11_Init(RenderCore::RenderContext::Get().g_pd3dDevice,
							RenderCore::RenderContext::Get().g_pd3dDeviceContext);

		
		// 序列化布局
		FILE_SERIALIZATION_LOADING(Switch, CACHE "Editor\\Windows\\", L"WindowSwitch.mtdata")
	}

	void RenderUI::Shutdown()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		// 序列化布局
		FILE_SERIALIZATION_SAVE(Switch, CACHE "Editor\\Windows\\", L"WindowSwitch.mtdata")
	}

	void RenderUI::Tick()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

		// Render Content
		LOG::GetLogInstance()->SwapBuffers(); // 交换日志缓冲区 这是每帧固定要做的
		BasicLayout();

		{
			ExampleWindow();
			LoggerWindow();
			RenderViewWindow();
			DrawContentBrowserWindow();

			DrawToolsWindows(); // 工具窗口
		}

        // Rendering
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
		
	}

	LRESULT RenderUI_Windows_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}
}