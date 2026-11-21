// ============================================================================
// ImGuiSmokeTest.cpp
// Windows-only smoke test for the fork of Dear ImGui living in
// Engine/ThirdParty/IMGUI.
//
// What it does:
//   1. Creates a real Win32 window + D3D11 swap chain.
//   2. Initialises the Win32 + DX11 ImGui backends (the ones this fork ships).
//   3. Pumps a few hundred frames while drawing every major widget family:
//      text/buttons/sliders/drags/inputs/combo/color/tree/list/table/plot/
//      tabs/child/popup/tooltip/disabled/main-menu-bar.
//   4. Exercises docking (DockBuilder + DockSpaceOverViewport) and, optionally,
//      multi-viewport (ImGuiConfigFlags_ViewportsEnable).
//   5. Runs a handful of assertions on font atlas, draw data, table rows,
//      state storage and dock node creation, then exits.
//
// Usage (from a VS developer prompt):
//   MagicTest-ImGuiSmokeTest.exe                 -> runs 240 frames then exits
//   MagicTest-ImGuiSmokeTest.exe --interactive   -> stays open until you close it
//   MagicTest-ImGuiSmokeTest.exe --frames 600    -> custom frame budget
//   MagicTest-ImGuiSmokeTest.exe --viewports     -> also enable multi-viewport
//   MagicTest-ImGuiSmokeTest.exe --no-vsync      -> uncapped frame rate
//   MagicTest-ImGuiSmokeTest.exe --help          -> cmdline-generated usage
//
// Short forms: -f <frames>, -i, -v, -h.
//
// Exit code: 0 = all checks passed, 1 = at least one check failed or init failed.
// ============================================================================

#include "../Common/TestFramework.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cmdline/cmdline.h>

#include <d3d11.h>
#include <dxgi.h>

#include <imgui.h>
#include <imgui_internal.h> // DockBuilder* API used to build a deterministic layout
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// D3D11 / Win32 state
// ---------------------------------------------------------------------------
static ID3D11Device*         g_device = nullptr;
static ID3D11DeviceContext*  g_context = nullptr;
static IDXGISwapChain*       g_swapChain = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static UINT                  g_resizeWidth = 0;
static UINT                  g_resizeHeight = 0;
static bool                  g_occluded = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_resizeWidth = (UINT)LOWORD(lParam);
        g_resizeHeight = (UINT)HIWORD(lParam);
        return 0;

    case WM_SYSCOMMAND:
        // Disable ALT application menu / screensaver while the test runs.
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

static bool CreateRenderTarget()
{
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))) || backBuffer == nullptr)
        return false;

    const HRESULT hr = g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
    backBuffer->Release();
    return SUCCEEDED(hr);
}

static void CleanupRenderTarget()
{
    if (g_rtv)
    {
        g_rtv->Release();
        g_rtv = nullptr;
    }
}

static bool CreateDeviceD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL obtained = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        levels, (UINT)IM_ARRAYSIZE(levels), D3D11_SDK_VERSION,
        &sd, &g_swapChain, &g_device, &obtained, &g_context);

    if (FAILED(hr))
    {
        std::cout << "[warn] hardware D3D11 device failed (0x" << std::hex << hr
                  << "), retrying with WARP\n" << std::dec;
        hr = ::D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            levels, (UINT)IM_ARRAYSIZE(levels), D3D11_SDK_VERSION,
            &sd, &g_swapChain, &g_device, &obtained, &g_context);
    }

    if (FAILED(hr))
        return false;

    return CreateRenderTarget();
}

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context)   { g_context->Release();   g_context = nullptr; }
    if (g_device)    { g_device->Release();    g_device = nullptr; }
}

static std::string Narrow(const wchar_t* wide)
{
    if (!wide)
        return {};
    char buffer[512] = {};
    ::WideCharToMultiByte(CP_UTF8, 0, wide, -1, buffer, (int)IM_ARRAYSIZE(buffer) - 1, nullptr, nullptr);
    return std::string(buffer);
}

// ---------------------------------------------------------------------------
// UI state (kept outside so the frame budget doesn't depend on widget scope)
// ---------------------------------------------------------------------------
struct SmokeState
{
    int   buttonClicks = 0;
    int   radio = 1;
    int   combo = 0;
    int   listItem = 0;
    int   selected = 1;
    bool  checkbox = true;
    bool  treeLeaf = false;
    float slider = 0.5f;
    float drag[3] = { 1.0f, 2.0f, 3.0f };
    float color[3] = { 0.92f, 0.34f, 0.34f };
    int   inputInt = 42;
    char  inputText[128] = "MagicToonEngine / ImGui 1.92.8";
};

