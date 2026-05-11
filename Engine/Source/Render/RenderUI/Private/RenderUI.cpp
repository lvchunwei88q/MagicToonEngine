
#include <EditorUIComponentSwitch.h>
#include <RenderUI.h>
#include <RenderContext.h>

#include <ILog.h>
#include <Theme.h> // 主题

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

        // Set Magic Engine style - from https://github.com/ChenlizheMe/Infernux.git
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* c = style.Colors;

            // No conversion needed: UNORM swapchain writes values as-is.
            auto L = [](float r, float g, float b, float a) -> ImVec4 { return ImVec4(r, g, b, a); };

            // Accent color shorthand 
            constexpr float AR = Theme::ACCENT_R;
            constexpr float AG = Theme::ACCENT_G;
            constexpr float AB = Theme::ACCENT_B;

            // --- Text ---
            c[ImGuiCol_Text] = L(0.812f, 0.812f, 0.812f, 1.00f);           // #CFCFCF
            c[ImGuiCol_TextDisabled] = L(0.333f, 0.333f, 0.333f, 1.00f);   // #555555
            c[ImGuiCol_TextSelectedBg] = L(0.200f, 0.200f, 0.200f, 0.60f); // #333333

            // --- Backgrounds ---
            c[ImGuiCol_WindowBg] = L(0.098f, 0.098f, 0.098f, 1.00f);       // #191919
            c[ImGuiCol_ChildBg] = L(0.125f, 0.125f, 0.125f, 1.00f);        // #202020
            c[ImGuiCol_PopupBg] = L(0.125f, 0.125f, 0.125f, 0.98f);        // #202020
            c[ImGuiCol_FrameBg] = L(0.125f, 0.125f, 0.125f, 1.00f);        // #202020
            c[ImGuiCol_FrameBgHovered] = L(0.165f, 0.165f, 0.165f, 1.00f); // #2A2A2A
            c[ImGuiCol_FrameBgActive] = L(0.210f, 0.170f, 0.240f, 1.00f);  // reddish tint when active

            // --- Title bar ---
            c[ImGuiCol_TitleBg] = L(0.098f, 0.098f, 0.098f, 1.00f);
            c[ImGuiCol_TitleBgActive] = L(0.098f, 0.098f, 0.098f, 1.00f);
            c[ImGuiCol_TitleBgCollapsed] = L(0.098f, 0.098f, 0.098f, 0.75f);

            // --- MenuBar ---
            c[ImGuiCol_MenuBarBg] = L(0.098f, 0.098f, 0.098f, 1.00f);

            // --- Scrollbar ---
            c[ImGuiCol_ScrollbarBg] = L(0.098f, 0.098f, 0.098f, 0.00f);
            c[ImGuiCol_ScrollbarGrab] = L(0.184f, 0.184f, 0.184f, 1.00f);
            c[ImGuiCol_ScrollbarGrabHovered] = L(0.333f, 0.333f, 0.333f, 1.00f);
            c[ImGuiCol_ScrollbarGrabActive] = L(0.439f, 0.439f, 0.439f, 1.00f);

            // --- Interactive accent ---
            c[ImGuiCol_CheckMark] = L(AR, AG, AB, 1.00f);              // #EB5757
            c[ImGuiCol_SliderGrab] = L(AR, AG, AB, 0.88f);             // #EB5757
            c[ImGuiCol_SliderGrabActive] = L(AR, AG, AB, 1.00f);       // #EB5757
            c[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // no outline on active fields

            // --- Buttons --- subtle dark surface with red accent on interaction
            c[ImGuiCol_Button] = L(0.165f, 0.165f, 0.165f, 1.00f); // #2A2A2A
            c[ImGuiCol_ButtonHovered] = L(0.185f, 0.165f, 0.220f, 1.00f);
            c[ImGuiCol_ButtonActive] = L(0.220f, 0.180f, 0.270f, 1.00f);

            // --- Header ---
            c[ImGuiCol_Header] = L(0.200f, 0.200f, 0.200f, 1.00f);
            c[ImGuiCol_HeaderHovered] = L(0.160f, 0.160f, 0.260f, 1.00f);
            c[ImGuiCol_HeaderActive] = L(0.200f, 0.170f, 0.240f, 1.00f);

            // --- Border / Separator ---
            c[ImGuiCol_Border] = L(0.184f, 0.184f, 0.184f, 1.00f);
            c[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            c[ImGuiCol_Separator] = L(0.184f, 0.184f, 0.184f, 1.00f);
            c[ImGuiCol_SeparatorHovered] = L(AR, AG, AB, 0.60f); // #EB5757
            c[ImGuiCol_SeparatorActive] = L(AR, AG, AB, 0.80f);  // #EB5757

            // --- Resize grip ---
            c[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            c[ImGuiCol_ResizeGripHovered] = L(AR, AG, AB, 0.30f);
            c[ImGuiCol_ResizeGripActive] = L(AR, AG, AB, 0.50f);

            // --- Tabs ---
            c[ImGuiCol_Tab] = L(0.098f, 0.098f, 0.098f, 1.00f);               // #191919
            c[ImGuiCol_TabHovered] = L(0.165f, 0.165f, 0.165f, 1.00f);        // #2A2A2A
            c[ImGuiCol_TabSelected] = L(0.125f, 0.125f, 0.125f, 1.00f);       // #202020
            c[ImGuiCol_TabSelectedOverline] = L(AR, AG, AB, 1.00f);           // #EB5757 red overline
            c[ImGuiCol_TabDimmed] = L(0.098f, 0.098f, 0.098f, 1.00f);         // #191919
            c[ImGuiCol_TabDimmedSelected] = L(0.125f, 0.125f, 0.125f, 1.00f); // #202020
            c[ImGuiCol_TabDimmedSelectedOverline] = L(AR, AG, AB, 0.60f);     // dimmer red overline

            // --- Docking ---
            c[ImGuiCol_DockingPreview] = L(AR, AG, AB, 0.25f); // #EB5757
            c[ImGuiCol_DockingEmptyBg] = L(0.060f, 0.060f, 0.060f, 1.00f);

            // --- Plots ---
            c[ImGuiCol_PlotLines] = L(0.439f, 0.439f, 0.439f, 1.00f);
            c[ImGuiCol_PlotHistogram] = L(0.812f, 0.812f, 0.812f, 1.00f);

            // --- Drag-drop target highlight --- pure white border
            c[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            c[ImGuiCol_DragDropTargetBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

            // --- Modal dim ---
            c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.56f);

            // --- Table ---
            c[ImGuiCol_TableHeaderBg] = L(0.125f, 0.125f, 0.125f, 1.00f);
            c[ImGuiCol_TableBorderStrong] = L(0.184f, 0.184f, 0.184f, 1.00f);
            c[ImGuiCol_TableBorderLight] = L(0.149f, 0.149f, 0.149f, 1.00f);
            c[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            c[ImGuiCol_TableRowBgAlt] = L(1.000f, 1.000f, 1.000f, 0.02f);

            // =====================================================================
            // Style dimensions — Notion-style clean, modern spacing
            // =====================================================================
            style.WindowPadding = ImVec2(10.0f, 10.0f);
            style.FramePadding = ImVec2(8.0f, 3.0f);
            style.CellPadding = ImVec2(4.0f, 4.0f);
            style.ItemSpacing = ImVec2(8.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
            style.IndentSpacing = 18.0f;
            style.ScrollbarSize = 8.0f; // thin Notion scrollbar
            style.GrabMinSize = 6.0f;

            // Borders — minimal, but keep inputs readable
            style.WindowBorderSize = 1.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.FrameBorderSize = 1.0f; // visible border around input fields
            style.TabBorderSize = 0.0f;
            style.TabBarBorderSize = 1.0f;

            // Rounding — project-wide square language
            style.WindowRounding = 0.0f; // main window stays square
            style.ChildRounding = 0.0f;
            style.FrameRounding = 0.0f;
            style.PopupRounding = 0.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;

            // Anti-aliasing
            style.AntiAliasedLines = true;
            style.AntiAliasedFill = true;
        }

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

		// Create a font configuration structure (ImFontConfig) for customizing various parameters when loading fonts
		ImFontConfig ifc;

		// Set ownership of font data: false means the font data (front_data_data) is not managed by ImGui's font atlas (FontAtlas)
		ifc.FontDataOwnedByAtlas = false;

		// Load a font from TTF font data in memory and add it to the font atlas
		ImFont* front = io.Fonts->AddFontFromMemoryTTF(
			(void*)front_data_data,
			front_data_size,
			18.0f,
			&ifc,
			io.Fonts->GetGlyphRangesChineseFull()
		);

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hwnd);

		ImGui_ImplDX11_Init(RenderCore::RenderContext::Get().g_pd3dDevice,
							RenderCore::RenderContext::Get().g_pd3dDeviceContext);

		
		// 序列化布局
		FILE_SERIALIZATION_LOADING(Switch, CONFIG "Editor\\Windows\\", L"WindowSwitch.mtdata")
	}

	void RenderUI::Shutdown()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		// 序列化布局
		FILE_SERIALIZATION_SAVE(Switch, CONFIG "Editor\\Windows\\", L"WindowSwitch.mtdata")
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
            DetailsWindow();
            LuaUIControlWindow();

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