static SmokeState g_ui;
static int   g_frameIndex = 0;
static int   g_tableRows = 0;
static int   g_popupOpenCount = 0;
static int   g_storageValue = -1;
static bool  g_dockBuilt = false;
static bool  g_autoPopupClose = false; // automated runs close the modal after a few frames
static ImVec2 g_textSize{};
static bool  g_requestExit = false;
static int   g_popupOpenFrame = -1;

static float g_plotValues[90] = {};

// Build a deterministic dock layout: [ Widgets | Diagnostics ].
static void BuildDockLayout(ImGuiID dockId)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 size = viewport->WorkSize;
    if (size.x <= 0.0f || size.y <= 0.0f)
        size = viewport->Size;

    ImGui::DockBuilderRemoveNode(dockId);
    ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockId, size);

    ImGuiID left = 0;
    ImGuiID right = dockId;
    ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Left, 0.42f, &left, &right);
    ImGui::DockBuilderDockWindow("Widgets", left);
    ImGui::DockBuilderDockWindow("Diagnostics", right);
    ImGui::DockBuilderFinish(dockId);

    g_dockBuilt = ImGui::DockBuilderGetNode(dockId) != nullptr;
}

static void DrawMenuBar(ImGuiIO& io)
{
    if (!ImGui::BeginMainMenuBar())
        return;

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Request exit"))
            g_requestExit = true;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("Docked", nullptr, true, false);
        ImGui::MenuItem("Multi-viewport", nullptr,
            (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0, false);
        ImGui::EndMenu();
    }

    ImGui::Text("  %.1f FPS (%d frames)", io.Framerate, g_frameIndex);
    ImGui::EndMainMenuBar();
}

static void DrawWidgetsWindow(ImGuiIO& io)
{
    ImGui::Begin("Widgets");

    ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
    ImGui::TextColored(ImVec4(0.92f, 0.34f, 0.34f, 1.0f), "MagicToonEngine ImGui smoke test");
    ImGui::TextDisabled("Windows + D3D11 backends, docking, dynamic font atlas");
    ImGui::Separator();

    // --- buttons / toggles -------------------------------------------------
    if (ImGui::Button("Click me"))
        ++g_ui.buttonClicks;
    ImGui::SameLine();
    ImGui::Text("clicks = %d", g_ui.buttonClicks);
    ImGui::SameLine();
    if (ImGui::Button("Hover me"))
        ImGui::SetTooltip("Tooltip path is alive");
    ImGui::Checkbox("Checkbox", &g_ui.checkbox);
    ImGui::SameLine();
    ImGui::RadioButton("Radio A", &g_ui.radio, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Radio B", &g_ui.radio, 1);

    // --- sliders / drags / scalars ----------------------------------------
    ImGui::SliderFloat("SliderFloat", &g_ui.slider, 0.0f, 1.0f);
    ImGui::DragFloat3("DragFloat3", g_ui.drag, 0.01f);
    ImGui::InputInt("InputInt", &g_ui.inputInt);
    ImGui::InputText("InputText", g_ui.inputText, IM_ARRAYSIZE(g_ui.inputText));

    static const char* comboItems[] = { "Toon", "PBR", "Unlit", "Debug" };
    ImGui::Combo("Combo", &g_ui.combo, comboItems, IM_ARRAYSIZE(comboItems));
    ImGui::ColorEdit3("ColorEdit3", g_ui.color);

    // --- tree / collapsing -------------------------------------------------
    if (ImGui::CollapsingHeader("CollapsingHeader"))
    {
        ImGui::Indent();
        ImGui::Text("Nested content");
        if (ImGui::TreeNode("TreeNode"))
        {
            ImGui::Checkbox("Tree leaf", &g_ui.treeLeaf);
            ImGui::TreePop();
        }
        ImGui::Unindent();
    }

    // --- list / child region ----------------------------------------------
    ImGui::BeginChild("SelectableList", ImVec2(0.0f, 96.0f), ImGuiChildFlags_Borders);
    for (int i = 0; i < 6; ++i)
    {
        char label[32];
        std::snprintf(label, sizeof(label), "Scene object %d", i);
        if (ImGui::Selectable(label, g_ui.selected == i))
            g_ui.selected = i;
    }
    ImGui::EndChild();

    static const char* listItems[] = { "Item 0", "Item 1", "Item 2", "Item 3", "Item 4", "Item 5" };
    ImGui::ListBox("ListBox", &g_ui.listItem, listItems, IM_ARRAYSIZE(listItems), 4);

    // --- progress / plot ---------------------------------------------------
    ImGui::ProgressBar((float)(std::fmod(ImGui::GetTime(), 2.0) * 0.5), ImVec2(0.0f, 0.0f), "progress");
    ImGui::PlotLines("PlotLines", g_plotValues, IM_ARRAYSIZE(g_plotValues),
        0, nullptr, -1.0f, 1.0f, ImVec2(0.0f, 60.0f));

    // --- tables ------------------------------------------------------------
    if (ImGui::BeginTable("SmokeTable", 3,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 5; ++row)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", row);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("row_%d", row);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.2f", (double)row * 0.25);
            ++g_tableRows;
        }
        ImGui::EndTable();
    }

    // --- tabs --------------------------------------------------------------
    if (ImGui::BeginTabBar("SmokeTabs"))
    {
        if (ImGui::BeginTabItem("Tab A"))
        {
            ImGui::Text("Tab A body");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tab B"))
        {
            ImGui::Text("Tab B body");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // --- disabled block ----------------------------------------------------
    ImGui::BeginDisabled();
    ImGui::Button("Disabled button");
    ImGui::EndDisabled();

    // --- storage + text metrics (also used by the assertions) --------------
    ImGuiStorage* storage = ImGui::GetStateStorage();
    if (storage)
    {
        const ImGuiID key = ImGui::GetID("smoke.storage.key");
        storage->SetInt(key, 1337);
        g_storageValue = storage->GetInt(key, -1);
    }
    g_textSize = ImGui::CalcTextSize("MagicToonEngine");

    ImGui::End();
}

static void DrawDiagnosticsWindow(ImGuiIO& io)
{
    ImGui::Begin("Diagnostics");

    ImGui::Text("Frame index : %d", g_frameIndex);
    ImGui::Text("Display size: %.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Text("Delta time  : %.2f ms", io.DeltaTime * 1000.0f);
    ImGui::Text("Font atlas  : %s", io.Fonts->IsBuilt() ? "built" : "pending");
    ImGui::Text("Fonts       : %d", io.Fonts->Fonts.Size);

    const ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData)
    {
        ImGui::Text("Draw lists  : %d", drawData->CmdListsCount);
        ImGui::Text("Vertices    : %d", drawData->TotalVtxCount);
        ImGui::Text("Indices     : %d", drawData->TotalIdxCount);
    }

    ImGui::Separator();
    ImGui::Text("Dock node built : %s", g_dockBuilt ? "yes" : "no");
    ImGui::Text("Table rows       : %d", g_tableRows);
    ImGui::Text("Storage value    : %d", g_storageValue);
    ImGui::Text("Text size        : %.1f x %.1f", g_textSize.x, g_textSize.y);
    ImGui::Text("Viewports        : %s",
        (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ? "enabled" : "disabled");

    ImGui::Separator();
    // Open the modal once, without requiring a mouse click, so the automated
    // run still covers the popup path.
    if (g_frameIndex == 5)
    {
        ImGui::OpenPopup("Smoke Modal");
        g_popupOpenFrame = g_frameIndex;
    }
    if (ImGui::Button("Open modal popup"))
    {
        ImGui::OpenPopup("Smoke Modal");
        g_popupOpenFrame = g_frameIndex;
    }
    if (ImGui::Button("Request exit"))
        g_requestExit = true;

    // Modal popups must be opened and queried inside the same window/ID
    // scope, so this lives here rather than in a standalone helper.
    if (ImGui::BeginPopupModal("Smoke Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ++g_popupOpenCount;
        ImGui::Text("Popup path works.");
        ImGui::Text("Frame %d, %.1f FPS", g_frameIndex, io.Framerate);
        if (ImGui::Button("OK") || (g_autoPopupClose && g_frameIndex >= g_popupOpenFrame + 12))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}

static void InitStyle()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.18f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.20f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.92f, 0.34f, 0.34f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.92f, 0.34f, 0.34f, 0.88f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.92f, 0.34f, 0.34f, 1.00f);
    style.Colors[ImGuiCol_TabSelected] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
}

static bool InitFonts(ImGuiIO& io, bool& cjkLoaded)
{
    cjkLoaded = false;

    // Prefer the system Chinese font so CJK labels and the dynamic atlas get
    // exercised on a normal Windows install.
    const char* cjkPath = "C:\\Windows\\Fonts\\msyh.ttc";
    if (::GetFileAttributesA(cjkPath) != INVALID_FILE_ATTRIBUTES)
    {
        ImFont* font = io.Fonts->AddFontFromFileTTF(
            cjkPath, 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        if (font)
        {
            io.FontDefault = font;
            cjkLoaded = true;
            return true;
        }
    }

    io.FontDefault = io.Fonts->AddFontDefault();
    return io.FontDefault != nullptr;
}

// ---------------------------------------------------------------------------
// Command line (parsed with the project's third-party cmdline library)
// ---------------------------------------------------------------------------
struct Options
{
    bool interactive = false;
    int  frames = 0;
    bool viewports = false;
    bool vsync = true;
    bool help = false;
};

static bool ParseCommandLine(int argc, char** argv, Options& options)
{
    cmdline::parser cli;
    cli.set_program_name("MagicTest-ImGuiSmokeTest");
    cli.add("help", 'h', "print this message");
    cli.add<int>("frames", 'f', "frames to render before exiting (0 = interactive)", false, 240);
    cli.add("interactive", 'i', "run until the window is closed");
    cli.add("viewports", 'v', "enable ImGui multi-viewport (platform windows)");
    cli.add("no-vsync", 0, "disable vsync (uncapped frame rate)");
    cli.footer("\nExamples: MagicTest-ImGuiSmokeTest --frames 120 --no-vsync\n"
               "          MagicTest-ImGuiSmokeTest --interactive --viewports");

    if (!cli.parse_check(argc, argv))
    {
        // parse_check already printed usage/help or the error to stderr.
        options.help = cli.exist("help");
        return false;
    }

    options.help = cli.exist("help");
    options.interactive = cli.exist("interactive");
    options.viewports = cli.exist("viewports");
    options.vsync = !cli.exist("no-vsync");
    options.frames = cli.get<int>("frames");
    if (options.interactive && !cli.exist("frames"))
        options.frames = 0;
    return true;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char** argv)
{
    Options options;
    if (!ParseCommandLine(argc, argv, options))
        return options.help ? 0 : 2; // --help is a success, bad arguments are not

    std::cout << "=== MagicToonEngine ImGui smoke test ===\n";

    const int  frameBudget = options.frames;
    const bool enableViewports = options.viewports;
    const bool vsync = options.vsync;

    std::cout << "Frame budget : " << (frameBudget > 0 ? std::to_string(frameBudget) : "interactive") << '\n'
              << "Viewports    : " << (enableViewports ? "on" : "off") << '\n'
              << "Vsync        : " << (vsync ? "on" : "off") << '\n';
    g_autoPopupClose = frameBudget > 0;

    // Make the window crisp on high-DPI monitors before creating it.
    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = ::GetModuleHandleW(nullptr);
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"MagicToonImGuiSmokeTest";
    if (!::RegisterClassExW(&wc))
    {
        std::cerr << "RegisterClassExW failed: " << ::GetLastError() << '\n';
        return 1;
    }

    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName, L"MagicToonEngine - ImGui Smoke Test",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd)
    {
        std::cerr << "CreateWindowW failed: " << ::GetLastError() << '\n';
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    if (!CreateDeviceD3D(hwnd))
    {
        std::cerr << "Failed to create a D3D11 device/swap chain.\n";
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Report the adapter so the console log proves which GPU path was used.
    {
        IDXGIDevice* dxgiDevice = nullptr;
        IDXGIAdapter* adapter = nullptr;
        if (SUCCEEDED(g_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))
            && SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
        {
            DXGI_ADAPTER_DESC desc{};
            adapter->GetDesc(&desc);
            std::cout << "Adapter: " << Narrow(desc.Description)
                      << " (" << desc.DedicatedVideoMemory / (1024 * 1024) << " MB)\n";
            adapter->Release();
        }
        if (dxgiDevice)
            dxgiDevice->Release();
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // --- ImGui setup -------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // keep the test hermetic: no layout file writes
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    if (enableViewports)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigViewportsNoTaskBarIcon = true;
    }

    InitStyle();

    bool cjkLoaded = false;
    if (!InitFonts(io, cjkLoaded))
    {
        std::cerr << "Failed to create any font.\n";
        ImGui::DestroyContext();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    std::cout << "Font: " << (cjkLoaded ? "C:\\Windows\\Fonts\\msyh.ttc (CJK)" : "ImGui default") << '\n';

    if (!ImGui_ImplWin32_Init(hwnd))
    {
        std::cerr << "ImGui_ImplWin32_Init failed.\n";
        return 1;
    }
    if (!ImGui_ImplDX11_Init(g_device, g_context))
    {
        std::cerr << "ImGui_ImplDX11_Init failed.\n";
        return 1;
    }

    // Docking will be enabled per-frame; the dock node itself is built once
    // after the main viewport has a valid work size (frame 2).
    // ImHashStr() is used instead of ImGui::GetID() because we are outside
    // any window scope here (GetID() would require a current window).
    const ImGuiID dockId = ImHashStr("SmokeDockSpace");
    bool dockLayoutBuilt = false;

    // Pre-fill the plot with a deterministic waveform.
    for (int i = 0; i < IM_ARRAYSIZE(g_plotValues); ++i)
        g_plotValues[i] = std::sinf(i * 0.16f) * 0.75f + std::sinf(i * 0.47f) * 0.2f;

    bool  done = false;
    int   renderedFrames = 0;
    bool  fontBuilt = false;
    int   drawVertices = 0;
    int   drawLists = 0;
    ImVec2 displaySize{};

    while (!done)
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_occluded)
        {
            if (g_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
            {
                ::Sleep(10);
                continue;
            }
            g_occluded = false;
        }

        if (g_resizeWidth != 0 && g_resizeHeight != 0)
        {
            CleanupRenderTarget();
            const HRESULT hr = g_swapChain->ResizeBuffers(
                0, g_resizeWidth, g_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_resizeWidth = g_resizeHeight = 0;
            if (FAILED(hr) || !CreateRenderTarget())
            {
                std::cerr << "Swap chain resize failed (0x" << std::hex << hr << ")\n";
                break;
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ++g_frameIndex;

        // The main menu bar must be submitted before the dockspace so the
        // viewport work area already excludes it.
        DrawMenuBar(io);

        if (!dockLayoutBuilt)
        {
            BuildDockLayout(dockId);
            dockLayoutBuilt = true;
        }
        ImGui::DockSpaceOverViewport(dockId, ImGui::GetMainViewport(),
            ImGuiDockNodeFlags_PassthruCentralNode);

        DrawWidgetsWindow(io);
        DrawDiagnosticsWindow(io);

        ImGui::Render();

        fontBuilt = io.Fonts->IsBuilt();
        displaySize = io.DisplaySize;
        if (const ImDrawData* drawData = ImGui::GetDrawData())
        {
            drawVertices = drawData->TotalVtxCount;
            drawLists = drawData->CmdListsCount;
        }

        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        const float clear[4] = { 0.055f, 0.055f, 0.065f, 1.0f };
        g_context->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            // The platform-window pass changes the render target; restore
            // ours before the next frame.
            g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        }

        const HRESULT present = g_swapChain->Present(vsync ? 1 : 0, 0);
        if (present == DXGI_STATUS_OCCLUDED)
            g_occluded = true;
        else if (FAILED(present))
        {
            std::cerr << "Present failed (0x" << std::hex << present << ")\n" << std::dec;
            break;
        }

        ++renderedFrames;

        if (g_requestExit)
            done = true;
        if (frameBudget > 0 && renderedFrames >= frameBudget)
            done = true;
    }

    // Capture the last known good state before shutting ImGui down.
    const bool atlasBuilt = fontBuilt;
    const int  framesRendered = renderedFrames;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    // --- assertions --------------------------------------------------------
    std::cout << "\n--- checks -------------------------------------------------\n";
    EXPECT_TRUE(framesRendered > 0);
    EXPECT_TRUE(displaySize.x > 0.0f && displaySize.y > 0.0f);
    EXPECT_TRUE(atlasBuilt);
    EXPECT_TRUE(drawLists > 0);
    EXPECT_TRUE(drawVertices > 0);
    EXPECT_TRUE(g_tableRows >= 5);          // 5 rows x at least one frame
    EXPECT_TRUE(g_storageValue == 1337);
    EXPECT_TRUE(g_textSize.x > 0.0f && g_textSize.y > 0.0f);
    EXPECT_TRUE(g_dockBuilt);
    EXPECT_TRUE(g_popupOpenCount >= 1);
    EXPECT_TRUE(g_frameIndex >= framesRendered);

    std::cout << "frames=" << framesRendered
              << " fontsBuilt=" << (atlasBuilt ? 1 : 0)
              << " drawLists=" << drawLists
              << " verts=" << drawVertices
              << " tableRows=" << g_tableRows
              << " storage=" << g_storageValue
              << " dock=" << (g_dockBuilt ? 1 : 0)
              << " popupOpens=" << g_popupOpenCount
              << "\n";

    if (GetFailedCount() > 0)
    {
        std::cerr << "SMOKE TEST FAILED (" << GetFailedCount() << " check(s))\n";
        return 1;
    }

    std::cout << "SMOKE TEST PASSED\n";
    return 0;
}
