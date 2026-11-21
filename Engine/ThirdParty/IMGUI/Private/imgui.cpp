// dear imgui, v1.92.8 WIP
// (main code and documentation)

// Help:
// - Call and read ImGui::ShowDemoWindow() in imgui_demo.cpp. All applications in examples/ are doing that.
// - Read top of imgui.cpp for more details, links and comments.
// - Add '#define IMGUI_DEFINE_MATH_OPERATORS' before including imgui.h (or in imconfig.h) to access courtesy maths operators for ImVec2 and ImVec4.

// Resources:
// - FAQ ........................ https://dearimgui.com/faq (in repository as docs/FAQ.md)
// - Homepage ................... https://github.com/ocornut/imgui
// - Releases & Changelog ....... https://github.com/ocornut/imgui/releases
// - Gallery .................... https://github.com/ocornut/imgui/issues?q=label%3Agallery (please post your screenshots/video there!)
// - Wiki ....................... https://github.com/ocornut/imgui/wiki (lots of good stuff there)
//   - Getting Started            https://github.com/ocornut/imgui/wiki/Getting-Started (how to integrate in an existing app by adding ~25 lines of code)
//   - Third-party Extensions     https://github.com/ocornut/imgui/wiki/Useful-Extensions (ImPlot & many more)
//   - Bindings/Backends          https://github.com/ocornut/imgui/wiki/Bindings (language bindings + backends for various tech/engines)
//   - Debug Tools                https://github.com/ocornut/imgui/wiki/Debug-Tools
//   - Glossary                   https://github.com/ocornut/imgui/wiki/Glossary
//   - Software using Dear ImGui  https://github.com/ocornut/imgui/wiki/Software-using-dear-imgui
// - Issues & support ........... https://github.com/ocornut/imgui/issues
// - Test Engine & Automation ... https://github.com/ocornut/imgui_test_engine (test suite, test engine to automate your apps)
// - Web version of the Demo .... https://pthom.github.io/imgui_explorer (w/ source code browser)

// For FIRST-TIME users having issues compiling/linking/running:
// please post in https://github.com/ocornut/imgui/discussions if you cannot find a solution in resources above.
// Everything else should be asked in 'Issues'! We are building a database of cross-linked knowledge there.
// Since 1.92, we encourage font loading questions to also be posted in 'Issues'.

// Copyright (c) 2014-2026 Omar Cornut
// Developed by Omar Cornut and every direct or indirect contributors to the GitHub.
// See LICENSE.txt for copyright and licensing details (standard MIT License).
// This library is free but needs your support to sustain development and maintenance.
// Businesses: you can support continued development via B2B invoiced technical support, maintenance and sponsoring contracts.
// PLEASE reach out at omar AT dearimgui DOT com. See https://github.com/ocornut/imgui/wiki/Funding
// Businesses: you can also purchase licenses for the Dear ImGui Automation/Test Engine.

// It is recommended that you don't modify imgui.cpp! It will become difficult for you to update the library.
// Note that 'ImGui::' being a namespace, you can add functions into the namespace from your own source files, without
// modifying imgui.h or imgui.cpp. You may include imgui_internal.h to access internal data structures, but it doesn't
// come with any guarantee of forward compatibility. Discussing your changes on the GitHub Issue Tracker may lead you
// to a better solution or official support for them.

/*

Index of this file:

DOCUMENTATION

- MISSION STATEMENT
- CONTROLS GUIDE
- PROGRAMMER GUIDE
  - READ FIRST
  - HOW TO UPDATE TO A NEWER VERSION OF DEAR IMGUI
  - GETTING STARTED WITH INTEGRATING DEAR IMGUI IN YOUR CODE/ENGINE
  - HOW A SIMPLE APPLICATION MAY LOOK LIKE
  - USING CUSTOM BACKEND / CUSTOM ENGINE
- API BREAKING CHANGES (read me when you update!)
- FREQUENTLY ASKED QUESTIONS (FAQ)
  - Read all answers online: https://www.dearimgui.com/faq, or in docs/FAQ.md (with a Markdown viewer)

CODE
(search for "[SECTION]" in the code to find them)

// [SECTION] INCLUDES
// [SECTION] FORWARD DECLARATIONS
// [SECTION] CONTEXT AND MEMORY ALLOCATORS
// [SECTION] USER FACING STRUCTURES (ImGuiStyle, ImGuiIO, ImGuiPlatformIO)
// [SECTION] MISC HELPERS/UTILITIES (Geometry functions)
// [SECTION] MISC HELPERS/UTILITIES (String, Format, Hash functions)
// [SECTION] MISC HELPERS/UTILITIES (File functions)
// [SECTION] MISC HELPERS/UTILITIES (ImText* functions)
// [SECTION] MISC HELPERS/UTILITIES (Color functions)
// [SECTION] ImGuiStorage
// [SECTION] ImGuiTextFilter
// [SECTION] ImGuiTextBuffer, ImGuiTextIndex
// [SECTION] ImGuiListClipper
// [SECTION] STYLING
// [SECTION] RENDER HELPERS
// [SECTION] INITIALIZATION, SHUTDOWN
// [SECTION] MAIN CODE (most of the code! lots of stuff, needs tidying up!)
// [SECTION] FONTS, TEXTURES
// [SECTION] ID STACK
// [SECTION] INPUTS
// [SECTION] ERROR CHECKING, STATE RECOVERY
// [SECTION] ITEM SUBMISSION
// [SECTION] LAYOUT
// [SECTION] SCROLLING
// [SECTION] TOOLTIPS
// [SECTION] POPUPS
// [SECTION] WINDOW FOCUS
// [SECTION] KEYBOARD/GAMEPAD NAVIGATION
// [SECTION] DRAG AND DROP
// [SECTION] LOGGING/CAPTURING
// [SECTION] SETTINGS
// [SECTION] LOCALIZATION
// [SECTION] VIEWPORTS, PLATFORM WINDOWS
// [SECTION] DOCKING
// [SECTION] PLATFORM DEPENDENT HELPERS
// [SECTION] METRICS/DEBUGGER WINDOW
// [SECTION] DEBUG LOG WINDOW
// [SECTION] OTHER DEBUG TOOLS (ITEM PICKER, ID STACK TOOL)

*/

//-----------------------------------------------------------------------------
// DOCUMENTATION
//-----------------------------------------------------------------------------

/*

 MISSION STATEMENT
 =================

 - Easy to use to create code-driven and data-driven tools.
 - Easy to use to create ad hoc short-lived tools and long-lived, more elaborate tools.
 - Easy to hack and improve.
 - Minimize setup and maintenance.
 - Minimize state storage on user side.
 - Minimize state synchronization.
 - Portable, minimize dependencies, run on target (consoles, phones, etc.).
 - Efficient runtime and memory consumption.

 Designed primarily for developers and content-creators, not the typical end-user!
 Some of the current weaknesses (which we aim to address in the future) includes:

 - Doesn't look fancy by default.
 - Limited layout features, intricate layouts are typically crafted in code.


 CONTROLS GUIDE
 ==============

 - MOUSE CONTROLS
   - Mouse wheel:                   Scroll vertically.
   - Shift+Mouse wheel:             Scroll horizontally.
   - Click [X]:                     Close a window, available when 'bool* p_open' is passed to ImGui::Begin().
   - Click ^, Double-Click title:   Collapse window.
   - Drag on corner/border:         Resize window (double-click to auto fit window to its contents).
   - Drag on any empty space:       Move window (unless io.ConfigWindowsMoveFromTitleBarOnly = true).
   - Left-click outside popup:      Close popup stack (right-click over underlying popup: Partially close popup stack).

 - TEXT EDITOR
   - Hold Shift or Drag Mouse:      Select text.
   - Ctrl+Left/Right:               Word jump.
   - Ctrl+Shift+Left/Right:         Select words.
   - Ctrl+A or Double-Click:        Select All.
   - Ctrl+X, Ctrl+C, Ctrl+V:        Use OS clipboard.
   - Ctrl+Z                         Undo.
   - Ctrl+Y or Ctrl+Shift+Z:        Redo.
   - ESCAPE:                        Revert text to its original value.
   - On macOS, controls are automatically adjusted to match standard macOS text editing and behaviors.
     (for 99% of shortcuts, Ctrl is replaced by Cmd on macOS).

 - KEYBOARD CONTROLS
   - Basic:
     - Tab, Shift+Tab               Cycle through text editable fields.
     - Ctrl+Tab, Ctrl+Shift+Tab     Cycle through windows.
     - Ctrl+Click                   Input text into a Slider or Drag widget.
   - Extended features with `io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard`:
     - Tab, Shift+Tab:              Cycle through every items.
     - Arrow keys                   Move through items using directional navigation. Tweak value.
     - Arrow keys + Alt, Shift      Tweak slower, tweak faster (when using arrow keys).
     - Enter                        Activate item (prefer text input when possible).
     - Space                        Activate item (prefer tweaking with arrows when possible).
     - Escape                       Deactivate item, leave child window, close popup.
     - Page Up, Page Down           Previous page, next page.
     - Home, End                    Scroll to top, scroll to bottom.
     - Alt                          Toggle between scrolling layer and menu layer.
     - Ctrl+Tab then Ctrl+Arrows    Move window. Hold Shift to resize instead of moving.
     - Menu or Shift+F10            Open context menu.
   - Output when ImGuiConfigFlags_NavEnableKeyboard set,
     - io.WantCaptureKeyboard flag is set when keyboard is claimed.
     - io.NavActive: true when a window is focused and it doesn't have the ImGuiWindowFlags_NoNavInputs flag set.
     - io.NavVisible: true when the navigation cursor is visible (usually goes to back false when mouse is used).

 - GAMEPAD CONTROLS
   - Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
   - Particularly useful to use Dear ImGui on a console system (e.g. PlayStation, Switch, Xbox) without a mouse!
   - Download controller mapping PNG/PSD at http://dearimgui.com/controls_sheets
   - Backend support: backend needs to:
      - Set 'io.BackendFlags |= ImGuiBackendFlags_HasGamepad' + call io.AddKeyEvent/AddKeyAnalogEvent() with ImGuiKey_Gamepad_XXX keys.
      - For analog values (0.0f to 1.0f), backend is responsible to handling a dead-zone and rescaling inputs accordingly.
        Backend code will probably need to transform your raw inputs (such as e.g. remapping your 0.2..0.9 raw input range to 0.0..1.0 imgui range, etc.).
   - If you need to share inputs between your game and the Dear ImGui interface, the easiest approach is to go all-or-nothing,
     with a buttons combo to toggle the target. Please reach out if you think the game vs navigation input sharing could be improved.

 - REMOTE INPUTS SHARING & MOUSE EMULATION
   - PS4/PS5 users: Consider emulating a mouse cursor with DualShock touch pad or a spare analog stick as a mouse-emulation fallback.
   - Consoles/Tablet/Phone users: Consider using a Synergy 1.x server (on your PC) + run examples/libs/synergy/uSynergy.c (on your console/tablet/phone app)
     in order to share your PC mouse/keyboard.
   - See https://github.com/ocornut/imgui/wiki/Useful-Extensions#remoting for other remoting solutions.
   - On a TV/console system where readability may be lower or mouse inputs may be awkward, you may want to set the io.ConfigNavMoveSetMousePos flag.
     Enabling io.ConfigNavMoveSetMousePos + ImGuiBackendFlags_HasSetMousePos instructs Dear ImGui to move your mouse cursor along with navigation movements.
     When enabled, the NewFrame() function may alter 'io.MousePos' and set 'io.WantSetMousePos' to notify you that it wants the mouse cursor to be moved.
     When that happens your backend NEEDS to move the OS or underlying mouse cursor on the next frame. Some of the backends in examples/ do that.
     (If you set the NavEnableSetMousePos flag but don't honor 'io.WantSetMousePos' properly, Dear ImGui will misbehave as it will see your mouse moving back & forth!)
     (In a setup when you may not have easy control over the mouse cursor, e.g. uSynergy.c doesn't expose moving remote mouse cursor, you may want
     to set a boolean to ignore your other external mouse positions until the external source is moved again.)


 PROGRAMMER GUIDE
 ================

 READ FIRST
 ----------
 - Remember to check the wonderful Wiki: https://github.com/ocornut/imgui/wiki
 - Your code creates the UI every frame of your application loop, if your code doesn't run the UI is gone!
   The UI can be highly dynamic, there are no construction or destruction steps, less superfluous
   data retention on your side, less state duplication, less state synchronization, fewer bugs.
 - Call and read ImGui::ShowDemoWindow() for demo code demonstrating most features.
   Or browse pthom's online imgui_explorer: https://pthom.github.io/imgui_explorer for a web version w/ source code browser.
 - The library is designed to be built from sources. Avoid pre-compiled binaries and packaged versions. See imconfig.h to configure your build.
 - Dear ImGui is an implementation of the IMGUI paradigm (immediate-mode graphical user interface, a term coined by Casey Muratori).
   You can learn about IMGUI principles at http://www.johno.se/book/imgui.html, http://mollyrocket.com/861 & more links in Wiki.
 - Dear ImGui is a "single pass" rasterizing implementation of the IMGUI paradigm, aimed at ease of use and high-performances.
   For every application frame, your UI code will be called only once. This is in contrast to e.g. Unity's implementation of an IMGUI,
   where the UI code is called multiple times ("multiple passes") from a single entry point. There are pros and cons to both approaches.
 - Our origin is on the top-left. In axis aligned bounding boxes, Min = top-left, Max = bottom-right.
 - Please make sure you have asserts enabled (IM_ASSERT redirects to assert() by default, but can be redirected).
   If you get an assert, read the messages and comments around the assert.
 - This codebase aims to be highly optimized:
   - A typical idle frame should never call malloc/free.
   - We rely on a maximum of constant-time or O(N) algorithms. Limiting searches/scans as much as possible.
   - We put particular energy in making sure performances are decent with typical "Debug" build settings as well.
     Which mean we tend to avoid over-relying on "zero-cost abstraction" as they aren't zero-cost at all.
 - This codebase aims to be both highly opinionated and highly flexible:
   - This code works because of the things it choose to solve or not solve.
   - C++: this is a pragmatic C-ish codebase: we don't use fancy C++ features, we don't include C++ headers,
     and ImGui:: is a namespace. We rarely use member functions (and when we did, I am mostly regretting it now).
     This is to increase compatibility, increase maintainability and facilitate use from other languages.
   - C++: ImVec2/ImVec4 do not expose math operators by default, because it is expected that you use your own math types.
     See FAQ "How can I use my own math types instead of ImVec2/ImVec4?" for details about setting up imconfig.h for that.
     We can can optionally export math operators for ImVec2/ImVec4 using IMGUI_DEFINE_MATH_OPERATORS, which we use internally.
   - C++: pay attention that ImVector<> manipulates plain-old-data and does not honor construction/destruction
     (so don't use ImVector in your code or at our own risk!).
   - Building: We don't use nor mandate a build system for the main library.
     This is in an effort to ensure that it works in the real world aka with any esoteric build setup.
     This is also because providing a build system for the main library would be of little-value.
     The build problems are almost never coming from the main library but from specific backends.


 HOW TO UPDATE TO A NEWER VERSION OF DEAR IMGUI
 ----------------------------------------------
 - Update submodule or copy/overwrite every file.
 - About imconfig.h:
   - You may modify your copy of imconfig.h, in this case don't overwrite it.
   - or you may locally branch to modify imconfig.h and merge/rebase latest.
   - or you may '#define IMGUI_USER_CONFIG "my_config_file.h"' globally from your build system to
     specify a custom path for your imconfig.h file and instead not have to modify the default one.

 - Overwrite all the sources files except for imconfig.h (if you have modified your copy of imconfig.h)
 - Or maintain your own branch where you have imconfig.h modified as a top-most commit which you can regularly rebase over "master".
 - You can also use '#define IMGUI_USER_CONFIG "my_config_file.h" to redirect configuration to your own file.
 - Read the "API BREAKING CHANGES" section (below). This is where we list occasional API breaking changes.
   If a function/type has been renamed / or marked obsolete, try to fix the name in your code before it is permanently removed
   from the public API. If you have a problem with a missing function/symbols, search for its name in the code, there will
   likely be a comment about it. Please report any issue to the GitHub page!
 - To find out usage of old API, you can add '#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS' in your configuration file.
 - Try to keep your copy of Dear ImGui reasonably up to date!


 GETTING STARTED WITH INTEGRATING DEAR IMGUI IN YOUR CODE/ENGINE
 ---------------------------------------------------------------
 - See https://github.com/ocornut/imgui/wiki/Getting-Started.
 - Run and study the examples and demo in imgui_demo.cpp to get acquainted with the library.
 - In the majority of cases you should be able to use unmodified backends files available in the backends/ folder.
 - Add the Dear ImGui source files + selected backend source files to your projects or using your preferred build system.
   It is recommended you build and statically link the .cpp files as part of your project and NOT as a shared library (DLL).
 - You can later customize the imconfig.h file to tweak some compile-time behavior, such as integrating Dear ImGui types with your own maths types.
 - When using Dear ImGui, your programming IDE is your friend: follow the declaration of variables, functions and types to find comments about them.
 - Dear ImGui never touches or knows about your GPU state. The only function that knows about GPU is the draw function that you provide.
   Effectively it means you can create widgets at any time in your code, regardless of considerations of being in "update" vs "render"
   phases of your own application. All rendering information is stored into command-lists that you will retrieve after calling ImGui::Render().
 - Refer to the backends and demo applications in the examples/ folder for instruction on how to setup your code.
 - If you are running over a standard OS with a common graphics API, you should be able to use unmodified imgui_impl_*** files from the examples/ folder.


 HOW A SIMPLE APPLICATION MAY LOOK LIKE
 --------------------------------------

 USING THE EXAMPLE BACKENDS (= imgui_impl_XXX.cpp files from the backends/ folder).
 The sub-folders in examples/ contain examples applications following this structure.

     // Application init: create a dear imgui context, setup some options, load fonts
     ImGui::CreateContext();
     ImGuiIO& io = ImGui::GetIO();
     // TODO: Set optional io.ConfigFlags values, e.g. 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard' to enable keyboard controls.
     // TODO: Fill optional fields of the io structure later.
     // TODO: Load TTF/OTF fonts if you don't want to use the default font.

     // Initialize helper Platform and Renderer backends (here we are using imgui_impl_win32.cpp and imgui_impl_dx11.cpp)
     ImGui_ImplWin32_Init(hwnd);
     ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

     // Application main loop
     while (true)
     {
         // Feed inputs to dear imgui, start new frame
         ImGui_ImplDX11_NewFrame();
         ImGui_ImplWin32_NewFrame();
         ImGui::NewFrame();

         // Any application code here
         ImGui::Text("Hello, world!");

         // Render dear imgui into framebuffer
         ImGui::Render();
         ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
         g_pSwapChain->Present(1, 0);
     }

     // Shutdown
     ImGui_ImplDX11_Shutdown();
     ImGui_ImplWin32_Shutdown();
     ImGui::DestroyContext();

 To decide whether to dispatch mouse/keyboard inputs to Dear ImGui to the rest of your application,
 you should read the 'io.WantCaptureMouse', 'io.WantCaptureKeyboard' and 'io.WantTextInput' flags!
 Please read the FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?" about this.


USING CUSTOM BACKEND / CUSTOM ENGINE
------------------------------------

IMPLEMENTING YOUR PLATFORM BACKEND:
 -> see https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md for basic instructions.
 -> the Platform backends in impl_impl_XXX.cpp files contain many implementations.

IMPLEMENTING YOUR RenderDrawData() function:
 -> see https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md
 -> the Renderer Backends in impl_impl_XXX.cpp files contain many implementations of a ImGui_ImplXXXX_RenderDrawData() function.

IMPLEMENTING SUPPORT for ImGuiBackendFlags_RendererHasTextures:
 -> see https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md
 -> the Renderer Backends in impl_impl_XXX.cpp files contain many implementations of a ImGui_ImplXXXX_UpdateTexture() function.

 Basic application/backend skeleton:

     // Application init: create a Dear ImGui context, setup some options, load fonts
     ImGui::CreateContext();
     ImGuiIO& io = ImGui::GetIO();
     // TODO: set io.ConfigXXX values, e.g.
     io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls

     // TODO: Load TTF/OTF fonts if you don't want to use the default font.
     io.Fonts->AddFontFromFileTTF("NotoSans.ttf");

     // Application main loop
     while (true)
     {
        // Setup low-level inputs, e.g. on Win32: calling GetKeyboardState(), or write to those fields from your Windows message handlers, etc.
        // (In the examples/ app this is usually done within the ImGui_ImplXXX_NewFrame() function from one of the demo Platform Backends)
        io.DeltaTime = 1.0f/60.0f;              // set the time elapsed since the previous frame (in seconds)
        io.DisplaySize.x = 1920.0f;             // set the current display width
        io.DisplaySize.y = 1280.0f;             // set the current display height here
        io.AddMousePosEvent(mouse_x, mouse_y);  // update mouse position
        io.AddMouseButtonEvent(0, mouse_b[0]);  // update mouse button states
        io.AddMouseButtonEvent(1, mouse_b[1]);  // update mouse button states

        // Call NewFrame(), after this point you can use ImGui::* functions anytime
        // (So you want to try calling NewFrame() as early as you can in your main loop to be able to use Dear ImGui everywhere)
        ImGui::NewFrame();

        // Most of your application code here
        ImGui::Text("Hello, world!");
        MyGameUpdate(); // may use any Dear ImGui functions, e.g. ImGui::Begin("My window"); ImGui::Text("Hello, world!"); ImGui::End();
        MyGameRender(); // may use any Dear ImGui functions as well!

        // End the dear imgui frame
        // (You want to try calling EndFrame/Render as late as you can, to be able to use Dear ImGui in your own game rendering code)
        ImGui::EndFrame(); // this is automatically called by Render(), but available
        ImGui::Render();

        // Update textures
        ImDrawData* draw_data = ImGui::GetDrawData();
        for (ImTextureData* tex : *draw_data->Textures)
            if (tex->Status != ImTextureStatus_OK)
                MyImGuiBackend_UpdateTexture(tex);

        // Render dear imgui contents, swap buffers
        MyImGuiBackend_RenderDrawData(draw_data);
        SwapBuffers();
     }

     // Shutdown
     ImGui::DestroyContext();



 API BREAKING CHANGES
 ====================

 Occasionally introducing changes that are breaking the API. We try to make the breakage minor and easy to fix.
 Below is a change-log of API breaking changes only. If you are using one of the functions listed, expect to have to fix some code.
 When you are not sure about an old symbol or function name, try using the Search/Find function of your IDE to look for comments or references in all imgui files.
 You can read releases logs https://github.com/ocornut/imgui/releases for more details.

(Docking/Viewport Branch)
 - 2026/XX/XX (1.XXXX) - when multi-viewports are enabled, all positions will be in your natural OS coordinates space. It means that:
                          - reference to hard-coded positions such as in SetNextWindowPos(ImVec2(0,0)) are probably not what you want anymore.
                            you may use GetMainViewport()->Pos to offset hard-coded positions, e.g. SetNextWindowPos(GetMainViewport()->Pos)
                          - likewise io.MousePos and GetMousePos() will use OS coordinates.
                            If you query mouse positions to interact with non-imgui coordinates you will need to offset them, e.g. subtract GetWindowViewport()->Pos.

 - 2026/04/23 (1.92.8) - Obsoleted `ImDrawCallback_ResetRenderState` in favor of using `ImGui::GetPlatformIO().DrawCallback_ResetRenderState`, which is part of our new standard draw callbacks. (#9378)
 - 2026/04/22 (1.92.8) - Backends: Vulkan: redesigned to use separate ImageView + Sampler instead of Combined Image Sampler.
                         - When registering custom textures: changed ImGui_ImplVulkan_AddTexture() signature to remove Sampler.
                         - When creating your own descriptor pool (instead of letting backend creates its own): need at least IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE descriptors of type VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE + IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE descriptors of type VK_DESCRIPTOR_TYPE_SAMPLER.
 - 2026/03/19 (1.92.7) - MultiSelect: renamed ImGuiMultiSelectFlags_SelectOnClick to ImGuiMultiSelectFlags_SelectOnAuto.
 - 2026/02/26 (1.92.7) - Separator: fixed a legacy quirk where Separator() was submitting a zero-height item for layout purpose, even though it draws a 1-pixel separator.
                         The fix could affect code e.g. computing height from multiple widgets in order to allocate vertical space for a footer or multi-line status bar. (#2657, #9263)
                         The "Console" example had such a bug:
                            float footer_height = style.ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
                            BeginChild("ScrollingRegion", { 0, -footer_height });
                         Should be:
                            float footer_height = style.ItemSpacing.y + style.SeparatorSize + ImGui::GetFrameHeightWithSpacing();
                            BeginChild("ScrollingRegion", { 0, -footer_height });
                         When such idiom was used and assuming zero-height Separator, it is likely that in 1.92.7 the resulting window will have unexpected 1 pixel scrolling range.
 - 2026/02/23 (1.92.7) - Commented out legacy signature for Combo(), ListBox(), signatures which were obsoleted in 1.90 (Nov 2023), when the getter callback type was changed.
                         - Old getter type:   bool (*getter)(void* user_data, int idx, const char** out_text)   // Set label + return bool. False replaced label with placeholder.
                         - New getter type:   const char* (*getter)(void* user_data, int idx)                   // Return label or NULL/empty label if missing
 - 2026/01/08 (1.92.6) - Commented out legacy names obsoleted in 1.90 (Sept 2023): 'BeginChildFrame()' --> 'BeginChild()' with 'ImGuiChildFlags_FrameStyle'. 'EndChildFrame()' --> 'EndChild()'. 'ShowStackToolWindow()' --> 'ShowIDStackToolWindow()'. 'IM_OFFSETOF()' --> 'offsetof()'.
 - 2026/01/07 (1.92.6) - Popups: changed compile-time 'ImGuiPopupFlags popup_flags = 1' default value to be '= 0' for BeginPopupContextItem(), BeginPopupContextWindow(), BeginPopupContextVoid(), OpenPopupOnItemClick(). Default value has same meaning before and after.
                         - Refer to GitHub topic #9157 if you have any question.
                         - Before this version, those functions had a 'ImGuiPopupFlags popup_flags = 1' default value in their function signature.
                           Explicitly passing a literal 0 meant ImGuiPopupFlags_MouseButtonLeft. The default literal 1 meant ImGuiPopupFlags_MouseButtonRight.
                           This was introduced by a change on 2020/06/23 (1.77) while changing the signature from 'int mouse_button' to 'ImGuiPopupFlags popup_flags' and trying to preserve then-legacy behavior.
                           We have now changed this behavior to cleanup a very old API quirk, facilitate use by bindings, and to remove the last and error-prone non-zero default value.
                           Also because we deemed it extremely rare to use those helper functions with the Left mouse button! As using the LMB would generally be triggered via another widget, e.g. a Button() + a OpenPopup()/BeginPopup() call.
                         - Before: The default = 1 means ImGuiPopupFlags_MouseButtonRight. Explicitly passing a literal 0 means ImGuiPopupFlags_MouseButtonLeft.
                         - After:  The default = 0 means ImGuiPopupFlags_MouseButtonRight. Explicitly passing a literal 1 also means ImGuiPopupFlags_MouseButtonRight (if legacy behavior are enabled) or will assert (if legacy behavior are disabled).
                         - TL;DR: if you don't want to use right mouse button for popups, always specify it explicitly using a named ImGuiPopupFlags_MouseButtonXXXX value.
                         Recap:
                         - BeginPopupContextItem("foo");                                         // Behavior unchanged (use Right button)
                         - BeginPopupContextItem("foo", ImGuiPopupFlags_MouseButtonLeft);        // Behavior unchanged (use Left button)
                         - BeginPopupContextItem("foo", ImGuiPopupFlags_MouseButtonLeft | xxx);  // Behavior unchanged (use Left button + flags)
                         - BeginPopupContextItem("foo", ImGuiPopupFlags_MouseButtonRight | xxx); // Behavior unchanged (use Right button + flags)
                         - BeginPopupContextItem("foo", 1);                                      // Behavior unchanged (as a courtesy we legacy interpret 1 as ImGuiPopupFlags_MouseButtonRight, will assert if disabling legacy behaviors.
                         - BeginPopupContextItem("foo", 0);                                      // !! Behavior changed !! Was Left button. Now will defaults to Right button! --> Use ImGuiPopupFlags_MouseButtonLeft.
                         - BeginPopupContextItem("foo", ImGuiPopupFlags_NoReopen);               // !! Behavior changed !! Was Left button + flags. Now will defaults to Right button! --> Use ImGuiPopupFlags_MouseButtonLeft | xxx.
 - 2025/12/23 (1.92.6) - Fonts: AddFontDefault() now automatically selects an embedded font between the new scalable AddFontDefaultVector() and the classic pixel-clean AddFontDefaultBitmap().
                         The default selection is based on (style.FontSizeBase * FontScaleMain * FontScaleDpi) reaching a small threshold, but old codebases may not set any of them properly. As as a result, it is likely that old codebase may still default to AddFontDefaultBitmap().
                         Prefer calling either based on your own logic. You can call AddFontDefaultBitmap() to ensure legacy behavior.
 - 2025/12/23 (1.92.6) - Fonts: removed ImFontConfig::PixelSnapV added in 1.92 which turns out is unnecessary (and misdocumented). Post-rescale GlyphOffset is always rounded.
 - 2025/12/17 (1.92.6) - Renamed helper macro IM_ARRAYSIZE() -> IM_COUNTOF(). Kept redirection/legacy name for now.
 - 2025/12/11 (1.92.6) - Hashing: handling of "###" operator to reset to seed within a string identifier doesn't include the "###" characters in the output hash anymore.
                         - Before: GetID("Hello###World") == GetID("###World") != GetID("World")
                         - After:  GetID("Hello###World") == GetID("###World") == GetID("World")
                         - This has the property of facilitating concatenating and manipulating identifiers using "###", and will allow fixing other dangling issues.
                         - This will invalidate hashes (stored in .ini data) for Tables and Windows that are using the "###" operators. (#713, #1698)
 - 2025/11/24 (1.92.6) - Fonts: Fixed handling of `ImFontConfig::FontDataOwnedByAtlas = false` which did erroneously make a copy of the font data, essentially defeating the purpose of this flag and wasting memory.
                         (trivia: undetected since July 2015, this is perhaps the oldest bug in Dear ImGui history, albeit for a rarely used feature, see #9086)
                         HOWEVER, fixing this bug is likely to surface bugs in user code using `FontDataOwnedByAtlas = false`.
                         - Prior to 1.92, font data only needed to be available during the atlas->AddFontXXX() call.
                         - Since 1.92, font data needs to available until atlas->RemoveFont(), or more typically until a shutdown of the owning context or font atlas.
                         - The fact that handling of `FontDataOwnedByAtlas = false` was broken bypassed the issue altogether.
 - 2025/11/06 (1.92.5) - BeginChild: commented out some legacy names which were obsoleted in 1.90.0 (Nov 2023), 1.90.9 (July 2024), 1.91.1 (August 2024):
                         - ImGuiChildFlags_Border                    --> ImGuiChildFlags_Borders
                         - ImGuiWindowFlags_NavFlattened             --> ImGuiChildFlags_NavFlattened (moved to ImGuiChildFlags). BeginChild(name, size, 0, ImGuiWindowFlags_NavFlattened) --> BeginChild(name, size, ImGuiChildFlags_NavFlattened, 0)
                         - ImGuiWindowFlags_AlwaysUseWindowPadding   --> ImGuiChildFlags_AlwaysUseWindowPadding (moved to ImGuiChildFlags). BeginChild(name, size, 0, ImGuiWindowFlags_AlwaysUseWindowPadding) --> BeginChild(name, size, ImGuiChildFlags_AlwaysUseWindowPadding, 0)
 - 2025/11/06 (1.92.5) - Keys: commented out legacy names which were obsoleted in 1.89.0 (August 2022):
                         - ImGuiKey_ModCtrl  --> ImGuiMod_Ctrl
                         - ImGuiKey_ModShift --> ImGuiMod_Shift
                         - ImGuiKey_ModAlt   --> ImGuiMod_Alt
                         - ImGuiKey_ModSuper --> ImGuiMod_Super
 - 2025/11/06 (1.92.5) - IO: commented out legacy io.ClearInputCharacters() obsoleted in 1.89.8 (Aug 2023). Calling io.ClearInputKeys() is enough.
 - 2025/11/06 (1.92.5) - Commented out legacy SetItemAllowOverlap() obsoleted in 1.89.7: this never worked right. Use SetNextItemAllowOverlap() _before_ item instead.
 - 2025/10/14 (1.92.4) - TreeNode, Selectable, Clipper: commented out legacy names which were obsoleted in 1.89.7 (July 2023) and 1.89.9 (Sept 2023);
                         - ImGuiTreeNodeFlags_AllowItemOverlap       --> ImGuiTreeNodeFlags_AllowOverlap
                         - ImGuiSelectableFlags_AllowItemOverlap     --> ImGuiSelectableFlags_AllowOverlap
                         - ImGuiListClipper::IncludeRangeByIndices() --> ImGuiListClipper::IncludeItemsByIndex()
 - 2025/09/22 (1.92.4) - Viewports: renamed io.ConfigViewportPlatformFocusSetsImGuiFocus to io.ConfigViewportsPlatformFocusSetsImGuiFocus. Was a typo in the first place. (#6299, #6462)
 - 2025/08/08 (1.92.2) - Backends: SDL_GPU3: Changed ImTextureID type from SDL_GPUTextureSamplerBinding* to SDL_GPUTexture*, which is more natural and easier for user to manage. If you need to change the current sampler, you can access the ImGui_ImplSDLGPU3_RenderState struct. (#8866, #8163, #7998, #7988)
 - 2025/07/31 (1.92.2) - Tabs: Renamed ImGuiTabBarFlags_FittingPolicyResizeDown to ImGuiTabBarFlags_FittingPolicyShrink. Kept inline redirection enum (will obsolete).
 - 2025/06/25 (1.92.0) - Layout: commented out legacy ErrorCheckUsingSetCursorPosToExtendParentBoundaries() fallback obsoleted in 1.89 (August 2022) which allowed a SetCursorPos()/SetCursorScreenPos() call WITHOUT AN ITEM
                         to extend parent window/cell boundaries. Replaced with assert/tooltip that would already happens if previously using IMGUI_DISABLE_OBSOLETE_FUNCTIONS. (#5548, #4510, #3355, #1760, #1490, #4152, #150)
                         - Incorrect way to make a window content size 200x200:
                              Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + ImVec2(200,200)) + End();
                         - Correct ways to make a window content size 200x200:
                              Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + ImVec2(200,200)) + Dummy(ImVec2(0,0)) + End();
                              Begin(...) + Dummy(ImVec2(200,200)) + End();
                         - TL;DR; if the assert triggers, you can add a Dummy({0,0}) call to validate extending parent boundaries.
 - 2025/06/11 (1.92.0) - Renamed/moved ImGuiConfigFlags_DpiEnableScaleFonts -> bool io.ConfigDpiScaleFonts.
                       - Renamed/moved ImGuiConfigFlags_DpiEnableScaleViewports -> bool io.ConfigDpiScaleViewports. **Neither of those flags are very useful in current code. They will be useful once we merge font changes.**
                         [there was a bug on 2025/06/12: when using the old config flags names, they were not imported correctly into the new ones, fixed on 2025/09/12]
 - 2025/06/11 (1.92.0) - THIS VERSION CONTAINS THE LARGEST AMOUNT OF BREAKING CHANGES SINCE 2015! I TRIED REALLY HARD TO KEEP THEM TO A MINIMUM, REDUCE THE AMOUNT OF INTERFERENCES, BUT INEVITABLY SOME USERS WILL BE AFFECTED.
                         IN ORDER TO HELP US IMPROVE THE TRANSITION PROCESS, INCL. DOCUMENTATION AND COMMENTS, PLEASE REPORT **ANY** DOUBT, CONFUSION, QUESTIONS, FEEDBACK TO: https://github.com/ocornut/imgui/issues/
                         As part of the plan to reduce impact of API breaking changes, several unfinished changes/features/refactors related to font and text systems and scaling will be part of subsequent releases (1.92.1+).
                         If you are updating from an old version, and expecting a massive or difficult update, consider first updating to 1.91.9 to reduce the amount of changes.
                       - Hard to read? Refer to 'docs/Changelog.txt' for a less compact and more complete version of this!
                       - Fonts: **IMPORTANT**: if your app was solving the OSX/iOS Retina screen specific logical vs display scale problem by setting io.DisplayFramebufferScale (e.g. to 2.0f) + setting io.FontGlobalScale (e.g. to 1.0f/2.0f) + loading fonts at scaled sizes (e.g. size X * 2.0f):
                         This WILL NOT map correctly to the new system! Because font will rasterize as requested size.
                         - With a legacy backend (< 1.92): Instead of setting io.FontGlobalScale = 1.0f/N -> set ImFontCfg::RasterizerDensity = N. This already worked before, but is now pretty much required.
                         - With a new backend (1.92+): This should be all automatic. FramebufferScale is automatically used to set current font RasterizerDensity. FramebufferScale is a per-viewport property provided by backend through the Platform_GetWindowFramebufferScale() handler in 'docking' branch.
                       - Fonts: **IMPORTANT** on Font Sizing: Before 1.92, fonts were of a single size. They can now be dynamically sized.
                         - PushFont() API now has a REQUIRED size parameter.
                         - Before 1.92: PushFont() always used font "default" size specified in AddFont() call. It is equivalent to calling PushFont(font, font->LegacySize).
                         - Since  1.92: PushFont(font, 0.0f) preserve the current font size which is a shared value.
                         - To use old behavior: use 'ImGui::PushFont(font, font->LegacySize)' at call site.
                         - Kept inline single parameter function. Will obsolete.
                       - Fonts: **IMPORTANT** on Font Merging:
                         - When searching for a glyph in multiple merged fonts: we search for the FIRST font source which contains the desired glyph.
                           Because the user doesn't need to provide glyph ranges any more, it is possible that a glyph that you expected to fetch from a secondary/merged icon font may be erroneously fetched from the primary font.
                         - When searching for a glyph in multiple merged fonts: we now search for the FIRST font source which contains the desired glyph. This is technically a different behavior than before!
                         - e.g. If you are merging fonts you may have glyphs that you expected to load from Font Source 2 which exists in Font Source 1.
                           After the update and when using a new backend, those glyphs may now loaded from Font Source 1!
                         - We added `ImFontConfig::GlyphExcludeRanges[]` to specify ranges to exclude from a given font source:
                             // Add Font Source 1 but ignore ICON_MIN_FA..ICON_MAX_FA range
                             static ImWchar exclude_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
                             ImFontConfig cfg1;
                             cfg1.GlyphExcludeRanges = exclude_ranges;
                             io.Fonts->AddFontFromFileTTF("segoeui.ttf", 0.0f, &cfg1);
                             // Add Font Source 2, which expects to use the range above
                             ImFontConfig cfg2;
                             cfg2.MergeMode = true;
                             io.Fonts->AddFontFromFileTTF("FontAwesome4.ttf", 0.0f, &cfg2);
                         - You can use `Metrics/Debugger->Fonts->Font->Input Glyphs Overlap Detection Tool` to see list of glyphs available in multiple font sources. This can facilitate understanding which font input is providing which glyph.
                       - Fonts: **IMPORTANT** on Thread Safety:
                          - A few functions such as font->CalcTextSizeA() were, by sheer luck (== accidentally) thread-safe even though we had never provided that guarantee. They are definitively not thread-safe anymore as new glyphs may be loaded.
                       - Fonts: ImFont::FontSize was removed and does not make sense anymore. ImFont::LegacySize is the size passed to AddFont().
                       - Fonts: Removed support for PushFont(NULL) which was a shortcut for "default font".
                       - Fonts: Renamed/moved 'io.FontGlobalScale' to 'style.FontScaleMain'.
                       - Textures: all API functions taking a 'ImTextureID' parameter are now taking a 'ImTextureRef'. Affected functions are: ImGui::Image(), ImGui::ImageWithBg(), ImGui::ImageButton(), ImDrawList::AddImage(), ImDrawList::AddImageQuad(), ImDrawList::AddImageRounded().
                       - Fonts: obsoleted ImFontAtlas::GetTexDataAsRGBA32(), GetTexDataAsAlpha8(), Build(), SetTexID(), IsBuilt() functions. The new protocol for backends to handle textures doesn't need them. Kept redirection functions (will obsolete).
                       - Fonts: ImFontConfig::OversampleH/OversampleV default to automatic (== 0) since v1.91.8. It is quite important you keep it automatic until we decide if we want to provide a way to express finer policy, otherwise you will likely waste texture space when using large glyphs. Note that the imgui_freetype backend doesn't use and does not need oversampling.
                       - Fonts: specifying glyph ranges is now unnecessary. The value of ImFontConfig::GlyphRanges[] is only useful for legacy backends. All GetGlyphRangesXXXX() functions are now marked obsolete: GetGlyphRangesDefault(), GetGlyphRangesGreek(), GetGlyphRangesKorean(), GetGlyphRangesJapanese(), GetGlyphRangesChineseSimplifiedCommon(), GetGlyphRangesChineseFull(), GetGlyphRangesCyrillic(), GetGlyphRangesThai(), GetGlyphRangesVietnamese().
                       - Fonts: removed ImFontAtlas::TexDesiredWidth to enforce a texture width. (#327)
                       - Fonts: if you create and manage ImFontAtlas instances yourself (instead of relying on ImGuiContext to create one), you'll need to call ImFontAtlasUpdateNewFrame() yourself. An assert will trigger if you don't.
                       - Fonts: obsolete ImGui::SetWindowFontScale() which is not useful anymore. Prefer using 'PushFont(NULL, style.FontSizeBase * factor)' or to manipulate other scaling factors.
                       - Fonts: obsoleted ImFont::Scale which is not useful anymore.
                       - Fonts: generally reworked Internals of ImFontAtlas and ImFont. While in theory a vast majority of users shouldn't be affected, some use cases or extensions might be. Among other things:
                          - ImDrawCmd::TextureId has been changed to ImDrawCmd::TexRef.
                          - ImFontAtlas::TexID has been changed to ImFontAtlas::TexRef.
                          - ImFontAtlas::ConfigData[] has been renamed to ImFontAtlas::Sources[]
                          - ImFont::ConfigData[], ConfigDataCount has been renamed to Sources[], SourceCount.
                          - Each ImFont has a number of ImFontBaked instances corresponding to actively used sizes. ImFont::GetFontBaked(size) retrieves the one for a given size.
                          - Fields moved from ImFont to ImFontBaked: IndexAdvanceX[], Glyphs[], Ascent, Descent, FindGlyph(), FindGlyphNoFallback(), GetCharAdvance().
                          - Fields moved from ImFontAtlas to ImFontAtlas->Tex: ImFontAtlas::TexWidth => TexData->Width, ImFontAtlas::TexHeight => TexData->Height, ImFontAtlas::TexPixelsAlpha8/TexPixelsRGBA32 => TexData->GetPixels().
                          - Widget code may use ImGui::GetFontBaked() instead of ImGui::GetFont() to access font data for current font at current font size (and you may use font->GetFontBaked(size) to access it for any other size.)
                       - Fonts: (users of imgui_freetype): renamed ImFontAtlas::FontBuilderFlags to ImFontAtlas::FontLoaderFlags. Renamed ImFontConfig::FontBuilderFlags to ImFontConfig::FontLoaderFlags. Renamed ImGuiFreeTypeBuilderFlags to ImGuiFreeTypeLoaderFlags.
                         If you used runtime imgui_freetype selection rather than the default IMGUI_ENABLE_FREETYPE compile-time option: Renamed/reworked ImFontBuilderIO into ImFontLoader. Renamed ImGuiFreeType::GetBuilderForFreeType() to ImGuiFreeType::GetFontLoader().
                           - old:  io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType()
                           - new:  io.Fonts->FontLoader = ImGuiFreeType::GetFontLoader()
                           - new:  io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader()) to change dynamically at runtime [from 1.92.1]
                       - Fonts: (users of custom rectangles, see #8466): Renamed AddCustomRectRegular() to AddCustomRect(). Added GetCustomRect() as a replacement for GetCustomRectByIndex() + CalcCustomRectUV().
                           - The output type of GetCustomRect() is now ImFontAtlasRect, which include UV coordinates. X->x, Y->y, Width->w, Height->h.
                           - old:
                                const ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(custom_rect_id);
                                ImVec2 uv0, uv1;
                                atlas->GetCustomRectUV(r, &uv0, &uv1);
                                ImGui::Image(atlas->TexRef, ImVec2(r->w, r->h), uv0, uv1);
                           - new;
                                ImFontAtlasRect r;
                                atlas->GetCustomRect(custom_rect_id, &r);
                                ImGui::Image(atlas->TexRef, ImVec2(r.w, r.h), r.uv0, r.uv1);
                           - We added a redirecting typedef but haven't attempted to magically redirect the field names, as this API is rarely used and the fix is simple.
                           - Obsoleted AddCustomRectFontGlyph() as the API does not make sense for scalable fonts. Kept existing function which uses the font "default size" (Sources[0]->LegacySize). Added a helper AddCustomRectFontGlyphForSize() which is immediately marked obsolete, but can facilitate transitioning old code.
                           - Prefer adding a font source (ImFontConfig) using a custom/procedural loader.
                       - DrawList: Renamed ImDrawList::PushTextureID()/PopTextureID() to PushTexture()/PopTexture().
                       - Backends: removed ImGui_ImplXXXX_CreateFontsTexture()/ImGui_ImplXXXX_DestroyFontsTexture() for all backends that had them. They should not be necessary any more.
 - 2025/05/23 (1.92.0) - Fonts: changed ImFont::CalcWordWrapPositionA() to ImFont::CalcWordWrapPosition()
                            - old:  const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, ....);
                            - new:  const char* ImFont::CalcWordWrapPosition (float size,  const char* text, ....);
                         The leading 'float scale' parameters was changed to 'float size'. This was necessary as 'scale' is assuming standard font size which is a concept we aim to eliminate in an upcoming update. Kept inline redirection function.
 - 2025/05/15 (1.92.0) - TreeNode: renamed ImGuiTreeNodeFlags_NavLeftJumpsBackHere to ImGuiTreeNodeFlags_NavLeftJumpsToParent for clarity. Kept inline redirection enum (will obsolete).
 - 2025/05/15 (1.92.0) - Commented out PushAllowKeyboardFocus()/PopAllowKeyboardFocus() which was obsoleted in 1.89.4. Use PushItemFlag(ImGuiItemFlags_NoTabStop, !tab_stop)/PopItemFlag() instead. (#3092)
 - 2025/05/15 (1.92.0) - Commented out ImGuiListClipper::ForceDisplayRangeByIndices() which was obsoleted in 1.89.6. Use ImGuiListClipper::IncludeItemsByIndex() instead.
 - 2025/03/05 (1.91.9) - BeginMenu(): Internals: reworked mangling of menu windows to use "###Menu_00" etc. instead of "##Menu_00", allowing them to also store the menu name before it. This shouldn't affect code unless directly accessing menu window from their mangled name.
 - 2025/04/16 (1.91.9) - Internals: RenderTextEllipsis() function removed the 'float clip_max_x' parameter directly preceding 'float ellipsis_max_x'. Values were identical for a vast majority of users. (#8387)
 - 2025/02/27 (1.91.9) - Image(): removed 'tint_col' and 'border_col' parameter from Image() function. Added ImageWithBg() replacement. (#8131, #8238)
                            - old: void Image      (ImTextureID tex_id, ImVec2 image_size, ImVec2 uv0 = (0,0), ImVec2 uv1 = (1,1), ImVec4 tint_col = (1,1,1,1), ImVec4 border_col = (0,0,0,0));
                            - new: void Image      (ImTextureID tex_id, ImVec2 image_size, ImVec2 uv0 = (0,0), ImVec2 uv1 = (1,1));
                            - new: void ImageWithBg(ImTextureID tex_id, ImVec2 image_size, ImVec2 uv0 = (0,0), ImVec2 uv1 = (1,1), ImVec4 bg_col = (0,0,0,0), ImVec4 tint_col = (1,1,1,1));
                            - TL;DR: 'border_col' had misleading side-effect on layout, 'bg_col' was missing, parameter order couldn't be consistent with ImageButton().
                            - new behavior always use ImGuiCol_Border color + style.ImageBorderSize / ImGuiStyleVar_ImageBorderSize.
                            - old behavior altered border size (and therefore layout) based on border color's alpha, which caused variety of problems + old behavior a fixed 1.0f for border size which was not tweakable.
                            - kept legacy signature (will obsolete), which mimics the old behavior,  but uses Max(1.0f, style.ImageBorderSize) when border_col is specified.
                            - added ImageWithBg() function which has both 'bg_col' (which was missing) and 'tint_col'. It was impossible to add 'bg_col' to Image() with a parameter order consistent with other functions, so we decided to remove 'tint_col' and introduce ImageWithBg().
 - 2025/02/25 (1.91.9) - internals: fonts: ImFontAtlas::ConfigData[] has been renamed to ImFontAtlas::Sources[]. ImFont::ConfigData[], ConfigDataCount has been renamed to Sources[], SourcesCount.
 - 2025/02/06 (1.91.9) - renamed ImFontConfig::GlyphExtraSpacing.x to ImFontConfig::GlyphExtraAdvanceX.
 - 2025/01/22 (1.91.8) - removed ImGuiColorEditFlags_AlphaPreview (made value 0): it is now the default behavior.
                         prior to 1.91.8: alpha was made opaque in the preview by default _unless_ using ImGuiColorEditFlags_AlphaPreview. We now display the preview as transparent by default. You can use ImGuiColorEditFlags_AlphaOpaque to use old behavior.
                         the new flags (ImGuiColorEditFlags_AlphaOpaque, ImGuiColorEditFlags_AlphaNoBg + existing ImGuiColorEditFlags_AlphaPreviewHalf) may be combined better and allow finer controls:
 - 2025/01/14 (1.91.7) - renamed ImGuiTreeNodeFlags_SpanTextWidth to ImGuiTreeNodeFlags_SpanLabelWidth for consistency with other names. Kept redirection enum (will obsolete). (#6937)
 - 2024/11/27 (1.91.6) - changed CRC32 table from CRC32-adler to CRC32c polynomial in order to be compatible with the result of SSE 4.2 instructions.
                         As a result, old .ini data may be partially lost (docking and tables information particularly).
                         Because some users have crafted and storing .ini data as a way to workaround limitations of the docking API, we are providing a '#define IMGUI_USE_LEGACY_CRC32_ADLER' compile-time option to keep using old CRC32 tables if you cannot afford invalidating old .ini data.
 - 2024/11/06 (1.91.5) - commented/obsoleted out pre-1.87 IO system (equivalent to using IMGUI_DISABLE_OBSOLETE_KEYIO or IMGUI_DISABLE_OBSOLETE_FUNCTIONS before)
                            - io.KeyMap[] and io.KeysDown[] are removed (obsoleted February 2022).
                            - io.NavInputs[] and ImGuiNavInput are removed (obsoleted July 2022).
                            - GetKeyIndex() is removed (obsoleted March 2022). The indirection is now unnecessary.
                            - pre-1.87 backends are not supported:
                               - backends need to call io.AddKeyEvent(), io.AddMouseEvent() instead of writing to io.KeysDown[], io.MouseDown[] fields.
                               - backends need to call io.AddKeyAnalogEvent() for gamepad values instead of writing to io.NavInputs[] fields.
                            - for more reference:
                              - read 1.87 and 1.88 part of this section or read Changelog for 1.87 and 1.88.
                              - read https://github.com/ocornut/imgui/issues/4921
                            - if you have trouble updating a very old codebase using legacy backend-specific key codes: consider updating to 1.91.4 first, then #define IMGUI_DISABLE_OBSOLETE_KEYIO, then update to latest.
                       - obsoleted ImGuiKey_COUNT (it is unusually error-prone/misleading since valid keys don't start at 0). probably use ImGuiKey_NamedKey_BEGIN/ImGuiKey_NamedKey_END?
                       - fonts: removed const qualifiers from most font functions in prevision for upcoming font improvements.
 - 2024/10/18 (1.91.4) - renamed ImGuiCol_NavHighlight to ImGuiCol_NavCursor (for consistency with newly exposed and reworked features). Kept inline redirection enum (will obsolete).
 - 2024/10/14 (1.91.4) - moved ImGuiConfigFlags_NavEnableSetMousePos to standalone io.ConfigNavMoveSetMousePos bool.
                         moved ImGuiConfigFlags_NavNoCaptureKeyboard to standalone io.ConfigNavCaptureKeyboard bool (note the inverted value!).
                         kept legacy names (will obsolete) + code that copies settings once the first time. Dynamically changing the old value won't work. Switch to using the new value!
 - 2024/10/10 (1.91.4) - the typedef for ImTextureID now defaults to ImU64 instead of void*. (#1641)
                         this removes the requirement to redefine it for backends which are e.g. storing descriptor sets or other 64-bits structures when building on 32-bits archs. It therefore simplify various building scripts/helpers.
                         you may have compile-time issues if you were casting to 'void*' instead of 'ImTextureID' when passing your types to functions taking ImTextureID values, e.g. ImGui::Image().
                         in doubt it is almost always better to do an intermediate intptr_t cast, since it allows casting any pointer/integer type without warning:
                            - May warn:    ImGui::Image((void*)MyTextureData, ...);
                            - May warn:    ImGui::Image((void*)(intptr_t)MyTextureData, ...);
                            - Won't warn:  ImGui::Image((ImTextureID)(intptr_t)MyTextureData, ...);
  -                      note that you can always define ImTextureID to be your own high-level structures (with dedicated constructors) if you like.
 - 2024/10/03 (1.91.3) - drags: treat v_min==v_max as a valid clamping range when != 0.0f. Zero is a still special value due to legacy reasons, unless using ImGuiSliderFlags_ClampZeroRange. (#7968, #3361, #76)
                       - drags: extended behavior of ImGuiSliderFlags_AlwaysClamp to include _ClampZeroRange. It considers v_min==v_max==0.0f as a valid clamping range (aka edits not allowed).
                         although unlikely, it you wish to only clamp on text input but want v_min==v_max==0.0f to mean unclamped drags, you can use _ClampOnInput instead of _AlwaysClamp. (#7968, #3361, #76)
 - 2024/09/10 (1.91.2) - internals: using multiple overlaid ButtonBehavior() with same ID will now have io.ConfigDebugHighlightIdConflicts=true feature emit a warning. (#8030)
                         it was one of the rare case where using same ID is legal. workarounds: (1) use single ButtonBehavior() call with multiple _MouseButton flags, or (2) surround the calls with PushItemFlag(ImGuiItemFlags_AllowDuplicateId, true); ... PopItemFlag()
 - 2024/08/23 (1.91.1) - renamed ImGuiChildFlags_Border to ImGuiChildFlags_Borders for consistency. kept inline redirection flag.
 - 2024/08/22 (1.91.1) - moved some functions from ImGuiIO to ImGuiPlatformIO structure:
                            - io.GetClipboardTextFn         -> platform_io.Platform_GetClipboardTextFn + changed 'void* user_data' to 'ImGuiContext* ctx'. Pull your user data from platform_io.ClipboardUserData.
                            - io.SetClipboardTextFn         -> platform_io.Platform_SetClipboardTextFn + same as above line.
                            - io.PlatformOpenInShellFn      -> platform_io.Platform_OpenInShellFn (#7660)
                            - io.PlatformSetImeDataFn       -> platform_io.Platform_SetImeDataFn
                            - io.PlatformLocaleDecimalPoint -> platform_io.Platform_LocaleDecimalPoint (#7389, #6719, #2278)
                            - access those via GetPlatformIO() instead of GetIO().
                         some were introduced very recently and often automatically setup by core library and backends, so for those we are exceptionally not maintaining a legacy redirection symbol.
                       - commented the old ImageButton() signature obsoleted in 1.89 (~August 2022). As a reminder:
                            - old ImageButton() before 1.89 used ImTextureId as item id (created issue with e.g. multiple buttons in same scope, transient texture id values, opaque computation of ID)
                            - new ImageButton() since 1.89 requires an explicit 'const char* str_id'
                            - old ImageButton() before 1.89 had frame_padding' override argument.
                            - new ImageButton() since 1.89 always use style.FramePadding, which you can freely override with PushStyleVar()/PopStyleVar().
 - 2024/07/25 (1.91.0) - obsoleted GetContentRegionMax(), GetWindowContentRegionMin() and GetWindowContentRegionMax(). (see #7838 on GitHub for more info)
                         you should never need those functions. you can do everything with GetCursorScreenPos() and GetContentRegionAvail() in a more simple way.
                            - instead of:  GetWindowContentRegionMax().x - GetCursorPos().x
                            - you can use: GetContentRegionAvail().x
                            - instead of:  GetWindowContentRegionMax().x + GetWindowPos().x
                            - you can use: GetCursorScreenPos().x + GetContentRegionAvail().x // when called from left edge of window
                            - instead of:  GetContentRegionMax()
                            - you can use: GetContentRegionAvail() + GetCursorScreenPos() - GetWindowPos() // right edge in local coordinates
                            - instead of:  GetWindowContentRegionMax().x - GetWindowContentRegionMin().x
                            - you can use: GetContentRegionAvail() // when called from left edge of window
 - 2024/07/15 (1.91.0) - renamed ImGuiSelectableFlags_DontClosePopups to ImGuiSelectableFlags_NoAutoClosePopups. (#1379, #1468, #2200, #4936, #5216, #7302, #7573)
                         (internals: also renamed ImGuiItemFlags_SelectableDontClosePopup into ImGuiItemFlags_AutoClosePopups with inverted behaviors)
 - 2024/07/15 (1.91.0) - obsoleted PushButtonRepeat()/PopButtonRepeat() in favor of using new PushItemFlag(ImGuiItemFlags_ButtonRepeat, ...)/PopItemFlag().
 - 2024/07/02 (1.91.0) - commented out obsolete ImGuiModFlags (renamed to ImGuiKeyChord in 1.89). (#4921, #456)
                       - commented out obsolete ImGuiModFlags_XXX values (renamed to ImGuiMod_XXX in 1.89). (#4921, #456)
                            - ImGuiModFlags_Ctrl -> ImGuiMod_Ctrl, ImGuiModFlags_Shift -> ImGuiMod_Shift etc.
 - 2024/07/02 (1.91.0) - IO, IME: renamed platform IME hook and added explicit context for consistency and future-proofness.
                            - old: io.SetPlatformImeDataFn(ImGuiViewport* viewport, ImGuiPlatformImeData* data);
                            - new: io.PlatformSetImeDataFn(ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data);
 - 2024/06/21 (1.90.9) - BeginChild: added ImGuiChildFlags_NavFlattened as a replacement for the window flag ImGuiWindowFlags_NavFlattened: the feature only ever made sense for BeginChild() anyhow.
                            - old: BeginChild("Name", size, 0, ImGuiWindowFlags_NavFlattened);
                            - new: BeginChild("Name", size, ImGuiChildFlags_NavFlattened, 0);
 - 2024/06/21 (1.90.9) - io: ClearInputKeys() (first exposed in 1.89.8) doesn't clear mouse data, newly added ClearInputMouse() does.
 - 2024/06/20 (1.90.9) - renamed ImGuiDragDropFlags_SourceAutoExpirePayload to ImGuiDragDropFlags_PayloadAutoExpire.
 - 2024/06/18 (1.90.9) - style: renamed ImGuiCol_TabActive -> ImGuiCol_TabSelected, ImGuiCol_TabUnfocused -> ImGuiCol_TabDimmed, ImGuiCol_TabUnfocusedActive -> ImGuiCol_TabDimmedSelected.
 - 2024/06/10 (1.90.9) - removed old nested structure: renaming ImGuiStorage::ImGuiStoragePair type to ImGuiStoragePair (simpler for many languages).
 - 2024/06/06 (1.90.8) - reordered ImGuiInputTextFlags values. This should not be breaking unless you are using generated headers that have values not matching the main library.
 - 2024/06/06 (1.90.8) - removed 'ImGuiButtonFlags_MouseButtonDefault_ = ImGuiButtonFlags_MouseButtonLeft', was mostly unused and misleading.
 - 2024/05/27 (1.90.7) - commented out obsolete symbols marked obsolete in 1.88 (May 2022):
                            - old: CaptureKeyboardFromApp(bool)
                            - new: SetNextFrameWantCaptureKeyboard(bool)
                            - old: CaptureMouseFromApp(bool)
                            - new: SetNextFrameWantCaptureMouse(bool)
 - 2024/05/22 (1.90.7) - inputs (internals): renamed ImGuiKeyOwner_None to ImGuiKeyOwner_NoOwner, to make use more explicit and reduce confusion with the default it is a non-zero value and cannot be the default value (never made public, but disclosing as I expect a few users caught on owner-aware inputs).
                       - inputs (internals): renamed ImGuiInputFlags_RouteGlobalLow -> ImGuiInputFlags_RouteGlobal, ImGuiInputFlags_RouteGlobal -> ImGuiInputFlags_RouteGlobalOverFocused, ImGuiInputFlags_RouteGlobalHigh -> ImGuiInputFlags_RouteGlobalHighest.
                       - inputs (internals): Shortcut(), SetShortcutRouting(): swapped last two parameters order in function signatures:
                            - old: Shortcut(ImGuiKeyChord key_chord, ImGuiID owner_id = 0, ImGuiInputFlags flags = 0);
                            - new: Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags = 0, ImGuiID owner_id = 0);
                       - inputs (internals): owner-aware versions of IsKeyPressed(), IsKeyChordPressed(), IsMouseClicked(): swapped last two parameters order in function signatures.
                            - old: IsKeyPressed(ImGuiKey key, ImGuiID owner_id, ImGuiInputFlags flags = 0);
                            - new: IsKeyPressed(ImGuiKey key, ImGuiInputFlags flags, ImGuiID owner_id = 0);
                            - old: IsMouseClicked(ImGuiMouseButton button, ImGuiID owner_id, ImGuiInputFlags flags = 0);
                            - new: IsMouseClicked(ImGuiMouseButton button, ImGuiInputFlags flags, ImGuiID owner_id = 0);
                         for various reasons those changes makes sense. They are being made because making some of those API public.
                         only past users of imgui_internal.h with the extra parameters will be affected. Added asserts for valid flags in various functions to detect _some_ misuses, BUT NOT ALL.
 - 2024/05/21 (1.90.7) - docking: changed signature of DockSpaceOverViewport() to add explicit dockspace id if desired. pass 0 to use old behavior. (#7611)
                           - old: DockSpaceOverViewport(const ImGuiViewport* viewport = NULL, ImGuiDockNodeFlags flags = 0, ...);
                           - new: DockSpaceOverViewport(ImGuiID dockspace_id = 0, const ImGuiViewport* viewport = NULL, ImGuiDockNodeFlags flags = 0, ...);
 - 2024/05/16 (1.90.7) - inputs: on macOS X, Cmd and Ctrl keys are now automatically swapped by io.AddKeyEvent() as this naturally align with how macOS X uses those keys.
                           - it shouldn't really affect you unless you had custom shortcut swapping in place for macOS X apps.
                           - removed ImGuiMod_Shortcut which was previously dynamically remapping to Ctrl or Cmd/Super. It is now unnecessary to specific cross-platform idiomatic shortcuts. (#2343, #4084, #5923, #456)
 - 2024/05/14 (1.90.7) - backends: SDL_Renderer2 and SDL_Renderer3 backend now take a SDL_Renderer* in their RenderDrawData() functions.
 - 2024/04/18 (1.90.6) - TreeNode: Fixed a layout inconsistency when using an empty/hidden label followed by a SameLine() call. (#7505, #282)
                           - old: TreeNode("##Hidden"); SameLine(); Text("Hello");     // <-- This was actually incorrect! BUT appeared to look ok with the default style where ItemSpacing.x == FramePadding.x * 2 (it didn't look aligned otherwise).
                           - new: TreeNode("##Hidden"); SameLine(0, 0); Text("Hello"); // <-- This is correct for all styles values.
                         with the fix, IF you were successfully using TreeNode("")+SameLine(); you will now have extra spacing between your TreeNode and the following item.
                         You'll need to change the SameLine() call to SameLine(0,0) to remove this extraneous spacing. This seemed like the more sensible fix that's not making things less consistent.
                         (Note: when using this idiom you are likely to also use ImGuiTreeNodeFlags_SpanAvailWidth).
 - 2024/03/18 (1.90.5) - merged the radius_x/radius_y parameters in ImDrawList::AddEllipse(), AddEllipseFilled() and PathEllipticalArcTo() into a single ImVec2 parameter. Exceptionally, because those functions were added in 1.90, we are not adding inline redirection functions. The transition is easy and should affect few users. (#2743, #7417)
 - 2024/03/08 (1.90.5) - inputs: more formally obsoleted GetKeyIndex() when IMGUI_DISABLE_OBSOLETE_FUNCTIONS is set. It has been unnecessary and a no-op since 1.87 (it returns the same value as passed when used with a 1.87+ backend using io.AddKeyEvent() function). (#4921)
                           - IsKeyPressed(GetKeyIndex(ImGuiKey_XXX)) -> use IsKeyPressed(ImGuiKey_XXX)
 - 2024/01/15 (1.90.2) - commented out obsolete ImGuiIO::ImeWindowHandle marked obsolete in 1.87, favor of writing to 'void* ImGuiViewport::PlatformHandleRaw'.
 - 2023/12/19 (1.90.1) - commented out obsolete ImGuiKey_KeyPadEnter redirection to ImGuiKey_KeypadEnter.
 - 2023/11/06 (1.90.1) - removed CalcListClipping() marked obsolete in 1.86. Prefer using ImGuiListClipper which can return non-contiguous ranges.
 - 2023/11/05 (1.90.1) - imgui_freetype: commented out ImGuiFreeType::BuildFontAtlas() obsoleted in 1.81. prefer using #define IMGUI_ENABLE_FREETYPE or see commented code for manual calls.
 - 2023/11/05 (1.90.1) - internals,columns: commented out legacy ImGuiColumnsFlags_XXX symbols redirecting to ImGuiOldColumnsFlags_XXX, obsoleted from imgui_internal.h in 1.80.
 - 2023/11/09 (1.90.0) - removed IM_OFFSETOF() macro in favor of using offsetof() available in C++11. Kept redirection define (will obsolete).
 - 2023/11/07 (1.90.0) - removed BeginChildFrame()/EndChildFrame() in favor of using BeginChild() with the ImGuiChildFlags_FrameStyle flag. kept inline redirection function (will obsolete).
                         those functions were merely PushStyle/PopStyle helpers, the removal isn't so much motivated by needing to add the feature in BeginChild(), but by the necessity to avoid BeginChildFrame() signature mismatching BeginChild() signature and features.
 - 2023/11/02 (1.90.0) - BeginChild: upgraded 'bool border = true' parameter to 'ImGuiChildFlags flags' type, added ImGuiChildFlags_Border equivalent. As with our prior "bool-to-flags" API updates, the ImGuiChildFlags_Border value is guaranteed to be == true forever to ensure a smoother transition, meaning all existing calls will still work.
                           - old: BeginChild("Name", size, true)
                           - new: BeginChild("Name", size, ImGuiChildFlags_Border)
                           - old: BeginChild("Name", size, false)
                           - new: BeginChild("Name", size) or BeginChild("Name", 0) or BeginChild("Name", size, ImGuiChildFlags_None)
                         **AMEND FROM THE FUTURE: from 1.91.1, 'ImGuiChildFlags_Border' is called 'ImGuiChildFlags_Borders'**
 - 2023/11/02 (1.90.0) - BeginChild: added child-flag ImGuiChildFlags_AlwaysUseWindowPadding as a replacement for the window-flag ImGuiWindowFlags_AlwaysUseWindowPadding: the feature only ever made sense for BeginChild() anyhow.
                           - old: BeginChild("Name", size, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
                           - new: BeginChild("Name", size, ImGuiChildFlags_AlwaysUseWindowPadding, 0);
 - 2023/09/27 (1.90.0) - io: removed io.MetricsActiveAllocations introduced in 1.63. Same as 'g.DebugMemAllocCount - g.DebugMemFreeCount' (still displayed in Metrics, unlikely to be accessed by end-user).
 - 2023/09/26 (1.90.0) - debug tools: Renamed ShowStackToolWindow() ("Stack Tool") to ShowIDStackToolWindow() ("ID Stack Tool"), as earlier name was misleading. Kept inline redirection function. (#4631)
 - 2023/09/15 (1.90.0) - ListBox, Combo: changed signature of "name getter" callback in old one-liner ListBox()/Combo() apis. kept inline redirection function (will obsolete).
                           - old: bool Combo(const char* label, int* current_item, bool (*getter)(void* user_data, int idx, const char** out_text), ...)
                           - new: bool Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), ...);
                           - old: bool ListBox(const char* label, int* current_item, bool (*getting)(void* user_data, int idx, const char** out_text), ...);
                           - new: bool ListBox(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), ...);
 - 2023/09/08 (1.90.0) - commented out obsolete redirecting functions:
                           - GetWindowContentRegionWidth()  -> use GetWindowContentRegionMax().x - GetWindowContentRegionMin().x. Consider that generally 'GetContentRegionAvail().x' is more useful.
                           - ImDrawCornerFlags_XXX          -> use ImDrawFlags_RoundCornersXXX flags. Read 1.82 Changelog for details + grep commented names in sources.
                       - commented out runtime support for hardcoded ~0 or 0x01..0x0F rounding flags values for AddRect()/AddRectFilled()/PathRect()/AddImageRounded() -> use ImDrawFlags_RoundCornersXXX flags. Read 1.82 Changelog for details
 - 2023/08/25 (1.89.9) - clipper: Renamed IncludeRangeByIndices() (also called ForceDisplayRangeByIndices() before 1.89.6) to IncludeItemsByIndex(). Kept inline redirection function. Sorry!
 - 2023/07/12 (1.89.8) - ImDrawData: CmdLists now owned, changed from ImDrawList** to ImVector<ImDrawList*>. Majority of users shouldn't be affected, but you cannot compare to NULL nor reassign manually anymore. Instead use AddDrawList(). (#6406, #4879, #1878)
 - 2023/06/28 (1.89.7) - overlapping items: obsoleted 'SetItemAllowOverlap()' (called after item) in favor of calling 'SetNextItemAllowOverlap()' (called before item). 'SetItemAllowOverlap()' didn't and couldn't work reliably since 1.89 (2022-11-15).
 - 2023/06/28 (1.89.7) - overlapping items: renamed 'ImGuiTreeNodeFlags_AllowItemOverlap' to 'ImGuiTreeNodeFlags_AllowOverlap', 'ImGuiSelectableFlags_AllowItemOverlap' to 'ImGuiSelectableFlags_AllowOverlap'. Kept redirecting enums (will obsolete).
 - 2023/06/28 (1.89.7) - overlapping items: IsItemHovered() now by default return false when querying an item using AllowOverlap mode which is being overlapped. Use ImGuiHoveredFlags_AllowWhenOverlappedByItem to revert to old behavior.
 - 2023/06/28 (1.89.7) - overlapping items: Selectable and TreeNode don't allow overlap when active so overlapping widgets won't appear as hovered. While this fixes a common small visual issue, it also means that calling IsItemHovered() after a non-reactive elements - e.g. Text() - overlapping an active one may fail if you don't use IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem). (#6610)
 - 2023/06/20 (1.89.7) - moved io.HoverDelayShort/io.HoverDelayNormal to style.HoverDelayShort/style.HoverDelayNormal. As the fields were added in 1.89 and expected to be left unchanged by most users, or only tweaked once during app initialization, we are exceptionally accepting the breakage.
 - 2023/05/30 (1.89.6) - backends: renamed "imgui_impl_sdlrenderer.cpp" to "imgui_impl_sdlrenderer2.cpp" and "imgui_impl_sdlrenderer.h" to "imgui_impl_sdlrenderer2.h". This is in prevision for the future release of SDL3.
 - 2023/05/22 (1.89.6) - listbox: commented out obsolete/redirecting functions that were marked obsolete more than two years ago:
                           - ListBoxHeader()  -> use BeginListBox() (note how two variants of ListBoxHeader() existed. Check commented versions in imgui.h for reference)
                           - ListBoxFooter()  -> use EndListBox()
 - 2023/05/15 (1.89.6) - clipper: commented out obsolete redirection constructor 'ImGuiListClipper(int items_count, float items_height = -1.0f)' that was marked obsolete in 1.79. Use default constructor + clipper.Begin().
 - 2023/05/15 (1.89.6) - clipper: renamed ImGuiListClipper::ForceDisplayRangeByIndices() to ImGuiListClipper::IncludeRangeByIndices().
 - 2023/03/14 (1.89.4) - commented out redirecting enums/functions names that were marked obsolete two years ago:
                           - ImGuiSliderFlags_ClampOnInput        -> use ImGuiSliderFlags_AlwaysClamp
                           - ImGuiInputTextFlags_AlwaysInsertMode -> use ImGuiInputTextFlags_AlwaysOverwrite
                           - ImDrawList::AddBezierCurve()         -> use ImDrawList::AddBezierCubic()
                           - ImDrawList::PathBezierCurveTo()      -> use ImDrawList::PathBezierCubicCurveTo()
 - 2023/03/09 (1.89.4) - renamed PushAllowKeyboardFocus()/PopAllowKeyboardFocus() to PushTabStop()/PopTabStop(). Kept inline redirection functions (will obsolete).
 - 2023/03/09 (1.89.4) - tooltips: Added 'bool' return value to BeginTooltip() for API consistency. Please only submit contents and call EndTooltip() if BeginTooltip() returns true. In reality the function will _currently_ always return true, but further changes down the line may change this, best to clarify API sooner.
 - 2023/02/15 (1.89.4) - moved the optional "courtesy maths operators" implementation from imgui_internal.h in imgui.h.
                         Even though we encourage using your own maths types and operators by setting up IM_VEC2_CLASS_EXTRA,
                         it has been frequently requested by people to use our own. We had an opt-in define which was
                         previously fulfilled in imgui_internal.h. It is now fulfilled in imgui.h. (#6164)
                           - OK:     #define IMGUI_DEFINE_MATH_OPERATORS / #include "imgui.h" / #include "imgui_internal.h"
                           - Error:  #include "imgui.h" / #define IMGUI_DEFINE_MATH_OPERATORS / #include "imgui_internal.h"
 - 2023/02/07 (1.89.3) - backends: renamed "imgui_impl_sdl.cpp" to "imgui_impl_sdl2.cpp" and "imgui_impl_sdl.h" to "imgui_impl_sdl2.h". (#6146) This is in prevision for the future release of SDL3.
 - 2022/10/26 (1.89)   - commented out redirecting OpenPopupContextItem() which was briefly the name of OpenPopupOnItemClick() from 1.77 to 1.79.
 - 2022/10/12 (1.89)   - removed runtime patching of invalid "%f"/"%0.f" format strings for DragInt()/SliderInt(). This was obsoleted in 1.61 (May 2018). See 1.61 changelog for details.
 - 2022/09/26 (1.89)   - renamed and merged keyboard modifiers key enums and flags into a same set. Kept inline redirection enums (will obsolete).
                           - ImGuiKey_ModCtrl  and ImGuiModFlags_Ctrl  -> ImGuiMod_Ctrl
                           - ImGuiKey_ModShift and ImGuiModFlags_Shift -> ImGuiMod_Shift
                           - ImGuiKey_ModAlt   and ImGuiModFlags_Alt   -> ImGuiMod_Alt
                           - ImGuiKey_ModSuper and ImGuiModFlags_Super -> ImGuiMod_Super
                         the ImGuiKey_ModXXX were introduced in 1.87 and mostly used by backends.
                         the ImGuiModFlags_XXX have been exposed in imgui.h but not really used by any public api only by third-party extensions.
                         exceptionally commenting out the older ImGuiKeyModFlags_XXX names ahead of obsolescence schedule to reduce confusion and because they were not meant to be used anyway.
 - 2022/09/20 (1.89)   - ImGuiKey is now a typed enum, allowing ImGuiKey_XXX symbols to be named in debuggers.
                         this will require uses of legacy backend-dependent indices to be casted, e.g.
                            - with imgui_impl_glfw:  IsKeyPressed(GLFW_KEY_A) -> IsKeyPressed((ImGuiKey)GLFW_KEY_A);
                            - with imgui_impl_win32: IsKeyPressed('A')        -> IsKeyPressed((ImGuiKey)'A')
                            - etc. However if you are upgrading code you might well use the better, backend-agnostic IsKeyPressed(ImGuiKey_A) now!
 - 2022/09/12 (1.89) - removed the bizarre legacy default argument for 'TreePush(const void* ptr = NULL)', always pass a pointer value explicitly. NULL/nullptr is ok but require cast, e.g. TreePush((void*)nullptr);
 - 2022/09/05 (1.89) - commented out redirecting functions/enums names that were marked obsolete in 1.77 and 1.78 (June 2020):
                         - DragScalar(), DragScalarN(), DragFloat(), DragFloat2(), DragFloat3(), DragFloat4(): For old signatures ending with (..., const char* format, float power = 1.0f) -> use (..., format ImGuiSliderFlags_Logarithmic) if power != 1.0f.
                         - SliderScalar(), SliderScalarN(), SliderFloat(), SliderFloat2(), SliderFloat3(), SliderFloat4(): For old signatures ending with (..., const char* format, float power = 1.0f) -> use (..., format ImGuiSliderFlags_Logarithmic) if power != 1.0f.
                         - BeginPopupContextWindow(const char*, ImGuiMouseButton, bool) -> use BeginPopupContextWindow(const char*, ImGuiPopupFlags)
 - 2022/09/02 (1.89) - obsoleted using SetCursorPos()/SetCursorScreenPos() to extend parent window/cell boundaries.
                       this relates to when moving the cursor position beyond current boundaries WITHOUT submitting an item.
                         - previously this would make the window content size ~200x200:
                              Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + ImVec2(200,200)) + End();
                         - instead, please submit an item:
                              Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + ImVec2(200,200)) + Dummy(ImVec2(0,0)) + End();
                         - alternative:
                              Begin(...) + Dummy(ImVec2(200,200)) + End();
                         - content size is now only extended when submitting an item!
                         - with '#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS' this will now be detected and assert.
                         - without '#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS' this will silently be fixed until we obsolete it.
 - 2022/08/03 (1.89) - changed signature of ImageButton() function. Kept redirection function (will obsolete).
                        - added 'const char* str_id' parameter + removed 'int frame_padding = -1' parameter.
                        - old signature: bool ImageButton(ImTextureID tex_id, ImVec2 size, ImVec2 uv0 = ImVec2(0,0), ImVec2 uv1 = ImVec2(1,1), int frame_padding = -1, ImVec4 bg_col = ImVec4(0,0,0,0), ImVec4 tint_col = ImVec4(1,1,1,1));
                          - used the ImTextureID value to create an ID. This was inconsistent with other functions, led to ID conflicts, and caused problems with engines using transient ImTextureID values.
                          - had a FramePadding override which was inconsistent with other functions and made the already-long signature even longer.
                        - new signature: bool ImageButton(const char* str_id, ImTextureID tex_id, ImVec2 size, ImVec2 uv0 = ImVec2(0,0), ImVec2 uv1 = ImVec2(1,1), ImVec4 bg_col = ImVec4(0,0,0,0), ImVec4 tint_col = ImVec4(1,1,1,1));
                          - requires an explicit identifier. You may still use e.g. PushID() calls and then pass an empty identifier.
                          - always uses style.FramePadding for padding, to be consistent with other buttons. You may use PushStyleVar() to alter this.
 - 2022/07/08 (1.89) - inputs: removed io.NavInputs[] and ImGuiNavInput enum (following 1.87 changes).
                        - Official backends from 1.87+                  -> no issue.
                        - Official backends from 1.60 to 1.86           -> will build and convert gamepad inputs, unless IMGUI_DISABLE_OBSOLETE_KEYIO is defined. Need updating!
                        - Custom backends not writing to io.NavInputs[] -> no issue.
                        - Custom backends writing to io.NavInputs[]     -> will build and convert gamepad inputs, unless IMGUI_DISABLE_OBSOLETE_KEYIO is defined. Need fixing!
                        - TL;DR: Backends should call io.AddKeyEvent()/io.AddKeyAnalogEvent() with ImGuiKey_GamepadXXX values instead of filling io.NavInput[].
 - 2022/06/15 (1.88) - renamed IMGUI_DISABLE_METRICS_WINDOW to IMGUI_DISABLE_DEBUG_TOOLS for correctness. kept support for old define (will obsolete).
 - 2022/05/03 (1.88) - backends: osx: removed ImGui_ImplOSX_HandleEvent() from backend API in favor of backend automatically handling event capture. All ImGui_ImplOSX_HandleEvent() calls should be removed as they are now unnecessary.
 - 2022/04/05 (1.88) - inputs: renamed ImGuiKeyModFlags to ImGuiModFlags. Kept inline redirection enums (will obsolete). This was never used in public API functions but technically present in imgui.h and ImGuiIO.
 - 2022/01/20 (1.87) - inputs: reworded gamepad IO.
                        - Backend writing to io.NavInputs[]            -> backend should call io.AddKeyEvent()/io.AddKeyAnalogEvent() with ImGuiKey_GamepadXXX values.
 - 2022/01/19 (1.87) - sliders, drags: removed support for legacy arithmetic operators (+,+-,*,/) when inputting text. This doesn't break any api/code but a feature that used to be accessible by end-users (which seemingly no one used).
 - 2022/01/17 (1.87) - inputs: reworked mouse IO.
                        - Backend writing to io.MousePos               -> backend should call io.AddMousePosEvent()
                        - Backend writing to io.MouseDown[]            -> backend should call io.AddMouseButtonEvent()
                        - Backend writing to io.MouseWheel             -> backend should call io.AddMouseWheelEvent()
                        - Backend writing to io.MouseHoveredViewport   -> backend should call io.AddMouseViewportEvent() [Docking branch w/ multi-viewports only]
                       note: for all calls to IO new functions, the Dear ImGui context should be bound/current.
                       read https://github.com/ocornut/imgui/issues/4921 for details.
 - 2022/01/10 (1.87) - inputs: reworked keyboard IO. Removed io.KeyMap[], io.KeysDown[] in favor of calling io.AddKeyEvent(), ImGui::IsKeyDown(). Removed GetKeyIndex(), now unnecessary. All IsKeyXXX() functions now take ImGuiKey values. All features are still functional until IMGUI_DISABLE_OBSOLETE_KEYIO is defined. Read Changelog and Release Notes for details.
                        - IsKeyPressed(MY_NATIVE_KEY_XXX)              -> use IsKeyPressed(ImGuiKey_XXX)
                        - IsKeyPressed(GetKeyIndex(ImGuiKey_XXX))      -> use IsKeyPressed(ImGuiKey_XXX)
                        - Backend writing to io.KeyMap[],io.KeysDown[] -> backend should call io.AddKeyEvent() (+ call io.SetKeyEventNativeData() if you want legacy user code to still function with legacy key codes).
                        - Backend writing to io.KeyCtrl, io.KeyShift.. -> backend should call io.AddKeyEvent() with ImGuiMod_XXX values. *IF YOU PULLED CODE BETWEEN 2021/01/10 and 2021/01/27: We used to have a io.AddKeyModsEvent() function which was now replaced by io.AddKeyEvent() with ImGuiMod_XXX values.*
                     - one case won't work with backward compatibility: if your custom backend used ImGuiKey as mock native indices (e.g. "io.KeyMap[ImGuiKey_A] = ImGuiKey_A") because those values are now larger than the legacy KeyDown[] array. Will assert.
                     - inputs: added ImGuiKey_ModCtrl/ImGuiKey_ModShift/ImGuiKey_ModAlt/ImGuiKey_ModSuper values to submit keyboard modifiers using io.AddKeyEvent(), instead of writing directly to io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper.
 - 2022/01/05 (1.87) - inputs: renamed ImGuiKey_KeyPadEnter to ImGuiKey_KeypadEnter to align with new symbols. Kept redirection enum.
 - 2022/01/05 (1.87) - removed io.ImeSetInputScreenPosFn() in favor of more flexible io.SetPlatformImeDataFn(). Removed 'void* io.ImeWindowHandle' in favor of writing to 'void* ImGuiViewport::PlatformHandleRaw'.
 - 2022/01/01 (1.87) - commented out redirecting functions/enums names that were marked obsolete in 1.69, 1.70, 1.71, 1.72 (March-July 2019)
                        - ImGui::SetNextTreeNodeOpen()        -> use ImGui::SetNextItemOpen()
                        - ImGui::GetContentRegionAvailWidth() -> use ImGui::GetContentRegionAvail().x
                        - ImGui::TreeAdvanceToLabelPos()      -> use ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetTreeNodeToLabelSpacing());
                        - ImFontAtlas::CustomRect             -> use ImFontAtlasCustomRect
                        - ImGuiColorEditFlags_RGB/HSV/HEX     -> use ImGuiColorEditFlags_DisplayRGB/HSV/Hex
 - 2021/12/20 (1.86) - backends: removed obsolete Marmalade backend (imgui_impl_marmalade.cpp) + example. Find last supported version at https://github.com/ocornut/imgui/wiki/Bindings
 - 2021/11/04 (1.86) - removed CalcListClipping() function. Prefer using ImGuiListClipper which can return non-contiguous ranges. Please open an issue if you think you really need this function.
 - 2021/08/23 (1.85) - removed GetWindowContentRegionWidth() function. keep inline redirection helper. can use 'GetWindowContentRegionMax().x - GetWindowContentRegionMin().x' instead for generally 'GetContentRegionAvail().x' is more useful.
 - 2021/07/26 (1.84) - commented out redirecting functions/enums names that were marked obsolete in 1.67 and 1.69 (March 2019):
                        - ImGui::GetOverlayDrawList() -> use ImGui::GetForegroundDrawList()
                        - ImFont::GlyphRangesBuilder  -> use ImFontGlyphRangesBuilder
 - 2021/05/19 (1.83) - backends: obsoleted direct access to ImDrawCmd::TextureId in favor of calling ImDrawCmd::GetTexID().
                        - if you are using official backends from the source tree: you have nothing to do.
                        - if you have copied old backend code or using your own: change access to draw_cmd->TextureId to draw_cmd->GetTexID().
 - 2021/03/12 (1.82) - upgraded ImDrawList::AddRect(), AddRectFilled(), PathRect() to use ImDrawFlags instead of ImDrawCornersFlags.
                        - ImDrawCornerFlags_TopLeft  -> use ImDrawFlags_RoundCornersTopLeft
                        - ImDrawCornerFlags_BotRight -> use ImDrawFlags_RoundCornersBottomRight
                        - ImDrawCornerFlags_None     -> use ImDrawFlags_RoundCornersNone etc.
                       flags now sanely defaults to 0 instead of 0x0F, consistent with all other flags in the API.
                       breaking: the default with rounding > 0.0f is now "round all corners" vs old implicit "round no corners":
                        - rounding == 0.0f + flags == 0 --> meant no rounding  --> unchanged (common use)
                        - rounding  > 0.0f + flags != 0 --> meant rounding     --> unchanged (common use)
                        - rounding == 0.0f + flags != 0 --> meant no rounding  --> unchanged (unlikely use)
                        - rounding  > 0.0f + flags == 0 --> meant no rounding  --> BREAKING (unlikely use): will now round all corners --> use ImDrawFlags_RoundCornersNone or rounding == 0.0f.
                       this ONLY matters for hard coded use of 0 + rounding > 0.0f. Use of named ImDrawFlags_RoundCornersNone (new) or ImDrawCornerFlags_None (old) are ok.
                       the old ImDrawCornersFlags used awkward default values of ~0 or 0xF (4 lower bits set) to signify "round all corners" and we sometimes encouraged using them as shortcuts.
                       legacy path still support use of hard coded ~0 or any value from 0x1 or 0xF. They will behave the same with legacy paths enabled (will assert otherwise).
 - 2021/03/11 (1.82) - removed redirecting functions/enums names that were marked obsolete in 1.66 (September 2018):
                        - ImGui::SetScrollHere()              -> use ImGui::SetScrollHereY()
 - 2021/03/11 (1.82) - clarified that ImDrawList::PathArcTo(), ImDrawList::PathArcToFast() won't render with radius < 0.0f. Previously it sorts of accidentally worked but would generally lead to counter-clockwise paths and have an effect on anti-aliasing.
 - 2021/03/10 (1.82) - upgraded ImDrawList::AddPolyline() and PathStroke() "bool closed" parameter to "ImDrawFlags flags". The matching ImDrawFlags_Closed value is guaranteed to always stay == 1 in the future.
 - 2021/02/22 (1.82) - (*undone in 1.84*) win32+mingw: Re-enabled IME functions by default even under MinGW. In July 2016, issue #738 had me incorrectly disable those default functions for MinGW. MinGW users should: either link with -limm32, either set their imconfig file  with '#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS'.
 - 2021/02/17 (1.82) - renamed rarely used style.CircleSegmentMaxError (old default = 1.60f) to style.CircleTessellationMaxError (new default = 0.30f) as the meaning of the value changed.
 - 2021/02/03 (1.81) - renamed ListBoxHeader(const char* label, ImVec2 size) to BeginListBox(). Kept inline redirection function (will obsolete).
                     - removed ListBoxHeader(const char* label, int items_count, int height_in_items = -1) in favor of specifying size. Kept inline redirection function (will obsolete).
                     - renamed ListBoxFooter() to EndListBox(). Kept inline redirection function (will obsolete).
 - 2021/01/26 (1.81) - removed ImGuiFreeType::BuildFontAtlas(). Kept inline redirection function. Prefer using '#define IMGUI_ENABLE_FREETYPE', but there's a runtime selection path available too. The shared extra flags parameters (very rarely used) are now stored in ImFontAtlas::FontBuilderFlags.
                     - renamed ImFontConfig::RasterizerFlags (used by FreeType) to ImFontConfig::FontBuilderFlags.
                     - renamed ImGuiFreeType::XXX flags to ImGuiFreeTypeBuilderFlags_XXX for consistency with other API.
 - 2020/10/12 (1.80) - removed redirecting functions/enums that were marked obsolete in 1.63 (August 2018):
                        - ImGui::IsItemDeactivatedAfterChange() -> use ImGui::IsItemDeactivatedAfterEdit().
                        - ImGuiCol_ModalWindowDarkening       -> use ImGuiCol_ModalWindowDimBg
                        - ImGuiInputTextCallback              -> use ImGuiTextEditCallback
                        - ImGuiInputTextCallbackData          -> use ImGuiTextEditCallbackData
 - 2020/12/21 (1.80) - renamed ImDrawList::AddBezierCurve() to AddBezierCubic(), and PathBezierCurveTo() to PathBezierCubicCurveTo(). Kept inline redirection function (will obsolete).
 - 2020/12/04 (1.80) - added imgui_tables.cpp file! Manually constructed project files will need the new file added!
 - 2020/11/18 (1.80) - renamed undocumented/internals ImGuiColumnsFlags_* to ImGuiOldColumnFlags_* in prevision of incoming Tables API.
 - 2020/11/03 (1.80) - renamed io.ConfigWindowsMemoryCompactTimer to io.ConfigMemoryCompactTimer as the feature will apply to other data structures
 - 2020/10/14 (1.80) - backends: moved all backends files (imgui_impl_XXXX.cpp, imgui_impl_XXXX.h) from examples/ to backends/.
 - 2020/10/12 (1.80) - removed redirecting functions/enums that were marked obsolete in 1.60 (April 2018):
                        - io.RenderDrawListsFn pointer        -> use ImGui::GetDrawData() value and call the render function of your backend
                        - ImGui::IsAnyWindowFocused()         -> use ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)
                        - ImGui::IsAnyWindowHovered()         -> use ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)
                        - ImGuiStyleVar_Count_                -> use ImGuiStyleVar_COUNT
                        - ImGuiMouseCursor_Count_             -> use ImGuiMouseCursor_COUNT
                      - removed redirecting functions names that were marked obsolete in 1.61 (May 2018):
                        - InputFloat (... int decimal_precision ...) -> use InputFloat (... const char* format ...) with format = "%.Xf" where X is your value for decimal_precision.
                        - same for InputFloat2()/InputFloat3()/InputFloat4() variants taking a `int decimal_precision` parameter.
 - 2020/10/05 (1.79) - removed ImGuiListClipper: Renamed constructor parameters which created an ambiguous alternative to using the ImGuiListClipper::Begin() function, with misleading edge cases (note: imgui_memory_editor <0.40 from imgui_club/ used this old clipper API. Update your copy if needed).
 - 2020/09/25 (1.79) - renamed ImGuiSliderFlags_ClampOnInput to ImGuiSliderFlags_AlwaysClamp. Kept redirection enum (will obsolete sooner because previous name was added recently).
 - 2020/09/25 (1.79) - renamed style.TabMinWidthForUnselectedCloseButton to style.TabMinWidthForCloseButton.
 - 2020/09/21 (1.79) - renamed OpenPopupContextItem() back to OpenPopupOnItemClick(), reverting the change from 1.77. For varieties of reason this is more self-explanatory.
 - 2020/09/21 (1.79) - removed return value from OpenPopupOnItemClick() - returned true on mouse release on an item - because it is inconsistent with other popup APIs and makes others misleading. It's also and unnecessary: you can use IsWindowAppearing() after BeginPopup() for a similar result.
 - 2020/09/17 (1.79) - removed ImFont::DisplayOffset in favor of ImFontConfig::GlyphOffset. DisplayOffset was applied after scaling and not very meaningful/useful outside of being needed by the default ProggyClean font. If you scaled this value after calling AddFontDefault(), this is now done automatically. It was also getting in the way of better font scaling, so let's get rid of it now!
 - 2020/08/17 (1.78) - obsoleted use of the trailing 'float power=1.0f' parameter for DragFloat(), DragFloat2(), DragFloat3(), DragFloat4(), DragFloatRange2(), DragScalar(), DragScalarN(), SliderFloat(), SliderFloat2(), SliderFloat3(), SliderFloat4(), SliderScalar(), SliderScalarN(), VSliderFloat() and VSliderScalar().
                       replaced the 'float power=1.0f' argument with integer-based flags defaulting to 0 (as with all our flags).
                       worked out a backward-compatibility scheme so hopefully most C++ codebase should not be affected. in short, when calling those functions:
                       - if you omitted the 'power' parameter (likely!), you are not affected.
                       - if you set the 'power' parameter to 1.0f (same as previous default value): 1/ your compiler may warn on float>int conversion, 2/ everything else will work. 3/ you can replace the 1.0f value with 0 to fix the warning, and be technically correct.
                       - if you set the 'power' parameter to >1.0f (to enable non-linear editing): 1/ your compiler may warn on float>int conversion, 2/ code will assert at runtime, 3/ in case asserts are disabled, the code will not crash and enable the _Logarithmic flag. 4/ you can replace the >1.0f value with ImGuiSliderFlags_Logarithmic to fix the warning/assert and get a _similar_ effect as previous uses of power >1.0f.
                       see https://github.com/ocornut/imgui/issues/3361 for all details.
                       kept inline redirection functions (will obsolete) apart for: DragFloatRange2(), VSliderFloat(), VSliderScalar(). For those three the 'float power=1.0f' version was removed directly as they were most unlikely ever used.
                       for shared code, you can version check at compile-time with `#if IMGUI_VERSION_NUM >= 17704`.
                     - obsoleted use of v_min > v_max in DragInt, DragFloat, DragScalar to lock edits (introduced in 1.73, was not demoed nor documented very), will be replaced by a more generic ReadOnly feature. You may use the ImGuiSliderFlags_ReadOnly internal flag in the meantime.
 - 2020/06/23 (1.77) - removed BeginPopupContextWindow(const char*, int mouse_button, bool also_over_items) in favor of BeginPopupContextWindow(const char*, ImGuiPopupFlags flags) with ImGuiPopupFlags_NoOverItems.
 - 2020/06/15 (1.77) - renamed OpenPopupOnItemClick() to OpenPopupContextItem(). Kept inline redirection function (will obsolete). [NOTE: THIS WAS REVERTED IN 1.79]
 - 2020/06/15 (1.77) - removed CalcItemRectClosestPoint() entry point which was made obsolete and asserting in December 2017.
 - 2020/04/23 (1.77) - removed unnecessary ID (first arg) of ImFontAtlas::AddCustomRectRegular().
 - 2020/01/22 (1.75) - ImDrawList::AddCircle()/AddCircleFilled() functions don't accept negative radius any more.
 - 2019/12/17 (1.75) - [undid this change in 1.76] made Columns() limited to 64 columns by asserting above that limit. While the current code technically supports it, future code may not so we're putting the restriction ahead.
 - 2019/12/13 (1.75) - [imgui_internal.h] changed ImRect() default constructor initializes all fields to 0.0f instead of (FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX). If you used ImRect::Add() to create bounding boxes by adding multiple points into it, you may need to fix your initial value.
 - 2019/12/08 (1.75) - removed redirecting functions/enums that were marked obsolete in 1.53 (December 2017):
                       - ShowTestWindow()                    -> use ShowDemoWindow()
                       - IsRootWindowFocused()               -> use IsWindowFocused(ImGuiFocusedFlags_RootWindow)
                       - IsRootWindowOrAnyChildFocused()     -> use IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
                       - SetNextWindowContentWidth(w)        -> use SetNextWindowContentSize(ImVec2(w, 0.0f))
                       - GetItemsLineHeightWithSpacing()     -> use GetFrameHeightWithSpacing()
                       - ImGuiCol_ChildWindowBg              -> use ImGuiCol_ChildBg
                       - ImGuiStyleVar_ChildWindowRounding   -> use ImGuiStyleVar_ChildRounding
                       - ImGuiTreeNodeFlags_AllowOverlapMode -> use ImGuiTreeNodeFlags_AllowItemOverlap
                       - IMGUI_DISABLE_TEST_WINDOWS          -> use IMGUI_DISABLE_DEMO_WINDOWS
 - 2019/12/08 (1.75) - obsoleted calling ImDrawList::PrimReserve() with a negative count (which was vaguely documented and rarely if ever used). Instead, we added an explicit PrimUnreserve() API.
 - 2019/12/06 (1.75) - removed implicit default parameter to IsMouseDragging(int button = 0) to be consistent with other mouse functions (none of the other functions have it).
 - 2019/11/21 (1.74) - ImFontAtlas::AddCustomRectRegular() now requires an ID larger than 0x110000 (instead of 0x10000) to conform with supporting Unicode planes 1-16 in a future update. ID below 0x110000 will now assert.
 - 2019/11/19 (1.74) - renamed IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS to IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS for consistency.
 - 2019/11/19 (1.74) - renamed IMGUI_DISABLE_MATH_FUNCTIONS to IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS for consistency.
 - 2019/10/22 (1.74) - removed redirecting functions/enums that were marked obsolete in 1.52 (October 2017):
                       - Begin() [old 5 args version]        -> use Begin() [3 args], use SetNextWindowSize() SetNextWindowBgAlpha() if needed
                       - IsRootWindowOrAnyChildHovered()     -> use IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)
                       - AlignFirstTextHeightToWidgets()     -> use AlignTextToFramePadding()
                       - SetNextWindowPosCenter()            -> use SetNextWindowPos() with a pivot of (0.5f, 0.5f)
                       - ImFont::Glyph                       -> use ImFontGlyph
 - 2019/10/14 (1.74) - inputs: Fixed a miscalculation in the keyboard/mouse "typematic" repeat delay/rate calculation, used by keys and e.g. repeating mouse buttons as well as the GetKeyPressedAmount() function.
                       if you were using a non-default value for io.KeyRepeatRate (previous default was 0.250), you can add +io.KeyRepeatDelay to it to compensate for the fix.
                       The function was triggering on: 0.0 and (delay+rate*N) where (N>=1). Fixed formula responds to (N>=0). Effectively it made io.KeyRepeatRate behave like it was set to (io.KeyRepeatRate + io.KeyRepeatDelay).
                       If you never altered io.KeyRepeatRate nor used GetKeyPressedAmount() this won't affect you.
 - 2019/07/15 (1.72) - removed TreeAdvanceToLabelPos() which is rarely used and only does SetCursorPosX(GetCursorPosX() + GetTreeNodeToLabelSpacing()). Kept redirection function (will obsolete).
 - 2019/07/12 (1.72) - renamed ImFontAtlas::CustomRect to ImFontAtlasCustomRect. Kept redirection typedef (will obsolete).
 - 2019/06/14 (1.72) - removed redirecting functions/enums names that were marked obsolete in 1.51 (June 2017): ImGuiCol_Column*, ImGuiSetCond_*, IsItemHoveredRect(), IsPosHoveringAnyWindow(), IsMouseHoveringAnyWindow(), IsMouseHoveringWindow(), IMGUI_ONCE_UPON_A_FRAME. Grep this log for details and new names, or see how they were implemented until 1.71.
 - 2019/06/07 (1.71) - rendering of child window outer decorations (bg color, border, scrollbars) is now performed as part of the parent window. If you have
                       overlapping child windows in a same parent, and relied on their relative z-order to be mapped to their submission order, this will affect your rendering.
                       This optimization is disabled if the parent window has no visual output, because it appears to be the most common situation leading to the creation of overlapping child windows.
                       Please reach out if you are affected.
 - 2019/05/13 (1.71) - renamed SetNextTreeNodeOpen() to SetNextItemOpen(). Kept inline redirection function (will obsolete).
 - 2019/05/11 (1.71) - changed io.AddInputCharacter(unsigned short c) signature to io.AddInputCharacter(unsigned int c).
 - 2019/04/29 (1.70) - improved ImDrawList thick strokes (>1.0f) preserving correct thickness up to 90 degrees angles (e.g. rectangles). If you have custom rendering using thick lines, they will appear thicker now.
 - 2019/04/29 (1.70) - removed GetContentRegionAvailWidth(), use GetContentRegionAvail().x instead. Kept inline redirection function (will obsolete).
 - 2019/03/04 (1.69) - renamed GetOverlayDrawList() to GetForegroundDrawList(). Kept redirection function (will obsolete).
 - 2019/02/26 (1.69) - renamed ImGuiColorEditFlags_RGB/ImGuiColorEditFlags_HSV/ImGuiColorEditFlags_HEX to ImGuiColorEditFlags_DisplayRGB/ImGuiColorEditFlags_DisplayHSV/ImGuiColorEditFlags_DisplayHex. Kept redirection enums (will obsolete).
 - 2019/02/14 (1.68) - made it illegal/assert when io.DisplayTime == 0.0f (with an exception for the first frame). If for some reason your time step calculation gives you a zero value, replace it with an arbitrarily small value!
 - 2019/02/01 (1.68) - removed io.DisplayVisibleMin/DisplayVisibleMax (which were marked obsolete and removed from viewport/docking branch already).
 - 2019/01/06 (1.67) - renamed io.InputCharacters[], marked internal as was always intended. Please don't access directly, and use AddInputCharacter() instead!
 - 2019/01/06 (1.67) - renamed ImFontAtlas::GlyphRangesBuilder to ImFontGlyphRangesBuilder. Kept redirection typedef (will obsolete).
 - 2018/12/20 (1.67) - made it illegal to call Begin("") with an empty string. This somehow half-worked before but had various undesirable side-effects.
 - 2018/12/10 (1.67) - renamed io.ConfigResizeWindowsFromEdges to io.ConfigWindowsResizeFromEdges as we are doing a large pass on configuration flags.
 - 2018/10/12 (1.66) - renamed misc/stl/imgui_stl.* to misc/cpp/imgui_stdlib.* in prevision for other C++ helper files.
 - 2018/09/28 (1.66) - renamed SetScrollHere() to SetScrollHereY(). Kept redirection function (will obsolete).
 - 2018/09/06 (1.65) - renamed stb_truetype.h to imstb_truetype.h, stb_textedit.h to imstb_textedit.h, and stb_rect_pack.h to imstb_rectpack.h.
                       If you were conveniently using the imgui copy of those STB headers in your project you will have to update your include paths.
 - 2018/09/05 (1.65) - renamed io.OptCursorBlink/io.ConfigCursorBlink to io.ConfigInputTextCursorBlink. (#1427)
 - 2018/08/31 (1.64) - added imgui_widgets.cpp file, extracted and moved widgets code out of imgui.cpp into imgui_widgets.cpp. Re-ordered some of the code remaining in imgui.cpp.
                       NONE OF THE FUNCTIONS HAVE CHANGED. THE CODE IS SEMANTICALLY 100% IDENTICAL, BUT _EVERY_ FUNCTION HAS BEEN MOVED.
                       Because of this, any local modifications to imgui.cpp will likely conflict when you update. Read docs/CHANGELOG.txt for suggestions.
 - 2018/08/22 (1.63) - renamed IsItemDeactivatedAfterChange() to IsItemDeactivatedAfterEdit() for consistency with new IsItemEdited() API. Kept redirection function (will obsolete soonish as IsItemDeactivatedAfterChange() is very recent).
 - 2018/08/21 (1.63) - renamed ImGuiTextEditCallback to ImGuiInputTextCallback, ImGuiTextEditCallbackData to ImGuiInputTextCallbackData for consistency. Kept redirection types (will obsolete).
 - 2018/08/21 (1.63) - removed ImGuiInputTextCallbackData::ReadOnly since it is a duplication of (ImGuiInputTextCallbackData::Flags & ImGuiInputTextFlags_ReadOnly).
 - 2018/08/01 (1.63) - removed per-window ImGuiWindowFlags_ResizeFromAnySide beta flag in favor of a global io.ConfigResizeWindowsFromEdges [update 1.67 renamed to ConfigWindowsResizeFromEdges] to enable the feature.
 - 2018/08/01 (1.63) - renamed io.OptCursorBlink to io.ConfigCursorBlink [-> io.ConfigInputTextCursorBlink in 1.65], io.OptMacOSXBehaviors to ConfigMacOSXBehaviors for consistency.
 - 2018/07/22 (1.63) - changed ImGui::GetTime() return value from float to double to avoid accumulating floating point imprecisions over time.
 - 2018/07/08 (1.63) - style: renamed ImGuiCol_ModalWindowDarkening to ImGuiCol_ModalWindowDimBg for consistency with other features. Kept redirection enum (will obsolete).
 - 2018/06/08 (1.62) - examples: the imgui_impl_XXX files have been split to separate platform (Win32, GLFW, SDL2, etc.) from renderer (DX11, OpenGL, Vulkan,  etc.).
                       old backends will still work as is, however prefer using the separated backends as they will be updated to support multi-viewports.
                       when adopting new backends follow the main.cpp code of your preferred examples/ folder to know which functions to call.
                       in particular, note that old backends called ImGui::NewFrame() at the end of their ImGui_ImplXXXX_NewFrame() function.
 - 2018/06/06 (1.62) - renamed GetGlyphRangesChinese() to GetGlyphRangesChineseFull() to distinguish other variants and discourage using the full set.
 - 2018/06/06 (1.62) - TreeNodeEx()/TreeNodeBehavior(): the ImGuiTreeNodeFlags_CollapsingHeader helper now include the ImGuiTreeNodeFlags_NoTreePushOnOpen flag. See Changelog for details.
 - 2018/05/03 (1.61) - DragInt(): the default compile-time format string has been changed from "%.0f" to "%d", as we are not using integers internally any more.
                       If you used DragInt() with custom format strings, make sure you change them to use %d or an integer-compatible format.
                       To honor backward-compatibility, the DragInt() code will currently parse and modify format strings to replace %*f with %d, giving time to users to upgrade their code.
                       If you have IMGUI_DISABLE_OBSOLETE_FUNCTIONS enabled, the code will instead assert! You may run a reg-exp search on your codebase for e.g. "DragInt.*%f" to help you find them.
 - 2018/04/28 (1.61) - obsoleted InputFloat() functions taking an optional "int decimal_precision" in favor of an equivalent and more flexible "const char* format",
                       consistent with other functions. Kept redirection functions (will obsolete).
 - 2018/04/09 (1.61) - IM_DELETE() helper function added in 1.60 doesn't clear the input _pointer_ reference, more consistent with expectation and allows passing r-value.
 - 2018/03/20 (1.60) - renamed io.WantMoveMouse to io.WantSetMousePos for consistency and ease of understanding (was added in 1.52, _not_ used by core and only honored by some backend ahead of merging the Nav branch).
 - 2018/03/12 (1.60) - removed ImGuiCol_CloseButton, ImGuiCol_CloseButtonActive, ImGuiCol_CloseButtonHovered as the closing cross uses regular button colors now.
 - 2018/03/08 (1.60) - changed ImFont::DisplayOffset.y to default to 0 instead of +1. Fixed rounding of Ascent/Descent to match TrueType renderer. If you were adding or subtracting to ImFont::DisplayOffset check if your fonts are correctly aligned vertically.
 - 2018/03/03 (1.60) - renamed ImGuiStyleVar_Count_ to ImGuiStyleVar_COUNT and ImGuiMouseCursor_Count_ to ImGuiMouseCursor_COUNT for consistency with other public enums.
 - 2018/02/18 (1.60) - BeginDragDropSource(): temporarily removed the optional mouse_button=0 parameter because it is not really usable in many situations at the moment.
 - 2018/02/16 (1.60) - obsoleted the io.RenderDrawListsFn callback, you can call your graphics engine render function after ImGui::Render(). Use ImGui::GetDrawData() to retrieve the ImDrawData* to display.
 - 2018/02/07 (1.60) - reorganized context handling to be more explicit,
                       - YOU NOW NEED TO CALL ImGui::CreateContext() AT THE BEGINNING OF YOUR APP, AND CALL ImGui::DestroyContext() AT THE END.
                       - removed Shutdown() function, as DestroyContext() serve this purpose.
                       - you may pass a ImFontAtlas* pointer to CreateContext() to share a font atlas between contexts. Otherwise CreateContext() will create its own font atlas instance.
                       - removed allocator parameters from CreateContext(), they are now setup with SetAllocatorFunctions(), and shared by all contexts.
                       - removed the default global context and font atlas instance, which were confusing for users of DLL reloading and users of multiple contexts.
 - 2018/01/31 (1.60) - moved sample TTF files from extra_fonts/ to misc/fonts/. If you loaded files directly from the imgui repo you may need to update your paths.
 - 2018/01/11 (1.60) - obsoleted IsAnyWindowHovered() in favor of IsWindowHovered(ImGuiHoveredFlags_AnyWindow). Kept redirection function (will obsolete).
 - 2018/01/11 (1.60) - obsoleted IsAnyWindowFocused() in favor of IsWindowFocused(ImGuiFocusedFlags_AnyWindow). Kept redirection function (will obsolete).
 - 2018/01/03 (1.60) - renamed ImGuiSizeConstraintCallback to ImGuiSizeCallback, ImGuiSizeConstraintCallbackData to ImGuiSizeCallbackData.
 - 2017/12/29 (1.60) - removed CalcItemRectClosestPoint() which was weird and not really used by anyone except demo code. If you need it it's easy to replicate on your side.
 - 2017/12/24 (1.53) - renamed the emblematic ShowTestWindow() function to ShowDemoWindow(). Kept redirection function (will obsolete).
 - 2017/12/21 (1.53) - ImDrawList: renamed style.AntiAliasedShapes to style.AntiAliasedFill for consistency and as a way to explicitly break code that manipulate those flag at runtime. You can now manipulate ImDrawList::Flags
 - 2017/12/21 (1.53) - ImDrawList: removed 'bool anti_aliased = true' final parameter of ImDrawList::AddPolyline() and ImDrawList::AddConvexPolyFilled(). Prefer manipulating ImDrawList::Flags if you need to toggle them during the frame.
 - 2017/12/14 (1.53) - using the ImGuiWindowFlags_NoScrollWithMouse flag on a child window forwards the mouse wheel event to the parent window, unless either ImGuiWindowFlags_NoInputs or ImGuiWindowFlags_NoScrollbar are also set.
 - 2017/12/13 (1.53) - renamed GetItemsLineHeightWithSpacing() to GetFrameHeightWithSpacing(). Kept redirection function (will obsolete).
 - 2017/12/13 (1.53) - obsoleted IsRootWindowFocused() in favor of using IsWindowFocused(ImGuiFocusedFlags_RootWindow). Kept redirection function (will obsolete).
                     - obsoleted IsRootWindowOrAnyChildFocused() in favor of using IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows). Kept redirection function (will obsolete).
 - 2017/12/12 (1.53) - renamed ImGuiTreeNodeFlags_AllowOverlapMode to ImGuiTreeNodeFlags_AllowItemOverlap. Kept redirection enum (will obsolete).
 - 2017/12/10 (1.53) - removed SetNextWindowContentWidth(), prefer using SetNextWindowContentSize(). Kept redirection function (will obsolete).
 - 2017/11/27 (1.53) - renamed ImGuiTextBuffer::append() helper to appendf(), appendv() to appendfv(). If you copied the 'Log' demo in your code, it uses appendv() so that needs to be renamed.
 - 2017/11/18 (1.53) - Style, Begin: removed ImGuiWindowFlags_ShowBorders window flag. Borders are now fully set up in the ImGuiStyle structure (see e.g. style.FrameBorderSize, style.WindowBorderSize). Use ImGui::ShowStyleEditor() to look them up.
                       Please note that the style system will keep evolving (hopefully stabilizing in Q1 2018), and so custom styles will probably subtly break over time. It is recommended you use the StyleColorsClassic(), StyleColorsDark(), StyleColorsLight() functions.
 - 2017/11/18 (1.53) - Style: removed ImGuiCol_ComboBg in favor of combo boxes using ImGuiCol_PopupBg for consistency.
 - 2017/11/18 (1.53) - Style: renamed ImGuiCol_ChildWindowBg to ImGuiCol_ChildBg.
 - 2017/11/18 (1.53) - Style: renamed style.ChildWindowRounding to style.ChildRounding, ImGuiStyleVar_ChildWindowRounding to ImGuiStyleVar_ChildRounding.
 - 2017/11/02 (1.53) - obsoleted IsRootWindowOrAnyChildHovered() in favor of using IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
 - 2017/10/24 (1.52) - renamed IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS/IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS to IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS/IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS for consistency.
 - 2017/10/20 (1.52) - changed IsWindowHovered() default parameters behavior to return false if an item is active in another window (e.g. click-dragging item from another window to this window). You can use the newly introduced IsWindowHovered() flags to requests this specific behavior if you need it.
 - 2017/10/20 (1.52) - marked IsItemHoveredRect()/IsMouseHoveringWindow() as obsolete, in favor of using the newly introduced flags for IsItemHovered() and IsWindowHovered(). See https://github.com/ocornut/imgui/issues/1382 for details.
                       removed the IsItemRectHovered()/IsWindowRectHovered() names introduced in 1.51 since they were merely more consistent names for the two functions we are now obsoleting.
                         IsItemHoveredRect()        --> IsItemHovered(ImGuiHoveredFlags_RectOnly)
                         IsMouseHoveringAnyWindow() --> IsWindowHovered(ImGuiHoveredFlags_AnyWindow)
                         IsMouseHoveringWindow()    --> IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) [weird, old behavior]
 - 2017/10/17 (1.52) - marked the old 5-parameters version of Begin() as obsolete (still available). Use SetNextWindowSize()+Begin() instead!
 - 2017/10/11 (1.52) - renamed AlignFirstTextHeightToWidgets() to AlignTextToFramePadding(). Kept inline redirection function (will obsolete).
 - 2017/09/26 (1.52) - renamed ImFont::Glyph to ImFontGlyph. Kept redirection typedef (will obsolete).
 - 2017/09/25 (1.52) - removed SetNextWindowPosCenter() because SetNextWindowPos() now has the optional pivot information to do the same and more. Kept redirection function (will obsolete).
 - 2017/08/25 (1.52) - io.MousePos needs to be set to ImVec2(-FLT_MAX,-FLT_MAX) when mouse is unavailable/missing. Previously ImVec2(-1,-1) was enough but we now accept negative mouse coordinates. In your backend if you need to support unavailable mouse, make sure to replace "io.MousePos = ImVec2(-1,-1)" with "io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX)".
 - 2017/08/22 (1.51) - renamed IsItemHoveredRect() to IsItemRectHovered(). Kept inline redirection function (will obsolete). -> (1.52) use IsItemHovered(ImGuiHoveredFlags_RectOnly)!
                     - renamed IsMouseHoveringAnyWindow() to IsAnyWindowHovered() for consistency. Kept inline redirection function (will obsolete).
                     - renamed IsMouseHoveringWindow() to IsWindowRectHovered() for consistency. Kept inline redirection function (will obsolete).
 - 2017/08/20 (1.51) - renamed GetStyleColName() to GetStyleColorName() for consistency.
 - 2017/08/20 (1.51) - added PushStyleColor(ImGuiCol idx, ImU32 col) overload, which _might_ cause an "ambiguous call" compilation error if you are using ImColor() with implicit cast. Cast to ImU32 or ImVec4 explicitly to fix.
 - 2017/08/15 (1.51) - marked the weird IMGUI_ONCE_UPON_A_FRAME helper macro as obsolete. prefer using the more explicit ImGuiOnceUponAFrame type.
 - 2017/08/15 (1.51) - changed parameter order for BeginPopupContextWindow() from (const char*,int buttons,bool also_over_items) to (const char*,int buttons,bool also_over_items). Note that most calls relied on default parameters completely.
 - 2017/08/13 (1.51) - renamed ImGuiCol_Column to ImGuiCol_Separator, ImGuiCol_ColumnHovered to ImGuiCol_SeparatorHovered, ImGuiCol_ColumnActive to ImGuiCol_SeparatorActive. Kept redirection enums (will obsolete).
 - 2017/08/11 (1.51) - renamed ImGuiSetCond_Always to ImGuiCond_Always, ImGuiSetCond_Once to ImGuiCond_Once, ImGuiSetCond_FirstUseEver to ImGuiCond_FirstUseEver, ImGuiSetCond_Appearing to ImGuiCond_Appearing. Kept redirection enums (will obsolete).
 - 2017/08/09 (1.51) - removed ValueColor() helpers, they are equivalent to calling Text(label) + SameLine() + ColorButton().
 - 2017/08/08 (1.51) - removed ColorEditMode() and ImGuiColorEditMode in favor of ImGuiColorEditFlags and parameters to the various Color*() functions. The SetColorEditOptions() allows to initialize default but the user can still change them with right-click context menu.
                     - changed prototype of 'ColorEdit4(const char* label, float col[4], bool show_alpha = true)' to 'ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0)', where passing flags = 0x01 is a safe no-op (hello dodgy backward compatibility!). - check and run the demo window, under "Color/Picker Widgets", to understand the various new options.
                     - changed prototype of rarely used 'ColorButton(ImVec4 col, bool small_height = false, bool outline_border = true)' to 'ColorButton(const char* desc_id, ImVec4 col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0, 0))'
 - 2017/07/20 (1.51) - removed IsPosHoveringAnyWindow(ImVec2), which was partly broken and misleading. ASSERT + redirect user to io.WantCaptureMouse
 - 2017/05/26 (1.50) - removed ImFontConfig::MergeGlyphCenterV in favor of a more multipurpose ImFontConfig::GlyphOffset.
 - 2017/05/01 (1.50) - renamed ImDrawList::PathFill() (rarely used directly) to ImDrawList::PathFillConvex() for clarity.
 - 2016/11/06 (1.50) - BeginChild(const char*) now applies the stack id to the provided label, consistently with other functions as it should always have been. It shouldn't affect you unless (extremely unlikely) you were appending multiple times to a same child from different locations of the stack id. If that's the case, generate an id with GetID() and use it instead of passing string to BeginChild().
 - 2016/10/15 (1.50) - avoid 'void* user_data' parameter to io.SetClipboardTextFn/io.GetClipboardTextFn pointers. We pass io.ClipboardUserData to it.
 - 2016/09/25 (1.50) - style.WindowTitleAlign is now a ImVec2 (ImGuiAlign enum was removed). set to (0.5f,0.5f) for horizontal+vertical centering, (0.0f,0.0f) for upper-left, etc.
 - 2016/07/30 (1.50) - SameLine(x) with x>0.0f is now relative to left of column/group if any, and not always to left of window. This was sort of always the intent and hopefully, breakage should be minimal.
 - 2016/05/12 (1.49) - title bar (using ImGuiCol_TitleBg/ImGuiCol_TitleBgActive colors) isn't rendered over a window background (ImGuiCol_WindowBg color) anymore.
                       If your TitleBg/TitleBgActive alpha was 1.0f or you are using the default theme it will not affect you, otherwise if <1.0f you need to tweak your custom theme to readjust for the fact that we don't draw a WindowBg background behind the title bar.
                       This helper function will convert an old TitleBg/TitleBgActive color into a new one with the same visual output, given the OLD color and the OLD WindowBg color:
                       ImVec4 ConvertTitleBgCol(const ImVec4& win_bg_col, const ImVec4& title_bg_col) { float new_a = 1.0f - ((1.0f - win_bg_col.w) * (1.0f - title_bg_col.w)), k = title_bg_col.w / new_a; return ImVec4((win_bg_col.x * win_bg_col.w + title_bg_col.x) * k, (win_bg_col.y * win_bg_col.w + title_bg_col.y) * k, (win_bg_col.z * win_bg_col.w + title_bg_col.z) * k, new_a); }
                       If this is confusing, pick the RGB value from title bar from an old screenshot and apply this as TitleBg/TitleBgActive. Or you may just create TitleBgActive from a tweaked TitleBg color.
 - 2016/05/07 (1.49) - removed confusing set of GetInternalState(), GetInternalStateSize(), SetInternalState() functions. Now using CreateContext(), DestroyContext(), GetCurrentContext(), SetCurrentContext().
 - 2016/05/02 (1.49) - renamed SetNextTreeNodeOpened() to SetNextTreeNodeOpen(), no redirection.
 - 2016/05/01 (1.49) - obsoleted old signature of CollapsingHeader(const char* label, const char* str_id = NULL, bool display_frame = true, bool default_open = false) as extra parameters were badly designed and rarely used. You can replace the "default_open = true" flag in new API with CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen).
 - 2016/04/26 (1.49) - changed ImDrawList::PushClipRect(ImVec4 rect) to ImDrawList::PushClipRect(Imvec2 min,ImVec2 max,bool intersect_with_current_clip_rect=false). Note that higher-level ImGui::PushClipRect() is preferable because it will clip at logic/widget level, whereas ImDrawList::PushClipRect() only affect your renderer.
 - 2016/04/03 (1.48) - removed style.WindowFillAlphaDefault setting which was redundant. Bake default BG alpha inside style.Colors[ImGuiCol_WindowBg] and all other Bg color values. (ref GitHub issue #337).
 - 2016/04/03 (1.48) - renamed ImGuiCol_TooltipBg to ImGuiCol_PopupBg, used by popups/menus and tooltips. popups/menus were previously using ImGuiCol_WindowBg. (ref github issue #337)
 - 2016/03/21 (1.48) - renamed GetWindowFont() to GetFont(), GetWindowFontSize() to GetFontSize(). Kept inline redirection function (will obsolete).
 - 2016/03/02 (1.48) - InputText() completion/history/always callbacks: if you modify the text buffer manually (without using DeleteChars()/InsertChars() helper) you need to maintain the BufTextLen field. added an assert.
 - 2016/01/23 (1.48) - fixed not honoring exact width passed to PushItemWidth(), previously it would add extra FramePadding.x*2 over that width. if you had manual pixel-perfect alignment in place it might affect you.
 - 2015/12/27 (1.48) - fixed ImDrawList::AddRect() which used to render a rectangle 1 px too large on each axis.
 - 2015/12/04 (1.47) - renamed Color() helpers to ValueColor() - dangerously named, rarely used and probably to be made obsolete.
 - 2015/08/29 (1.45) - with the addition of horizontal scrollbar we made various fixes to inconsistencies with dealing with cursor position.
                       GetCursorPos()/SetCursorPos() functions now include the scrolled amount. It shouldn't affect the majority of users, but take note that SetCursorPosX(100.0f) puts you at +100 from the starting x position which may include scrolling, not at +100 from the window left side.
                       GetContentRegionMax()/GetWindowContentRegionMin()/GetWindowContentRegionMax() functions allow include the scrolled amount. Typically those were used in cases where no scrolling would happen so it may not be a problem, but watch out!
 - 2015/08/29 (1.45) - renamed style.ScrollbarWidth to style.ScrollbarSize
 - 2015/08/05 (1.44) - split imgui.cpp into extra files: imgui_demo.cpp imgui_draw.cpp imgui_internal.h that you need to add to your project.
 - 2015/07/18 (1.44) - fixed angles in ImDrawList::PathArcTo(), PathArcToFast() (introduced in 1.43) being off by an extra PI for no justifiable reason
 - 2015/07/14 (1.43) - add new ImFontAtlas::AddFont() API. For the old AddFont***, moved the 'font_no' parameter of ImFontAtlas::AddFont** functions to the ImFontConfig structure.
                       you need to render your textured triangles with bilinear filtering to benefit from sub-pixel positioning of text.
 - 2015/07/08 (1.43) - switched rendering data to use indexed rendering. this is saving a fair amount of CPU/GPU and enables us to get anti-aliasing for a marginal cost.
                       this necessary change will break your rendering function! the fix should be very easy. sorry for that :(
                     - if you are using a vanilla copy of one of the imgui_impl_XXX.cpp provided in the example, you just need to update your copy and you can ignore the rest.
                     - the signature of the io.RenderDrawListsFn handler has changed!
                       old: ImGui_XXXX_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
                       new: ImGui_XXXX_RenderDrawLists(ImDrawData* draw_data).
                         parameters: 'cmd_lists' becomes 'draw_data->CmdLists', 'cmd_lists_count' becomes 'draw_data->CmdListsCount'
                         ImDrawList: 'commands' becomes 'CmdBuffer', 'vtx_buffer' becomes 'VtxBuffer', 'IdxBuffer' is new.
                         ImDrawCmd:  'vtx_count' becomes 'ElemCount', 'clip_rect' becomes 'ClipRect', 'user_callback' becomes 'UserCallback', 'texture_id' becomes 'TextureId'.
                     - each ImDrawList now contains both a vertex buffer and an index buffer. For each command, render ElemCount/3 triangles using indices from the index buffer.
                     - if you REALLY cannot render indexed primitives, you can call the draw_data->DeIndexAllBuffers() method to de-index the buffers. This is slow and a waste of CPU/GPU. Prefer using indexed rendering!
                     - refer to code in the examples/ folder or ask on the GitHub if you are unsure of how to upgrade. please upgrade!
 - 2015/07/10 (1.43) - changed SameLine() parameters from int to float.
 - 2015/07/02 (1.42) - renamed SetScrollPosHere() to SetScrollFromCursorPos(). Kept inline redirection function (will obsolete).
 - 2015/07/02 (1.42) - renamed GetScrollPosY() to GetScrollY(). Necessary to reduce confusion along with other scrolling functions, because positions (e.g. cursor position) are not equivalent to scrolling amount.
 - 2015/06/14 (1.41) - changed ImageButton() default bg_col parameter from (0,0,0,1) (black) to (0,0,0,0) (transparent) - makes a difference when texture have transparence
 - 2015/06/14 (1.41) - changed Selectable() API from (label, selected, size) to (label, selected, flags, size). Size override should have been rarely used. Sorry!
 - 2015/05/31 (1.40) - renamed GetWindowCollapsed() to IsWindowCollapsed() for consistency. Kept inline redirection function (will obsolete).
 - 2015/05/31 (1.40) - renamed IsRectClipped() to IsRectVisible() for consistency. Note that return value is opposite! Kept inline redirection function (will obsolete).
 - 2015/05/27 (1.40) - removed the third 'repeat_if_held' parameter from Button() - sorry! it was rarely used and inconsistent. Use PushButtonRepeat(true) / PopButtonRepeat() to enable repeat on desired buttons.
 - 2015/05/11 (1.40) - changed BeginPopup() API, takes a string identifier instead of a bool. ImGui needs to manage the open/closed state of popups. Call OpenPopup() to actually set the "open" state of a popup. BeginPopup() returns true if the popup is opened.
 - 2015/05/03 (1.40) - removed style.AutoFitPadding, using style.WindowPadding makes more sense (the default values were already the same).
 - 2015/04/13 (1.38) - renamed IsClipped() to IsRectClipped(). Kept inline redirection function until 1.50.
 - 2015/04/09 (1.38) - renamed ImDrawList::AddArc() to ImDrawList::AddArcFast() for compatibility with future API
 - 2015/04/03 (1.38) - removed ImGuiCol_CheckHovered, ImGuiCol_CheckActive, replaced with the more general ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive.
 - 2014/04/03 (1.38) - removed support for passing -FLT_MAX..+FLT_MAX as the range for a SliderFloat(). Use DragFloat() or Inputfloat() instead.
 - 2015/03/17 (1.36) - renamed GetItemBoxMin()/GetItemBoxMax()/IsMouseHoveringBox() to GetItemRectMin()/GetItemRectMax()/IsMouseHoveringRect(). Kept inline redirection function until 1.50.
 - 2015/03/15 (1.36) - renamed style.TreeNodeSpacing to style.IndentSpacing, ImGuiStyleVar_TreeNodeSpacing to ImGuiStyleVar_IndentSpacing
 - 2015/03/13 (1.36) - renamed GetWindowIsFocused() to IsWindowFocused(). Kept inline redirection function until 1.50.
 - 2015/03/08 (1.35) - renamed style.ScrollBarWidth to style.ScrollbarWidth (casing)
 - 2015/02/27 (1.34) - renamed OpenNextNode(bool) to SetNextTreeNodeOpened(bool, ImGuiSetCond). Kept inline redirection function until 1.50.
 - 2015/02/27 (1.34) - renamed ImGuiSetCondition_*** to ImGuiSetCond_***, and _FirstUseThisSession becomes _Once.
 - 2015/02/11 (1.32) - changed text input callback ImGuiTextEditCallback return type from void-->int. reserved for future use, return 0 for now.
 - 2015/02/10 (1.32) - renamed GetItemWidth() to CalcItemWidth() to clarify its evolving behavior
 - 2015/02/08 (1.31) - renamed GetTextLineSpacing() to GetTextLineHeightWithSpacing()
 - 2015/02/01 (1.31) - removed IO.MemReallocFn (unused)
 - 2015/01/19 (1.30) - renamed ImGuiStorage::GetIntPtr()/GetFloatPtr() to GetIntRef()/GetIntRef() because Ptr was conflicting with actual pointer storage functions.
 - 2015/01/11 (1.30) - big font/image API change! now loads TTF file. allow for multiple fonts. no need for a PNG loader.
 - 2015/01/11 (1.30) - removed GetDefaultFontData(). uses io.Fonts->GetTextureData*() API to retrieve uncompressed pixels.
                       - old:  const void* png_data; unsigned int png_size; ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size); [..Upload texture to GPU..];
                       - new:  unsigned char* pixels; int width, height; io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); [..Upload texture to GPU..]; io.Fonts->SetTexID(YourTexIdentifier);
                       you now have more flexibility to load multiple TTF fonts and manage the texture buffer for internal needs. It is now recommended that you sample the font texture with bilinear interpolation.
 - 2015/01/11 (1.30) - added texture identifier in ImDrawCmd passed to your render function (we can now render images). make sure to call io.Fonts->SetTexID()
 - 2015/01/11 (1.30) - removed IO.PixelCenterOffset (unnecessary, can be handled in user projection matrix)
 - 2015/01/11 (1.30) - removed ImGui::IsItemFocused() in favor of ImGui::IsItemActive() which handles all widgets
 - 2014/12/10 (1.18) - removed SetNewWindowDefaultPos() in favor of new generic API SetNextWindowPos(pos, ImGuiSetCondition_FirstUseEver)
 - 2014/11/28 (1.17) - moved IO.Font*** options to inside the IO.Font-> structure (FontYOffset, FontTexUvForWhite, FontBaseScale, FontFallbackGlyph)
 - 2014/11/26 (1.17) - reworked syntax of IMGUI_ONCE_UPON_A_FRAME helper macro to increase compiler compatibility
 - 2014/11/07 (1.15) - renamed IsHovered() to IsItemHovered()
 - 2014/10/02 (1.14) - renamed IMGUI_INCLUDE_IMGUI_USER_CPP to IMGUI_INCLUDE_IMGUI_USER_INL and imgui_user.cpp to imgui_user.inl (more IDE friendly)
 - 2014/09/25 (1.13) - removed 'text_end' parameter from IO.SetClipboardTextFn (the string is now always zero-terminated for simplicity)
 - 2014/09/24 (1.12) - renamed SetFontScale() to SetWindowFontScale()
 - 2014/09/24 (1.12) - moved IM_MALLOC/IM_REALLOC/IM_FREE preprocessor defines to IO.MemAllocFn/IO.MemReallocFn/IO.MemFreeFn
 - 2014/08/30 (1.09) - removed IO.FontHeight (now computed automatically)
 - 2014/08/30 (1.09) - moved IMGUI_FONT_TEX_UV_FOR_WHITE preprocessor define to IO.FontTexUvForWhite
 - 2014/08/28 (1.09) - changed the behavior of IO.PixelCenterOffset following various rendering fixes


 FREQUENTLY ASKED QUESTIONS (FAQ)
 ================================

 Read all answers online:
   https://www.dearimgui.com/faq or https://github.com/ocornut/imgui/blob/master/docs/FAQ.md (same url)
 Read all answers locally (with a text editor or ideally a Markdown viewer):
   docs/FAQ.md
 Some answers are copied down here to facilitate searching in code.

 Q&A: Basics
 ===========

 Q: Where is the documentation?
 A: This library is poorly documented at the moment and expects the user to be acquainted with C/C++.
    - Run the examples/ applications and explore them.
    - Read Getting Started (https://github.com/ocornut/imgui/wiki/Getting-Started) guide.
    - See demo code in imgui_demo.cpp and particularly the ImGui::ShowDemoWindow() function.
    - See pthom's online imgui_explorer (https://pthom.github.io/imgui_explorer) which is a web
      version of the demo with a source code browser.
    - The demo covers most features of Dear ImGui, so you can read the code and see its output.
    - See documentation and comments at the top of imgui.cpp + effectively imgui.h.
    - 20+ standalone example applications using e.g. OpenGL/DirectX are provided in the
      examples/ folder to explain how to integrate Dear ImGui with your own engine/application.
    - The Wiki (https://github.com/ocornut/imgui/wiki) has many resources and links.
    - The Glossary (https://github.com/ocornut/imgui/wiki/Glossary) page also may be useful.
    - Your programming IDE is your friend, find the type or function declaration to find comments
      associated with it.

 Q: What is this library called?
 Q: What is the difference between Dear ImGui and traditional UI toolkits?
 Q: Which version should I get?
 >> This library is called "Dear ImGui", please don't call it "ImGui" :)
 >> See https://www.dearimgui.com/faq for details.

 Q&A: Integration
 ================

 Q: How to get started?
 A: Read https://github.com/ocornut/imgui/wiki/Getting-Started. Read 'PROGRAMMER GUIDE' above. Read examples/README.txt.

 Q: How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?
 A: You should read the 'io.WantCaptureMouse', 'io.WantCaptureKeyboard' and 'io.WantTextInput' flags!
 >> See https://www.dearimgui.com/faq for a fully detailed answer. You really want to read this.

 Q. How can I enable keyboard or gamepad controls?
 Q: How can I use this on a machine without mouse, keyboard or screen? (input share, remote display)
 Q: I integrated Dear ImGui in my engine and little squares are showing instead of text...
 Q: I integrated Dear ImGui in my engine and some elements are clipping or disappearing when I move windows around...
 Q: I integrated Dear ImGui in my engine and some elements are displaying outside their expected windows boundaries...
 >> See https://www.dearimgui.com/faq

 Q&A: Usage
 ----------

 Q: About the ID Stack system..
   - How can I have multiple widgets with the same label? (using ## or PushID)
   - How can I have widgets with an empty label? (using ##)
   - How can I make a label dynamic? (using ###)
   - General description of the label and ID Stack system.
 Q: How can I display an image? What is ImTextureID, how does it work?
 Q: How can I use my own math types instead of ImVec2?
 Q: How can I interact with standard C++ types (such as std::string and std::vector)?
 Q: How can I display custom shapes? (using low-level ImDrawList API)
 >> See https://www.dearimgui.com/faq

 Q&A: Fonts, Text
 ================

 Q: How should I handle DPI in my application?
 Q: How can I load a different font than the default?
 Q: How can I easily use icons in my application?
 Q: How can I load multiple fonts?
 Q: How can I display and input non-Latin characters such as Chinese, Japanese, Korean, Cyrillic?
 >> See https://www.dearimgui.com/faq and https://github.com/ocornut/imgui/blob/master/docs/FONTS.md

 Q&A: Concerns
 =============

 Q: Who uses Dear ImGui?
 Q: Can you create elaborate/serious tools with Dear ImGui?
 Q: Can you reskin the look of Dear ImGui?
 Q: Why using C++ (as opposed to C)?
 >> See https://www.dearimgui.com/faq

 Q&A: Community
 ==============

 Q: How can I help?
 A: - Businesses: please reach out to "omar AT dearimgui DOT com" if you work in a place using Dear ImGui!
      We can discuss ways for your company to fund development via invoiced technical support, maintenance or sponsoring contacts.
      This is among the most useful thing you can do for Dear ImGui. With increased funding, we sustain and grow work on this project.
      >>> See https://github.com/ocornut/imgui/wiki/Funding
    - Businesses: you can also purchase licenses for the Dear ImGui Automation/Test Engine.
    - If you are experienced with Dear ImGui and C++, look at the GitHub issues, look at the Wiki, and see how you want to help and can help!
    - Disclose your usage of Dear ImGui via a dev blog post, a tweet, a screenshot, a mention somewhere etc.
      You may post screenshot or links in the gallery threads. Visuals are ideal as they inspire other programmers.
      But even without visuals, disclosing your use of dear imgui helps the library grow credibility, and help other teams and programmers with taking decisions.
    - If you have issues or if you need to hack into the library, even if you don't expect any support it is useful that you share your issues (on GitHub or privately).

*/

//-------------------------------------------------------------------------
// [SECTION] INCLUDES
//-------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"

// System includes
#include <stdio.h>      // vsnprintf, sscanf, printf
#include <stdint.h>     // intptr_t

// io moudel
#include <IO.h> 

#include "imgui_platform_private.h"

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)             // condition expression is constant
#pragma warning (disable: 4996)             // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#if defined(_MSC_VER) && _MSC_VER >= 1922   // MSVC 2019 16.2 or later
#pragma warning (disable: 5054)             // operator '|': deprecated between enumerations of different types
#endif
#pragma warning (disable: 26451)            // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to an 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26495)            // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#pragma warning (disable: 26812)            // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat"                         // warning: format specifies type 'int' but the argument has type 'unsigned int'
#pragma clang diagnostic ignored "-Wformat-nonliteral"              // warning: format string is not a string literal            // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wformat-pedantic"                // warning: format specifies type 'void *' but the argument has type 'xxxx *' // unreasonable, would lead to casting every %p arg to void*. probably enabled by -pedantic.
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor     // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wglobal-constructors"            // warning: declaration requires a global destructor         // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type 'int'
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#pragma clang diagnostic ignored "-Wswitch-default"                 // warning: 'switch' missing 'default' label
#elif defined(__GNUC__)
// We disable -Wpragmas because GCC doesn't provide a has_warning equivalent and some forks/patches may not follow the warning/version association.
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"                  // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"              // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wformat"                           // warning: format '%p' expects argument of type 'int'/'void*', but argument X has type 'unsigned int'/'ImGuiWindow*'
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wformat-nonliteral"                // warning: format not a string literal, format string not checked
#pragma GCC diagnostic ignored "-Wstrict-overflow"                  // warning: assuming signed overflow does not occur when assuming that (X - c) > X is always false
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#pragma GCC diagnostic ignored "-Wcast-qual"                        // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#pragma GCC diagnostic ignored "-Wsign-conversion"                  // warning: conversion to 'xxxx' from 'xxxx' may change the sign of the result
#endif

// Debug options

// Default font size if unspecified in both style.FontSizeBase and AddFontXXX() calls.
static const float FONT_DEFAULT_SIZE_BASE = 20.0f;

// When using Ctrl+Tab (or Gamepad Square+L/R) we delay the visual a little in order to reduce visual noise doing a fast switch.
// Tooltip offset
// Docking
static const float DOCKING_TRANSPARENT_PAYLOAD_ALPHA        = 0.50f;    // For use with io.ConfigDockingTransparentPayload. Apply to Viewport _or_ WindowBg in host viewport.

//-------------------------------------------------------------------------
// [SECTION] FORWARD DECLARATIONS
//-------------------------------------------------------------------------

static void             SetCurrentWindow(ImGuiWindow* window);
static ImGuiWindow*     CreateNewWindow(const char* name, ImGuiWindowFlags flags);

static void             AddWindowToSortBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window);

// Settings
static void             WindowSettingsHandler_ClearAll(ImGuiContext*, ImGuiSettingsHandler*);
static void*            WindowSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
static void             WindowSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
static void             WindowSettingsHandler_ApplyAll(ImGuiContext*, ImGuiSettingsHandler*);
static void             WindowSettingsHandler_WriteAll(ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* buf);

namespace ImGui
{
// Item

// Window Focus
static void             UpdateWindowInFocusOrderList(ImGuiWindow* window, bool just_created, ImGuiWindowFlags new_flags);

// Navigation

// Misc
static void             UpdateFontsNewFrame();
static void             UpdateFontsEndFrame();
static void             UpdateTexturesNewFrame();
static void             UpdateTexturesEndFrame();
static void             UpdateSettings();
static int              UpdateWindowManualResize(ImGuiWindow* window, int* border_hovered, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect);
static void             RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size);
static void             RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open);
static void             RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col);
static void             RenderDimmedBackgrounds();
static void             SetLastItemDataForWindow(ImGuiWindow* window, const ImRect& rect);
static void             SetLastItemDataForChildWindowItem(ImGuiWindow* window, const ImRect& rect);

// Viewports

}

//-----------------------------------------------------------------------------
// [SECTION] CONTEXT AND MEMORY ALLOCATORS
//-----------------------------------------------------------------------------

// DLL users:
// - Heaps and globals are not shared across DLL boundaries!
// - You will need to call SetCurrentContext() + SetAllocatorFunctions() for each static/DLL boundary you are calling from.
// - Same applies for hot-reloading mechanisms that are reliant on reloading DLL (note that many hot-reloading mechanisms work without DLL).
// - Using Dear ImGui via a shared library is not recommended, because of function call overhead and because we don't guarantee backward nor forward ABI compatibility.
// - Confused? In a debugger: add GImGui to your watch window and notice how its value changes depending on your current location (which DLL boundary you are in).

// Current context pointer. Implicitly used by all Dear ImGui functions. Always assumed to be != NULL.
// - ImGui::CreateContext() will automatically set this pointer if it is NULL.
//   Change to a different context by calling ImGui::SetCurrentContext().
// - Important: Dear ImGui functions are not thread-safe because of this pointer.
//   If you want thread-safety to allow N threads to access N different contexts:
//   - Change this variable to use thread local storage so each thread can refer to a different context, in your imconfig.h:
//         struct ImGuiContext;
//         extern thread_local ImGuiContext* MyImGuiTLS;
//         #define GImGui MyImGuiTLS
//     And then define MyImGuiTLS in one of your cpp files. Note that thread_local is a C++11 keyword, earlier C++ uses compiler-specific keyword.
//   - Future development aims to make this context pointer explicit to all calls. Also read https://github.com/ocornut/imgui/issues/586
//   - If you need a finite number of contexts, you may compile and use multiple instances of the ImGui code from a different namespace.
// - DLL users: read comments above.
#ifndef GImGui
ImGuiContext*   GImGui = NULL;
#endif

// Memory Allocator functions. Use SetAllocatorFunctions() to change them.
// - You probably don't want to modify that mid-program, and if you use global/static e.g. ImVector<> instances you may need to keep them accessible during program destruction.
// - DLL users: read comments above.
#ifndef IMGUI_DISABLE_DEFAULT_ALLOCATORS
static void*   MallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); return malloc(size); }
static void    FreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); free(ptr); }
#else
static void*   MallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); IM_UNUSED(size); IM_ASSERT(0); return NULL; }
static void    FreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); IM_UNUSED(ptr); IM_ASSERT(0); }
#endif
static ImGuiMemAllocFunc    GImAllocatorAllocFunc = MallocWrapper;
static ImGuiMemFreeFunc     GImAllocatorFreeFunc = FreeWrapper;
static void*                GImAllocatorUserData = NULL;

//-----------------------------------------------------------------------------
// [SECTION] USER FACING STRUCTURES (ImGuiStyle, ImGuiIO, ImGuiPlatformIO)
//-----------------------------------------------------------------------------

ImGuiStyle::ImGuiStyle()
{
    FontSizeBase                = 0.0f;             // Will default to io.Fonts->Fonts[0] on first frame.
    FontScaleMain               = 1.0f;             // Main scale factor. May be set by application once, or exposed to end-user.
    FontScaleDpi                = 1.0f;             // Additional scale factor from viewport/monitor contents scale. When io.ConfigDpiScaleFonts is enabled, this is automatically overwritten when changing monitor DPI.

    Alpha                       = 1.0f;             // Global alpha applies to everything in Dear ImGui.
    DisabledAlpha               = 0.60f;            // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    WindowPadding               = ImVec2(8,8);      // Padding within a window
    WindowRounding              = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    WindowBorderSize            = 1.0f;             // Thickness of border around windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    WindowBorderHoverPadding    = 4.0f;             // Hit-testing extent outside/inside resizing border. Also extend determination of hovered window. Generally meaningfully larger than WindowBorderSize to make it easy to reach borders.
    WindowMinSize               = ImVec2(32,32);    // Minimum window size
    WindowTitleAlign            = ImVec2(0.0f,0.5f);// Alignment for title bar text
    WindowMenuButtonPosition    = ImGuiDir_Left;    // Position of the collapsing/docking button in the title bar (left/right). Defaults to ImGuiDir_Left.
    ChildRounding               = 0.0f;             // Radius of child window corners rounding. Set to 0.0f to have rectangular child windows
    ChildBorderSize             = 1.0f;             // Thickness of border around child windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    PopupRounding               = 0.0f;             // Radius of popup window corners rounding. Set to 0.0f to have rectangular child windows
    PopupBorderSize             = 1.0f;             // Thickness of border around popup or tooltip windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    FramePadding                = ImVec2(4,3);      // Padding within a framed rectangle (used by most widgets)
    FrameRounding               = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
    FrameBorderSize             = 0.0f;             // Thickness of border around frames. Generally set to 0.0f or 1.0f. Other values not well tested.
    ItemSpacing                 = ImVec2(8,4);      // Horizontal and vertical spacing between widgets/lines
    ItemInnerSpacing            = ImVec2(4,4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
    CellPadding                 = ImVec2(4,2);      // Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows.
    TouchExtraPadding           = ImVec2(0,0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    IndentSpacing               = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    ColumnsMinSpacing           = 6.0f;             // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    ScrollbarSize               = 14.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
    ScrollbarRounding           = 9.0f;             // Radius of grab corners rounding for scrollbar
    ScrollbarPadding            = 2.0f;             // Padding of scrollbar grab within its frame (same for both axes)
    GrabMinSize                 = 12.0f;            // Minimum width/height of a grab box for slider/scrollbar
    GrabRounding                = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    LogSliderDeadzone           = 4.0f;             // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    ImageRounding               = 0.0f;             // Rounding of Image() calls.
    ImageBorderSize             = 0.0f;             // Thickness of border around tabs.
    TabRounding                 = 5.0f;             // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    TabBorderSize               = 0.0f;             // Thickness of border around tabs.
    TabMinWidthBase             = 1.0f;             // Minimum tab width, to make tabs larger than their contents. TabBar buttons are not affected.
    TabMinWidthShrink           = 80.0f;            // Minimum tab width after shrinking, when using ImGuiTabBarFlags_FittingPolicyMixed policy. FLT_MAX: never shrink, will behave like ImGuiTabBarFlags_FittingPolicyScroll.
    TabCloseButtonMinWidthSelected   = -1.0f;       // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width.
    TabCloseButtonMinWidthUnselected = 0.0f;        // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never show close button when unselected.
    TabBarBorderSize            = 1.0f;             // Thickness of tab-bar separator, which takes on the tab active color to denote focus.
    TabBarOverlineSize          = 1.0f;             // Thickness of tab-bar overline, which highlights the selected tab-bar.
    TableAngledHeadersAngle     = 35.0f * (IM_PI / 180.0f); // Angle of angled headers (supported values range from -50 degrees to +50 degrees).
    TableAngledHeadersTextAlign = ImVec2(0.5f,0.0f);// Alignment of angled headers within the cell
    TreeLinesFlags              = ImGuiTreeNodeFlags_DrawLinesNone;
    TreeLinesSize               = 1.0f;             // Thickness of outlines when using ImGuiTreeNodeFlags_DrawLines.
    TreeLinesRounding           = 0.0f;             // Radius of lines connecting child nodes to the vertical line.
    DragDropTargetRounding      = 0.0f;             // Radius of the drag and drop target frame.
    DragDropTargetBorderSize    = 2.0f;             // Thickness of the drag and drop target border.
    DragDropTargetPadding       = 3.0f;             // Size to expand the drag and drop target from actual target item size.
    ColorMarkerSize             = 3.0f;             // Size of R/G/B/A color markers for ColorEdit4() and for Drags/Sliders when using ImGuiSliderFlags_ColorMarkers.
    ColorButtonPosition         = ImGuiDir_Right;   // Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
    ButtonTextAlign             = ImVec2(0.5f,0.5f);// Alignment of button text when button is larger than text.
    SelectableTextAlign         = ImVec2(0.0f,0.0f);// Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    SeparatorSize               = 1.0f;             // Thickness of border in Separator().
    SeparatorTextBorderSize     = 3.0f;             // Thickness of border in SeparatorText().
    SeparatorTextAlign          = ImVec2(0.0f,0.5f);// Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center).
    SeparatorTextPadding        = ImVec2(20.0f,3.f);// Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y.
    DisplayWindowPadding        = ImVec2(19,19);    // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
    DisplaySafeAreaPadding      = ImVec2(3,3);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
    DockingNodeHasCloseButton   = true;             // Docking nodes have their own CloseButton() to close all docked windows.
    DockingSeparatorSize        = 2.0f;             // Thickness of resizing border between docked windows
    MouseCursorScale            = 1.0f;             // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
    AntiAliasedLines            = true;             // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU.
    AntiAliasedLinesUseTex      = true;             // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    AntiAliasedFill             = true;             // Enable anti-aliased filled shapes (rounded rectangles, circles, etc.).
    CurveTessellationTol        = 1.25f;            // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    CircleTessellationMaxError  = 0.30f;            // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

    // Behaviors
    HoverStationaryDelay        = 0.15f;            // Delay for IsItemHovered(ImGuiHoveredFlags_Stationary). Time required to consider mouse stationary.
    HoverDelayShort             = 0.15f;            // Delay for IsItemHovered(ImGuiHoveredFlags_DelayShort). Usually used along with HoverStationaryDelay.
    HoverDelayNormal            = 0.40f;            // Delay for IsItemHovered(ImGuiHoveredFlags_DelayNormal). "
    HoverFlagsForTooltipMouse   = ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled;    // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using mouse.
    HoverFlagsForTooltipNav     = ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled;  // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.

    // [Internal]
    _MainScale                  = 1.0f;
    _NextFrameFontSizeBase      = 0.0f;

    // Default theme
    ImGui::StyleColorsDark(this);
}


// Scale all spacing/padding/thickness values. Do not scale fonts.
// Consider not calling this if your initial scale factor if <1.0.
// Important: This operation is lossy because we round all sizes to integer. If you need to change your scale multiples, call this over a freshly initialized ImGuiStyle structure rather than scaling multiple times.
void ImGuiStyle::ScaleAllSizes(float scale_factor)
{
    _MainScale *= scale_factor;
    WindowPadding = ImTrunc(WindowPadding * scale_factor);
    WindowRounding = ImTrunc(WindowRounding * scale_factor);
    WindowBorderSize = ImTrunc(WindowBorderSize * scale_factor);
    WindowMinSize = ImTrunc(WindowMinSize * scale_factor);
    WindowBorderHoverPadding = ImTrunc(WindowBorderHoverPadding * scale_factor);
    ChildRounding = ImTrunc(ChildRounding * scale_factor);
    ChildBorderSize = ImTrunc(ChildBorderSize * scale_factor);
    PopupRounding = ImTrunc(PopupRounding * scale_factor);
    PopupBorderSize = ImTrunc(PopupBorderSize * scale_factor);
    FramePadding = ImTrunc(FramePadding * scale_factor);
    FrameBorderSize = ImTrunc(FrameBorderSize * scale_factor);
    FrameRounding = ImTrunc(FrameRounding * scale_factor);
    ItemSpacing = ImTrunc(ItemSpacing * scale_factor);
    ItemInnerSpacing = ImTrunc(ItemInnerSpacing * scale_factor);
    CellPadding = ImTrunc(CellPadding * scale_factor);
    TouchExtraPadding = ImTrunc(TouchExtraPadding * scale_factor);
    IndentSpacing = ImTrunc(IndentSpacing * scale_factor);
    ColumnsMinSpacing = ImTrunc(ColumnsMinSpacing * scale_factor);
    ScrollbarSize = ImTrunc(ScrollbarSize * scale_factor);
    ScrollbarRounding = ImTrunc(ScrollbarRounding * scale_factor);
    ScrollbarPadding = ImTrunc(ScrollbarPadding * scale_factor);
    GrabMinSize = ImTrunc(GrabMinSize * scale_factor);
    GrabRounding = ImTrunc(GrabRounding * scale_factor);
    LogSliderDeadzone = ImTrunc(LogSliderDeadzone * scale_factor);
    ImageRounding = ImTrunc(ImageRounding * scale_factor);
    ImageBorderSize = ImTrunc(ImageBorderSize * scale_factor);
    TabRounding = ImTrunc(TabRounding * scale_factor);
    TabBorderSize = ImTrunc(TabBorderSize * scale_factor);
    TabMinWidthBase = ImTrunc(TabMinWidthBase * scale_factor);
    TabMinWidthShrink = ImTrunc(TabMinWidthShrink * scale_factor);
    TabCloseButtonMinWidthSelected = (TabCloseButtonMinWidthSelected > 0.0f && TabCloseButtonMinWidthSelected != FLT_MAX) ? ImTrunc(TabCloseButtonMinWidthSelected * scale_factor) : TabCloseButtonMinWidthSelected;
    TabCloseButtonMinWidthUnselected = (TabCloseButtonMinWidthUnselected > 0.0f && TabCloseButtonMinWidthUnselected != FLT_MAX) ? ImTrunc(TabCloseButtonMinWidthUnselected * scale_factor) : TabCloseButtonMinWidthUnselected;
    TabBarBorderSize = ImTrunc(TabBarBorderSize * scale_factor);
    TabBarOverlineSize = ImTrunc(TabBarOverlineSize * scale_factor);
    TreeLinesSize = ImTrunc(TreeLinesSize * scale_factor);
    TreeLinesRounding = ImTrunc(TreeLinesRounding * scale_factor);
    DragDropTargetRounding = ImTrunc(DragDropTargetRounding * scale_factor);
    DragDropTargetBorderSize = ImTrunc(DragDropTargetBorderSize * scale_factor);
    DragDropTargetPadding = ImTrunc(DragDropTargetPadding * scale_factor);
    ColorMarkerSize = ImTrunc(ColorMarkerSize * scale_factor);
    SeparatorSize = ImTrunc(SeparatorSize * scale_factor);
    SeparatorTextBorderSize = ImTrunc(SeparatorTextBorderSize * scale_factor);
    SeparatorTextPadding = ImTrunc(SeparatorTextPadding * scale_factor);
    DockingSeparatorSize = ImTrunc(DockingSeparatorSize * scale_factor);
    DisplayWindowPadding = ImTrunc(DisplayWindowPadding * scale_factor);
    DisplaySafeAreaPadding = ImTrunc(DisplaySafeAreaPadding * scale_factor);
    MouseCursorScale = ImTrunc(MouseCursorScale * scale_factor);
}

ImGuiIO::ImGuiIO()
{
    // Most fields are initialized with zero
    memset((void*)this, 0, sizeof(*this));
    IM_STATIC_ASSERT(IM_COUNTOF(ImGuiIO::MouseDown) == ImGuiMouseButton_COUNT && IM_COUNTOF(ImGuiIO::MouseClicked) == ImGuiMouseButton_COUNT);

    // Settings
    ConfigFlags = ImGuiConfigFlags_None;
    BackendFlags = ImGuiBackendFlags_None;
    DisplaySize = ImVec2(-1.0f, -1.0f);
    DeltaTime = 1.0f / 60.0f;
    IniSavingRate = 5.0f;
    IniFilename = "imgui.ini"; // Important: "imgui.ini" is relative to current working dir, most apps will want to lock this to an absolute path (e.g. same path as executables).
    LogFilename = "imgui_log.txt";
    UserData = NULL;

    Fonts = NULL;
    FontDefault = NULL;
    FontAllowUserScaling = false;
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    FontGlobalScale = 1.0f; // Use style.FontScaleMain instead!
#endif
    DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Keyboard/Gamepad Navigation options
    ConfigNavSwapGamepadButtons = false;
    ConfigNavMoveSetMousePos = false;
    ConfigNavCaptureKeyboard = true;
    ConfigNavEscapeClearFocusItem = true;
    ConfigNavEscapeClearFocusWindow = false;
    ConfigNavCursorVisibleAuto = true;
    ConfigNavCursorVisibleAlways = false;

    // Docking options (when ImGuiConfigFlags_DockingEnable is set)
    ConfigDockingNoSplit = false;
    ConfigDockingNoDockingOver = false;
    ConfigDockingWithShift = false;
    ConfigDockingAlwaysTabBar = false;
    ConfigDockingTransparentPayload = false;

    // Viewport options (when ImGuiConfigFlags_ViewportsEnable is set)
    ConfigViewportsNoAutoMerge = false;
    ConfigViewportsNoTaskBarIcon = false;
    ConfigViewportsNoDecoration = true;
    ConfigViewportsNoDefaultParent = true;
    ConfigViewportsPlatformFocusSetsImGuiFocus = true;

    // Miscellaneous options
    MouseDrawCursor = false;
#ifdef __APPLE__
    ConfigMacOSXBehaviors = true;  // Set Mac OS X style defaults based on __APPLE__ compile time flag
#else
    ConfigMacOSXBehaviors = false;
#endif
    ConfigInputTrickleEventQueue = true;
    ConfigInputTextCursorBlink = true;
    ConfigInputTextEnterKeepActive = false;
    ConfigDragClickToInputText = false;
    ConfigWindowsResizeFromEdges = true;
    ConfigWindowsMoveFromTitleBarOnly = false;
    ConfigWindowsCopyContentsWithCtrlC = false;
    ConfigScrollbarScrollByPage = true;
    ConfigMemoryCompactTimer = 60.0f;
    ConfigDebugIsDebuggerPresent = false;
    ConfigDebugHighlightIdConflicts = true;
    ConfigDebugHighlightIdConflictsShowItemPicker = true;
    ConfigDebugBeginReturnValueOnce = false;
    ConfigDebugBeginReturnValueLoop = false;

    ConfigErrorRecovery = true;
    ConfigErrorRecoveryEnableAssert = true;
    ConfigErrorRecoveryEnableDebugLog = true;
    ConfigErrorRecoveryEnableTooltip = true;

    // Inputs Behaviors
    MouseDoubleClickTime = 0.30f;
    MouseDoubleClickMaxDist = 6.0f;
    MouseDragThreshold = 6.0f;
    KeyRepeatDelay = 0.275f;
    KeyRepeatRate = 0.050f;

    // Platform Functions
    // Note: Initialize() will setup default clipboard/ime handlers.
    BackendPlatformName = BackendRendererName = NULL;
    BackendPlatformUserData = BackendRendererUserData = BackendLanguageUserData = NULL;

    // Input (NB: we already have memset zero the entire structure!)
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    MousePosPrev = ImVec2(-FLT_MAX, -FLT_MAX);
    MouseSource = ImGuiMouseSource_Mouse;
    for (int i = 0; i < IM_COUNTOF(MouseDownDuration); i++) MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
    for (int i = 0; i < IM_COUNTOF(KeysData); i++) { KeysData[i].DownDuration = KeysData[i].DownDurationPrev = -1.0f; }
    AppAcceptingEvents = true;
}

ImGuiPlatformIO::ImGuiPlatformIO()
{
    // Most fields are initialized with zero
    memset((void*)this, 0, sizeof(*this));
    Platform_LocaleDecimalPoint = '.';
}

//-----------------------------------------------------------------------------
// [SECTION] ImGuiStorage
// Helper: Key->value storage
//-----------------------------------------------------------------------------

// std::lower_bound but without the bullshit
ImGuiStoragePair* ImLowerBound(ImGuiStoragePair* in_begin, ImGuiStoragePair* in_end, ImGuiID key)
{
    ImGuiStoragePair* in_p = in_begin;
    for (size_t count = (size_t)(in_end - in_p); count > 0; )
    {
        size_t count2 = count >> 1;
        ImGuiStoragePair* mid = in_p + count2;
        if (mid->key < key)
        {
            in_p = ++mid;
            count -= count2 + 1;
        }
        else
        {
            count = count2;
        }
    }
    return in_p;
}

IM_MSVC_RUNTIME_CHECKS_OFF
static int IMGUI_CDECL PairComparerByID(const void* lhs, const void* rhs)
{
    // We can't just do a subtraction because qsort uses signed integers and subtracting our ID doesn't play well with that.
    ImGuiID lhs_v = ((const ImGuiStoragePair*)lhs)->key;
    ImGuiID rhs_v = ((const ImGuiStoragePair*)rhs)->key;
    return (lhs_v > rhs_v ? +1 : lhs_v < rhs_v ? -1 : 0);
}

// For quicker full rebuild of a storage (instead of an incremental one), you may add all your contents and then sort once.
void ImGuiStorage::BuildSortByKey()
{
    ImQsort(Data.Data, (size_t)Data.Size, sizeof(ImGuiStoragePair), PairComparerByID);
}

int ImGuiStorage::GetInt(ImGuiID key, int default_val) const
{
    ImGuiStoragePair* it = ImLowerBound(const_cast<ImGuiStoragePair*>(Data.Data), const_cast<ImGuiStoragePair*>(Data.Data + Data.Size), key);
    if (it == Data.Data + Data.Size || it->key != key)
        return default_val;
    return it->val_i;
}

bool ImGuiStorage::GetBool(ImGuiID key, bool default_val) const
{
    return GetInt(key, default_val ? 1 : 0) != 0;
}

float ImGuiStorage::GetFloat(ImGuiID key, float default_val) const
{
    ImGuiStoragePair* it = ImLowerBound(const_cast<ImGuiStoragePair*>(Data.Data), const_cast<ImGuiStoragePair*>(Data.Data + Data.Size), key);
    if (it == Data.Data + Data.Size || it->key != key)
        return default_val;
    return it->val_f;
}

void* ImGuiStorage::GetVoidPtr(ImGuiID key) const
{
    ImGuiStoragePair* it = ImLowerBound(const_cast<ImGuiStoragePair*>(Data.Data), const_cast<ImGuiStoragePair*>(Data.Data + Data.Size), key);
    if (it == Data.Data + Data.Size || it->key != key)
        return NULL;
    return it->val_p;
}

// References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
int* ImGuiStorage::GetIntRef(ImGuiID key, int default_val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_i;
}

bool* ImGuiStorage::GetBoolRef(ImGuiID key, bool default_val)
{
    return (bool*)GetIntRef(key, default_val ? 1 : 0);
}

float* ImGuiStorage::GetFloatRef(ImGuiID key, float default_val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_f;
}

void** ImGuiStorage::GetVoidPtrRef(ImGuiID key, void* default_val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_p;
}

// FIXME-OPT: Need a way to reuse the result of lower_bound when doing GetInt()/SetInt() - not too bad because it only happens on explicit interaction (maximum one a frame)
void ImGuiStorage::SetInt(ImGuiID key, int val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        Data.insert(it, ImGuiStoragePair(key, val));
    else
        it->val_i = val;
}

void ImGuiStorage::SetBool(ImGuiID key, bool val)
{
    SetInt(key, val ? 1 : 0);
}

void ImGuiStorage::SetFloat(ImGuiID key, float val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        Data.insert(it, ImGuiStoragePair(key, val));
    else
        it->val_f = val;
}

void ImGuiStorage::SetVoidPtr(ImGuiID key, void* val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        Data.insert(it, ImGuiStoragePair(key, val));
    else
        it->val_p = val;
}

void ImGuiStorage::SetAllInt(int v)
{
    for (int i = 0; i < Data.Size; i++)
        Data[i].val_i = v;
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] ImGuiTextFilter
//-----------------------------------------------------------------------------

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
ImGuiTextFilter::ImGuiTextFilter(const char* default_filter) //-V1077
{
    InputBuf[0] = 0;
    CountGrep = 0;
    if (default_filter)
    {
        ImStrncpy(InputBuf, default_filter, IM_COUNTOF(InputBuf));
        Build();
    }
}

bool ImGuiTextFilter::Draw(const char* label, float width)
{
    if (width != 0.0f)
        ImGui::SetNextItemWidth(width);
    bool value_changed = ImGui::InputText(label, InputBuf, IM_COUNTOF(InputBuf));
    if (value_changed)
        Build();
    return value_changed;
}

void ImGuiTextFilter::ImGuiTextRange::split(char separator, ImVector<ImGuiTextRange>* out) const
{
    out->resize(0);
    const char* wb = b;
    const char* we = wb;
    while (we < e)
    {
        if (*we == separator)
        {
            out->push_back(ImGuiTextRange(wb, we));
            wb = we + 1;
        }
        we++;
    }
    if (wb != we)
        out->push_back(ImGuiTextRange(wb, we));
}

void ImGuiTextFilter::Build()
{
    Filters.resize(0);
    ImGuiTextRange input_range(InputBuf, InputBuf + ImStrlen(InputBuf));
    input_range.split(',', &Filters);

    CountGrep = 0;
    for (ImGuiTextRange& f : Filters)
    {
        while (f.b < f.e && ImCharIsBlankA(f.b[0]))
            f.b++;
        while (f.e > f.b && ImCharIsBlankA(f.e[-1]))
            f.e--;
        if (f.empty())
            continue;
        if (f.b[0] != '-')
            CountGrep += 1;
    }
}

bool ImGuiTextFilter::PassFilter(const char* text, const char* text_end) const
{
    if (Filters.Size == 0)
        return true;

    if (text == NULL)
        text = text_end = "";

    for (const ImGuiTextRange& f : Filters)
    {
        if (f.b == f.e)
            continue;
        if (f.b[0] == '-')
        {
            // Subtract
            if (ImStristr(text, text_end, f.b + 1, f.e) != NULL)
                return false;
        }
        else
        {
            // Grep
            if (ImStristr(text, text_end, f.b, f.e) != NULL)
                return true;
        }
    }

    // Implicit * grep
    if (CountGrep == 0)
        return true;

    return false;
}

//-----------------------------------------------------------------------------
// [SECTION] ImGuiTextBuffer, ImGuiTextIndex
//-----------------------------------------------------------------------------

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#if defined(__GNUC__) || defined(__clang__)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#else
#define va_copy(dest, src) (dest = src)
#endif
#endif

char ImGuiTextBuffer::EmptyString[1] = { 0 };

void ImGuiTextBuffer::append(const char* str, const char* str_end)
{
    int len = str_end ? (int)(str_end - str) : (int)ImStrlen(str);

    // Add zero-terminator the first time
    const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
    const int needed_sz = write_off + len;
    if (write_off + len >= Buf.Capacity)
    {
        int new_capacity = Buf.Capacity * 2;
        Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
    }

    Buf.resize(needed_sz);
    memcpy(&Buf[write_off - 1], str, (size_t)len);
    Buf[write_off - 1 + len] = 0;
}

void ImGuiTextBuffer::appendf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    appendfv(fmt, args);
    va_end(args);
}

// Helper: Text buffer for logging/accumulating text
void ImGuiTextBuffer::appendfv(const char* fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    int len = ImFormatStringV(NULL, 0, fmt, args);         // FIXME-OPT: could do a first pass write attempt, likely successful on first pass.
    if (len <= 0)
    {
        va_end(args_copy);
        return;
    }

    // Add zero-terminator the first time
    const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
    const int needed_sz = write_off + len;
    if (write_off + len >= Buf.Capacity)
    {
        int new_capacity = Buf.Capacity * 2;
        Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
    }

    Buf.resize(needed_sz);
    ImFormatStringV(&Buf[write_off - 1], (size_t)len + 1, fmt, args_copy);
    va_end(args_copy);
}

IM_MSVC_RUNTIME_CHECKS_OFF
void ImGuiTextIndex::append(const char* base, int old_size, int new_size)
{
    IM_ASSERT(old_size >= 0 && new_size >= old_size && new_size >= EndOffset);
    if (old_size == new_size)
        return;
    if (EndOffset == 0 || base[EndOffset - 1] == '\n')
        Offsets.push_back(EndOffset);
    const char* base_end = base + new_size;
    for (const char* p = base + old_size; (p = (const char*)ImMemchr(p, '\n', base_end - p)) != 0; )
        if (++p < base_end) // Don't push a trailing offset on last \n
            Offsets.push_back((int)(intptr_t)(p - base));
    EndOffset = ImMax(EndOffset, new_size);
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] ImGuiListClipper
//-----------------------------------------------------------------------------

// FIXME-TABLE: This prevents us from using ImGuiListClipper _inside_ a table cell.
// The problem we have is that without a Begin/End scheme for rows using the clipper is ambiguous.
static bool GetSkipItemForListClipping()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentTable ? g.CurrentTable->HostSkipItems : g.CurrentWindow->SkipItems;
}

static void ImGuiListClipper_SortAndFuseRanges(ImVector<ImGuiListClipperRange>& ranges, int offset = 0)
{
    if (ranges.Size - offset <= 1)
        return;

    // Helper to order ranges and fuse them together if possible (bubble sort is fine as we are only sorting 2-3 entries)
    for (int sort_end = ranges.Size - offset - 1; sort_end > 0; --sort_end)
        for (int i = offset; i < sort_end + offset; ++i)
            if (ranges[i].Min > ranges[i + 1].Min)
                ImSwap(ranges[i], ranges[i + 1]);

    // Now fuse ranges together as much as possible.
    for (int i = 1 + offset; i < ranges.Size; i++)
    {
        IM_ASSERT(!ranges[i].PosToIndexConvert && !ranges[i - 1].PosToIndexConvert);
        if (ranges[i - 1].Max < ranges[i].Min)
            continue;
        ranges[i - 1].Min = ImMin(ranges[i - 1].Min, ranges[i].Min);
        ranges[i - 1].Max = ImMax(ranges[i - 1].Max, ranges[i].Max);
        ranges.erase(ranges.Data + i);
        i--;
    }
}

static void ImGuiListClipper_SeekCursorAndSetupPrevLine(ImGuiListClipper* clipper, float pos_y, float line_height)
{
    // Set cursor position and a few other things so that SetScrollHereY() and Columns() can work when seeking cursor.
    // FIXME: It is problematic that we have to do that here, because custom/equivalent end-user code would stumble on the same issue.
    // The clipper should probably have a final step to display the last item in a regular manner, maybe with an opt-out flag for data sets which may have costly seek?
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float off_y = pos_y - window->DC.CursorPos.y;
    window->DC.CursorPos.y = pos_y;
    window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, pos_y - g.Style.ItemSpacing.y);
    window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y - line_height;  // Setting those fields so that SetScrollHereY() can properly function after the end of our clipper usage.
    window->DC.PrevLineSize.y = (line_height - g.Style.ItemSpacing.y);      // If we end up needing more accurate data (to e.g. use SameLine) we may as well make the clipper have a fourth step to let user process and display the last item in their list.
    if (ImGuiOldColumns* columns = window->DC.CurrentColumns)
        columns->LineMinY = window->DC.CursorPos.y;                         // Setting this so that cell Y position are set properly
    if (ImGuiTable* table = g.CurrentTable)
    {
        if (table->IsInsideRow)
            ImGui::TableEndRow(table);
        const int row_increase = (int)((off_y / line_height) + 0.5f);
        if (row_increase > 0 && (clipper->Flags & ImGuiListClipperFlags_NoSetTableRowCounters) == 0) // If your clipper item height is != from actual table row height, consider using ImGuiListClipperFlags_NoSetTableRowCounters. See #8886.
        {
            table->CurrentRow += row_increase;
            table->RowBgColorCounter += row_increase;
        }
        table->RowPosY2 = window->DC.CursorPos.y;
    }
}

ImGuiListClipper::ImGuiListClipper()
{
    memset((void*)this, 0, sizeof(*this));
}

ImGuiListClipper::~ImGuiListClipper()
{
    End();
}

void ImGuiListClipper::Begin(int items_count, float items_height)
{
    Ctx = ImGui::GetCurrentContext();

    ImGuiContext& g = *Ctx;
    ImGuiWindow* window = g.CurrentWindow;
    IMGUI_DEBUG_LOG_CLIPPER("Clipper: Begin(%d,%.2f) in '%s'\n", items_count, items_height, window->Name);

    if (ImGuiTable* table = g.CurrentTable)
        if (table->IsInsideRow)
            ImGui::TableEndRow(table);

    StartPosY = window->DC.CursorPos.y;
    ItemsHeight = items_height;
    ItemsCount = items_count;
    DisplayStart = -1;
    DisplayEnd = 0;

    // Acquire temporary buffer
    if (++g.ClipperTempDataStacked > g.ClipperTempData.Size)
        g.ClipperTempData.resize(g.ClipperTempDataStacked, ImGuiListClipperData());
    ImGuiListClipperData* data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
    data->Reset(this);
    data->LossynessOffset = window->DC.CursorStartPosLossyness.y;
    TempData = data;
    StartSeekOffsetY = data->LossynessOffset;
}

void ImGuiListClipper::End()
{
    if (ImGuiListClipperData* data = (ImGuiListClipperData*)TempData)
    {
        // In theory here we should assert that we are already at the right position, but it seems saner to just seek at the end and not assert/crash the user.
        ImGuiContext& g = *Ctx;
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: End() in '%s'\n", g.CurrentWindow->Name);
        if (ItemsCount >= 0 && ItemsCount < INT_MAX && DisplayStart >= 0)
            SeekCursorForItem(ItemsCount);

        // Restore temporary buffer and fix back pointers which may be invalidated when nesting
        IM_ASSERT(data->ListClipper == this);
        data->StepNo = data->Ranges.Size;
        if (--g.ClipperTempDataStacked > 0)
        {
            data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
            data->ListClipper->TempData = data;
        }
        TempData = NULL;
    }
    DisplayStart = DisplayEnd = ItemsCount; // Clear this so code which may be reused past last Step() won't trip on a non-empty range.
    ItemsCount = -1;
}

void ImGuiListClipper::IncludeItemsByIndex(int item_begin, int item_end)
{
    ImGuiListClipperData* data = (ImGuiListClipperData*)TempData;
    IM_ASSERT(DisplayStart < 0); // Only allowed after Begin() and if there has not been a specified range yet.
    IM_ASSERT(item_begin <= item_end);
    if (item_begin < item_end)
        data->Ranges.push_back(ImGuiListClipperRange::FromIndices(item_begin, item_end));
}

// This is already called while stepping.
// The ONLY reason you may want to call this is if you passed INT_MAX to ImGuiListClipper::Begin() because you couldn't step item count beforehand.
void ImGuiListClipper::SeekCursorForItem(int item_n)
{
    // - Perform the add and multiply with double to allow seeking through larger ranges.
    // - StartPosY starts from ItemsFrozen, by adding SeekOffsetY we generally cancel that out (SeekOffsetY == LossynessOffset - ItemsFrozen * ItemsHeight).
    // - The reason we store SeekOffsetY instead of inferring it, is because we want to allow user to perform Seek after the last step, where ImGuiListClipperData is already done.
    float pos_y = (float)((double)StartPosY + StartSeekOffsetY + (double)item_n * ItemsHeight);
    ImGuiListClipper_SeekCursorAndSetupPrevLine(this, pos_y, ItemsHeight);
}

static bool ImGuiListClipper_StepInternal(ImGuiListClipper* clipper)
{
    ImGuiContext& g = *clipper->Ctx;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiListClipperData* data = (ImGuiListClipperData*)clipper->TempData;
    IM_ASSERT(data != NULL && "Called ImGuiListClipper::Step() too many times, or before ImGuiListClipper::Begin() ?");

    ImGuiTable* table = g.CurrentTable;
    if (table && table->IsInsideRow)
        ImGui::TableEndRow(table);

    // No items
    if (clipper->ItemsCount == 0 || GetSkipItemForListClipping())
        return false;

    // While we are in frozen row state, keep displaying items one by one, unclipped
    // FIXME: Could be stored as a table-agnostic state.
    if (data->StepNo == 0 && table != NULL && !table->IsUnfrozenRows)
    {
        clipper->DisplayStart = data->ItemsFrozen;
        clipper->DisplayEnd = ImMin(data->ItemsFrozen + 1, clipper->ItemsCount);
        if (clipper->DisplayStart < clipper->DisplayEnd)
            data->ItemsFrozen++;
        return true;
    }

    // Step 0: Let you process the first element (regardless of it being visible or not, so we can measure the element height)
    bool calc_clipping = false;
    if (data->StepNo == 0)
    {
        clipper->StartPosY = window->DC.CursorPos.y;
        if (clipper->ItemsHeight <= 0.0f)
        {
            // Submit the first item (or range) so we can measure its height (generally the first range is 0..1)
            data->Ranges.push_front(ImGuiListClipperRange::FromIndices(data->ItemsFrozen, data->ItemsFrozen + 1));
            clipper->DisplayStart = ImMax(data->Ranges[0].Min, data->ItemsFrozen);
            clipper->DisplayEnd = ImMin(data->Ranges[0].Max, clipper->ItemsCount);
            data->StepNo = 1;
            return true;
        }
        calc_clipping = true;   // If on the first step with known item height, calculate clipping.
    }

    // Step 1: Let the clipper infer height from first range
    if (clipper->ItemsHeight <= 0.0f)
    {
        IM_ASSERT(data->StepNo == 1);
        bool affected_by_floating_point_precision = ImIsFloatAboveGuaranteedIntegerPrecision((float)clipper->StartPosY) || ImIsFloatAboveGuaranteedIntegerPrecision(window->DC.CursorPos.y);
        if (affected_by_floating_point_precision)
        {
            // Mitigation/hack for very large range: assume last time height constitute line height.
            clipper->ItemsHeight = window->DC.PrevLineSize.y + g.Style.ItemSpacing.y; // FIXME: Technically wouldn't allow multi-line entries.
            window->DC.CursorPos.y = (float)(clipper->StartPosY + clipper->ItemsHeight);
        }
        else
        {
            clipper->ItemsHeight = (float)(window->DC.CursorPos.y - clipper->StartPosY) / (float)(clipper->DisplayEnd - clipper->DisplayStart);
        }
        if (clipper->ItemsHeight == 0.0f && clipper->ItemsCount == INT_MAX) // Accept that no item have been submitted if in indeterminate mode.
            return false;
        if (clipper->ItemsHeight <= 0.0f)
        {
            IM_ASSERT_USER_ERROR(clipper->ItemsHeight > 0.0f, "ImGuiListClipper: Failed to calculate item height! First item hasn't been submitted by user code, or has not moved the cursor vertically!");
            return false;
        }
        if (table)
            IM_ASSERT(table->RowPosY1 == clipper->StartPosY && table->RowPosY2 == window->DC.CursorPos.y);

        calc_clipping = true;   // If item height had to be calculated, calculate clipping afterwards.
    }

    // Step 0 or 1: Calculate the actual ranges of visible elements.
    const int already_submitted = clipper->DisplayEnd;
    if (calc_clipping)
    {
        // Record seek offset, this is so ImGuiListClipper::Seek() can be called after ImGuiListClipperData is done
        clipper->StartSeekOffsetY = (double)data->LossynessOffset - data->ItemsFrozen * (double)clipper->ItemsHeight;

        if (g.LogEnabled)
        {
            // If logging is active, do not perform any clipping
            data->Ranges.push_back(ImGuiListClipperRange::FromIndices(0, clipper->ItemsCount));
        }
        else
        {
            // Add range selected to be included for navigation
            const bool is_nav_request = (g.NavMoveScoringItems && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
            const int nav_off_min = (is_nav_request && g.NavMoveClipDir == ImGuiDir_Up) ? -1 : 0;
            const int nav_off_max = (is_nav_request && g.NavMoveClipDir == ImGuiDir_Down) ? 1 : 0;
            if (is_nav_request)
            {
                data->Ranges.push_back(ImGuiListClipperRange::FromPositions(g.NavScoringRect.Min.y, g.NavScoringRect.Max.y, nav_off_min, nav_off_max));
                if (!g.NavScoringNoClipRect.IsInverted())
                    data->Ranges.push_back(ImGuiListClipperRange::FromPositions(g.NavScoringNoClipRect.Min.y, g.NavScoringNoClipRect.Max.y, nav_off_min, nav_off_max));
            }
            if (is_nav_request && (g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) && g.NavTabbingDir == -1)
                data->Ranges.push_back(ImGuiListClipperRange::FromIndices(clipper->ItemsCount - 1, clipper->ItemsCount));

            // Add focused/active item
            ImRect nav_rect_abs = ImGui::WindowRectRelToAbs(window, window->NavRectRel[0]);
            if (g.NavId != 0 && window->NavLastIds[0] == g.NavId)
                data->Ranges.push_back(ImGuiListClipperRange::FromPositions(nav_rect_abs.Min.y, nav_rect_abs.Max.y, 0, 0));

            float min_y = window->ClipRect.Min.y;
            float max_y = window->ClipRect.Max.y;

            // Add box selection range
            ImGuiBoxSelectState* bs = &g.BoxSelectState;
            if (bs->IsActive && bs->Window == window)
            {
                // FIXME: Selectable() use of half-ItemSpacing isn't consistent in matter of layout, as ItemAdd(bb) stray above ItemSize()'s CursorPos.
                // RangeSelect's BoxSelect relies on comparing overlap of previous and current rectangle and is sensitive to that.
                // As a workaround we currently half ItemSpacing worth on each side.
                float pad_y = g.Style.ItemSpacing.y;
                min_y -= pad_y;
                max_y += pad_y;

                // Box-select on 2D area requires different clipping.
                // (best adding pad_y here than in BeginBoxSelect() as we are closer to current state)
                if (bs->UnclipMode)
                    data->Ranges.push_back(ImGuiListClipperRange::FromPositions(bs->UnclipRect.Min.y - pad_y, bs->UnclipRect.Max.y + pad_y, 0, 0));
            }

            // Add main visible range
            data->Ranges.push_back(ImGuiListClipperRange::FromPositions(min_y, max_y, nav_off_min, nav_off_max));
        }

        // Convert position ranges to item index ranges
        // - Very important: when a starting position is after our maximum item, we set Min to (ItemsCount - 1). This allows us to handle most forms of wrapping.
        // - Due to how Selectable extra padding they tend to be "unaligned" with exact unit in the item list,
        //   which with the flooring/ceiling tend to lead to 2 items instead of one being submitted.
        for (ImGuiListClipperRange& range : data->Ranges)
            if (range.PosToIndexConvert)
            {
                int m1 = (int)(((double)range.Min - window->DC.CursorPos.y - data->LossynessOffset) / clipper->ItemsHeight);
                int m2 = (int)((((double)range.Max - window->DC.CursorPos.y - data->LossynessOffset) / clipper->ItemsHeight) + 0.999999f);
                range.Min = ImClamp(already_submitted + m1 + range.PosToIndexOffsetMin, already_submitted, clipper->ItemsCount - 1);
                range.Max = ImClamp(already_submitted + m2 + range.PosToIndexOffsetMax, range.Min + 1, clipper->ItemsCount);
                range.PosToIndexConvert = false;
            }
        ImGuiListClipper_SortAndFuseRanges(data->Ranges, data->StepNo);
    }

    // Step 0+ (if item height is given in advance) or 1+: Display the next range in line.
    while (data->StepNo < data->Ranges.Size)
    {
        clipper->DisplayStart = ImMax(data->Ranges[data->StepNo].Min, already_submitted);
        clipper->DisplayEnd = ImMin(data->Ranges[data->StepNo].Max, clipper->ItemsCount);
        data->StepNo++;
        if (clipper->DisplayStart >= clipper->DisplayEnd)
            continue;
        if (clipper->DisplayStart > already_submitted)
            clipper->SeekCursorForItem(clipper->DisplayStart);
        return true;
    }

    // After the last step: Let the clipper validate that we have reached the expected Y position (corresponding to element DisplayEnd),
    // Advance the cursor to the end of the list and then returns 'false' to end the loop.
    if (clipper->ItemsCount < INT_MAX)
        clipper->SeekCursorForItem(clipper->ItemsCount);

    return false;
}

bool ImGuiListClipper::Step()
{
    ImGuiContext& g = *Ctx;
    bool need_items_height = (ItemsHeight <= 0.0f);
    bool ret = ImGuiListClipper_StepInternal(this);
    if (ret && (DisplayStart >= DisplayEnd))
        ret = false;
    if (g.CurrentTable && g.CurrentTable->IsUnfrozenRows == false)
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): inside frozen table row.\n");
    if (need_items_height && ItemsHeight > 0.0f)
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): computed ItemsHeight: %.2f.\n", ItemsHeight);
    if (ret)
    {
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): display %d to %d.\n", DisplayStart, DisplayEnd);
    }
    else
    {
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): End.\n");
        End();
    }
    return ret;
}

// Generic helper, equivalent to old ImGui::CalcListClipping() but stateless
void ImGui::CalcClipRectVisibleItemsY(const ImRect& clip_rect, const ImVec2& pos, float items_height, int* out_visible_start, int* out_visible_end)
{
    *out_visible_start = ImMax((int)((clip_rect.Min.y - pos.y) / items_height), 0);
    *out_visible_end = ImMax((int)ImCeil((clip_rect.Max.y - pos.y) / items_height), *out_visible_start);
}

//-----------------------------------------------------------------------------
// [SECTION] STYLING
//-----------------------------------------------------------------------------

ImGuiStyle& ImGui::GetStyle()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->Style;
}

ImU32 ImGui::GetColorU32(ImGuiCol idx, float alpha_mul)
{
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = style.Colors[idx];
    c.w *= style.Alpha * alpha_mul;
    return ColorConvertFloat4ToU32(c);
}

ImU32 ImGui::GetColorU32(const ImVec4& col)
{
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = col;
    c.w *= style.Alpha;
    return ColorConvertFloat4ToU32(c);
}

const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol idx)
{
    ImGuiStyle& style = GImGui->Style;
    return style.Colors[idx];
}

ImU32 ImGui::GetColorU32(ImU32 col, float alpha_mul)
{
    ImGuiStyle& style = GImGui->Style;
    alpha_mul *= style.Alpha;
    if (alpha_mul >= 1.0f)
        return col;
    ImU32 a = (col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;
    a = (ImU32)(a * alpha_mul); // We don't need to clamp 0..255 because alpha is in 0..1 range.
    return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}

// FIXME: This may incur a round-trip (if the end user got their data from a float4) but eventually we aim to store the in-flight colors as ImU32
void ImGui::PushStyleColor(ImGuiCol idx, ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorStack.push_back(backup);
    if (g.DebugFlashStyleColorIdx != idx)
        g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void ImGui::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorStack.push_back(backup);
    if (g.DebugFlashStyleColorIdx != idx)
        g.Style.Colors[idx] = col;
}

void ImGui::PopStyleColor(int count)
{
    ImGuiContext& g = *GImGui;
    if (g.ColorStack.Size < count)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopStyleColor() too many times!");
        count = g.ColorStack.Size;
    }
    while (count > 0)
    {
        ImGuiColorMod& backup = g.ColorStack.back();
        g.Style.Colors[backup.Col] = backup.BackupValue;
        g.ColorStack.pop_back();
        count--;
    }
}

static const ImGuiStyleVarInfo GStyleVarsInfo[] =
{
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, Alpha) },                     // ImGuiStyleVar_Alpha
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DisabledAlpha) },             // ImGuiStyleVar_DisabledAlpha
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowPadding) },             // ImGuiStyleVar_WindowPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowRounding) },            // ImGuiStyleVar_WindowRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowBorderSize) },          // ImGuiStyleVar_WindowBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowMinSize) },             // ImGuiStyleVar_WindowMinSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowTitleAlign) },          // ImGuiStyleVar_WindowTitleAlign
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ChildRounding) },             // ImGuiStyleVar_ChildRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ChildBorderSize) },           // ImGuiStyleVar_ChildBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, PopupRounding) },             // ImGuiStyleVar_PopupRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, PopupBorderSize) },           // ImGuiStyleVar_PopupBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FramePadding) },              // ImGuiStyleVar_FramePadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FrameRounding) },             // ImGuiStyleVar_FrameRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FrameBorderSize) },           // ImGuiStyleVar_FrameBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ItemSpacing) },               // ImGuiStyleVar_ItemSpacing
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ItemInnerSpacing) },          // ImGuiStyleVar_ItemInnerSpacing
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, IndentSpacing) },             // ImGuiStyleVar_IndentSpacing
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, CellPadding) },               // ImGuiStyleVar_CellPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarSize) },             // ImGuiStyleVar_ScrollbarSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarRounding) },         // ImGuiStyleVar_ScrollbarRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarPadding) },          // ImGuiStyleVar_ScrollbarPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, GrabMinSize) },               // ImGuiStyleVar_GrabMinSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, GrabRounding) },              // ImGuiStyleVar_GrabRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ImageRounding) },             // ImGuiStyleVar_ImageRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ImageBorderSize) },           // ImGuiStyleVar_ImageBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabRounding) },               // ImGuiStyleVar_TabRounding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBorderSize) },             // ImGuiStyleVar_TabBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabMinWidthBase) },           // ImGuiStyleVar_TabMinWidthBase
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabMinWidthShrink) },         // ImGuiStyleVar_TabMinWidthShrink
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBarBorderSize) },          // ImGuiStyleVar_TabBarBorderSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBarOverlineSize) },        // ImGuiStyleVar_TabBarOverlineSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TableAngledHeadersAngle)},    // ImGuiStyleVar_TableAngledHeadersAngle
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TableAngledHeadersTextAlign)},// ImGuiStyleVar_TableAngledHeadersTextAlign
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TreeLinesSize)},              // ImGuiStyleVar_TreeLinesSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TreeLinesRounding)},          // ImGuiStyleVar_TreeLinesRounding
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ButtonTextAlign) },           // ImGuiStyleVar_ButtonTextAlign
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SelectableTextAlign) },       // ImGuiStyleVar_SelectableTextAlign
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorSize)},              // ImGuiStyleVar_SeparatorSize
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextBorderSize)},    // ImGuiStyleVar_SeparatorTextBorderSize
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextAlign) },        // ImGuiStyleVar_SeparatorTextAlign
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextPadding) },      // ImGuiStyleVar_SeparatorTextPadding
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DockingSeparatorSize) },      // ImGuiStyleVar_DockingSeparatorSize
};

const ImGuiStyleVarInfo* ImGui::GetStyleVarInfo(ImGuiStyleVar idx)
{
    IM_ASSERT(idx >= 0 && idx < ImGuiStyleVar_COUNT);
    IM_STATIC_ASSERT(IM_COUNTOF(GStyleVarsInfo) == ImGuiStyleVar_COUNT);
    return &GStyleVarsInfo[idx];
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, float val)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 1, "Calling PushStyleVar() variant with wrong type!");
    float* pvar = (float*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    *pvar = val;
}

void ImGui::PushStyleVarX(ImGuiStyleVar idx, float val_x)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    pvar->x = val_x;
}

void ImGui::PushStyleVarY(ImGuiStyleVar idx, float val_y)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    pvar->y = val_y;
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    *pvar = val;
}

void ImGui::PopStyleVar(int count)
{
    ImGuiContext& g = *GImGui;
    if (g.StyleVarStack.Size < count)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopStyleVar() too many times!");
        count = g.StyleVarStack.Size;
    }
    while (count > 0)
    {
        // We avoid a generic memcpy(data, &backup.Backup.., GDataTypeSize[info->Type] * info->Count), the overhead in Debug is not worth it.
        ImGuiStyleMod& backup = g.StyleVarStack.back();
        const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(backup.VarIdx);
        void* data = var_info->GetVarPtr(&g.Style);
        if (var_info->DataType == ImGuiDataType_Float && var_info->Count == 1)      { ((float*)data)[0] = backup.BackupFloat[0]; }
        else if (var_info->DataType == ImGuiDataType_Float && var_info->Count == 2) { ((float*)data)[0] = backup.BackupFloat[0]; ((float*)data)[1] = backup.BackupFloat[1]; }
        g.StyleVarStack.pop_back();
        count--;
    }
}

const char* ImGui::GetStyleColorName(ImGuiCol idx)
{
    // Create switch-case from enum with regexp: ImGuiCol_{.*}, --> case ImGuiCol_\1: return "\1";
    switch (idx)
    {
    case ImGuiCol_Text: return "Text";
    case ImGuiCol_TextDisabled: return "TextDisabled";
    case ImGuiCol_WindowBg: return "WindowBg";
    case ImGuiCol_ChildBg: return "ChildBg";
    case ImGuiCol_PopupBg: return "PopupBg";
    case ImGuiCol_Border: return "Border";
    case ImGuiCol_BorderShadow: return "BorderShadow";
    case ImGuiCol_FrameBg: return "FrameBg";
    case ImGuiCol_FrameBgHovered: return "FrameBgHovered";
    case ImGuiCol_FrameBgActive: return "FrameBgActive";
    case ImGuiCol_TitleBg: return "TitleBg";
    case ImGuiCol_TitleBgActive: return "TitleBgActive";
    case ImGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
    case ImGuiCol_MenuBarBg: return "MenuBarBg";
    case ImGuiCol_ScrollbarBg: return "ScrollbarBg";
    case ImGuiCol_ScrollbarGrab: return "ScrollbarGrab";
    case ImGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
    case ImGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
    case ImGuiCol_CheckMark: return "CheckMark";
    case ImGuiCol_SliderGrab: return "SliderGrab";
    case ImGuiCol_SliderGrabActive: return "SliderGrabActive";
    case ImGuiCol_Button: return "Button";
    case ImGuiCol_ButtonHovered: return "ButtonHovered";
    case ImGuiCol_ButtonActive: return "ButtonActive";
    case ImGuiCol_Header: return "Header";
    case ImGuiCol_HeaderHovered: return "HeaderHovered";
    case ImGuiCol_HeaderActive: return "HeaderActive";
    case ImGuiCol_Separator: return "Separator";
    case ImGuiCol_SeparatorHovered: return "SeparatorHovered";
    case ImGuiCol_SeparatorActive: return "SeparatorActive";
    case ImGuiCol_ResizeGrip: return "ResizeGrip";
    case ImGuiCol_ResizeGripHovered: return "ResizeGripHovered";
    case ImGuiCol_ResizeGripActive: return "ResizeGripActive";
    case ImGuiCol_InputTextCursor: return "InputTextCursor";
    case ImGuiCol_TabHovered: return "TabHovered";
    case ImGuiCol_Tab: return "Tab";
    case ImGuiCol_TabSelected: return "TabSelected";
    case ImGuiCol_TabSelectedOverline: return "TabSelectedOverline";
    case ImGuiCol_TabDimmed: return "TabDimmed";
    case ImGuiCol_TabDimmedSelected: return "TabDimmedSelected";
    case ImGuiCol_TabDimmedSelectedOverline: return "TabDimmedSelectedOverline";
    case ImGuiCol_DockingPreview: return "DockingPreview";
    case ImGuiCol_DockingEmptyBg: return "DockingEmptyBg";
    case ImGuiCol_PlotLines: return "PlotLines";
    case ImGuiCol_PlotLinesHovered: return "PlotLinesHovered";
    case ImGuiCol_PlotHistogram: return "PlotHistogram";
    case ImGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
    case ImGuiCol_TableHeaderBg: return "TableHeaderBg";
    case ImGuiCol_TableBorderStrong: return "TableBorderStrong";
    case ImGuiCol_TableBorderLight: return "TableBorderLight";
    case ImGuiCol_TableRowBg: return "TableRowBg";
    case ImGuiCol_TableRowBgAlt: return "TableRowBgAlt";
    case ImGuiCol_TextLink: return "TextLink";
    case ImGuiCol_TextSelectedBg: return "TextSelectedBg";
    case ImGuiCol_TreeLines: return "TreeLines";
    case ImGuiCol_DragDropTarget: return "DragDropTarget";
    case ImGuiCol_DragDropTargetBg: return "DragDropTargetBg";
    case ImGuiCol_UnsavedMarker: return "UnsavedMarker";
    case ImGuiCol_NavCursor: return "NavCursor";
    case ImGuiCol_NavWindowingHighlight: return "NavWindowingHighlight";
    case ImGuiCol_NavWindowingDimBg: return "NavWindowingDimBg";
    case ImGuiCol_ModalWindowDimBg: return "ModalWindowDimBg";
    }
    IM_ASSERT(0);
    return "Unknown";
}

//-----------------------------------------------------------------------------
// [SECTION] RENDER HELPERS
// Some of those (internal) functions are currently quite a legacy mess - their signature and behavior will change,
// we need a nicer separation between low-level functions and high-level functions relying on the ImGui context.
// Also see imgui_draw.cpp for some more which have been reworked to not rely on ImGui:: context.
//-----------------------------------------------------------------------------

const char* ImGui::FindRenderedTextEnd(const char* text, const char* text_end)
{
    const char* text_display_end = text;
    if (!text_end)
        text_end = (const char*)-1;

    while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
        text_display_end++;
    return text_display_end;
}

// Internal ImGui functions to render text
// RenderText***() functions calls ImDrawList::AddText() calls ImBitmapFont::RenderText()
void ImGui::RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Hide anything after a '##' string
    const char* text_display_end;
    if (hide_text_after_hash)
    {
        text_display_end = FindRenderedTextEnd(text, text_end);
    }
    else
    {
        if (!text_end)
            text_end = text + ImStrlen(text); // FIXME-OPT (not reached by our internal calls)
        text_display_end = text_end;
    }

    if (text != text_display_end)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_display_end);
    }
}

void ImGui::RenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (!text_end)
        text_end = text + ImStrlen(text); // FIXME-OPT (not reached by our internal calls)

    if (text != text_end)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_end, wrap_width);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_end);
    }
}

// Default clip_rect uses (pos_min,pos_max)
// Handle clipping on CPU immediately (vs typically let the GPU clip the triangles that are overlapping the clipping rectangle edges)
// FIXME-OPT: Since we have or calculate text_size we could coarse clip whole block immediately, especially for text above draw_list->DrawList.
// Effectively as this is called from widget doing their own coarse clipping it's not very valuable presently. Next time function will take
// better advantage of the render function taking size into account for coarse clipping.
void ImGui::RenderTextClippedEx(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    // Perform CPU side clipping for single clipped element to avoid using scissor state
    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_display_end, false, 0.0f);

    const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
    const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
        need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    // Align whole block. We should defer that to the better rendering function when we'll have support for individual line alignment.
    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

    // Render
    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        draw_list->AddText(NULL, 0.0f, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect);
    }
    else
    {
        draw_list->AddText(NULL, 0.0f, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, NULL);
    }
}

void ImGui::RenderTextClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    // Hide anything after a '##' string
    const char* text_display_end = FindRenderedTextEnd(text, text_end);
    const int text_len = (int)(text_display_end - text);
    if (text_len == 0)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    RenderTextClippedEx(window->DrawList, pos_min, pos_max, text, text_display_end, text_size_if_known, align, clip_rect);
    if (g.LogEnabled)
        LogRenderedText(&pos_min, text, text_display_end);
}

// Another overly complex function until we reorganize everything into a nice all-in-one helper.
// This is made more complex because we have dissociated the layout rectangle (pos_min..pos_max) from 'ellipsis_max_x' which may be beyond it.
// This is because in the context of tabs we selectively hide part of the text when the Close Button appears, but we don't want the ellipsis to move.
// (BREAKING) On 2025/04/16 we removed the 'float clip_max_x' parameters which was preceding 'float ellipsis_max' and was the same value for 99% of users.
void ImGui::RenderTextEllipsis(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end_full, const ImVec2* text_size_if_known)
{
    ImGuiContext& g = *GImGui;
    if (text_end_full == NULL)
        text_end_full = FindRenderedTextEnd(text);
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_end_full, false, 0.0f);

    //draw_list->AddLine(ImVec2(pos_max.x, pos_min.y - 4), ImVec2(pos_max.x, pos_max.y + 6), IM_COL32(0, 0, 255, 255));
    //draw_list->AddLine(ImVec2(ellipsis_max_x, pos_min.y - 2), ImVec2(ellipsis_max_x, pos_max.y + 3), IM_COL32(0, 255, 0, 255));

    // FIXME: We could technically remove (last_glyph->AdvanceX - last_glyph->X1) from text_size.x here and save a few pixels.
    if (text_size.x > pos_max.x - pos_min.x)
    {
        // Hello wo...
        // |       |   |
        // min   max   ellipsis_max
        //          <-> this is generally some padding value

        ImFont* font = draw_list->_Data->Font;
        const float font_size = draw_list->_Data->FontSize;
        const float font_scale = draw_list->_Data->FontScale;
        const char* text_end_ellipsis = NULL;
        ImFontBaked* baked = font->GetFontBaked(font_size);
        const float ellipsis_width = baked->GetCharAdvance(font->EllipsisChar) * font_scale;

        // We can now claim the space between pos_max.x and ellipsis_max.x
        const float text_avail_width = ImMax((ImMax(pos_max.x, ellipsis_max_x) - ellipsis_width) - pos_min.x, 1.0f);
        const float text_size_clipped_x = font->CalcTextSizeA(font_size, text_avail_width, 0.0f, text, text_end_full, &text_end_ellipsis).x;

        // Render text, render ellipsis
        RenderTextClippedEx(draw_list, pos_min, pos_max, text, text_end_ellipsis, &text_size, ImVec2(0.0f, 0.0f));
        ImVec4 cpu_fine_clip_rect(pos_min.x, pos_min.y, pos_max.x, pos_max.y);
        ImVec2 ellipsis_pos = ImTrunc(ImVec2(pos_min.x + text_size_clipped_x, pos_min.y));
        font->RenderChar(draw_list, font_size, ellipsis_pos, GetColorU32(ImGuiCol_Text), font->EllipsisChar, &cpu_fine_clip_rect);
    }
    else
    {
        RenderTextClippedEx(draw_list, pos_min, pos_max, text, text_end_full, &text_size, ImVec2(0.0f, 0.0f));
    }

    if (g.LogEnabled)
        LogRenderedText(&pos_min, text, text_end_full);
}

// Render a rectangle shaped with optional rounding and borders
void ImGui::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool borders, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
    const float border_size = g.Style.FrameBorderSize;
    if (borders && border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
    }
}

void ImGui::RenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
    }
}

void ImGui::RenderColorComponentMarker(const ImRect& bb, ImU32 col, float rounding)
{
    if (bb.Min.x + 1 >= bb.Max.x)
        return;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    RenderRectFilledInRangeH(window->DrawList, bb, col, bb.Min.x, ImMin(bb.Min.x + g.Style.ColorMarkerSize, bb.Max.x), rounding);
}

void ImGui::RenderNavCursor(const ImRect& bb, ImGuiID id, ImGuiNavRenderCursorFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (id != g.NavId)
        return;
    if (!g.NavCursorVisible && !(flags & ImGuiNavRenderCursorFlags_AlwaysDraw))
        return;
    if (id == g.LastItemData.ID && (g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav))
        return;

    // We don't early out on 'window->Flags & ImGuiWindowFlags_NoNavInputs' because it would be inconsistent with
    // other code directly checking NavCursorVisible. Instead we aim for NavCursorVisible to always be false.
    ImGuiWindow* window = g.CurrentWindow;
    if (window->DC.NavHideHighlightOneFrame)
        return;

    float rounding = (flags & ImGuiNavRenderCursorFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
    ImRect display_rect = bb;
    display_rect.ClipWith(window->ClipRect);
    const float thickness = 2.0f;
    if (flags & ImGuiNavRenderCursorFlags_Compact)
    {
        window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavCursor), rounding, 0, thickness);
    }
    else
    {
        const float distance = 3.0f + thickness * 0.5f;
        display_rect.Expand(ImVec2(distance, distance));
        bool fully_visible = window->ClipRect.Contains(display_rect);
        if (!fully_visible)
            window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
        window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavCursor), rounding, 0, thickness);
        if (!fully_visible)
            window->DrawList->PopClipRect();
    }
}

void ImGui::RenderMouseCursor(ImVec2 base_pos, float base_scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow)
{
    ImGuiContext& g = *GImGui;
    if (mouse_cursor <= ImGuiMouseCursor_None || mouse_cursor >= ImGuiMouseCursor_COUNT) // We intentionally accept out of bound values.
        mouse_cursor = ImGuiMouseCursor_Arrow;
    ImFontAtlas* font_atlas = g.DrawListSharedData.FontAtlas;
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        // We scale cursor with current viewport/monitor, however Windows 10 for its own hardware cursor seems to be using a different scale factor.
        ImVec2 offset, size, uv[4];
        if (!ImFontAtlasGetMouseCursorTexData(font_atlas, mouse_cursor, &offset, &size, &uv[0], &uv[2]))
            continue;
        const ImVec2 pos = base_pos - offset;
        const float scale = base_scale * viewport->DpiScale;
        if (!viewport->GetMainRect().Overlaps(ImRect(pos, pos + ImVec2(size.x + 2, size.y + 2) * scale)))
            continue;
        ImDrawList* draw_list = GetForegroundDrawList(viewport);
        ImTextureRef tex_ref = font_atlas->TexRef;
        draw_list->PushTexture(tex_ref);
        draw_list->AddImage(tex_ref, pos + ImVec2(1, 0) * scale, pos + (ImVec2(1, 0) + size) * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_ref, pos + ImVec2(2, 0) * scale, pos + (ImVec2(2, 0) + size) * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_ref, pos,                        pos + size * scale,                  uv[2], uv[3], col_border);
        draw_list->AddImage(tex_ref, pos,                        pos + size * scale,                  uv[0], uv[1], col_fill);
        if (mouse_cursor == ImGuiMouseCursor_Wait || mouse_cursor == ImGuiMouseCursor_Progress)
        {
            float a_min = ImFmod((float)g.Time * 5.0f, 2.0f * IM_PI);
            float a_max = a_min + IM_PI * 1.65f;
            draw_list->PathArcTo(pos + ImVec2(14, -1) * scale, 6.0f * scale, a_min, a_max);
            draw_list->PathStroke(col_fill, ImDrawFlags_None, 3.0f * scale);
        }
        draw_list->PopTexture();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] INITIALIZATION, SHUTDOWN
//-----------------------------------------------------------------------------

// Internal state access - if you want to share Dear ImGui state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
ImGuiContext* ImGui::GetCurrentContext()
{
    return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext* ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
    GImGui = ctx;
#endif
}

void ImGui::SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void* user_data)
{
    GImAllocatorAllocFunc = alloc_func;
    GImAllocatorFreeFunc = free_func;
    GImAllocatorUserData = user_data;
}

// This is provided to facilitate copying allocators from one static/DLL boundary to another (e.g. retrieve default allocator of your executable address space)
void ImGui::GetAllocatorFunctions(ImGuiMemAllocFunc* p_alloc_func, ImGuiMemFreeFunc* p_free_func, void** p_user_data)
{
    *p_alloc_func = GImAllocatorAllocFunc;
    *p_free_func = GImAllocatorFreeFunc;
    *p_user_data = GImAllocatorUserData;
}

ImGuiContext* ImGui::CreateContext(ImFontAtlas* shared_font_atlas)
{
    ImGuiContext* prev_ctx = GetCurrentContext();
    ImGuiContext* ctx = IM_NEW(ImGuiContext)(shared_font_atlas);
    SetCurrentContext(ctx);
    Initialize();
    if (prev_ctx != NULL)
        SetCurrentContext(prev_ctx); // Restore previous context if any, else keep new one.
    return ctx;
}

void ImGui::DestroyContext(ImGuiContext* ctx)
{
    ImGuiContext* prev_ctx = GetCurrentContext();
    if (ctx == NULL) //-V1051
        ctx = prev_ctx;
    SetCurrentContext(ctx);
    Shutdown();
    SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
    IM_DELETE(ctx);
}

// IMPORTANT: interactive elements requires a fixed ###xxx suffix, it must be same in ALL languages to allow for automation.
static const ImGuiLocEntry GLocalizationEntriesEnUS[] =
{
    { ImGuiLocKey_VersionStr,           "Dear ImGui " IMGUI_VERSION " (" IM_STRINGIFY(IMGUI_VERSION_NUM) ")" },
    { ImGuiLocKey_TableSizeOne,         "Size column to fit###SizeOne"          },
    { ImGuiLocKey_TableSizeAllFit,      "Size all columns to fit###SizeAll"     },
    { ImGuiLocKey_TableSizeAllDefault,  "Size all columns to default###SizeAll" },
    { ImGuiLocKey_TableResetOrder,      "Reset order###ResetOrder"              },
    { ImGuiLocKey_WindowingMainMenuBar, "(Main menu bar)"                       },
    { ImGuiLocKey_WindowingPopup,       "(Popup)"                               },
    { ImGuiLocKey_WindowingUntitled,    "(Untitled)"                            },
    { ImGuiLocKey_OpenLink_s,           "Open '%s'"                             },
    { ImGuiLocKey_CopyLink,             "Copy Link###CopyLink"                  },
    { ImGuiLocKey_DockingHideTabBar,            "Hide tab bar###HideTabBar"             },
    { ImGuiLocKey_DockingHoldShiftToDock,       "Hold SHIFT to enable Docking window."  },
    { ImGuiLocKey_DockingDragToUndockOrMoveNode,"Click and drag to move or undock whole node."    },
};

ImGuiContext::ImGuiContext(ImFontAtlas* shared_font_atlas)
{
    IO.Ctx = this;
    InputTextState.Ctx = this;

    Initialized = false;
    WithinFrameScope = WithinFrameScopeWithImplicitWindow = false;
    TestEngineHookItems = false;
    FrameCount = 0;
    FrameCountEnded = FrameCountPlatformEnded = FrameCountRendered = -1;
    Time = 0.0f;
    memset(ContextName, 0, sizeof(ContextName));
    ConfigFlagsCurrFrame = ConfigFlagsLastFrame = ImGuiConfigFlags_None;

    Font = NULL;
    FontBaked = NULL;
    FontSize = FontSizeBase = FontBakedScale = CurrentDpiScale = 0.0f;
    FontRasterizerDensity = 1.0f;
    IO.Fonts = shared_font_atlas ? shared_font_atlas : IM_NEW(ImFontAtlas)();
    if (shared_font_atlas == NULL)
        IO.Fonts->OwnerContext = this;
    WithinEndChildID = WithinEndPopupID = 0;
    TestEngine = NULL;

    InputEventsNextMouseSource = ImGuiMouseSource_Mouse;
    InputEventsNextEventId = 1;

    WindowsActiveCount = 0;
    WindowsBorderHoverPadding = 0.0f;
    CurrentWindow = NULL;
    HoveredWindow = NULL;
    HoveredWindowUnderMovingWindow = NULL;
    HoveredWindowBeforeClear = NULL;
    MovingWindow = NULL;
    WheelingWindow = NULL;
    WheelingWindowStartFrame = WheelingWindowScrolledFrame = -1;
    WheelingWindowReleaseTimer = 0.0f;

    DebugDrawIdConflictsId = 0;
    DebugHookIdInfoId = 0;
    HoveredId = HoveredIdPreviousFrame = 0;
    HoveredIdPreviousFrameItemCount = 0;
    HoveredIdAllowOverlap = false;
    HoveredIdIsDisabled = false;
    HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
    ItemUnclipByLog = false;
    ActiveId = 0;
    ActiveIdIsAlive = 0;
    ActiveIdTimer = 0.0f;
    ActiveIdIsJustActivated = false;
    ActiveIdAllowOverlap = false;
    ActiveIdNoClearOnFocusLoss = false;
    ActiveIdHasBeenPressedBefore = false;
    ActiveIdHasBeenEditedBefore = false;
    ActiveIdHasBeenEditedThisFrame = false;
    ActiveIdFromShortcut = false;
    ActiveIdClickOffset = ImVec2(-1, -1);
    ActiveIdSource = ImGuiInputSource_None;
    ActiveIdWindow = NULL;
    ActiveIdMouseButton = -1;
    ActiveIdDisabledId = 0;
    ActiveIdPreviousFrame = 0;
    memset(&DeactivatedItemData, 0, sizeof(DeactivatedItemData));
    memset(&ActiveIdValueOnActivation, 0, sizeof(ActiveIdValueOnActivation));
    LastActiveId = 0;
    LastActiveIdTimer = 0.0f;

    LastKeyboardKeyPressTime = LastKeyModsChangeTime = LastKeyModsChangeFromNoneTime = -1.0;

    ActiveIdUsingNavDirMask = 0x00;
    ActiveIdUsingAllKeyboardKeys = false;

    CurrentFocusScopeId = 0;
    CurrentItemFlags = ImGuiItemFlags_None;
    DebugShowGroupRects = false;
    GcCompactAll = false;

    CurrentViewport = NULL;
    MouseViewport = MouseLastHoveredViewport = NULL;
    PlatformLastFocusedViewportId = 0;
    ViewportCreatedCount = PlatformWindowsCreatedCount = 0;
    ViewportFocusedStampCount = 0;

    NavCursorVisible = false;
    NavHighlightItemUnderNav = false;
    NavMousePosDirty = false;
    NavIdIsAlive = false;
    NavId = 0;
    NavWindow = NULL;
    NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId = 0;
    NavLayer = ImGuiNavLayer_Main;
    NavIdItemFlags = ImGuiItemFlags_None;
    NavOpenContextMenuItemId = NavOpenContextMenuWindowId = 0;
    NavNextActivateId = 0;
    NavActivateFlags = NavNextActivateFlags = ImGuiActivateFlags_None;
    NavHighlightActivatedId = 0;
    NavHighlightActivatedTimer = 0.0f;
    NavInputSource = ImGuiInputSource_Keyboard;
    NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;
    NavCursorHideFrames = 0;

    NavAnyRequest = false;
    NavInitRequest = false;
    NavInitRequestFromMove = false;
    NavMoveSubmitted = false;
    NavMoveScoringItems = false;
    NavMoveForwardToNextFrame = false;
    NavMoveFlags = ImGuiNavMoveFlags_None;
    NavMoveScrollFlags = ImGuiScrollFlags_None;
    NavMoveKeyMods = ImGuiMod_None;
    NavMoveDir = NavMoveDirForDebug = NavMoveClipDir = ImGuiDir_None;
    NavScoringDebugCount = 0;
    NavTabbingDir = 0;
    NavTabbingCounter = 0;

    NavJustMovedFromFocusScopeId = NavJustMovedToId = NavJustMovedToFocusScopeId = 0;
    NavJustMovedToKeyMods = ImGuiMod_None;
    NavJustMovedToIsTabbing = false;
    NavJustMovedToHasSelectionData = false;

    // All platforms use Ctrl+Tab but Ctrl<>Super are swapped on Mac...
    // FIXME: Because this value is stored, it annoyingly interfere with toggling io.ConfigMacOSXBehaviors updating this..
    ConfigNavEnableTabbing = true;
    ConfigNavWindowingWithGamepad = true;
    ConfigNavWindowingKeyNext = IO.ConfigMacOSXBehaviors ? (ImGuiMod_Super | ImGuiKey_Tab) : (ImGuiMod_Ctrl | ImGuiKey_Tab);
    ConfigNavWindowingKeyPrev = IO.ConfigMacOSXBehaviors ? (ImGuiMod_Super | ImGuiMod_Shift | ImGuiKey_Tab) : (ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Tab);
    NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
    NavWindowingInputSource = ImGuiInputSource_None;
    NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
    NavWindowingToggleLayer = false;
    NavWindowingToggleKey = ImGuiKey_None;

    DimBgRatio = 0.0f;

    DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
    DragDropSourceFlags = ImGuiDragDropFlags_None;
    DragDropSourceFrameCount = -1;
    DragDropMouseButton = -1;
    DragDropTargetId = 0;
    DragDropTargetFullViewport = 0;
    DragDropAcceptFlagsCurr = DragDropAcceptFlagsPrev = ImGuiDragDropFlags_None;
    DragDropAcceptIdCurrRectSurface = 0.0f;
    DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
    DragDropAcceptFrameCount = -1;
    DragDropHoldJustPressedId = 0;
    memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

    ClipperTempDataStacked = 0;

    CurrentTable = NULL;
    TablesTempDataStacked = 0;
    CurrentTabBar = NULL;
    CurrentMultiSelect = NULL;
    MultiSelectTempDataStacked = 0;

    HoverItemDelayId = HoverItemDelayIdPreviousFrame = HoverItemUnlockedStationaryId = HoverWindowUnlockedStationaryId = 0;
    HoverItemDelayTimer = HoverItemDelayClearTimer = 0.0f;

    MouseCursor = ImGuiMouseCursor_Arrow;
    MouseStationaryTimer = 0.0f;

    InputTextPasswordFontBackupFlags = ImFontFlags_None;
    InputTextReactivateId = 0;
    TempInputId = 0;
    memset(&DataTypeZeroValue, 0, sizeof(DataTypeZeroValue));
    BeginMenuDepth = BeginComboDepth = 0;
    ColorEditOptions = ImGuiColorEditFlags_DefaultOptions_;
    ColorEditCurrentID = ColorEditSavedID = 0;
    ColorEditSavedHue = ColorEditSavedSat = 0.0f;
    ColorEditSavedColor = 0;
    WindowResizeRelativeMode = false;
    ScrollbarSeekMode = 0;
    ScrollbarClickDeltaToGrabCenter = 0.0f;
    SliderGrabClickOffset = 0.0f;
    SliderCurrentAccum = 0.0f;
    SliderCurrentAccumDirty = false;
    DragCurrentAccumDirty = false;
    DragCurrentAccum = 0.0f;
    DragSpeedDefaultRatio = 1.0f / 100.0f;
    DisabledAlphaBackup = 0.0f;
    DisabledStackSize = 0;
    TooltipOverrideCount = 0;
    TooltipPreviousWindow = NULL;

    PlatformImeData.InputPos = ImVec2(0.0f, 0.0f);
    PlatformImeDataPrev.InputPos = ImVec2(-1.0f, -1.0f); // Different to ensure initial submission

    DockNodeWindowMenuHandler = NULL;

    SettingsLoaded = false;
    SettingsDirtyTimer = 0.0f;
    HookIdNext = 0;
    DemoMarkerCallback = NULL;

    memset(LocalizationTable, 0, sizeof(LocalizationTable));

    LogEnabled = false;
    LogLineFirstItem = false;
    LogFlags = ImGuiLogFlags_None;
    LogWindow = NULL;
    LogNextPrefix = LogNextSuffix = NULL;
    LogFile = NULL;
    LogLinePosY = FLT_MAX;
    LogDepthRef = 0;
    LogDepthToExpand = LogDepthToExpandDefault = 2;

    ErrorCallback = NULL;
    ErrorCallbackUserData = NULL;
    ErrorFirst = true;
    ErrorCountCurrentFrame = 0;
    StackSizesInBeginForCurrentWindow = NULL;

    DebugDrawIdConflictsCount = 0;
    DebugLogFlags = ImGuiDebugLogFlags_EventError | ImGuiDebugLogFlags_OutputToTTY;
    DebugLocateId = 0;
    DebugLogSkippedErrors = 0;
    DebugLogAutoDisableFlags = ImGuiDebugLogFlags_None;
    DebugLogAutoDisableFrames = 0;
    DebugLocateFrames = 0;
    DebugBeginReturnValueCullDepth = -1;
    DebugItemPickerActive = false;
    DebugItemPickerMouseButton = ImGuiMouseButton_Left;
    DebugItemPickerBreakId = 0;
    DebugFlashStyleColorTime = 0.0f;
    DebugFlashStyleColorIdx = ImGuiCol_COUNT;
    DebugHoveredDockNode = NULL;

    // Same as DebugBreakClearData(). Those fields are scattered in their respective subsystem to stay in hot-data locations
    DebugBreakInWindow = 0;
    DebugBreakInTable = 0;
    DebugBreakInLocateId = false;
    DebugBreakKeyChord = ImGuiKey_Pause;
    DebugBreakInShortcutRouting = ImGuiKey_None;

    memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
    FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
    FramerateSecPerFrameAccum = 0.0f;
    WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
    memset(TempKeychordName, 0, sizeof(TempKeychordName));
}

ImGuiContext::~ImGuiContext()
{
    IM_ASSERT(Initialized == false && "Forgot to call DestroyContext()?");
}

void ImGui::Initialize()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(!g.Initialized && !g.SettingsLoaded);

    // Add .ini handle for ImGuiWindow and ImGuiTable types
    {
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "Window";
        ini_handler.TypeHash = ImHashStr("Window");
        ini_handler.ClearAllFn = WindowSettingsHandler_ClearAll;
        ini_handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
        ini_handler.ReadLineFn = WindowSettingsHandler_ReadLine;
        ini_handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
        ini_handler.WriteAllFn = WindowSettingsHandler_WriteAll;
        AddSettingsHandler(&ini_handler);
    }
    TableSettingsAddSettingsHandler();

    // Setup default localization table
    LocalizeRegisterEntries(GLocalizationEntriesEnUS, IM_COUNTOF(GLocalizationEntriesEnUS));

    // Setup default ImGuiPlatformIO clipboard/IME handlers.
    g.PlatformIO.Platform_GetClipboardTextFn = Platform_GetClipboardTextFn_DefaultImpl;    // Platform dependent default implementations
    g.PlatformIO.Platform_SetClipboardTextFn = Platform_SetClipboardTextFn_DefaultImpl;
    g.PlatformIO.Platform_OpenInShellFn = Platform_OpenInShellFn_DefaultImpl;
    g.PlatformIO.Platform_SetImeDataFn = Platform_SetImeDataFn_DefaultImpl;

    // Create default viewport
    ImGuiViewportP* viewport = IM_NEW(ImGuiViewportP)();
    viewport->ID = IMGUI_VIEWPORT_DEFAULT_ID;
    viewport->Idx = 0;
    viewport->PlatformWindowCreated = true;
    viewport->Flags = ImGuiViewportFlags_OwnedByApp;
    g.Viewports.push_back(viewport);
    g.TempBuffer.resize(1024 * 3 + 1, 0);
    g.ViewportCreatedCount++;
    g.PlatformIO.Viewports.push_back(g.Viewports[0]);

    // Build KeysMayBeCharInput[] lookup table (1 bit per named key)
    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
        if ((key >= ImGuiKey_0 && key <= ImGuiKey_9) || (key >= ImGuiKey_A && key <= ImGuiKey_Z) || (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9)
            || key == ImGuiKey_Tab || key == ImGuiKey_Space || key == ImGuiKey_Apostrophe || key == ImGuiKey_Comma || key == ImGuiKey_Minus || key == ImGuiKey_Period
            || key == ImGuiKey_Slash || key == ImGuiKey_Semicolon || key == ImGuiKey_Equal || key == ImGuiKey_LeftBracket || key == ImGuiKey_RightBracket || key == ImGuiKey_GraveAccent
            || key == ImGuiKey_KeypadDecimal || key == ImGuiKey_KeypadDivide || key == ImGuiKey_KeypadMultiply || key == ImGuiKey_KeypadSubtract || key == ImGuiKey_KeypadAdd || key == ImGuiKey_KeypadEqual)
            g.KeysMayBeCharInput.SetBit(key);

#ifdef IMGUI_HAS_DOCK
    // Initialize Docking
    DockContextInitialize(&g);
#endif

    // Print a debug message when running with debug feature IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS because it is very slow.
    // DO NOT COMMENT OUT THIS MESSAGE. IT IS DESIGNED TO REMIND YOU THAT IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS SHOULD ONLY BE TEMPORARILY ENABLED.
#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
    DebugLog("IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS is enabled.\nMust disable after use! Otherwise Dear ImGui will run slower.\n");
#endif

    // ImDrawList/ImFontAtlas are designed to function without ImGui, and 99% of it works without an ImGui context.
    // But this link allows us to facilitate/handle a few edge cases better.
    ImFontAtlas* atlas = g.IO.Fonts;
    g.DrawListSharedData.Context = &g;
    RegisterFontAtlas(atlas);

    g.Initialized = true;
}

// This function is merely here to free heap allocations.
void ImGui::Shutdown()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR(g.IO.BackendPlatformUserData == NULL, "Forgot to shutdown Platform backend?");
    IM_ASSERT_USER_ERROR(g.IO.BackendRendererUserData == NULL, "Forgot to shutdown Renderer backend?");
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        IM_UNUSED(viewport);
        IM_ASSERT_USER_ERROR(viewport->RendererUserData == NULL && viewport->PlatformUserData == NULL && viewport->PlatformHandle == NULL, "Backend or app forgot to call DestroyPlatformWindows()?");
    }

    // The fonts atlas can be used prior to calling NewFrame(), so we clear it even if g.Initialized is FALSE (which would happen if we never called NewFrame)
    for (ImFontAtlas* atlas : g.FontAtlases)
    {
        UnregisterFontAtlas(atlas);
        if (atlas->RefCount == 0)
        {
            atlas->Locked = false;
            IM_DELETE(atlas);
        }
    }
    g.DrawListSharedData.TempBuffer.clear();

    // Cleanup of other data are conditional on actually having initialized Dear ImGui.
    if (!g.Initialized)
        return;

    // Save settings (unless we haven't attempted to load them: CreateContext/DestroyContext without a call to NewFrame shouldn't save an empty file)
    if (g.SettingsLoaded && g.IO.IniFilename != NULL)
        SaveIniSettingsToDisk(g.IO.IniFilename);

    // Shutdown extensions
    DockContextShutdown(&g);

    CallContextHooks(&g, ImGuiContextHookType_Shutdown);

    // Clear everything else
    g.Windows.clear_delete();
    g.WindowsFocusOrder.clear();
    g.WindowsTempSortBuffer.clear();
    g.CurrentWindow = NULL;
    g.CurrentWindowStack.clear();
    g.WindowsById.Clear();
    g.NavWindow = NULL;
    g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;
    g.ActiveIdWindow = NULL;
    g.MovingWindow = NULL;

    g.KeysRoutingTable.Clear();

    g.ColorStack.clear();
    g.StyleVarStack.clear();
    g.FontStack.clear();
    g.OpenPopupStack.clear();
    g.BeginPopupStack.clear();
    g.TreeNodeStack.clear();

    g.CurrentViewport = g.MouseViewport = g.MouseLastHoveredViewport = NULL;
    g.Viewports.clear_delete();

    g.TabBars.Clear();
    g.CurrentTabBarStack.clear();
    g.ShrinkWidthBuffer.clear();

    g.ClipperTempData.clear_destruct();

    g.Tables.Clear();
    g.TablesTempData.clear_destruct();
    g.DrawChannelsTempMergeBuffer.clear();

    g.MultiSelectStorage.Clear();
    g.MultiSelectTempData.clear_destruct();

    g.ClipboardHandlerData.clear();
    g.MenusIdSubmittedThisFrame.clear();
    g.InputTextState.ClearFreeMemory();
    g.InputTextLineIndex.clear();
    g.InputTextDeactivatedState.ClearFreeMemory();

    g.SettingsWindows.clear();
    g.SettingsHandlers.clear();

    if (g.LogFile)
    {
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
        if (g.LogFile != stdout)
#endif
            ImFileClose(g.LogFile);
        g.LogFile = NULL;
    }
    g.LogBuffer.clear();
    g.DebugLogBuf.clear();
    g.DebugLogIndex.clear();

    g.Initialized = false;
}

// When using multiple context it can be helpful to give name a name.
// (A) Will be visible in debugger, (B) Will be included in all IMGUI_DEBUG_LOG() calls, (C) Should be <= 15 characters long.
void ImGui::SetContextName(ImGuiContext* ctx, const char* name)
{
    ImStrncpy(ctx->ContextName, name, IM_COUNTOF(ctx->ContextName));
}

// No specific ordering/dependency support, will see as needed
ImGuiID ImGui::AddContextHook(ImGuiContext* ctx, const ImGuiContextHook* hook)
{
    ImGuiContext& g = *ctx;
    IM_ASSERT(hook->Callback != NULL && hook->HookId == 0 && hook->Type != ImGuiContextHookType_PendingRemoval_);
    g.Hooks.push_back(*hook);
    g.Hooks.back().HookId = ++g.HookIdNext;
    return g.HookIdNext;
}

// Deferred removal, avoiding issue with changing vector while iterating it
void ImGui::RemoveContextHook(ImGuiContext* ctx, ImGuiID hook_id)
{
    ImGuiContext& g = *ctx;
    IM_ASSERT(hook_id != 0);
    for (ImGuiContextHook& hook : g.Hooks)
        if (hook.HookId == hook_id)
            hook.Type = ImGuiContextHookType_PendingRemoval_;
}

// Call context hooks (used by e.g. test engine)
// We assume a small number of hooks so all stored in same array
void ImGui::CallContextHooks(ImGuiContext* ctx, ImGuiContextHookType hook_type)
{
    ImGuiContext& g = *ctx;
    for (ImGuiContextHook& hook : g.Hooks)
        if (hook.Type == hook_type)
            hook.Callback(&g, &hook);
}

//-----------------------------------------------------------------------------
// [SECTION] MAIN CODE (most of the code! lots of stuff, needs tidying up!)
//-----------------------------------------------------------------------------

// ImGuiWindow is mostly a dumb struct. It merely has a constructor and a few helper methods
ImGuiWindow::ImGuiWindow(ImGuiContext* ctx, const char* name) : DrawListInst(NULL)
{
    memset((void*)this, 0, sizeof(*this));
    Ctx = ctx;
    Name = ImStrdup(name);
    NameBufLen = (int)ImStrlen(name) + 1;
    ID = ImHashStr(name);
    IDStack.push_back(ID);
    ViewportAllowPlatformMonitorExtend = -1;
    ViewportPos = ImVec2(FLT_MAX, FLT_MAX);
    MoveId = GetID("#MOVE");
    TabId = GetID("#TAB");
    ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
    ScrollTargetCenterRatio = ImVec2(0.5f, 0.5f);
    AutoPosLastDirection = ImGuiDir_None;
    AutoFitFramesX = AutoFitFramesY = -1;
    SetWindowPosAllowFlags = SetWindowSizeAllowFlags = SetWindowCollapsedAllowFlags = SetWindowDockAllowFlags = 0;
    SetWindowPosVal = SetWindowPosPivot = ImVec2(FLT_MAX, FLT_MAX);
    LastFrameActive = -1;
    LastFrameJustFocused = -1;
    LastTimeActive = -1.0f;
    FontRefSize = 0.0f;
    FontWindowScale = FontWindowScaleParents = 1.0f;
    SettingsOffset = -1;
    DockOrder = -1;
    DrawList = &DrawListInst;
    DrawList->_OwnerName = Name;
    DrawList->_SetDrawListSharedData(&Ctx->DrawListSharedData);
    NavPreferredScoringPosRel[0] = NavPreferredScoringPosRel[1] = ImVec2(FLT_MAX, FLT_MAX);
    IM_PLACEMENT_NEW(&WindowClass) ImGuiWindowClass();
}

ImGuiWindow::~ImGuiWindow()
{
    IM_ASSERT(DrawList == &DrawListInst);
    IM_DELETE(Name);
    ColumnsStorage.clear_destruct();
}

static void SetCurrentWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow = window;
    g.StackSizesInBeginForCurrentWindow = g.CurrentWindow ? &g.CurrentWindowStack.back().StackSizesInBegin : NULL;
    g.CurrentTable = window && window->DC.CurrentTableIdx != -1 ? g.Tables.GetByIndex(window->DC.CurrentTableIdx) : NULL;
    if (window)
    {
        if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        {
            ImGuiViewport* viewport = window->Viewport;
            g.FontRasterizerDensity = (viewport->FramebufferScale.x != 0.0f) ? viewport->FramebufferScale.x : g.IO.DisplayFramebufferScale.x; // == SetFontRasterizerDensity()
        }
        const bool backup_skip_items = window->SkipItems;
        window->SkipItems = false;
        ImGui::UpdateCurrentFontSize(0.0f);
        window->SkipItems = backup_skip_items;
        ImGui::NavUpdateCurrentWindowIsScrollPushableX();
    }
}

void ImGui::GcCompactTransientMiscBuffers()
{
    ImGuiContext& g = *GImGui;
    g.ItemFlagsStack.clear();
    g.GroupStack.clear();
    g.InputTextLineIndex.clear();
    g.MultiSelectTempDataStacked = 0;
    g.MultiSelectTempData.clear_destruct();
    TableGcCompactSettings();
    for (ImFontAtlas* atlas : g.FontAtlases)
        atlas->CompactCache();
}

// Free up/compact internal window buffers, we can use this when a window becomes unused.
// Not freed:
// - ImGuiWindow, ImGuiWindowSettings, Name, StateStorage, ColumnsStorage (may hold useful data)
// This should have no noticeable visual effect. When the window reappear however, expect new allocation/buffer growth/copy cost.
// FIXME: Consider exposing of elaborating GC policy, e.g. being able to trim excessive ImDrawList gaps. (#9303)
void ImGui::GcCompactTransientWindowBuffers(ImGuiWindow* window)
{
    window->MemoryCompacted = true;
    window->MemoryDrawListIdxCapacity = ImMin((int)(window->DrawList->IdxBuffer.Size * 1.05f), window->DrawList->IdxBuffer.Capacity);
    window->MemoryDrawListVtxCapacity = ImMin((int)(window->DrawList->VtxBuffer.Size * 1.05f), window->DrawList->VtxBuffer.Capacity);
    window->IDStack.clear();
    window->DrawList->_ClearFreeMemory();
    window->DC.ChildWindows.clear();
    window->DC.ItemWidthStack.clear();
    window->DC.TextWrapPosStack.clear();
}

void ImGui::GcAwakeTransientWindowBuffers(ImGuiWindow* window)
{
    // We stored capacity of the ImDrawList buffer to reduce growth-caused allocation/copy when awakening.
    // The other buffers tends to amortize much faster.
    window->MemoryCompacted = false;
    window->DrawList->IdxBuffer.reserve(window->MemoryDrawListIdxCapacity);
    window->DrawList->VtxBuffer.reserve(window->MemoryDrawListVtxCapacity);
    window->MemoryDrawListIdxCapacity = window->MemoryDrawListVtxCapacity = 0;
}

void ImGui::SetActiveID(ImGuiID id, ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;

    // Clear previous active id
    if (g.ActiveId != 0)
    {
        // Store deactivate data
        ImGuiDeactivatedItemData* deactivated_data = &g.DeactivatedItemData;
        deactivated_data->ID = g.ActiveId;
        deactivated_data->ElapseFrame = (g.LastItemData.ID == g.ActiveId) ? g.FrameCount : g.FrameCount + 1; // FIXME: OK to use LastItemData?
        deactivated_data->HasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
        deactivated_data->IsAlive = (g.ActiveIdIsAlive == g.ActiveId);

        // This could be written in a more general way (e.g associate a hook to ActiveId),
        // but since this is currently quite an exception we'll leave it as is.
        // One common scenario leading to this is: pressing Key ->NavMoveRequestApplyResult() -> ClearActiveID()
        if (g.InputTextState.ID == g.ActiveId)
            InputTextDeactivateHook(g.ActiveId);

        // While most behaved code would make an effort to not steal active id during window move/drag operations,
        // we at least need to be resilient to it. Canceling the move is rather aggressive and users of 'master' branch
        // may prefer the weird ill-defined half working situation ('docking' did assert), so may need to rework that.
        if (g.MovingWindow != NULL && g.ActiveId == g.MovingWindow->MoveId)
        {
            IMGUI_DEBUG_LOG_ACTIVEID("SetActiveID() cancel MovingWindow\n");
            StopMouseMovingWindow();
        }
    }

    // Set active id
    g.ActiveIdIsJustActivated = (g.ActiveId != id);
    if (g.ActiveIdIsJustActivated)
    {
        IMGUI_DEBUG_LOG_ACTIVEID("SetActiveID() old:0x%08X (window \"%s\") -> new:0x%08X (window \"%s\")\n", g.ActiveId, g.ActiveIdWindow ? g.ActiveIdWindow->Name : "", id, window ? window->Name : "");
        g.ActiveIdTimer = 0.0f;
        g.ActiveIdHasBeenPressedBefore = false;
        g.ActiveIdHasBeenEditedBefore = false;
        g.ActiveIdMouseButton = -1;
        if (id != 0)
        {
            g.LastActiveId = id;
            g.LastActiveIdTimer = 0.0f;
        }
    }
    g.ActiveId = id;
    g.ActiveIdAllowOverlap = false;
    g.ActiveIdNoClearOnFocusLoss = false;
    g.ActiveIdWindow = window;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdFromShortcut = false;
    g.ActiveIdDisabledId = 0;
    if (id)
    {
        g.ActiveIdIsAlive = id;
        g.ActiveIdSource = (g.NavActivateId == id || g.NavJustMovedToId == id) ? g.NavInputSource : ImGuiInputSource_Mouse;
        IM_ASSERT(g.ActiveIdSource != ImGuiInputSource_None);
    }

    // Clear declaration of inputs claimed by the widget
    // (Please note that this is WIP and not all keys/inputs are thoroughly declared by all widgets yet)
    g.ActiveIdUsingNavDirMask = 0x00;
    g.ActiveIdUsingAllKeyboardKeys = false;
}

void ImGui::ClearActiveID()
{
    SetActiveID(0, NULL); // g.ActiveId = 0;
}

void ImGui::SetHoveredID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.HoveredId = id;
    g.HoveredIdAllowOverlap = false;
    if (id != 0 && g.HoveredIdPreviousFrame != id)
        g.HoveredIdTimer = g.HoveredIdNotActiveTimer = 0.0f;
}

ImGuiID ImGui::GetHoveredID()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredId ? g.HoveredId : g.HoveredIdPreviousFrame;
}

void ImGui::MarkItemEdited(ImGuiID id)
{
    // This marking is to be able to provide info for IsItemDeactivatedAfterEdit().
    // ActiveId might have been released by the time we call this (as in the typical press/release button behavior) but still need to fill the data.
    ImGuiContext& g = *GImGui;

    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_EditedInternal;
    if (g.LastItemData.ItemFlags & ImGuiItemFlags_NoMarkEdited)
        return;
    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;

    if (g.ActiveId == id || g.ActiveId == 0)
    {
        // FIXME: Can't we fully rely on LastItemData yet?
        g.ActiveIdHasBeenEditedThisFrame = true;
        g.ActiveIdHasBeenEditedBefore = true;
        if (g.DeactivatedItemData.ID == id)
            g.DeactivatedItemData.HasBeenEditedBefore = true;
    }

    // We accept a MarkItemEdited() on drag and drop targets (see https://github.com/ocornut/imgui/issues/1875#issuecomment-978243343)
    // We accept 'ActiveIdPreviousFrame == id' for InputText() returning an edit after it has been taken ActiveId away (#4714)
    // FIXME: This assert is getting a bit meaningless over time. It helped detect some unusual use cases but eventually it is becoming an unnecessary restriction.
    IM_ASSERT(g.DragDropActive || g.ActiveId == id || g.ActiveId == 0 || g.ActiveIdPreviousFrame == id || g.NavJustMovedToId || (g.CurrentMultiSelect != NULL && g.BoxSelectState.IsActive));
}

bool ImGui::IsWindowContentHoverable(ImGuiWindow* window, ImGuiHoveredFlags flags)
{
    // An active popup disable hovering on other windows (apart from its own children)
    // FIXME-OPT: This could be cached/stored within the window.
    ImGuiContext& g = *GImGui;
    if (g.NavWindow)
        if (ImGuiWindow* focused_root_window = g.NavWindow->RootWindowDockTree)
            if (focused_root_window->WasActive && focused_root_window != window->RootWindowDockTree)
            {
                // For the purpose of those flags we differentiate "standard popup" from "modal popup"
                // NB: The 'else' is important because Modal windows are also Popups.
                bool want_inhibit = false;
                if (focused_root_window->Flags & ImGuiWindowFlags_Modal)
                    want_inhibit = true;
                else if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiHoveredFlags_AllowWhenBlockedByPopup))
                    want_inhibit = true;

                // Inhibit hover unless the window is within the stack of our modal/popup
                if (want_inhibit)
                    if (!IsWindowWithinBeginStackOf(window->RootWindow, focused_root_window))
                        return false;
            }

    // Filter by viewport
    if (window->Viewport != g.MouseViewport)
        if (g.MovingWindow == NULL || window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree)
            return false;

    return true;
}

static inline float CalcDelayFromHoveredFlags(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (flags & ImGuiHoveredFlags_DelayNormal)
        return g.Style.HoverDelayNormal;
    if (flags & ImGuiHoveredFlags_DelayShort)
        return g.Style.HoverDelayShort;
    return 0.0f;
}

static ImGuiHoveredFlags ApplyHoverFlagsForTooltip(ImGuiHoveredFlags user_flags, ImGuiHoveredFlags shared_flags)
{
    // Allow instance flags to override shared flags
    if (user_flags & (ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_DelayNormal))
        shared_flags &= ~(ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_DelayNormal);
    return user_flags | shared_flags;
}

// This is roughly matching the behavior of internal-facing ItemHoverable()
// - we allow hovering to be true when ActiveId==window->MoveID, so that clicking on non-interactive items such as a Text() item still returns true with IsItemHovered()
// - this should work even for non-interactive items that have no ID, so we cannot use LastItemId
bool ImGui::IsItemHovered(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR((flags & ~ImGuiHoveredFlags_AllowedMaskForIsItemHovered) == 0, "Invalid flags for IsItemHovered()!");

    if (g.NavHighlightItemUnderNav && g.NavCursorVisible && !(flags & ImGuiHoveredFlags_NoNavOverride))
    {
        if (!IsItemFocused())
            return false;
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled) && !(flags & ImGuiHoveredFlags_AllowWhenDisabled))
            return false;

        if (flags & ImGuiHoveredFlags_ForTooltip)
            flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipNav);
    }
    else
    {
        // Test for bounding box overlap, as updated as ItemAdd()
        ImGuiItemStatusFlags status_flags = g.LastItemData.StatusFlags;
        if (!(status_flags & ImGuiItemStatusFlags_HoveredRect))
            return false;

        if (flags & ImGuiHoveredFlags_ForTooltip)
            flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);

        // Done with rectangle culling so we can perform heavier checks now
        // Test if we are hovering the right window (our window could be behind another window)
        // [2021/03/02] Reworked / reverted the revert, finally. Note we want e.g. BeginGroup/ItemAdd/EndGroup to work as well. (#3851)
        // [2017/10/16] Reverted commit 344d48be3 and testing RootWindow instead. I believe it is correct to NOT test for RootWindow but this leaves us unable
        // to use IsItemHovered() after EndChild() itself. Until a solution is found I believe reverting to the test from 2017/09/27 is safe since this was
        // the test that has been running for a long while.
        if (g.HoveredWindow != window && (status_flags & ImGuiItemStatusFlags_HoveredWindow) == 0)
            if ((flags & ImGuiHoveredFlags_AllowWhenOverlappedByWindow) == 0)
                return false;

        // Test if another item is active (e.g. being dragged)
        const ImGuiID id = g.LastItemData.ID;
        if ((flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) == 0)
            if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap && !g.ActiveIdFromShortcut)
            {
                // When ActiveId == MoveId it means that either:
                // - (1) user clicked on void _or_ an item with no id, which triggers moving window (ActiveId is set even when window has _NoMove flag)
                //   - the (id == 0) test handles it, however, IsItemHovered() will leak between id==0 items (mostly visible when using _NoMove). // FIXME: May be fixed.
                // - (2) user clicked a disabled item. UpdateMouseMovingWindowEndFrame() uses ActiveId == MoveId to avoid interference with item logic + sets ActiveIdDisabledId.
                bool cancel_is_hovered = true;
                if (g.ActiveId == window->MoveId && (id == 0 || g.ActiveIdDisabledId == id))
                    cancel_is_hovered = false;
                // When ActiveId == TabId it means user clicked docking tab for the window.
                if (g.ActiveId == window->TabId)
                    cancel_is_hovered = false;
                if (cancel_is_hovered)
                    return false;
            }

        // Test if interactions on this window are blocked by an active popup or modal.
        // The ImGuiHoveredFlags_AllowWhenBlockedByPopup flag will be tested here.
        if (!IsWindowContentHoverable(window, flags) && !(g.LastItemData.ItemFlags & ImGuiItemFlags_NoWindowHoverableCheck))
            return false;

        // Test if the item is disabled
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled) && !(flags & ImGuiHoveredFlags_AllowWhenDisabled))
            return false;

        // Special handling for calling after Begin() which represent the title bar or tab.
        // When the window is skipped/collapsed (SkipItems==true) that last item (always ->MoveId submitted by Begin)
        // will never be overwritten so we need to detect the case.
        if (id == window->MoveId && window->WriteAccessed)
            return false;

        // Test if using AllowOverlap and overlapped
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap) && id != 0)
            if ((flags & ImGuiHoveredFlags_AllowWhenOverlappedByItem) == 0)
                if (g.HoveredIdPreviousFrame != g.LastItemData.ID)
                    return false;
    }

    // Handle hover delay
    // (some ideas: https://www.nngroup.com/articles/timing-exposing-content)
    const float delay = CalcDelayFromHoveredFlags(flags);
    if (delay > 0.0f || (flags & ImGuiHoveredFlags_Stationary))
    {
        ImGuiID hover_delay_id = (g.LastItemData.ID != 0) ? g.LastItemData.ID : window->GetIDFromPos(g.LastItemData.Rect.Min);
        if ((flags & ImGuiHoveredFlags_NoSharedDelay) && (g.HoverItemDelayIdPreviousFrame != hover_delay_id))
            g.HoverItemDelayTimer = 0.0f;
        g.HoverItemDelayId = hover_delay_id;

        // When changing hovered item we requires a bit of stationary delay before activating hover timer,
        // but once unlocked on a given item we also moving.
        //if (g.HoverDelayTimer >= delay && (g.HoverDelayTimer - g.IO.DeltaTime < delay || g.MouseStationaryTimer - g.IO.DeltaTime < g.Style.HoverStationaryDelay)) { IMGUI_DEBUG_LOG("HoverDelayTimer = %f/%f, MouseStationaryTimer = %f\n", g.HoverDelayTimer, delay, g.MouseStationaryTimer); }
        if ((flags & ImGuiHoveredFlags_Stationary) != 0 && g.HoverItemUnlockedStationaryId != hover_delay_id)
            return false;

        if (g.HoverItemDelayTimer < delay)
            return false;
    }

    return true;
}

// Internal facing ItemHoverable() used when submitting widgets. THIS IS A SUBMISSION NOT A HOVER CHECK.
// Returns whether the item was hovered, logic differs slightly from IsItemHovered().
// (this does not rely on LastItemData it can be called from a ButtonBehavior() call not following an ItemAdd() call)
// FIXME-LEGACY: the 'ImGuiItemFlags item_flags' parameter was added on 2023-06-28.
// If you used this in your legacy/custom widgets code:
// - Commonly: if your ItemHoverable() call comes after an ItemAdd() call: pass 'item_flags = g.LastItemData.ItemFlags'.
// - Rare: otherwise you may pass 'item_flags = 0' (ImGuiItemFlags_None) unless you want to benefit from special behavior handled by ItemHoverable.
bool ImGui::ItemHoverable(const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Detect ID conflicts
    // (this is specifically done here by comparing on hover because it allows us a detection of duplicates that is algorithmically extra cheap, 1 u32 compare per item. No O(log N) lookup whatsoever)
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0 && g.HoveredIdPreviousFrame == id && (item_flags & ImGuiItemFlags_AllowDuplicateId) == 0)
    {
        g.HoveredIdPreviousFrameItemCount++;
        if (g.DebugDrawIdConflictsId == id)
            window->DrawList->AddRect(bb.Min - ImVec2(1,1), bb.Max + ImVec2(1,1), IM_COL32(255, 0, 0, 255), 0.0f, ImDrawFlags_None, 2.0f);
    }
#endif

    if (g.HoveredWindow != window)
        return false;
    if (!IsMouseHoveringRect(bb.Min, bb.Max))
        return false;

    if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
        return false;
    if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
        if (!g.ActiveIdFromShortcut)
            return false;

    // We are done with rectangle culling so we can perform heavier checks now.
    if (!(item_flags & ImGuiItemFlags_NoWindowHoverableCheck) && !IsWindowContentHoverable(window, ImGuiHoveredFlags_None))
    {
        g.HoveredIdIsDisabled = true;
        return false;
    }

    // We exceptionally allow this function to be called with id==0 to allow using it for easy high-level
    // hover test in widgets code. We could also decide to split this function is two.
    if (id != 0)
    {
        // Drag source doesn't report as hovered
        if (g.DragDropActive && g.DragDropPayload.SourceId == id && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoDisableHover))
            return false;

        SetHoveredID(id);

        // AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match.
        // This allows using patterns where a later submitted widget overlaps a previous one. Generally perceived as a front-to-back hit-test.
        if (item_flags & ImGuiItemFlags_AllowOverlap)
        {
            g.HoveredIdAllowOverlap = true;
            if (g.HoveredIdPreviousFrame != id)
                return false;
        }

        // Display shortcut (only works with mouse)
        // (ImGuiItemStatusFlags_HasShortcut in LastItemData denotes we want a tooltip)
        if (id == g.LastItemData.ID && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasShortcut) && g.ActiveId != id)
            if (IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal))
                SetTooltip("%s", GetKeyChordName(g.LastItemData.Shortcut));
    }

    // When disabled we'll return false but still set HoveredId
    if (item_flags & ImGuiItemFlags_Disabled)
    {
        // Release active id if turning disabled
        if (g.ActiveId == id && id != 0)
            ClearActiveID();
        g.HoveredIdIsDisabled = true;
        return false;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0)
    {
        // [DEBUG] Item Picker tool!
        // We perform the check here because reaching is path is rare (1~ time a frame),
        // making the cost of this tool near-zero! We could get better call-stack and support picking non-hovered
        // items if we performed the test in ItemAdd(), but that would incur a bigger runtime cost.
        if (g.DebugItemPickerActive && g.HoveredIdPreviousFrame == id)
            GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 0, 255));
        if (g.DebugItemPickerBreakId == id)
            IM_DEBUG_BREAK();
    }
#endif

    if (g.NavHighlightItemUnderNav && (item_flags & ImGuiItemFlags_NoNavDisableMouseHover) == 0)
        return false;

    return true;
}

// FIXME: This is inlined/duplicated in ItemAdd()
// FIXME: The id != 0 path is not used by our codebase, may get rid of it?
bool ImGui::IsClippedEx(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!bb.Overlaps(window->ClipRect))
        if (id == 0 || (id != g.ActiveId && id != g.ActiveIdPreviousFrame && id != g.NavId && id != g.NavActivateId))
            if (!g.ItemUnclipByLog)
                return true;
    return false;
}

// This is also inlined in ItemAdd()
// Note: if ImGuiItemStatusFlags_HasDisplayRect is set, user needs to set g.LastItemData.DisplayRect.
void ImGui::SetLastItemData(ImGuiID item_id, ImGuiItemFlags item_flags, ImGuiItemStatusFlags status_flags, const ImRect& item_rect)
{
    ImGuiContext& g = *GImGui;
    g.LastItemData.ID = item_id;
    g.LastItemData.ItemFlags = item_flags;
    g.LastItemData.StatusFlags = status_flags;
    g.LastItemData.Rect = g.LastItemData.NavRect = item_rect;
}

static void ImGui::SetLastItemDataForWindow(ImGuiWindow* window, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    if (window->DockIsActive)
        SetLastItemData(window->MoveId, g.CurrentItemFlags, window->DC.DockTabItemStatusFlags, window->DC.DockTabItemRect);
    else
        SetLastItemData(window->MoveId, g.CurrentItemFlags, window->DC.WindowItemStatusFlags, rect);
}

static void ImGui::SetLastItemDataForChildWindowItem(ImGuiWindow* window, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    SetLastItemData(window->ChildId, g.CurrentItemFlags, window->DC.ChildItemStatusFlags, rect);
}

float ImGui::CalcWrapWidthForPos(const ImVec2& pos, float wrap_pos_x)
{
    if (wrap_pos_x < 0.0f)
        return 0.0f;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (wrap_pos_x == 0.0f)
    {
        // We could decide to setup a default wrapping max point for auto-resizing windows,
        // or have auto-wrap (with unspecified wrapping pos) behave as a ContentSize extending function?
        //if (window->Hidden && (window->Flags & ImGuiWindowFlags_AlwaysAutoResize))
        //    wrap_pos_x = ImMax(window->WorkRect.Min.x + g.FontSize * 10.0f, window->WorkRect.Max.x);
        //else
        wrap_pos_x = window->WorkRect.Max.x;
    }
    else if (wrap_pos_x > 0.0f)
    {
        wrap_pos_x += window->Pos.x - window->Scroll.x; // wrap_pos_x is provided is window local space
    }

    return ImMax(wrap_pos_x - pos.x, 1.0f);
}

// IM_ALLOC() == ImGui::MemAlloc()
void* ImGui::MemAlloc(size_t size)
{
    void* ptr = (*GImAllocatorAllocFunc)(size, GImAllocatorUserData);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (ImGuiContext* ctx = GImGui)
        DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, size);
#endif
    return ptr;
}

// IM_FREE() == ImGui::MemFree()
void ImGui::MemFree(void* ptr)
{
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (ptr != NULL)
        if (ImGuiContext* ctx = GImGui)
            DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, (size_t)-1);
#endif
    return (*GImAllocatorFreeFunc)(ptr, GImAllocatorUserData);
}

void ImGui::DemoMarker(const char* file, int line, const char* section)
{
    ImGuiContext& g = *GImGui;
    if (g.DemoMarkerCallback != NULL)
        g.DemoMarkerCallback(file, line, section);
}

// We record the number of allocation in recent frames, as a way to audit/sanitize our guiding principles of "no allocations on idle/repeating frames"
void ImGui::DebugAllocHook(ImGuiDebugAllocInfo* info, int frame_count, void* ptr, size_t size)
{
    ImGuiDebugAllocEntry* entry = &info->LastEntriesBuf[info->LastEntriesIdx];
    IM_UNUSED(ptr);
    if (entry->FrameCount != frame_count)
    {
        info->LastEntriesIdx = (info->LastEntriesIdx + 1) % IM_COUNTOF(info->LastEntriesBuf);
        entry = &info->LastEntriesBuf[info->LastEntriesIdx];
        entry->FrameCount = frame_count;
        entry->AllocCount = entry->FreeCount = 0;
    }
    if (size != (size_t)-1)
    {
        //printf("[%05d] MemAlloc(%d) -> 0x%p\n", frame_count, (int)size, ptr);
        entry->AllocCount++;
        info->TotalAllocCount++;
    }
    else
    {
        //printf("[%05d] MemFree(0x%p)\n", frame_count, ptr);
        entry->FreeCount++;
        info->TotalFreeCount++;
    }
}

// A conformant backend should return NULL on failure (e.g. clipboard data is not text).
const char* ImGui::GetClipboardText()
{
    ImGuiContext& g = *GImGui;
    return g.PlatformIO.Platform_GetClipboardTextFn ? g.PlatformIO.Platform_GetClipboardTextFn(&g) : NULL;
}

void ImGui::SetClipboardText(const char* text)
{
    ImGuiContext& g = *GImGui;
    if (g.PlatformIO.Platform_SetClipboardTextFn != NULL)
        g.PlatformIO.Platform_SetClipboardTextFn(&g, text);
}

const char* ImGui::GetVersion()
{
    return IMGUI_VERSION;
}

ImGuiIO& ImGui::GetIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->IO;
}

// This variant exists to facilitate backends experimenting with multi-threaded parallel context. (#8069, #6293, #5856)
ImGuiIO& ImGui::GetIO(ImGuiContext* ctx)
{
    IM_ASSERT(ctx != NULL);
    return ctx->IO;
}

ImGuiPlatformIO& ImGui::GetPlatformIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext()?");
    return GImGui->PlatformIO;
}

// This variant exists to facilitate backends experimenting with multi-threaded parallel context. (#8069, #6293, #5856)
ImGuiPlatformIO& ImGui::GetPlatformIO(ImGuiContext* ctx)
{
    IM_ASSERT(ctx != NULL);
    return ctx->PlatformIO;
}

// Pass this to your backend rendering function! Valid after Render() and until the next call to NewFrame()
ImDrawData* ImGui::GetDrawData()
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = g.Viewports[0];
    return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

double ImGui::GetTime()
{
    return GImGui->Time;
}

int ImGui::GetFrameCount()
{
    return GImGui->FrameCount;
}

static ImDrawList* GetViewportBgFgDrawList(ImGuiViewportP* viewport, size_t drawlist_no, const char* drawlist_name)
{
    // Create the draw list on demand, because they are not frequently used for all viewports
    ImGuiContext& g = *GImGui;
    IM_ASSERT(drawlist_no < IM_COUNTOF(viewport->BgFgDrawLists));
    ImDrawList* draw_list = viewport->BgFgDrawLists[drawlist_no];
    if (draw_list == NULL)
    {
        draw_list = IM_NEW(ImDrawList)(&g.DrawListSharedData);
        draw_list->_OwnerName = drawlist_name;
        viewport->BgFgDrawLists[drawlist_no] = draw_list;
    }

    // Our ImDrawList system requires that there is always a command
    if (viewport->BgFgDrawListsLastTimeActive[drawlist_no] != (float)g.Time)
    {
        draw_list->_ResetForNewFrame();
        draw_list->PushTexture(g.IO.Fonts->TexRef);
        draw_list->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size, false);
        viewport->BgFgDrawListsLastTimeActive[drawlist_no] = (float)g.Time;
    }
    return draw_list;
}

ImDrawList* ImGui::GetBackgroundDrawList(ImGuiViewport* viewport)
{
    if (viewport == NULL)
        viewport = GImGui->CurrentWindow->Viewport;
    return GetViewportBgFgDrawList((ImGuiViewportP*)viewport, 0, "##Background");
}

ImDrawList* ImGui::GetForegroundDrawList(ImGuiViewport* viewport)
{
    if (viewport == NULL)
        viewport = GImGui->CurrentWindow->Viewport;
    return GetViewportBgFgDrawList((ImGuiViewportP*)viewport, 1, "##Foreground");
}

ImDrawListSharedData* ImGui::GetDrawListSharedData()
{
    return &GImGui->DrawListSharedData;
}

void ImGui::StartMouseMovingWindow(ImGuiWindow* window)
{
    // Set ActiveId even if the _NoMove flag is set. Without it, dragging away from a window with _NoMove would activate hover on other windows.
    // We _also_ call this when clicking in a window empty space when io.ConfigWindowsMoveFromTitleBarOnly is set, but clear g.MovingWindow afterward.
    // This is because we want ActiveId to be set even when the window is not permitted to move.
    ImGuiContext& g = *GImGui;
    FocusWindow(window);
    SetActiveID(window->MoveId, window);
    if (g.IO.ConfigNavCursorVisibleAuto)
        g.NavCursorVisible = false;
    g.ActiveIdClickOffset = g.IO.MouseClickedPos[0] - window->RootWindowDockTree->Pos;
    g.ActiveIdNoClearOnFocusLoss = true;
    SetActiveIdUsingAllKeyboardKeys();

    bool can_move_window = true;
    if ((window->Flags & ImGuiWindowFlags_NoMove) || (window->RootWindowDockTree->Flags & ImGuiWindowFlags_NoMove))
        can_move_window = false;
    if (ImGuiDockNode* node = window->DockNodeAsHost)
        if (node->VisibleWindow && (node->VisibleWindow->Flags & ImGuiWindowFlags_NoMove))
            can_move_window = false;
    if (can_move_window)
        g.MovingWindow = window;
}

// We use 'undock == false' when dragging from title bar to allow moving groups of floating nodes without undocking them.
void ImGui::StartMouseMovingWindowOrNode(ImGuiWindow* window, ImGuiDockNode* node, bool undock)
{
    ImGuiContext& g = *GImGui;
    bool can_undock_node = false;
    if (undock && node != NULL && node->VisibleWindow && (node->VisibleWindow->Flags & ImGuiWindowFlags_NoMove) == 0 && (node->MergedFlags & ImGuiDockNodeFlags_NoUndocking) == 0)
    {
        // Can undock if:
        // - part of a hierarchy with more than one visible node (if only one is visible, we'll just move the root window)
        // - part of a dockspace node hierarchy: so we can undock the last single visible node too. Undocking from a fixed/central node will create a new node and copy windows.
        ImGuiDockNode* root_node = DockNodeGetRootNode(node);
        if (root_node->OnlyNodeWithWindows != node || root_node->CentralNode != NULL)   // -V1051 PVS-Studio thinks node should be root_node and is wrong about that.
            can_undock_node = true;
    }

    const bool clicked = IsMouseClicked(0);
    const bool dragging = IsMouseDragging(0);
    if (can_undock_node && dragging)
        DockContextQueueUndockNode(&g, node); // Will lead to DockNodeStartMouseMovingWindow() -> StartMouseMovingWindow() being called next frame
    else if (!can_undock_node && (clicked || dragging) && g.MovingWindow != window)
        StartMouseMovingWindow(window);
}

// This is not 100% symmetric with StartMouseMovingWindow().
// We do NOT clear ActiveID, because:
// - It would lead to rather confusing recursive code paths. Caller can call ClearActiveID() if desired.
// - Some code intentionally cancel moving but keep the ActiveID to lock inputs (e.g. code path taken when clicking a disabled item).
void ImGui::StopMouseMovingWindow()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.MovingWindow;

    // Ref commits 6b7766817, 36055213c for some partial history on checking if viewport != NULL.
    if (window && window->Viewport)
    {
        // Try to merge the window back into the main viewport.
        // This works because MouseViewport should be != MovingWindow->Viewport on release (as per code in UpdateViewports)
        if (g.ConfigFlagsCurrFrame & ImGuiConfigFlags_ViewportsEnable)
            UpdateTryMergeWindowIntoHostViewport(window->RootWindowDockTree, g.MouseViewport);

        // Restore the mouse viewport so that we don't hover the viewport _under_ the moved window during the frame we released the mouse button.
        if (!IsDragDropPayloadBeingAccepted())
            g.MouseViewport = window->Viewport;

        // Clear the NoInputs window flag set by the Viewport system in AddUpdateViewport()
        const bool window_can_use_inputs = ((window->Flags & ImGuiWindowFlags_NoMouseInputs) && (window->Flags & ImGuiWindowFlags_NoNavInputs)) == false;
        if (window_can_use_inputs)
            window->Viewport->Flags &= ~ImGuiViewportFlags_NoInputs;
    }
    g.MovingWindow = NULL;
}

// Handle mouse moving window
// Note: moving window with the navigation keys (Square + d-pad / Ctrl+Tab + Arrows) are processed in NavUpdateWindowing()
// FIXME: We don't have strong guarantee that g.MovingWindow stay synced with g.ActiveId == g.MovingWindow->MoveId.
// This is currently enforced by the fact that BeginDragDropSource() is setting all g.ActiveIdUsingXXXX flags to inhibit navigation inputs,
// but if we should more thoroughly test cases where g.ActiveId or g.MovingWindow gets changed and not the other.
void ImGui::UpdateMouseMovingWindowNewFrame()
{
    ImGuiContext& g = *GImGui;
    if (g.MovingWindow != NULL)
    {
        // We actually want to move the root window. g.MovingWindow == window we clicked on (could be a child window).
        // We track it to preserve Focus and so that generally ActiveIdWindow == MovingWindow and ActiveId == MovingWindow->MoveId for consistency.
        KeepAliveID(g.ActiveId);
        IM_ASSERT(g.MovingWindow && g.MovingWindow->RootWindowDockTree);
        ImGuiWindow* moving_window = g.MovingWindow->RootWindowDockTree;

        // When a window stop being submitted while being dragged, it may will its viewport until next Begin()
        const bool window_disappeared = (!moving_window->WasActive && !moving_window->Active);
        if (g.IO.MouseDown[0] && IsMousePosValid(&g.IO.MousePos) && !window_disappeared)
        {
            ImVec2 pos = g.IO.MousePos - g.ActiveIdClickOffset;
            if (moving_window->Pos.x != pos.x || moving_window->Pos.y != pos.y)
            {
                SetWindowPos(moving_window, pos, ImGuiCond_Always);
                if (moving_window->Viewport && moving_window->ViewportOwned) // Synchronize viewport immediately because some overlays may relies on clipping rectangle before we Begin() into the window.
                {
                    moving_window->Viewport->Pos = pos;
                    moving_window->Viewport->UpdateWorkRect();
                }
            }
            FocusWindow(g.MovingWindow);
        }
        else
        {
            StopMouseMovingWindow();
            ClearActiveID();
        }
    }
    else
    {
        // When clicking/dragging from a window that has the _NoMove flag, we still set the ActiveId in order to prevent hovering others.
        if (g.ActiveIdWindow && g.ActiveIdWindow->MoveId == g.ActiveId)
        {
            KeepAliveID(g.ActiveId);
            if (!g.IO.MouseDown[0])
                ClearActiveID();
        }
    }
}

// Initiate focusing and moving window when clicking on empty space or title bar.
// Initiate focusing window when clicking on a disabled item.
// Handle left-click and right-click focus.
void ImGui::UpdateMouseMovingWindowEndFrame()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId != 0 || (g.HoveredId != 0 && !g.HoveredIdIsDisabled))
        return;

    // Unless we just made a window/popup appear
    if (g.NavWindow && g.NavWindow->Appearing)
        return;

    ImGuiWindow* hovered_window = g.HoveredWindow;

    // Click on empty space to focus window and start moving
    // (after we're done with all our widgets, so e.g. clicking on docking tab-bar which have set HoveredId already and not get us here!)
    if (g.IO.MouseClicked[0])
    {
        // Handle the edge case of a popup being closed while clicking in its empty space.
        // If we try to focus it, FocusWindow() > ClosePopupsOverWindow() will accidentally close any parent popups because they are not linked together any more.
        ImGuiWindow* hovered_root = hovered_window ? hovered_window->RootWindow : NULL;
        const bool is_closed_popup = hovered_root && (hovered_root->Flags & ImGuiWindowFlags_Popup) && !IsPopupOpen(hovered_root->PopupId, ImGuiPopupFlags_AnyPopupLevel);

        if (hovered_window != NULL && !is_closed_popup)
        {
            StartMouseMovingWindow(hovered_window); //-V595

            // FIXME: In principle we might be able to call StopMouseMovingWindow() below.
            // Please note how StartMouseMovingWindow() and StopMouseMovingWindow() and not entirely symmetrical, at the later doesn't clear ActiveId.

            // Cancel moving if clicked outside of title bar
            if ((hovered_window->BgClickFlags & ImGuiWindowBgClickFlags_Move) == 0) // set by io.ConfigWindowsMoveFromTitleBarOnly
                if (!(hovered_root->Flags & ImGuiWindowFlags_NoTitleBar) || hovered_root->DockIsActive)
                    if (!hovered_root->TitleBarRect().Contains(g.IO.MouseClickedPos[0]))
                        g.MovingWindow = NULL;

            // Cancel moving if clicked over an item which was disabled or inhibited by popups
            // (when g.HoveredIdIsDisabled == true && g.HoveredId == 0 we are inhibited by popups, when g.HoveredIdIsDisabled == true && g.HoveredId != 0 we are over a disabled item)
            if (g.HoveredIdIsDisabled)
            {
                g.MovingWindow = NULL;
                g.ActiveIdDisabledId = g.HoveredId;
            }
        }
        else if (hovered_window == NULL && g.NavWindow != NULL)
        {
            // Clicking on void disable focus
            FocusWindow(NULL, ImGuiFocusRequestFlags_UnlessBelowModal);
        }
    }

    // With right mouse button we close popups without changing focus based on where the mouse is aimed
    // Instead, focus will be restored to the window under the bottom-most closed popup.
    // (The left mouse button path calls FocusWindow on the hovered window, which will lead NewFrame->ClosePopupsOverWindow to trigger)
    if (g.IO.MouseClicked[1] && g.HoveredId == 0)
    {
        // Find the top-most window between HoveredWindow and the top-most Modal Window.
        // This is where we can trim the popup stack.
        ImGuiWindow* modal = GetTopMostPopupModal();
        bool hovered_window_above_modal = hovered_window && (modal == NULL || IsWindowAbove(hovered_window, modal));
        ClosePopupsOverWindow(hovered_window_above_modal ? hovered_window : modal, true);
    }
}

// The reason this is exposed in imgui_internal.h is: on touch-based system that don't have hovering, we want to dispatch inputs to the right target (imgui vs imgui+app)
void ImGui::UpdateHoveredWindowAndCaptureFlags(const ImVec2& mouse_pos)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    // FIXME-DPI: This storage was added on 2021/03/31 for test engine, but if we want to multiply WINDOWS_HOVER_PADDING
    // by DpiScale, we need to make this window-agnostic anyhow, maybe need storing inside ImGuiWindow.
    g.WindowsBorderHoverPadding = ImMax(ImMax(g.Style.TouchExtraPadding.x, g.Style.TouchExtraPadding.y), g.Style.WindowBorderHoverPadding);

    // Find the window hovered by mouse:
    // - Child windows can extend beyond the limit of their parent so we need to derive HoveredRootWindow from HoveredWindow.
    // - When moving a window we can skip the search, which also conveniently bypasses the fact that window->WindowRectClipped is lagging as this point of the frame.
    // - We also support the moved window toggling the NoInputs flag after moving has started in order to be able to detect windows below it, which is useful for e.g. docking mechanisms.
    bool clear_hovered_windows = false;
    FindHoveredWindowEx(mouse_pos, false, &g.HoveredWindow, &g.HoveredWindowUnderMovingWindow);
    IM_ASSERT(g.HoveredWindow == NULL || g.HoveredWindow == g.MovingWindow || g.HoveredWindow->Viewport == g.MouseViewport);
    g.HoveredWindowBeforeClear = g.HoveredWindow;

    // Modal windows prevents mouse from hovering behind them.
    ImGuiWindow* modal_window = GetTopMostPopupModal();
    if (modal_window && g.HoveredWindow && !IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, modal_window)) // FIXME-MERGE: RootWindowDockTree ?
        clear_hovered_windows = true;

    // Disabled mouse hovering (we don't currently clear MousePos, we could)
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
        clear_hovered_windows = true;

    // We track click ownership. When clicked outside of a window the click is owned by the application and
    // won't report hovering nor request capture even while dragging over our windows afterward.
    const bool has_open_popup = (g.OpenPopupStack.Size > 0);
    const bool has_open_modal = (modal_window != NULL);
    int mouse_earliest_down = -1;
    bool mouse_any_down = false;
    for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++)
    {
        if (io.MouseClicked[i])
        {
            io.MouseDownOwned[i] = (g.HoveredWindow != NULL) || has_open_popup;
            io.MouseDownOwnedUnlessPopupClose[i] = (g.HoveredWindow != NULL) || has_open_modal;
        }
        mouse_any_down |= io.MouseDown[i];
        if (io.MouseDown[i] || io.MouseReleased[i]) // Increase release frame for our evaluation of earliest button (#1392)
            if (mouse_earliest_down == -1 || io.MouseClickedTime[i] < io.MouseClickedTime[mouse_earliest_down])
                mouse_earliest_down = i;
    }
    const bool mouse_avail = (mouse_earliest_down == -1) || io.MouseDownOwned[mouse_earliest_down];
    const bool mouse_avail_unless_popup_close = (mouse_earliest_down == -1) || io.MouseDownOwnedUnlessPopupClose[mouse_earliest_down];

    // If mouse was first clicked outside of ImGui bounds we also cancel out hovering.
    // FIXME: For patterns of drag and drop across OS windows, we may need to rework/remove this test (first committed 311c0ca9 on 2015/02)
    const bool mouse_dragging_extern_payload = g.DragDropActive && (g.DragDropSourceFlags & ImGuiDragDropFlags_SourceExtern) != 0;
    if (!mouse_avail && !mouse_dragging_extern_payload)
        clear_hovered_windows = true;

    if (clear_hovered_windows)
        g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;

    // Update io.WantCaptureMouse for the user application (true = dispatch mouse info to Dear ImGui only, false = dispatch mouse to Dear ImGui + underlying app)
    // Update io.WantCaptureMouseAllowPopupClose (experimental) to give a chance for app to react to popup closure with a drag
    if (g.WantCaptureMouseNextFrame != -1)
    {
        io.WantCaptureMouse = io.WantCaptureMouseUnlessPopupClose = (g.WantCaptureMouseNextFrame != 0);
    }
    else
    {
        io.WantCaptureMouse = (mouse_avail && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_popup;
        io.WantCaptureMouseUnlessPopupClose = (mouse_avail_unless_popup_close && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_modal;
    }

    // Update io.WantCaptureKeyboard for the user application (true = dispatch keyboard info to Dear ImGui only, false = dispatch keyboard info to Dear ImGui + underlying app)
    io.WantCaptureKeyboard = false;
    if ((io.ConfigFlags & ImGuiConfigFlags_NoKeyboard) == 0)
    {
        if ((g.ActiveId != 0) || (modal_window != NULL))
            io.WantCaptureKeyboard = true;
        else if (io.NavActive && (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) && io.ConfigNavCaptureKeyboard)
            io.WantCaptureKeyboard = true;
    }
    if (g.WantCaptureKeyboardNextFrame != -1) // Manual override
        io.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != 0);

    // Update io.WantTextInput flag, this is to allow systems without a keyboard (e.g. mobile, hand-held) to show a software keyboard if possible
    io.WantTextInput = (g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : false;
}

// Called once a frame. Followed by SetCurrentFont() which sets up the remaining data.
// FIXME-VIEWPORT: the concept of a single ClipRectFullscreen is not ideal!
static void SetupDrawListSharedData()
{
    ImGuiContext& g = *GImGui;
    ImRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (ImGuiViewportP* viewport : g.Viewports)
        virtual_space.Add(viewport->GetMainRect());
    g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
    g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
    g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
    g.DrawListSharedData.InitialFlags = ImDrawListFlags_None;
    if (g.Style.AntiAliasedLines)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLines;
    if (g.Style.AntiAliasedLinesUseTex && !(g.IO.Fonts->Flags & ImFontAtlasFlags_NoBakedLines))
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
    if (g.Style.AntiAliasedFill)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedFill;
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AllowVtxOffset;
    g.DrawListSharedData.InitialFringeScale = 1.0f; // FIXME-DPI: Change this for some DPI scaling experiments.
}

void ImGui::NewFrame()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    ImGuiContext& g = *GImGui;

    // Remove pending delete hooks before frame start.
    // This deferred removal avoid issues of removal while iterating the hook vector
    for (int n = g.Hooks.Size - 1; n >= 0; n--)
        if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
            g.Hooks.erase(&g.Hooks[n]);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

    // Check and assert for various common IO and Configuration mistakes
    g.ConfigFlagsLastFrame = g.ConfigFlagsCurrFrame;
    ErrorCheckNewFrameSanityChecks();
    g.ConfigFlagsCurrFrame = g.IO.ConfigFlags;

    // Load settings on first frame, save settings when modified (after a delay)
    UpdateSettings();

    g.Time += g.IO.DeltaTime;
    g.FrameCount += 1;
    g.TooltipOverrideCount = 0;
    g.WindowsActiveCount = 0;
    g.MenusIdSubmittedThisFrame.resize(0);

    // Calculate frame-rate for the user, as a purely luxurious feature
    g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
    g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
    g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_COUNTOF(g.FramerateSecPerFrame);
    g.FramerateSecPerFrameCount = ImMin(g.FramerateSecPerFrameCount + 1, IM_COUNTOF(g.FramerateSecPerFrame));
    g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f) ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount)) : FLT_MAX;

    // Process input queue (trickle as many events as possible), turn events into writes to IO structure
    g.InputEventsTrail.resize(0);
    UpdateInputEvents(g.IO.ConfigInputTrickleEventQueue);

    // Update viewports (after processing input queue, so io.MouseHoveredViewport is set)
    UpdateViewportsNewFrame();

    // Update texture list (collect destroyed textures, etc.)
    UpdateTexturesNewFrame();

    // Setup current font and draw list shared data
    SetupDrawListSharedData();
    UpdateFontsNewFrame();

    g.WithinFrameScope = true;

    // Mark rendering data as invalid to prevent user who may have a handle on it to use it.
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        viewport->DrawData = NULL;
        viewport->DrawDataP.Valid = false;
    }

    // Drag and drop keep the source ID alive so even if the source disappear our state is consistent
    if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
        KeepAliveID(g.DragDropPayload.SourceId);

    // [DEBUG]
    if (!g.IO.ConfigDebugHighlightIdConflicts || !g.IO.KeyCtrl) // Count is locked while holding Ctrl
        g.DebugDrawIdConflictsId = 0;
    if (g.IO.ConfigDebugHighlightIdConflicts && g.HoveredIdPreviousFrameItemCount > 1)
        g.DebugDrawIdConflictsId = g.HoveredIdPreviousFrame;

    // Update HoveredId data
    if (!g.HoveredIdPreviousFrame)
        g.HoveredIdTimer = 0.0f;
    if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
        g.HoveredIdNotActiveTimer = 0.0f;
    if (g.HoveredId)
        g.HoveredIdTimer += g.IO.DeltaTime;
    if (g.HoveredId && g.ActiveId != g.HoveredId)
        g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
    g.HoveredIdPreviousFrame = g.HoveredId;
    g.HoveredIdPreviousFrameItemCount = 0;
    g.HoveredId = 0;
    g.HoveredIdAllowOverlap = false;
    g.HoveredIdIsDisabled = false;

    // Clear ActiveID if the item is not alive anymore.
    // In 1.87, the common most call to KeepAliveID() was moved from GetID() to ItemAdd().
    // As a result, custom widget using ButtonBehavior() _without_ ItemAdd() need to call KeepAliveID() themselves.
    if (g.ActiveId != 0 && g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId)
    {
        IMGUI_DEBUG_LOG_ACTIVEID("NewFrame(): ClearActiveID() 0x%08X because it isn't marked alive anymore!\n", g.ActiveId);
        ClearActiveID();
    }

    // Update ActiveId data (clear reference to active widget if the widget isn't alive anymore)
    if (g.ActiveId)
        g.ActiveIdTimer += g.IO.DeltaTime;
    g.LastActiveIdTimer += g.IO.DeltaTime;
    g.ActiveIdPreviousFrame = g.ActiveId;
    g.ActiveIdIsAlive = 0;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdIsJustActivated = false;
    if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
        g.TempInputId = 0;
    if (g.InputTextReactivateId != 0 && g.InputTextReactivateId != g.DeactivatedItemData.ID)
        g.InputTextReactivateId = 0;
    if (g.ActiveId == 0)
    {
        g.ActiveIdUsingNavDirMask = 0x00;
        g.ActiveIdUsingAllKeyboardKeys = false;
    }
    if (g.DeactivatedItemData.ElapseFrame < g.FrameCount)
        g.DeactivatedItemData.ID = 0;
    g.DeactivatedItemData.IsAlive = false;

    // Record when we have been stationary as this state is preserved while over same item.
    // FIXME: The way this is expressed means user cannot alter HoverStationaryDelay during the frame to use varying values.
    // To allow this we should store HoverItemMaxStationaryTime+ID and perform the >= check in IsItemHovered() function.
    if (g.HoverItemDelayId != 0 && g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
        g.HoverItemUnlockedStationaryId = g.HoverItemDelayId;
    else if (g.HoverItemDelayId == 0)
        g.HoverItemUnlockedStationaryId = 0;
    if (g.HoveredWindow != NULL && g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
        g.HoverWindowUnlockedStationaryId = g.HoveredWindow->ID;
    else if (g.HoveredWindow == NULL)
        g.HoverWindowUnlockedStationaryId = 0;

    // Update hover delay for IsItemHovered() with delays and tooltips
    g.HoverItemDelayIdPreviousFrame = g.HoverItemDelayId;
    if (g.HoverItemDelayId != 0)
    {
        g.HoverItemDelayTimer += g.IO.DeltaTime;
        g.HoverItemDelayClearTimer = 0.0f;
        g.HoverItemDelayId = 0;
    }
    else if (g.HoverItemDelayTimer > 0.0f)
    {
        // This gives a little bit of leeway before clearing the hover timer, allowing mouse to cross gaps
        // We could expose 0.25f as style.HoverClearDelay but I am not sure of the logic yet, this is particularly subtle.
        g.HoverItemDelayClearTimer += g.IO.DeltaTime;
        if (g.HoverItemDelayClearTimer >= ImMax(0.25f, g.IO.DeltaTime * 2.0f)) // ~7 frames at 30 Hz + allow for low framerate
            g.HoverItemDelayTimer = g.HoverItemDelayClearTimer = 0.0f; // May want a decaying timer, in which case need to clamp at max first, based on max of caller last requested timer.
    }

    // Close popups on focus lost (currently wip/opt-in)
    //if (g.IO.AppFocusLost)
    //    ClosePopupsExceptModals();

    // Update keyboard input state
    UpdateKeyboardInputs();

    //IM_ASSERT(g.IO.KeyCtrl == IsKeyDown(ImGuiKey_LeftCtrl) || IsKeyDown(ImGuiKey_RightCtrl));
    //IM_ASSERT(g.IO.KeyShift == IsKeyDown(ImGuiKey_LeftShift) || IsKeyDown(ImGuiKey_RightShift));
    //IM_ASSERT(g.IO.KeyAlt == IsKeyDown(ImGuiKey_LeftAlt) || IsKeyDown(ImGuiKey_RightAlt));
    //IM_ASSERT(g.IO.KeySuper == IsKeyDown(ImGuiKey_LeftSuper) || IsKeyDown(ImGuiKey_RightSuper));

    // Drag and drop
    g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
    g.DragDropAcceptIdCurr = 0;
    g.DragDropAcceptFlagsPrev = g.DragDropAcceptFlagsCurr;
    g.DragDropAcceptFlagsCurr = ImGuiDragDropFlags_None;
    g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    g.DragDropWithinSource = false;
    g.DragDropWithinTarget = false;
    g.DragDropHoldJustPressedId = 0;
    if (g.DragDropActive)
    {
        // Also works when g.ActiveId==0 (aka leftover payload in progress, no active id)
        // You may disable this externally by hijacking the input route:
        //  'if (GetDragDropPayload() != NULL) { Shortcut(ImGuiKey_Escape, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverActive); }
        // but you will not get a return value from Shortcut() due to ActiveIdUsingAllKeyboardKeys logic. You can however poll IsKeyPressed(ImGuiKey_Escape) afterwards.
        ImGuiID owner_id = g.ActiveId ? g.ActiveId : ImHashStr("##DragDropCancelHandler");
        if (Shortcut(ImGuiKey_Escape, ImGuiInputFlags_RouteGlobal, owner_id))
        {
            ClearActiveID();
            ClearDragDrop();
        }
    }
    g.TooltipPreviousWindow = NULL;

    // Update keyboard/gamepad navigation
    NavUpdate();

    // Update mouse input state
    UpdateMouseInputs();

    // Undocking
    // (needs to be before UpdateMouseMovingWindowNewFrame so the window is already offset and following the mouse on the detaching frame)
    DockContextNewFrameUpdateUndocking(&g);

    // Mark all windows as not visible and compact unused memory.
    IM_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
    const bool gc_all = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f);
    const float memory_compact_start_time = gc_all ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
    for (ImGuiWindow* window : g.Windows)
    {
        window->WasActive = window->Active;
        window->Active = false;
        window->WriteAccessed = false;
        window->BeginCountPreviousFrame = window->BeginCount;
        window->BeginCount = 0;

        // Garbage collect transient buffers of recently unused windows
        if ((!window->WasActive || gc_all) && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
            GcCompactTransientWindowBuffers(window);
    }

    // Find hovered window
    // (needs to be before UpdateMouseMovingWindowNewFrame so we fill g.HoveredWindowUnderMovingWindow on the mouse release frame)
    // (currently needs to be done after the WasActive=Active loop and FindHoveredWindowEx uses ->Active)
    UpdateHoveredWindowAndCaptureFlags(g.IO.MousePos);

    // Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering)
    UpdateMouseMovingWindowNewFrame();

    // Background darkening/whitening
    if (GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
        g.DimBgRatio = ImMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
    else
        g.DimBgRatio = ImMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

    g.MouseCursor = ImGuiMouseCursor_Arrow;
    g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;

    // Platform IME data: reset for the frame
    g.PlatformImeDataPrev = g.PlatformImeData;
    g.PlatformImeData.WantVisible = g.PlatformImeData.WantTextInput = false;

    // Mouse wheel scrolling, scale
    UpdateMouseWheel();

    // Garbage collect transient buffers of recently unused tables
    for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
        if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
            TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
    for (ImGuiTableTempData& table_temp_data : g.TablesTempData)
        if (table_temp_data.LastTimeActive >= 0.0f && table_temp_data.LastTimeActive < memory_compact_start_time)
            TableGcCompactTransientBuffers(&table_temp_data);
    if (g.GcCompactAll)
        GcCompactTransientMiscBuffers();
    g.GcCompactAll = false;

    // Closing the focused window restore focus to the first active root window in descending z-order
    if (g.NavWindow && !g.NavWindow->WasActive)
        FocusTopMostWindowUnderOne(NULL, NULL, NULL, ImGuiFocusRequestFlags_RestoreFocusedChild);

    // No window should be open at the beginning of the frame.
    // But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
    g.CurrentWindowStack.resize(0);
    g.BeginPopupStack.resize(0);
    g.ItemFlagsStack.resize(0);
    g.ItemFlagsStack.push_back(ImGuiItemFlags_AutoClosePopups); // Default flags
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    g.GroupStack.resize(0);

    // Docking
    DockContextNewFrameUpdateDocking(&g);

    // [DEBUG] Update debug features
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    UpdateDebugToolItemPicker();
    UpdateDebugToolItemPathQuery();
    UpdateDebugToolFlashStyleColor();
    if (g.DebugLocateFrames > 0 && --g.DebugLocateFrames == 0)
    {
        g.DebugLocateId = 0;
        g.DebugBreakInLocateId = false;
    }
    if (g.DebugLogAutoDisableFrames > 0 && --g.DebugLogAutoDisableFrames == 0)
    {
        DebugLog("(Debug Log: Auto-disabled some ImGuiDebugLogFlags after 2 frames)\n");
        g.DebugLogFlags &= ~g.DebugLogAutoDisableFlags;
        g.DebugLogAutoDisableFlags = ImGuiDebugLogFlags_None;
    }
#endif

    // Create implicit/fallback window - which we will only render it if the user has added something to it.
    // We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
    // This fallback is particularly important as it prevents ImGui:: calls from crashing.
    g.WithinFrameScopeWithImplicitWindow = true;
    SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    Begin("Debug##Default");
    IM_ASSERT(g.CurrentWindow->IsFallbackWindow == true);

    // Store stack sizes
    g.ErrorCountCurrentFrame = 0;
    ErrorRecoveryStoreState(&g.StackSizesInNewFrame);

    // [DEBUG] When io.ConfigDebugBeginReturnValue is set, we make Begin()/BeginChild() return false at different level of the window-stack,
    // allowing to validate correct Begin/End behavior in user code.
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.IO.ConfigDebugBeginReturnValueLoop)
        g.DebugBeginReturnValueCullDepth = (g.DebugBeginReturnValueCullDepth == -1) ? 0 : ((g.DebugBeginReturnValueCullDepth + ((g.FrameCount % 4) == 0 ? 1 : 0)) % 10);
    else
        g.DebugBeginReturnValueCullDepth = -1;
#endif

    CallContextHooks(&g, ImGuiContextHookType_NewFramePost);
}

// FIXME: Add a more explicit sort order in the window structure.
static int IMGUI_CDECL ChildWindowComparer(const void* lhs, const void* rhs)
{
    const ImGuiWindow* const a = *(const ImGuiWindow* const *)lhs;
    const ImGuiWindow* const b = *(const ImGuiWindow* const *)rhs;
    if (int d = (a->Flags & ImGuiWindowFlags_Popup) - (b->Flags & ImGuiWindowFlags_Popup))
        return d;
    if (int d = (a->Flags & ImGuiWindowFlags_Tooltip) - (b->Flags & ImGuiWindowFlags_Tooltip))
        return d;
    return a->BeginOrderWithinParent - b->BeginOrderWithinParent;
}

static void AddWindowToSortBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window)
{
    out_sorted_windows->push_back(window);
    if (window->Active)
    {
        int count = window->DC.ChildWindows.Size;
        ImQsort(window->DC.ChildWindows.Data, (size_t)count, sizeof(ImGuiWindow*), ChildWindowComparer);
        for (int i = 0; i < count; i++)
        {
            ImGuiWindow* child = window->DC.ChildWindows[i];
            if (child->Active)
                AddWindowToSortBuffer(out_sorted_windows, child);
        }
    }
}

static void AddWindowToDrawData(ImGuiWindow* window, int layer)
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = window->Viewport;
    IM_ASSERT(viewport != NULL);
    g.IO.MetricsRenderWindows++;
    if (window->DrawList->_Splitter._Count > 1)
        window->DrawList->ChannelsMerge(); // Merge if user forgot to merge back. Also required in Docking branch for ImGuiWindowFlags_DockNodeHost windows.
    ImGui::AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[layer], window->DrawList);
    for (ImGuiWindow* child : window->DC.ChildWindows)
        if (IsWindowActiveAndVisible(child)) // Clipped children may have been marked not active
            AddWindowToDrawData(child, layer);
}

static inline int GetWindowDisplayLayer(ImGuiWindow* window)
{
    return (window->Flags & ImGuiWindowFlags_Tooltip) ? 1 : 0;
}

// Layer is locked for the root window, however child windows may use a different viewport (e.g. extruding menu)
static inline void AddRootWindowToDrawData(ImGuiWindow* window)
{
    AddWindowToDrawData(window, GetWindowDisplayLayer(window));
}

static void FlattenDrawDataIntoSingleLayer(ImDrawDataBuilder* builder)
{
    int n = builder->Layers[0]->Size;
    int full_size = n;
    for (int i = 1; i < IM_COUNTOF(builder->Layers); i++)
        full_size += builder->Layers[i]->Size;
    builder->Layers[0]->resize(full_size);
    for (int layer_n = 1; layer_n < IM_COUNTOF(builder->Layers); layer_n++)
    {
        ImVector<ImDrawList*>* layer = builder->Layers[layer_n];
        if (layer->empty())
            continue;
        memcpy(builder->Layers[0]->Data + n, layer->Data, layer->Size * sizeof(ImDrawList*));
        n += layer->Size;
        layer->resize(0);
    }
}

static void InitViewportDrawData(ImGuiViewportP* viewport)
{
    ImGuiIO& io = ImGui::GetIO();
    ImDrawData* draw_data = &viewport->DrawDataP;

    viewport->DrawData = draw_data; // Make publicly accessible
    viewport->DrawDataBuilder.Layers[0] = &draw_data->CmdLists;
    viewport->DrawDataBuilder.Layers[1] = &viewport->DrawDataBuilder.LayerData1;
    viewport->DrawDataBuilder.Layers[0]->resize(0);
    viewport->DrawDataBuilder.Layers[1]->resize(0);

    // When minimized, we report draw_data->DisplaySize as zero to be consistent with non-viewport mode,
    // and to allow applications/backends to easily skip rendering.
    // FIXME: Note that we however do NOT attempt to report "zero drawlist / vertices" into the ImDrawData structure.
    // This is because the work has been done already, and its wasted! We should fix that and add optimizations for
    // it earlier in the pipeline, rather than pretend to hide the data at the end of the pipeline.
    const bool is_minimized = (viewport->Flags & ImGuiViewportFlags_IsMinimized) != 0;

    draw_data->Valid = true;
    draw_data->CmdListsCount = 0;
    draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
    draw_data->DisplayPos = viewport->Pos;
    draw_data->DisplaySize = is_minimized ? ImVec2(0.0f, 0.0f) : viewport->Size;
    draw_data->FramebufferScale = (viewport->FramebufferScale.x != 0.0f) ? viewport->FramebufferScale : io.DisplayFramebufferScale;
    draw_data->OwnerViewport = viewport;
    draw_data->Textures = &ImGui::GetPlatformIO().Textures;
}

// Push a clipping rectangle for both ImGui logic (hit-testing etc.) and low-level ImDrawList rendering.
// - When using this function it is sane to ensure that float are perfectly rounded to integer values,
//   so that e.g. (int)(max.x-min.x) in user's render produce correct result.
// - If the code here changes, may need to update code of functions like NextColumn() and PushColumnClipRect():
//   some frequently called functions which to modify both channels and clipping simultaneously tend to use the
//   more specialized SetWindowClipRectBeforeSetChannel() to avoid extraneous updates of underlying ImDrawCmds.
// - This is analogous to PushFont()/PopFont() in the sense that are a mixing a global stack and a window stack,
//   which in the case of ClipRect is not so problematic but tends to be more restrictive for fonts.
void ImGui::PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void ImGui::PopClipRect()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PopClipRect();
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

static void ImGui::RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    ImGuiViewportP* viewport = window->Viewport;
    ImRect viewport_rect = viewport->GetMainRect();

    // Draw behind window by moving the draw command at the FRONT of the draw list
    {
        // Draw list have been trimmed already, hence the explicit recreation of a draw command if missing.
        // FIXME: This is a little bit complicated, solely to avoid creating/injecting an extra drawlist in drawdata.
        ImDrawList* draw_list = window->RootWindowDockTree->DrawList;
        draw_list->ChannelsMerge();
        if (draw_list->CmdBuffer.Size == 0)
            draw_list->AddDrawCmd();
        draw_list->PushClipRect(viewport_rect.Min - ImVec2(1, 1), viewport_rect.Max + ImVec2(1, 1), false); // FIXME: Need to strictly ensure ImDrawCmd are not merged (ElemCount==6 checks below will verify that)
        ImDrawCmd cmd = draw_list->CmdBuffer.back();
        IM_ASSERT(cmd.ElemCount == 0);
        draw_list->AddRectFilled(viewport_rect.Min, viewport_rect.Max, col);
        cmd = draw_list->CmdBuffer.back();
        draw_list->CmdBuffer.pop_back();
        draw_list->CmdBuffer.push_front(cmd);
        draw_list->AddDrawCmd(); // We need to create a command as CmdBuffer.back().IdxOffset won't be correct if we append to same command.
        draw_list->PopClipRect();
    }

    // Draw over sibling docking nodes in a same docking tree
    if (window->RootWindow->DockIsActive)
    {
        ImDrawList* draw_list = FindFrontMostVisibleChildWindow(window->RootWindowDockTree)->DrawList;
        draw_list->ChannelsMerge();
        if (draw_list->CmdBuffer.Size == 0)
            draw_list->AddDrawCmd();
        draw_list->PushClipRect(viewport_rect.Min, viewport_rect.Max, false);
        RenderRectFilledWithHole(draw_list, window->RootWindowDockTree->Rect(), window->RootWindow->Rect(), col, 0.0f);// window->RootWindowDockTree->WindowRounding);
        draw_list->PopClipRect();
    }
}

ImGuiWindow* ImGui::FindBottomMostVisibleWindowWithinBeginStack(ImGuiWindow* parent_window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* bottom_most_visible_window = parent_window;
    for (int i = FindWindowDisplayIndex(parent_window); i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        if (window->Flags & ImGuiWindowFlags_ChildWindow)
            continue;
        if (!IsWindowWithinBeginStackOf(window, parent_window))
            break;
        if (IsWindowActiveAndVisible(window) && GetWindowDisplayLayer(window) <= GetWindowDisplayLayer(parent_window))
            bottom_most_visible_window = window;
    }
    return bottom_most_visible_window;
}

// Important: AddWindowToDrawData() has not been called yet, meaning DockNodeHost windows needs a DrawList->ChannelsMerge() before usage.
// We call ChannelsMerge() lazily here at it is faster that doing a full iteration of g.Windows[] prior to calling RenderDimmedBackgrounds().
static void ImGui::RenderDimmedBackgrounds()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* modal_window = GetTopMostAndVisiblePopupModal();
    if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
        return;
    const bool dim_bg_for_modal = (modal_window != NULL);
    const bool dim_bg_for_window_list = (g.NavWindowingTargetAnim != NULL && g.NavWindowingTargetAnim->Active);
    if (!dim_bg_for_modal && !dim_bg_for_window_list)
        return;

    ImGuiViewport* viewports_already_dimmed[2] = { NULL, NULL };
    if (dim_bg_for_modal)
    {
        // Draw dimming behind modal or a begin stack child, whichever comes first in draw order.
        ImGuiWindow* dim_behind_window = FindBottomMostVisibleWindowWithinBeginStack(modal_window);
        RenderDimmedBackgroundBehindWindow(dim_behind_window, GetColorU32(modal_window->DC.ModalDimBgColor, g.DimBgRatio));
        viewports_already_dimmed[0] = modal_window->Viewport;
    }
    else if (dim_bg_for_window_list)
    {
        // Draw dimming behind Ctrl+Tab target window and behind Ctrl+Tab UI window
        RenderDimmedBackgroundBehindWindow(g.NavWindowingTargetAnim, GetColorU32(ImGuiCol_NavWindowingDimBg, g.DimBgRatio));
        viewports_already_dimmed[0] = g.NavWindowingTargetAnim->Viewport;
        if (g.NavWindowingListWindow != NULL && g.NavWindowingListWindow->Active && g.NavWindowingListWindow->Viewport && g.NavWindowingListWindow->Viewport != g.NavWindowingTargetAnim->Viewport)
        {
            RenderDimmedBackgroundBehindWindow(g.NavWindowingListWindow, GetColorU32(ImGuiCol_NavWindowingDimBg, g.DimBgRatio));
            viewports_already_dimmed[1] = g.NavWindowingListWindow->Viewport;
        }

        // Draw border around Ctrl+Tab target window
        ImGuiWindow* window = g.NavWindowingTargetAnim;
        ImGuiViewport* viewport = window->Viewport;
        float distance = g.FontSize;
        ImRect bb = window->Rect();
        bb.Expand(distance);
        if (bb.GetWidth() >= viewport->Size.x && bb.GetHeight() >= viewport->Size.y)
            bb.Expand(-distance - 1.0f); // If a window fits the entire viewport, adjust its highlight inward
        window->DrawList->ChannelsMerge();
        if (window->DrawList->CmdBuffer.Size == 0)
            window->DrawList->AddDrawCmd();
        window->DrawList->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size);
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha), window->WindowRounding, 0, 3.0f); // FIXME-DPI
        window->DrawList->PopClipRect();
    }

    // Draw dimming background on _other_ viewports than the ones our windows are in
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        if (viewport == viewports_already_dimmed[0] || viewport == viewports_already_dimmed[1])
            continue;
        if (modal_window && viewport->Window && IsWindowAbove(viewport->Window, modal_window))
            continue;
        ImDrawList* draw_list = GetForegroundDrawList(viewport);
        const ImU32 dim_bg_col = GetColorU32(dim_bg_for_modal ? ImGuiCol_ModalWindowDimBg : ImGuiCol_NavWindowingDimBg, g.DimBgRatio);
        draw_list->AddRectFilled(viewport->Pos, viewport->Pos + viewport->Size, dim_bg_col);
    }
}

// This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
void ImGui::EndFrame()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    // Don't process EndFrame() multiple times.
    if (g.FrameCountEnded == g.FrameCount)
        return;
    IM_ASSERT_USER_ERROR_RET(g.WithinFrameScope, "Forgot to call ImGui::NewFrame()?");

    CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

    // [EXPERIMENTAL] Recover from errors
    if (g.IO.ConfigErrorRecovery)
        ErrorRecoveryTryToRecoverState(&g.StackSizesInNewFrame);
    ErrorCheckEndFrameSanityChecks();
    ErrorCheckEndFrameFinalizeErrorTooltip();

    // Notify Platform/OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
    ImGuiPlatformImeData* ime_data = &g.PlatformImeData;
    if (g.PlatformIO.Platform_SetImeDataFn != NULL && memcmp(ime_data, &g.PlatformImeDataPrev, sizeof(ImGuiPlatformImeData)) != 0)
    {
        ImGuiViewport* viewport = FindViewportByID(ime_data->ViewportId);
        if (viewport == NULL)
            viewport = GetMainViewport();
        IMGUI_DEBUG_LOG_IO("[io] Calling Platform_SetImeDataFn(): WantVisible: %d, InputPos (%.2f,%.2f) for Viewport 0x%08X\n", ime_data->WantVisible, ime_data->InputPos.x, ime_data->InputPos.y, viewport->ID);
        g.PlatformIO.Platform_SetImeDataFn(&g, viewport, ime_data);
    }
    g.WantTextInputNextFrame = ime_data->WantTextInput ? 1 : 0;

    // Hide and unfocus implicit/fallback "Debug" window if it hasn't been used
    g.WithinFrameScopeWithImplicitWindow = false;
    if (g.CurrentWindow && g.CurrentWindow->IsFallbackWindow && g.CurrentWindow->WriteAccessed == false)
    {
        g.CurrentWindow->Active = false;
        if (g.NavWindow && g.NavWindow->RootWindow == g.CurrentWindow)
            FocusWindow(NULL);
    }
    End();

    // Update navigation: Ctrl+Tab, wrap-around requests
    NavEndFrame();

    // Update docking
    DockContextEndFrame(&g);

    SetCurrentViewport(NULL, NULL);

    // Drag and Drop: Elapse payload (if delivered, or if source stops being submitted)
    if (g.DragDropActive)
    {
        bool is_delivered = g.DragDropPayload.Delivery;
        bool is_elapsed = (g.DragDropSourceFrameCount + 1 < g.FrameCount) && ((g.DragDropSourceFlags & ImGuiDragDropFlags_PayloadAutoExpire) || g.DragDropMouseButton == -1 || !IsMouseDown(g.DragDropMouseButton));
        if (is_delivered || is_elapsed)
            ClearDragDrop();
    }

    // Drag and Drop: Fallback for missing source tooltip. This is not ideal but better than nothing.
    // If you want to handle source item disappearing: instead of submitting your description tooltip
    // in the BeginDragDropSource() block of the dragged item, you can submit them from a safe single spot
    // (e.g. end of your item loop, or before EndFrame) by reading payload data.
    // In the typical case, the contents of drag tooltip should be possible to infer solely from payload data.
    if (g.DragDropActive && g.DragDropSourceFrameCount + 1 < g.FrameCount && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    {
        g.DragDropWithinSource = true;
        SetTooltip("...");
        g.DragDropWithinSource = false;
    }

    // End frame
    g.WithinFrameScope = false;
    g.FrameCountEnded = g.FrameCount;
    UpdateFontsEndFrame();

    // Initiate moving window + handle left-click and right-click focus
    UpdateMouseMovingWindowEndFrame();

    // Update user-facing viewport list (g.Viewports -> g.PlatformIO.Viewports after filtering out some)
    UpdateViewportsEndFrame();

    // Sort the window list so that all child windows are after their parent
    // We cannot do that on FocusWindow() because children may not exist yet
    g.WindowsTempSortBuffer.resize(0);
    g.WindowsTempSortBuffer.reserve(g.Windows.Size);
    for (ImGuiWindow* window : g.Windows)
    {
        if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow))       // if a child is active its parent will add it
            continue;
        AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
    }

    // This usually assert if there is a mismatch between the ImGuiWindowFlags_ChildWindow / ParentWindow values and DC.ChildWindows[] in parents, aka we've done something wrong.
    IM_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
    g.Windows.swap(g.WindowsTempSortBuffer);
    g.IO.MetricsActiveWindows = g.WindowsActiveCount;

    UpdateTexturesEndFrame();

    // Unlock font atlas
    for (ImFontAtlas* atlas : g.FontAtlases)
        atlas->Locked = false;

    // Clear Input data for next frame
    g.IO.MousePosPrev = g.IO.MousePos;
    g.IO.AppFocusLost = false;
    g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
    g.IO.InputQueueCharacters.resize(0);

    CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
}

// Prepare the data for rendering so you can call GetDrawData()
// (As with anything within the ImGui:: namespace this doesn't touch your GPU or graphics API at all:
// it is the role of the ImGui_ImplXXXX_RenderDrawData() function provided by the renderer backend)
void ImGui::Render()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded != g.FrameCount)
        EndFrame();
    if (g.FrameCountRendered == g.FrameCount)
        return;
    g.FrameCountRendered = g.FrameCount;

    g.IO.MetricsRenderWindows = 0;
    CallContextHooks(&g, ImGuiContextHookType_RenderPre);

    // Add background ImDrawList (for each active viewport)
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        InitViewportDrawData(viewport);
        if (viewport->BgFgDrawLists[0] != NULL && viewport->BgFgDrawListsLastTimeActive[0] == (float)g.Time)
            AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
    }

    // Draw modal/window whitening backgrounds
    RenderDimmedBackgrounds();

    // Add ImDrawList to render
    ImGuiWindow* windows_to_render_top_most[2];
    windows_to_render_top_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindowDockTree : NULL;
    windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
    for (ImGuiWindow* window : g.Windows)
    {
        IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
        if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
            AddRootWindowToDrawData(window);
    }
    for (int n = 0; n < IM_COUNTOF(windows_to_render_top_most); n++)
        if (windows_to_render_top_most[n] && IsWindowActiveAndVisible(windows_to_render_top_most[n])) // NavWindowingTarget is always temporarily displayed as the top-most window
            AddRootWindowToDrawData(windows_to_render_top_most[n]);

    // Draw software mouse cursor if requested by io.MouseDrawCursor flag
    if (g.IO.MouseDrawCursor && g.MouseCursor != ImGuiMouseCursor_None)
        RenderMouseCursor(g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor, IM_COL32_WHITE, IM_COL32_BLACK, IM_COL32(0, 0, 0, 48));

    // Setup ImDrawData structures for end-user
    g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        FlattenDrawDataIntoSingleLayer(&viewport->DrawDataBuilder);

        // Add foreground ImDrawList (for each active viewport)
        if (viewport->BgFgDrawLists[1] != NULL && viewport->BgFgDrawListsLastTimeActive[1] == (float)g.Time)
            AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

        // We call _PopUnusedDrawCmd() last thing, as RenderDimmedBackgrounds() rely on a valid command being there (especially in docking branch).
        ImDrawData* draw_data = &viewport->DrawDataP;
        IM_ASSERT(draw_data->CmdLists.Size == draw_data->CmdListsCount);
        for (ImDrawList* draw_list : draw_data->CmdLists)
            draw_list->_PopUnusedDrawCmd();

        g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
        g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        for (ImFontAtlas* atlas : g.FontAtlases)
            ImFontAtlasDebugLogTextureRequests(atlas);
#endif

    CallContextHooks(&g, ImGuiContextHookType_RenderPost);
}

// Calculate text size. Text can be multi-line. Optionally ignore text after a ## marker.
// CalcTextSize("") should return ImVec2(0.0f, g.FontSize)
ImVec2 ImGui::CalcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImGuiContext& g = *GImGui;

    const char* text_display_end;
    if (hide_text_after_double_hash)
        text_display_end = FindRenderedTextEnd(text, text_end);      // Hide anything after a '##' string
    else
        text_display_end = text_end;

    ImFont* font = g.Font;
    const float font_size = g.FontSize;
    if (text == text_display_end)
        return ImVec2(0.0f, font_size);
    ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

    // Round
    // FIXME: This has been here since Dec 2015 (7b0bf230) but down the line we want this out.
    // FIXME: Investigate using ceilf or e.g.
    // - https://git.musl-libc.org/cgit/musl/tree/src/math/ceilf.c
    // - https://embarkstudios.github.io/rust-gpu/api/src/libm/math/ceilf.rs.html
    text_size.x = IM_TRUNC(text_size.x + 0.99999f);

    return text_size;
}

// Find window given position, search front-to-back
// - Typically write output back to g.HoveredWindow and g.HoveredWindowUnderMovingWindow.
// - FIXME: Note that we have an inconsequential lag here: OuterRectClipped is updated in Begin(), so windows moved programmatically
//   with SetWindowPos() and not SetNextWindowPos() will have that rectangle lagging by a frame at the time FindHoveredWindow() is
//   called, aka before the next Begin(). Moving window isn't affected.
// - The 'find_first_and_in_any_viewport = true' mode is only used by TestEngine. It is simpler to maintain here.
void ImGui::FindHoveredWindowEx(const ImVec2& pos, bool find_first_and_in_any_viewport, ImGuiWindow** out_hovered_window, ImGuiWindow** out_hovered_window_under_moving_window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* hovered_window = NULL;
    ImGuiWindow* hovered_window_under_moving_window = NULL;

    // Special handling for the window being moved: Ignore the mouse viewport check (because it may reset/lose its viewport during the undocking frame)
    ImGuiViewportP* backup_moving_window_viewport = NULL;
    if (find_first_and_in_any_viewport == false && g.MovingWindow)
    {
        backup_moving_window_viewport = g.MovingWindow->Viewport;
        g.MovingWindow->Viewport = g.MouseViewport;
        if (!(g.MovingWindow->Flags & ImGuiWindowFlags_NoMouseInputs))
            hovered_window = g.MovingWindow;
    }

    ImVec2 padding_regular = g.Style.TouchExtraPadding;
    ImVec2 padding_for_resize = ImMax(g.Style.TouchExtraPadding, ImVec2(g.Style.WindowBorderHoverPadding, g.Style.WindowBorderHoverPadding));
    for (int i = g.Windows.Size - 1; i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        IM_MSVC_WARNING_SUPPRESS(28182); // [Static Analyzer] Dereferencing NULL pointer.
        if (!window->WasActive || window->Hidden)
            continue;
        if (window->Flags & ImGuiWindowFlags_NoMouseInputs)
            continue;
        IM_ASSERT(window->Viewport);
        if (window->Viewport != g.MouseViewport)
            continue;

        // Using the clipped AABB, a child window will typically be clipped by its parent (not always)
        ImVec2 hit_padding = (window->Flags & (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) ? padding_regular : padding_for_resize;
        if (!window->OuterRectClipped.ContainsWithPad(pos, hit_padding))
            continue;

        // Support for one rectangular hole in any given window
        // FIXME: Consider generalizing hit-testing override (with more generic data, callback, etc.) (#1512)
        if (window->HitTestHoleSize.x != 0)
        {
            ImVec2 hole_pos(window->Pos.x + (float)window->HitTestHoleOffset.x, window->Pos.y + (float)window->HitTestHoleOffset.y);
            ImVec2 hole_size((float)window->HitTestHoleSize.x, (float)window->HitTestHoleSize.y);
            if (ImRect(hole_pos, hole_pos + hole_size).Contains(pos))
                continue;
        }

        if (find_first_and_in_any_viewport)
        {
            hovered_window = window;
            break;
        }
        else
        {
            if (hovered_window == NULL)
                hovered_window = window;
            IM_MSVC_WARNING_SUPPRESS(28182); // [Static Analyzer] Dereferencing NULL pointer.
            if (hovered_window_under_moving_window == NULL && (!g.MovingWindow || window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree))
                hovered_window_under_moving_window = window;
            if (hovered_window && hovered_window_under_moving_window)
                break;
        }
    }

    *out_hovered_window = hovered_window;
    if (out_hovered_window_under_moving_window != NULL)
        *out_hovered_window_under_moving_window = hovered_window_under_moving_window;
    if (find_first_and_in_any_viewport == false && g.MovingWindow)
        g.MovingWindow->Viewport = backup_moving_window_viewport;
}

bool ImGui::IsItemActive()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
        return g.ActiveId == g.LastItemData.ID;
    return false;
}

bool ImGui::IsItemActivated()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
        if (g.ActiveId == g.LastItemData.ID && g.ActiveIdPreviousFrame != g.LastItemData.ID)
            return true;
    return false;
}

bool ImGui::IsItemDeactivated()
{
    ImGuiContext& g = *GImGui;
    if (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDeactivated)
        return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Deactivated) != 0;
    return g.DeactivatedItemData.ID == g.LastItemData.ID && g.LastItemData.ID != 0 && g.DeactivatedItemData.ElapseFrame >= g.FrameCount;
}

bool ImGui::IsItemDeactivatedAfterEdit()
{
    ImGuiContext& g = *GImGui;
    return IsItemDeactivated() && g.DeactivatedItemData.HasBeenEditedBefore;
}

// == (GetItemID() == GetFocusID() && GetFocusID() != 0)
bool ImGui::IsItemFocused()
{
    ImGuiContext& g = *GImGui;
    if (g.NavId != g.LastItemData.ID || g.NavId == 0)
        return false;

    // Special handling for the dummy item after Begin() which represent the title bar or tab.
    // When the window is collapsed (SkipItems==true) that last item will never be overwritten so we need to detect the case.
    ImGuiWindow* window = g.CurrentWindow;
    if (g.LastItemData.ID == window->ID && window->WriteAccessed)
        return false;

    return true;
}

// Important: this can be useful but it is NOT equivalent to the behavior of e.g.Button()!
// Most widgets have specific reactions based on mouse-up/down state, mouse position etc.
bool ImGui::IsItemClicked(ImGuiMouseButton mouse_button)
{
    return IsMouseClicked(mouse_button) && IsItemHovered(ImGuiHoveredFlags_None);
}

bool ImGui::IsItemToggledOpen()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledOpen) ? true : false;
}

// Call after a Selectable() or TreeNode() involved in multi-selection.
// Useful if you need the per-item information before reaching EndMultiSelect(), e.g. for rendering purpose.
// This is only meant to be called inside a BeginMultiSelect()/EndMultiSelect() block.
// (Outside of multi-select, it would be misleading/ambiguous to report this signal, as widgets
// return e.g. a pressed event and user code is in charge of altering selection in ways we cannot predict.)
bool ImGui::IsItemToggledSelection()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentMultiSelect != NULL); // Can only be used inside a BeginMultiSelect()/EndMultiSelect()
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledSelection) ? true : false;
}

// IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear ImGui or to your underlying app,
// you should not use this function! Use the 'io.WantCaptureMouse' boolean for that!
// Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?" for details.
bool ImGui::IsAnyItemHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredId != 0 || g.HoveredIdPreviousFrame != 0;
}

bool ImGui::IsAnyItemActive()
{
    ImGuiContext& g = *GImGui;
    return g.ActiveId != 0;
}

bool ImGui::IsAnyItemFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavId != 0 && g.NavCursorVisible;
}

bool ImGui::IsItemVisible()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Visible) != 0;
}

bool ImGui::IsItemEdited()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Edited) != 0;
}

// Allow next item to be overlapped by subsequent items.
// This works by requiring HoveredId to match for two subsequent frames,
// so if a following items overwrite it our interactions will naturally be disabled.
void ImGui::SetNextItemAllowOverlap()
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.ItemFlags |= ImGuiItemFlags_AllowOverlap;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
// Allow last item to be overlapped by a subsequent item. Both may be activated during the same frame before the later one takes priority.
// Use SetNextItemAllowOverlap() *before* your item instead of calling this!
//void ImGui::SetItemAllowOverlap()
//{
//    ImGuiContext& g = *GImGui;
//    ImGuiID id = g.LastItemData.ID;
//    if (g.HoveredId == id)
//        g.HoveredIdAllowOverlap = true;
//    if (g.ActiveId == id) // Before we made this obsolete, most calls to SetItemAllowOverlap() used to avoid this path by testing g.ActiveId != id.
//        g.ActiveIdAllowOverlap = true;
//}
#endif

// This is a shortcut for not taking ownership of 100+ keys, frequently used by drag operations.
// FIXME: It might be undesirable that this will likely disable KeyOwner-aware shortcuts systems. Consider a more fine-tuned version if needed?
void ImGui::SetActiveIdUsingAllKeyboardKeys()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.ActiveId != 0);
    g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_COUNT) - 1;
    g.ActiveIdUsingAllKeyboardKeys = true;
    NavMoveRequestCancel();
}

ImGuiID ImGui::GetItemID()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.ID;
}

ImVec2 ImGui::GetItemRectMin()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.Min;
}

ImVec2 ImGui::GetItemRectMax()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.Max;
}

ImVec2 ImGui::GetItemRectSize()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.GetSize();
}

ImGuiItemFlags ImGui::GetItemFlags()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.ItemFlags;
}

// Prior to v1.90 2023/10/16, the BeginChild() function took a 'bool border = false' parameter instead of 'ImGuiChildFlags child_flags = 0'.
// ImGuiChildFlags_Borders is defined as always == 1 in order to allow old code passing 'true'. Read comments in imgui.h for details!
bool ImGui::BeginChild(const char* str_id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGuiID id = GetCurrentWindow()->GetID(str_id);
    return BeginChildEx(str_id, id, size_arg, child_flags, window_flags);
}

bool ImGui::BeginChild(ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    return BeginChildEx(NULL, id, size_arg, child_flags, window_flags);
}

bool ImGui::BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    IM_ASSERT(id != 0);

    // Sanity check as it is likely that some user will accidentally pass ImGuiWindowFlags into the ImGuiChildFlags argument.
    const ImGuiChildFlags ImGuiChildFlags_SupportedMask_ = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_FrameStyle | ImGuiChildFlags_NavFlattened;
    IM_UNUSED(ImGuiChildFlags_SupportedMask_);
    IM_ASSERT((child_flags & ~ImGuiChildFlags_SupportedMask_) == 0 && "Illegal ImGuiChildFlags value. Did you pass ImGuiWindowFlags values instead of ImGuiChildFlags?");
    IM_ASSERT((window_flags & ImGuiWindowFlags_AlwaysAutoResize) == 0 && "Cannot specify ImGuiWindowFlags_AlwaysAutoResize for BeginChild(). Use ImGuiChildFlags_AlwaysAutoResize!");
    if (child_flags & ImGuiChildFlags_AlwaysAutoResize)
    {
        IM_ASSERT((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0 && "Cannot use ImGuiChildFlags_ResizeX or ImGuiChildFlags_ResizeY with ImGuiChildFlags_AlwaysAutoResize!");
        IM_ASSERT((child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) != 0 && "Must use ImGuiChildFlags_AutoResizeX or ImGuiChildFlags_AutoResizeY with ImGuiChildFlags_AlwaysAutoResize!");
    }
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    //if (window_flags & ImGuiWindowFlags_AlwaysUseWindowPadding) { child_flags |= ImGuiChildFlags_AlwaysUseWindowPadding; }
    //if (window_flags & ImGuiWindowFlags_NavFlattened) { child_flags |= ImGuiChildFlags_NavFlattened; }
#endif
    if (child_flags & ImGuiChildFlags_AutoResizeX)
        child_flags &= ~ImGuiChildFlags_ResizeX;
    if (child_flags & ImGuiChildFlags_AutoResizeY)
        child_flags &= ~ImGuiChildFlags_ResizeY;

    // Set window flags
    window_flags |= ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking;
    window_flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove); // Inherit the NoMove flag
    if (child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize))
        window_flags |= ImGuiWindowFlags_AlwaysAutoResize; // FIXME: Would be sane to not make single-axis flag set this. (#9355)
    if ((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    // Special framed style
    if (child_flags & ImGuiChildFlags_FrameStyle)
    {
        PushStyleColor(ImGuiCol_ChildBg, g.Style.Colors[ImGuiCol_FrameBg]);
        PushStyleVar(ImGuiStyleVar_ChildRounding, g.Style.FrameRounding);
        PushStyleVar(ImGuiStyleVar_ChildBorderSize, g.Style.FrameBorderSize);
        PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.FramePadding);
        child_flags |= ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
        window_flags |= ImGuiWindowFlags_NoMove;
    }

    // Forward size
    // Important: Begin() has special processing to switch condition to ImGuiCond_FirstUseEver for a given axis when ImGuiChildFlags_ResizeXXX is set.
    // (the alternative would to store conditional flags per axis, which is possible but more code)
    const ImVec2 size_avail = GetContentRegionAvail();
    const ImVec2 size_default((child_flags & ImGuiChildFlags_AutoResizeX) ? 0.0f : size_avail.x, (child_flags & ImGuiChildFlags_AutoResizeY) ? 0.0f : size_avail.y);
    ImVec2 size = CalcItemSize(size_arg, size_default.x, size_default.y);

    // A SetNextWindowSize() call always has priority (#8020)
    // (since the code in Begin() never supported SizeVal==0.0f aka auto-resize via SetNextWindowSize() call, we don't support it here for now)
    // FIXME: We only support ImGuiCond_Always in this path. Supporting other paths would requires to obtain window pointer.
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) != 0 && (g.NextWindowData.SizeCond & ImGuiCond_Always) != 0)
    {
        if (g.NextWindowData.SizeVal.x > 0.0f)
        {
            size.x = g.NextWindowData.SizeVal.x;
            child_flags &= ~ImGuiChildFlags_ResizeX;
        }
        if (g.NextWindowData.SizeVal.y > 0.0f)
        {
            size.y = g.NextWindowData.SizeVal.y;
            child_flags &= ~ImGuiChildFlags_ResizeY;
        }
    }
    SetNextWindowSize(size);

    // Forward child flags (we allow prior settings to merge but it'll only work for adding flags)
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags)
        g.NextWindowData.ChildFlags |= child_flags;
    else
        g.NextWindowData.ChildFlags = child_flags;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasChildFlags;

    // Build up name. If you need to append to a same child from multiple location in the ID stack, use BeginChild(ImGuiID id) with a stable value.
    // FIXME: 2023/11/14: commented out shorted version. We had an issue with multiple ### in child window path names, which the trailing hash helped workaround.
    // e.g. "ParentName###ParentIdentifier/ChildName###ChildIdentifier" would get hashed incorrectly by ImHashStr(), trailing _%08X somehow fixes it.
    const char* temp_window_name;
    /*if (name && parent_window->IDStack.back() == parent_window->ID)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s", parent_window->Name, name); // May omit ID if in root of ID stack
    else*/
    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    // Set style
    const float backup_border_size = g.Style.ChildBorderSize;
    if ((child_flags & ImGuiChildFlags_Borders) == 0)
        g.Style.ChildBorderSize = 0.0f;

    // Begin into window
    const bool ret = Begin(temp_window_name, NULL, window_flags);

    // Restore style
    g.Style.ChildBorderSize = backup_border_size;
    if (child_flags & ImGuiChildFlags_FrameStyle)
    {
        PopStyleVar(3);
        PopStyleColor();
    }

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;

    // Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
    // While this is not really documented/defined, it seems that the expected thing to do.
    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    // Process navigation-in immediately so NavInit can run on first frame
    // Can enter a child if (A) it has navigable items or (B) it can be scrolled.
    const ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
    if (g.ActiveId == temp_id_for_activation)
        ClearActiveID();
    if (g.NavActivateId == id && !(child_flags & ImGuiChildFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(temp_id_for_activation, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
        g.ActiveIdSource = g.NavInputSource;
    }
    return ret;
}

void ImGui::EndChild()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* child_window = g.CurrentWindow;

    const ImGuiID backup_within_end_child_id = g.WithinEndChildID;
    IM_ASSERT(child_window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() calls

    g.WithinEndChildID = child_window->ID;
    ImVec2 child_size = child_window->Size;
    End();
    if (child_window->BeginCount == 1)
    {
        ImGuiWindow* parent_window = g.CurrentWindow;
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + child_size);
        ItemSize(child_size);
        const bool nav_flattened = (child_window->ChildFlags & ImGuiChildFlags_NavFlattened) != 0;
        if ((child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY) && !nav_flattened)
        {
            ItemAdd(bb, child_window->ChildId);
            RenderNavCursor(bb, child_window->ChildId);

            // When browsing a window that has no activable items (scroll only) we keep a highlight on the child (pass g.NavId to trick into always displaying)
            if (child_window->DC.NavLayersActiveMask == 0 && child_window == g.NavWindow)
                RenderNavCursor(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavRenderCursorFlags_Compact);
        }
        else
        {
            // Not navigable into
            // - This is a bit of a fringe use case, mostly useful for undecorated, non-scrolling contents childs, or empty childs.
            // - We could later decide to not apply this path if ImGuiChildFlags_FrameStyle or ImGuiChildFlags_Borders is set.
            ItemAdd(bb, child_window->ChildId, NULL, ImGuiItemFlags_NoNav);

            // But when flattened we directly reach items, adjust active layer mask accordingly
            if (nav_flattened)
                parent_window->DC.NavLayersActiveMaskNext |= child_window->DC.NavLayersActiveMaskNext;
        }
        if (g.HoveredWindow == child_window)
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
        child_window->DC.ChildItemStatusFlags = g.LastItemData.StatusFlags;
        //SetLastItemDataForChildWindowItem(child_window, child_window->Rect()); // Not needed, effectively done by ItemAdd()
    }
    else
    {
        SetLastItemDataForChildWindowItem(child_window, child_window->Rect());
    }

    g.WithinEndChildID = backup_within_end_child_id;
    g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

ImGuiWindow* ImGui::FindFrontMostVisibleChildWindow(ImGuiWindow* window)
{
    for (int n = window->DC.ChildWindows.Size - 1; n >= 0; n--)
        if (IsWindowActiveAndVisible(window->DC.ChildWindows[n]))
            return FindFrontMostVisibleChildWindow(window->DC.ChildWindows[n]);
    return window;
}

static void SetWindowConditionAllowFlags(ImGuiWindow* window, ImGuiCond flags, bool enabled)
{
    window->SetWindowPosAllowFlags       = enabled ? (window->SetWindowPosAllowFlags       | flags) : (window->SetWindowPosAllowFlags       & ~flags);
    window->SetWindowSizeAllowFlags      = enabled ? (window->SetWindowSizeAllowFlags      | flags) : (window->SetWindowSizeAllowFlags      & ~flags);
    window->SetWindowCollapsedAllowFlags = enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
    window->SetWindowDockAllowFlags      = enabled ? (window->SetWindowDockAllowFlags      | flags) : (window->SetWindowDockAllowFlags      & ~flags);
}

ImGuiWindow* ImGui::FindWindowByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    return (ImGuiWindow*)g.WindowsById.GetVoidPtr(id);
}

ImGuiWindow* ImGui::FindWindowByName(const char* name)
{
    ImGuiID id = ImHashStr(name);
    return FindWindowByID(id);
}

static void ApplyWindowSettings(ImGuiWindow* window, ImGuiWindowSettings* settings)
{
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    window->ViewportPos = main_viewport->Pos;
    if (settings->ViewportId)
    {
        window->ViewportId = settings->ViewportId;
        window->ViewportPos = ImVec2(settings->ViewportPos.x, settings->ViewportPos.y);
    }
    window->Pos = ImTrunc(ImVec2(settings->Pos.x + window->ViewportPos.x, settings->Pos.y + window->ViewportPos.y));
    if (settings->Size.x > 0 && settings->Size.y > 0)
        window->Size = window->SizeFull = ImTrunc(ImVec2(settings->Size.x, settings->Size.y));
    window->Collapsed = settings->Collapsed;
    window->DockId = settings->DockId;
    window->DockOrder = settings->DockOrder;
}

static void InitOrLoadWindowSettings(ImGuiWindow* window, ImGuiWindowSettings* settings)
{
    // Initial window state with e.g. default/arbitrary window position
    // Use SetNextWindowPos() with the appropriate condition flag to change the initial position of a window.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    window->Pos = main_viewport->Pos + ImVec2(60, 60);
    window->Size = window->SizeFull = ImVec2(0, 0);
    window->ViewportPos = main_viewport->Pos;
    window->SetWindowPosAllowFlags = window->SetWindowSizeAllowFlags = window->SetWindowCollapsedAllowFlags = window->SetWindowDockAllowFlags = ImGuiCond_Always | ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing;

    if (settings != NULL)
    {
        SetWindowConditionAllowFlags(window, ImGuiCond_FirstUseEver, false);
        ApplyWindowSettings(window, settings);
    }
    window->DC.CursorStartPos = window->DC.CursorMaxPos = window->DC.IdealMaxPos = window->Pos; // So first call to CalcWindowContentSizes() doesn't return crazy values

    if ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
    {
        window->AutoFitFramesX = window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
    else
    {
        if (window->Size.x <= 0.0f)
            window->AutoFitFramesX = 2;
        if (window->Size.y <= 0.0f)
            window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
    }
}

static ImGuiWindow* CreateNewWindow(const char* name, ImGuiWindowFlags flags)
{
    // Create window the first time
    //IMGUI_DEBUG_LOG("CreateNewWindow '%s', flags = 0x%08X\n", name, flags);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = IM_NEW(ImGuiWindow)(&g, name);
    window->Flags = flags;
    g.WindowsById.SetVoidPtr(window->ID, window);

    ImGuiWindowSettings* settings = NULL;
    if (!(flags & ImGuiWindowFlags_NoSavedSettings))
        if ((settings = ImGui::FindWindowSettingsByWindow(window)) != 0)
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);

    InitOrLoadWindowSettings(window, settings);

    if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
        g.Windows.push_front(window); // Quite slow but rare and only once
    else
        g.Windows.push_back(window);

    return window;
}

static ImGuiWindow* GetWindowForTitleAndMenuHeight(ImGuiWindow* window)
{
    return (window->DockNodeAsHost && window->DockNodeAsHost->VisibleWindow) ? window->DockNodeAsHost->VisibleWindow : window;
}

static inline ImVec2 CalcWindowMinSize(ImGuiWindow* window)
{
    // We give windows non-zero minimum size to facilitate understanding problematic cases (e.g. empty popups)
    // FIXME: Essentially we want to restrict manual resizing to WindowMinSize+Decoration, and allow api resizing to be smaller.
    // Perhaps should tend further a neater test for this.
    ImGuiContext& g = *GImGui;
    ImVec2 size_min;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_Popup))
    {
        size_min.x = (window->ChildFlags & ImGuiChildFlags_ResizeX) ? g.Style.WindowMinSize.x : IMGUI_WINDOW_HARD_MIN_SIZE;
        size_min.y = (window->ChildFlags & ImGuiChildFlags_ResizeY) ? g.Style.WindowMinSize.y : IMGUI_WINDOW_HARD_MIN_SIZE;
    }
    else
    {
        size_min.x = ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) == 0) ? g.Style.WindowMinSize.x : IMGUI_WINDOW_HARD_MIN_SIZE;
        size_min.y = ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) == 0) ? g.Style.WindowMinSize.y : IMGUI_WINDOW_HARD_MIN_SIZE;
    }

    // Reduce artifacts with very small windows
    ImGuiWindow* window_for_height = GetWindowForTitleAndMenuHeight(window);
    size_min.y = ImMax(size_min.y, window_for_height->TitleBarHeight + window_for_height->MenuBarHeight + ImMax(0.0f, g.Style.WindowRounding - 1.0f));
    return size_min;
}

static ImVec2 CalcWindowSizeAfterConstraint(ImGuiWindow* window, const ImVec2& size_desired)
{
    ImGuiContext& g = *GImGui;
    ImVec2 new_size = size_desired;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        // See comments in SetNextWindowSizeConstraints() for details about setting size_min an size_max.
        ImRect cr = g.NextWindowData.SizeConstraintRect;
        new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
        new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
        if (g.NextWindowData.SizeCallback)
        {
            ImGuiSizeCallbackData data;
            data.UserData = g.NextWindowData.SizeCallbackUserData;
            data.Pos = window->Pos;
            data.CurrentSize = window->SizeFull;
            data.DesiredSize = new_size;
            g.NextWindowData.SizeCallback(&data);
            new_size = data.DesiredSize;
        }
        new_size.x = IM_TRUNC(new_size.x);
        new_size.y = IM_TRUNC(new_size.y);
    }

    // Minimum size
    ImVec2 size_min = CalcWindowMinSize(window);
    return ImMax(new_size, size_min);
}

static void CalcWindowContentSizes(ImGuiWindow* window, ImVec2* content_size_current, ImVec2* content_size_ideal)
{
    bool preserve_old_content_sizes = false;
    if (window->Collapsed && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
        preserve_old_content_sizes = true;
    else if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 && window->HiddenFramesCanSkipItems > 0)
        preserve_old_content_sizes = true;
    if (preserve_old_content_sizes)
    {
        *content_size_current = window->ContentSize;
        *content_size_ideal = window->ContentSizeIdeal;
        return;
    }

    content_size_current->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : ImTrunc64(window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
    content_size_current->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : ImTrunc64(window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
    content_size_ideal->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : ImTrunc64(ImMax(window->DC.CursorMaxPos.x, window->DC.IdealMaxPos.x) - window->DC.CursorStartPos.x);
    content_size_ideal->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : ImTrunc64(ImMax(window->DC.CursorMaxPos.y, window->DC.IdealMaxPos.y) - window->DC.CursorStartPos.y);
}

static ImVec2 CalcWindowAutoFitSize(ImGuiWindow* window, const ImVec2& size_contents, int axis_mask)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    const float decoration_w_without_scrollbars = window->DecoOuterSizeX1 + window->DecoOuterSizeX2 - window->ScrollbarSizes.x;
    const float decoration_h_without_scrollbars = window->DecoOuterSizeY1 + window->DecoOuterSizeY2 - window->ScrollbarSizes.y;
    ImVec2 size_pad = window->WindowPadding * 2.0f;
    ImVec2 size_desired;
    size_desired.x = (axis_mask & 1) ? size_contents.x + size_pad.x + decoration_w_without_scrollbars : window->Size.x;
    size_desired.y = (axis_mask & 2) ? size_contents.y + size_pad.y + decoration_h_without_scrollbars : window->Size.y;

    // Determine maximum window size
    // Child windows are laid within their parent (unless they are also popups/menus) and thus have no restriction
    ImVec2 size_max = ImVec2(FLT_MAX, FLT_MAX);
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || (window->Flags & ImGuiWindowFlags_Popup) != 0)
    {
        if (!window->ViewportOwned)
            size_max = ImGui::GetMainViewport()->WorkSize - style.DisplaySafeAreaPadding * 2.0f;
        const int monitor_idx = window->ViewportAllowPlatformMonitorExtend;
        if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size)
            size_max = g.PlatformIO.Monitors[monitor_idx].WorkSize - style.DisplaySafeAreaPadding * 2.0f;
    }

    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {
        // Tooltip always resize (up to maximum size)
        return ImMin(size_desired, size_max);
    }
    else
    {
        ImVec2 size_min = CalcWindowMinSize(window);
        ImVec2 size_auto_fit = ImClamp(size_desired, ImMin(size_min, size_max), size_max);

        // When the window cannot fit all contents (either because of constraints, either because screen is too small),
        // we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than ViewportSize-WindowPadding.
        ImVec2 size_auto_fit_after_constraint = CalcWindowSizeAfterConstraint(window, size_auto_fit);
        float size_contents_for_scrollbar_x = (axis_mask & 1) ? size_contents.x : window->ContentSize.x; // See #9352. In theory this should use same logic as `window->ScrollbarY = ...` codepath in Begin(). Needs some plumbling.
        float size_contents_for_scrollbar_y = (axis_mask & 2) ? size_contents.y : window->ContentSize.y;
        bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x < size_contents_for_scrollbar_x + size_pad.x + decoration_w_without_scrollbars && !(window->Flags & ImGuiWindowFlags_NoScrollbar) && (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        bool will_have_scrollbar_y = (size_auto_fit_after_constraint.y < size_contents_for_scrollbar_y + size_pad.y + decoration_h_without_scrollbars && !(window->Flags & ImGuiWindowFlags_NoScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);
        if (will_have_scrollbar_x)
            size_auto_fit.y += style.ScrollbarSize;
        if (will_have_scrollbar_y)
            size_auto_fit.x += style.ScrollbarSize;
        return size_auto_fit;
    }
}

ImVec2 ImGui::CalcWindowNextAutoFitSize(ImGuiWindow* window)
{
    ImVec2 size_contents_current;
    ImVec2 size_contents_ideal;
    CalcWindowContentSizes(window, &size_contents_current, &size_contents_ideal);
    ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, size_contents_ideal, ~0);
    ImVec2 size_final = CalcWindowSizeAfterConstraint(window, size_auto_fit);
    return size_final;
}

static ImGuiCol GetWindowBgColorIdx(ImGuiWindow* window)
{
    if (window->Flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
        return ImGuiCol_PopupBg;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !window->DockIsActive)
        return ImGuiCol_ChildBg;
    return ImGuiCol_WindowBg;
}

static void CalcResizePosSizeFromAnyCorner(ImGuiWindow* window, const ImVec2& corner_target_arg, const ImVec2& corner_norm, ImVec2* out_pos, ImVec2* out_size)
{
    ImVec2 corner_target = corner_target_arg;
    if (window->Flags & ImGuiWindowFlags_ChildWindow) // Clamp resizing of childs within parent
    {
        ImGuiWindow* parent_window = window->ParentWindow;
        ImGuiWindowFlags parent_flags = parent_window->Flags;
        ImRect limit_rect = parent_window->InnerRect;
        limit_rect.Expand(ImVec2(-ImMax(parent_window->WindowPadding.x, parent_window->WindowBorderSize), -ImMax(parent_window->WindowPadding.y, parent_window->WindowBorderSize)));
        if ((parent_flags & (ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar)) == 0 || (parent_flags & ImGuiWindowFlags_NoScrollbar))
            corner_target.x = ImClamp(corner_target.x, limit_rect.Min.x, limit_rect.Max.x);
        if (parent_flags & ImGuiWindowFlags_NoScrollbar)
            corner_target.y = ImClamp(corner_target.y, limit_rect.Min.y, limit_rect.Max.y);
    }
    ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
    ImVec2 pos_max = ImLerp(window->Pos + window->Size, corner_target, corner_norm); // Expected window lower-right
    ImVec2 size_expected = pos_max - pos_min;
    ImVec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
    *out_pos = pos_min;
    if (corner_norm.x == 0.0f)
        out_pos->x -= (size_constrained.x - size_expected.x);
    if (corner_norm.y == 0.0f)
        out_pos->y -= (size_constrained.y - size_expected.y);
    *out_size = size_constrained;
}

// Data for resizing from resize grip / corner
struct ImGuiResizeGripDef
{
    ImVec2  CornerPosN;
    ImVec2  InnerDir;
    int     AngleMin12, AngleMax12;
};
static const ImGuiResizeGripDef resize_grip_def[4] =
{
    { ImVec2(1, 1), ImVec2(-1, -1), 0, 3 },  // Lower-right
    { ImVec2(0, 1), ImVec2(+1, -1), 3, 6 },  // Lower-left
    { ImVec2(0, 0), ImVec2(+1, +1), 6, 9 },  // Upper-left (Unused)
    { ImVec2(1, 0), ImVec2(-1, +1), 9, 12 }  // Upper-right (Unused)
};

// Data for resizing from borders
struct ImGuiResizeBorderDef
{
    ImVec2  InnerDir;               // Normal toward inside
    ImVec2  SegmentN1, SegmentN2;   // End positions, normalized (0,0: upper left)
    float   OuterAngle;             // Angle toward outside
};
static const ImGuiResizeBorderDef resize_border_def[4] =
{
    { ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
    { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
    { ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
    { ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
};

static ImRect GetResizeBorderRect(ImGuiWindow* window, int border_n, float perp_padding, float thickness)
{
    ImRect rect = window->Rect();
    if (thickness == 0.0f)
        rect.Max -= ImVec2(1, 1);
    if (border_n == ImGuiDir_Left)  { return ImRect(rect.Min.x - thickness,    rect.Min.y + perp_padding, rect.Min.x + thickness,    rect.Max.y - perp_padding); }
    if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness,    rect.Min.y + perp_padding, rect.Max.x + thickness,    rect.Max.y - perp_padding); }
    if (border_n == ImGuiDir_Up)    { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness,    rect.Max.x - perp_padding, rect.Min.y + thickness);    }
    if (border_n == ImGuiDir_Down)  { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness,    rect.Max.x - perp_padding, rect.Max.y + thickness);    }
    IM_ASSERT(0);
    return ImRect();
}

// 0..3: corners (Lower-right, Lower-left, Unused, Unused)
ImGuiID ImGui::GetWindowResizeCornerID(ImGuiWindow* window, int n)
{
    IM_ASSERT(n >= 0 && n < 4);
    ImGuiID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
    id = ImHashStr("#RESIZE", 0, id);
    id = ImHashData(&n, sizeof(int), id);
    return id;
}

// Borders (Left, Right, Up, Down)
ImGuiID ImGui::GetWindowResizeBorderID(ImGuiWindow* window, ImGuiDir dir)
{
    IM_ASSERT(dir >= 0 && dir < 4);
    int n = (int)dir + 4;
    ImGuiID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
    id = ImHashStr("#RESIZE", 0, id);
    id = ImHashData(&n, sizeof(int), id);
    return id;
}

// Handle resize for: Resize Grips, Borders, Gamepad
// Return true when using auto-fit (double-click on resize grip)
static int ImGui::UpdateWindowManualResize(ImGuiWindow* window, int* border_hovered, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindowFlags flags = window->Flags;

    if ((flags & ImGuiWindowFlags_NoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
        return false;
    if ((flags & ImGuiWindowFlags_AlwaysAutoResize) && (window->ChildFlags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
        return false;
    if (window->WasActive == false) // Early out to avoid running this code for e.g. a hidden implicit/fallback Debug window.
        return false;

    int ret_auto_fit_mask = 0x00;
    const float grip_draw_size = IM_TRUNC(ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
    const float grip_hover_inner_size = (resize_grip_count > 0) ? IM_TRUNC(grip_draw_size * 0.75f) : 0.0f;
    const float grip_hover_outer_size = g.WindowsBorderHoverPadding;

    ImRect clamp_rect = visibility_rect;
    const bool window_move_from_title_bar = !(window->BgClickFlags & ImGuiWindowBgClickFlags_Move) && !(window->Flags & ImGuiWindowFlags_NoTitleBar);
    if (window_move_from_title_bar)
        clamp_rect.Min.y -= window->TitleBarHeight;

    ImVec2 pos_target(FLT_MAX, FLT_MAX);
    ImVec2 size_target(FLT_MAX, FLT_MAX);

    // Clip mouse interaction rectangles within the viewport rectangle (in practice the narrowing is going to happen most of the time).
    // - Not narrowing would mostly benefit the situation where OS windows _without_ decoration have a threshold for hovering when outside their limits.
    //   This is however not the case with current backends under Win32, but a custom borderless window implementation would benefit from it.
    // - When decoration are enabled we typically benefit from that distance, but then our resize elements would be conflicting with OS resize elements, so we also narrow.
    // - Note that we are unable to tell if the platform setup allows hovering with a distance threshold (on Win32, decorated window have such threshold).
    // We only clip interaction so we overwrite window->ClipRect, cannot call PushClipRect() yet as DrawList is not yet setup.
    const bool clip_with_viewport_rect = !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport) || (g.IO.MouseHoveredViewport != window->ViewportId) || !(window->Viewport->Flags & ImGuiViewportFlags_NoDecoration);
    if (clip_with_viewport_rect)
        window->ClipRect = window->Viewport->GetMainRect();

    // Resize grips and borders are on layer 1
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Manual resize grips
    PushID("#RESIZE");
    for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
    {
        const ImGuiResizeGripDef& def = resize_grip_def[resize_grip_n];
        const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

        // Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
        bool hovered, held;
        ImRect resize_rect(corner - def.InnerDir * grip_hover_outer_size, corner + def.InnerDir * grip_hover_inner_size);
        if (resize_rect.Min.x > resize_rect.Max.x) ImSwap(resize_rect.Min.x, resize_rect.Max.x);
        if (resize_rect.Min.y > resize_rect.Max.y) ImSwap(resize_rect.Min.y, resize_rect.Max.y);
        ImGuiID resize_grip_id = window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
        ItemAdd(resize_rect, resize_grip_id, NULL, ImGuiItemFlags_NoNav);
        ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
        //GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, IM_COL32(255, 255, 0, 255));
        if (hovered || held)
            SetMouseCursor((resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE);

        if (held && g.IO.MouseDoubleClicked[0])
        {
            // Auto-fit when double-clicking
            ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, ~0);
            size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
            ret_auto_fit_mask = 0x03; // Both axes
            ClearActiveID();
        }
        else if (held)
        {
            // Resize from any of the four corners
            // We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
            ImVec2 clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? clamp_rect.Min.x : -FLT_MAX, (def.CornerPosN.y == 1.0f || (def.CornerPosN.y == 0.0f && window_move_from_title_bar)) ? clamp_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? clamp_rect.Max.x : +FLT_MAX, def.CornerPosN.y == 0.0f ? clamp_rect.Max.y : +FLT_MAX);
            ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset + ImLerp(def.InnerDir * grip_hover_outer_size, def.InnerDir * -grip_hover_inner_size, def.CornerPosN); // Corner of the window corresponding to our corner grip
            corner_target = ImClamp(corner_target, clamp_min, clamp_max);
            CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
        }

        // Only lower-left grip is visible before hovering/activating
        const bool resize_grip_visible = held || hovered || (resize_grip_n == 0 && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0);
        if (resize_grip_visible)
            resize_grip_col[resize_grip_n] = GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
    }

    int resize_border_mask = 0x00;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        resize_border_mask |= ((window->ChildFlags & ImGuiChildFlags_ResizeX) ? 0x02 : 0) | ((window->ChildFlags & ImGuiChildFlags_ResizeY) ? 0x08 : 0);
    else
        resize_border_mask = g.IO.ConfigWindowsResizeFromEdges ? 0x0F : 0x00;
    for (int border_n = 0; border_n < 4; border_n++)
    {
        if ((resize_border_mask & (1 << border_n)) == 0)
            continue;
        const ImGuiResizeBorderDef& def = resize_border_def[border_n];
        const ImGuiAxis axis = (border_n == ImGuiDir_Left || border_n == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;

        bool hovered, held;
        ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, g.WindowsBorderHoverPadding);
        ImGuiID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
        ItemAdd(border_rect, border_id, NULL, ImGuiItemFlags_NoNav);
        ButtonBehavior(border_rect, border_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
        //GetForegroundDrawList(window)->AddRect(border_rect.Min, border_rect.Max, IM_COL32(255, 255, 0, 255));
        if (hovered && g.HoveredIdTimer <= WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER)
            hovered = false;
        if (hovered || held)
            SetMouseCursor((axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
        if (held && g.IO.MouseDoubleClicked[0])
        {
            // Double-clicking bottom or right border auto-fit on this axis
            // FIXME: Support top and right borders: rework CalcResizePosSizeFromAnyCorner() to be reusable in both cases.
            if (border_n == 1 || border_n == 3) // Right and bottom border
            {
                ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, 1 << axis);
                size_target[axis] = CalcWindowSizeAfterConstraint(window, size_auto_fit)[axis];
                ret_auto_fit_mask |= (1 << axis);
                hovered = held = false; // So border doesn't show highlighted at new position
            }
            ClearActiveID();
        }
        else if (held)
        {
            // Switch to relative resizing mode when border geometry moved (e.g. resizing a child altering parent scroll), in order to avoid resizing feedback loop.
            // Currently only using relative mode on resizable child windows, as the problem to solve is more likely noticeable for them, but could apply for all windows eventually.
            // FIXME: May want to generalize this idiom at lower-level, so more widgets can use it!
            const bool just_scrolled_manually_while_resizing = (g.WheelingWindow != NULL && g.WheelingWindowScrolledFrame == g.FrameCount && IsWindowChildOf(window, g.WheelingWindow, false, true));
            if (g.ActiveIdIsJustActivated || just_scrolled_manually_while_resizing)
            {
                g.WindowResizeBorderExpectedRect = border_rect;
                g.WindowResizeRelativeMode = false;
            }
            if ((window->Flags & ImGuiWindowFlags_ChildWindow) && memcmp(&g.WindowResizeBorderExpectedRect, &border_rect, sizeof(ImRect)) != 0)
                g.WindowResizeRelativeMode = true;

            const ImVec2 border_curr = (window->Pos + ImMin(def.SegmentN1, def.SegmentN2) * window->Size);
            const float border_target_rel_mode_for_axis = border_curr[axis] + g.IO.MouseDelta[axis];
            const float border_target_abs_mode_for_axis = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + g.WindowsBorderHoverPadding; // Match ButtonBehavior() padding above.

            // Use absolute mode position
            ImVec2 border_target = window->Pos;
            border_target[axis] = border_target_abs_mode_for_axis;

            // Use relative mode target for child window, ignore resize when moving back toward the ideal absolute position.
            bool ignore_resize = false;
            if (g.WindowResizeRelativeMode)
            {
                //GetForegroundDrawList()->AddText(GetMainViewport()->WorkPos, IM_COL32_WHITE, "Relative Mode");
                border_target[axis] = border_target_rel_mode_for_axis;
                if (g.IO.MouseDelta[axis] == 0.0f || (g.IO.MouseDelta[axis] > 0.0f) == (border_target_rel_mode_for_axis > border_target_abs_mode_for_axis))
                    ignore_resize = true;
            }

            // Clamp, apply
            ImVec2 clamp_min(border_n == ImGuiDir_Right ? clamp_rect.Min.x : -FLT_MAX, border_n == ImGuiDir_Down || (border_n == ImGuiDir_Up && window_move_from_title_bar) ? clamp_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max(border_n == ImGuiDir_Left ? clamp_rect.Max.x : +FLT_MAX, border_n == ImGuiDir_Up ? clamp_rect.Max.y : +FLT_MAX);
            border_target = ImClamp(border_target, clamp_min, clamp_max);
            if (!ignore_resize)
                CalcResizePosSizeFromAnyCorner(window, border_target, ImMin(def.SegmentN1, def.SegmentN2), &pos_target, &size_target);
        }
        if (hovered)
            *border_hovered = border_n;
        if (held)
            *border_held = border_n;
    }
    PopID();

    // Restore nav layer
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

    // Navigation resize (keyboard/gamepad)
    // FIXME: This cannot be moved to NavUpdateWindowing() because CalcWindowSizeAfterConstraint() need to callback into user.
    // Not even sure the callback works here.
    if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindowDockTree == window)
    {
        ImVec2 nav_resize_dir;
        if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
            nav_resize_dir = GetKeyMagnitude2d(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
        if (g.NavInputSource == ImGuiInputSource_Gamepad)
            nav_resize_dir = GetKeyMagnitude2d(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadDpadDown);
        if (nav_resize_dir.x != 0.0f || nav_resize_dir.y != 0.0f)
        {
            const float NAV_RESIZE_SPEED = 600.0f;
            const float resize_step = NAV_RESIZE_SPEED * g.IO.DeltaTime * GetScale();
            g.NavWindowingAccumDeltaSize += nav_resize_dir * resize_step;
            g.NavWindowingAccumDeltaSize = ImMax(g.NavWindowingAccumDeltaSize, clamp_rect.Min - window->Pos - window->Size); // We need Pos+Size >= clmap_rect.Min, so Size >= clmap_rect.Min - Pos, so size_delta >= clmap_rect.Min - window->Pos - window->Size
            g.NavWindowingToggleLayer = false;
            g.NavHighlightItemUnderNav = true;
            resize_grip_col[0] = GetColorU32(ImGuiCol_ResizeGripActive);
            ImVec2 accum_floored = ImTrunc(g.NavWindowingAccumDeltaSize);
            if (accum_floored.x != 0.0f || accum_floored.y != 0.0f)
            {
                // FIXME-NAV: Should store and accumulate into a separate size buffer to handle sizing constraints properly, right now a constraint will make us stuck.
                size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull + accum_floored);
                g.NavWindowingAccumDeltaSize -= accum_floored;
            }
        }
    }

    // Apply back modified position/size to window
    const ImVec2 old_pos = window->Pos;
    const ImVec2 old_size = window->SizeFull;
    if (size_target.x != FLT_MAX && (window->Size.x != size_target.x || window->SizeFull.x != size_target.x))
        window->Size.x = window->SizeFull.x = size_target.x;
    if (size_target.y != FLT_MAX && (window->Size.y != size_target.y || window->SizeFull.y != size_target.y))
        window->Size.y = window->SizeFull.y = size_target.y;
    if (pos_target.x != FLT_MAX && window->Pos.x != ImTrunc(pos_target.x))
        window->Pos.x = ImTrunc(pos_target.x);
    if (pos_target.y != FLT_MAX && window->Pos.y != ImTrunc(pos_target.y))
        window->Pos.y = ImTrunc(pos_target.y);
    if (old_pos.x != window->Pos.x || old_pos.y != window->Pos.y || old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
        MarkIniSettingsDirty(window);

    // Recalculate next expected border expected coordinates
    if (*border_held != -1)
        g.WindowResizeBorderExpectedRect = GetResizeBorderRect(window, *border_held, grip_hover_inner_size, g.WindowsBorderHoverPadding);

    return ret_auto_fit_mask;
}

static inline void ClampWindowPos(ImGuiWindow* window, const ImRect& visibility_rect)
{
    ImVec2 size_for_clamping = window->Size;
    const bool move_from_title_bar_only = (window->BgClickFlags & ImGuiWindowBgClickFlags_Move) == 0;
    if (move_from_title_bar_only && window->DockNodeAsHost)
        size_for_clamping.y = ImGui::GetFrameHeight(); // Not using window->TitleBarHeight() as DockNodeAsHost will report 0.0f here.
    else if (move_from_title_bar_only && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        size_for_clamping.y = window->TitleBarHeight;
    window->Pos = ImClamp(window->Pos, visibility_rect.Min - size_for_clamping, visibility_rect.Max);
}

static void RenderWindowOuterSingleBorder(ImGuiWindow* window, int border_n, ImU32 border_col, float border_size)
{
    const ImGuiResizeBorderDef& def = resize_border_def[border_n];
    const float rounding = window->WindowRounding;
    const ImRect border_r = GetResizeBorderRect(window, border_n, rounding, 0.0f);
    window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN1) + ImVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
    window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN2) + ImVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
    window->DrawList->PathStroke(border_col, ImDrawFlags_None, border_size);
}

void ImGui::RenderWindowOuterBorders(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    const float border_size = window->WindowBorderSize;
    const ImU32 border_col = GetColorU32(ImGuiCol_Border);
    if (border_size > 0.0f && (window->Flags & ImGuiWindowFlags_NoBackground) == 0)
        window->DrawList->AddRect(window->Pos, window->Pos + window->Size, border_col, window->WindowRounding, 0, window->WindowBorderSize);
    else if (border_size > 0.0f)
    {
        if (window->ChildFlags & ImGuiChildFlags_ResizeX) // Similar code as 'resize_border_mask' computation in UpdateWindowManualResize() but we specifically only always draw explicit child resize border.
            RenderWindowOuterSingleBorder(window, 1, border_col, border_size);
        if (window->ChildFlags & ImGuiChildFlags_ResizeY)
            RenderWindowOuterSingleBorder(window, 3, border_col, border_size);
    }
    if (window->ResizeBorderHovered != -1 || window->ResizeBorderHeld != -1)
    {
        const int border_n = (window->ResizeBorderHeld != -1) ? window->ResizeBorderHeld : window->ResizeBorderHovered;
        const ImU32 border_col_resizing = GetColorU32((window->ResizeBorderHeld != -1) ? ImGuiCol_SeparatorActive : ImGuiCol_SeparatorHovered);
        RenderWindowOuterSingleBorder(window, border_n, border_col_resizing, ImMax(2.0f, window->WindowBorderSize)); // Thicker than usual
    }
    if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
    {
        float y = window->Pos.y + window->TitleBarHeight - 1;
        window->DrawList->AddLine(ImVec2(window->Pos.x + border_size * 0.5f, y), ImVec2(window->Pos.x + window->Size.x - border_size * 0.5f, y), border_col, g.Style.FrameBorderSize);
    }
}

// Draw background and borders
// Draw and handle scrollbars
void ImGui::RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    // Ensure that Scrollbar() doesn't read last frame's SkipItems
    IM_ASSERT(window->BeginCount == 0);
    window->SkipItems = false;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Draw window + handle manual resize
    // As we highlight the title bar when want_focus is set, multiple reappearing windows will have their title bar highlighted on their reappearing frame.
    const float window_rounding = window->WindowRounding;
    const float window_border_size = window->WindowBorderSize;
    if (window->Collapsed)
    {
        // Title bar only
        const float backup_border_size = style.FrameBorderSize;
        g.Style.FrameBorderSize = window->WindowBorderSize;
        ImU32 title_bar_col = GetColorU32((title_bar_is_highlight && g.NavCursorVisible) ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBgCollapsed);
        if (window->ViewportOwned)
            title_bar_col |= IM_COL32_A_MASK; // No alpha (we don't support is_docking_transparent_payload here because simpler and less meaningful, but could with a bit of code shuffle/reuse)
        RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
        g.Style.FrameBorderSize = backup_border_size;
    }
    else
    {
        // Window background
        if (!(flags & ImGuiWindowFlags_NoBackground))
        {
            bool is_docking_transparent_payload = false;
            if (g.DragDropActive && (g.FrameCount - g.DragDropAcceptFrameCount) <= 1 && g.IO.ConfigDockingTransparentPayload)
                if (g.DragDropPayload.IsDataType(IMGUI_PAYLOAD_TYPE_WINDOW) && *(ImGuiWindow**)g.DragDropPayload.Data == window)
                    is_docking_transparent_payload = true;

            ImU32 bg_col = GetColorU32(GetWindowBgColorIdx(window));
            if (window->ViewportOwned)
            {
                bg_col |= IM_COL32_A_MASK; // No alpha
                if (is_docking_transparent_payload)
                    window->Viewport->Alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA;
            }
            else
            {
                // Adjust alpha. For docking
                bool override_alpha = false;
                float alpha = 1.0f;
                if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasBgAlpha)
                {
                    alpha = g.NextWindowData.BgAlphaVal;
                    override_alpha = true;
                }
                if (is_docking_transparent_payload)
                {
                    alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA; // FIXME-DOCK: Should that be an override?
                    override_alpha = true;
                }
                if (override_alpha)
                    bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(alpha) << IM_COL32_A_SHIFT);
            }

            // Render, for docked windows and host windows we ensure BG goes before decorations
            if (window->DockIsActive)
                window->DockNode->LastBgColor = bg_col;
            if (flags & ImGuiWindowFlags_DockNodeHost)
                bg_col = 0;
            if (bg_col & IM_COL32_A_MASK)
            {
                ImRect bg_rect(window->Pos + ImVec2(0, window->TitleBarHeight), window->Pos + window->Size);
                ImDrawFlags bg_rounding_flags;
                if (window->DockIsActive)
                    bg_rounding_flags = CalcRoundingFlagsForRectInRect(bg_rect, window->DockNode->HostWindow->Rect(), 0.0f);
                else
                    bg_rounding_flags = (flags & ImGuiWindowFlags_NoTitleBar) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersBottom;
                ImDrawList* bg_draw_list = window->DockIsActive ? window->DockNode->HostWindow->DrawList : window->DrawList;
                if (window->DockIsActive)
                    bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
                bg_draw_list->AddRectFilled(bg_rect.Min, bg_rect.Max, bg_col, window_rounding, bg_rounding_flags);
                if (window->DockIsActive)
                    bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG);
            }
        }
        if (window->DockIsActive)
            window->DockNode->IsBgDrawnThisFrame = true;

        // Title bar
        // (when docked, DockNode are drawing their own title bar. Individual windows however do NOT set the _NoTitleBar flag,
        // in order for their pos/size to be matching their undocking state.)
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
        {
            ImU32 title_bar_col = GetColorU32(title_bar_is_highlight ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg);
            if (window->ViewportOwned)
                title_bar_col |= IM_COL32_A_MASK; // No alpha
            window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding, ImDrawFlags_RoundCornersTop);
        }

        // Menu bar
        if (flags & ImGuiWindowFlags_MenuBar)
        {
            ImRect menu_bar_rect = window->MenuBarRect();
            menu_bar_rect.ClipWith(window->Rect());  // Soft clipping, in particular child window don't have minimum size covering the menu bar so this is useful for them.
            window->DrawList->AddRectFilled(menu_bar_rect.Min, menu_bar_rect.Max, GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, ImDrawFlags_RoundCornersTop);
            if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
                window->DrawList->AddLine(menu_bar_rect.GetBL() + ImVec2(window_border_size * 0.5f, 0.0f), menu_bar_rect.GetBR() - ImVec2(window_border_size * 0.5f, 0.0f), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
        }

        // Docking: Unhide tab bar (small triangle in the corner), drag from small triangle to quickly undock
        ImGuiDockNode* node = window->DockNode;
        if (window->DockIsActive && node->IsHiddenTabBar() && !node->IsNoTabBar())
        {
            float unhide_sz_draw = ImTrunc(g.FontSize * 0.70f);
            float unhide_sz_hit = ImTrunc(g.FontSize * 0.55f);
            ImVec2 p = node->Pos;
            ImRect r(p, p + ImVec2(unhide_sz_hit, unhide_sz_hit));
            ImGuiID unhide_id = window->GetID("#UNHIDE");
            KeepAliveID(unhide_id);
            bool hovered, held;
            if (ButtonBehavior(r, unhide_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren))
                node->WantHiddenTabBarToggle = true;
            else if (held && IsMouseDragging(0))
                StartMouseMovingWindowOrNode(window, node, true); // Undock from tab-bar triangle = same as window/collapse menu button

            // FIXME-DOCK: Ideally we'd use ImGuiCol_TitleBgActive/ImGuiCol_TitleBg here, but neither is guaranteed to be visible enough at this sort of size..
            ImU32 col = GetColorU32(((held && hovered) || (node->IsFocused && !hovered)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            window->DrawList->AddTriangleFilled(p, p + ImVec2(unhide_sz_draw, 0.0f), p + ImVec2(0.0f, unhide_sz_draw), col);
        }

        // Scrollbars
        if (window->ScrollbarX)
            Scrollbar(ImGuiAxis_X);
        if (window->ScrollbarY)
            Scrollbar(ImGuiAxis_Y);

        // Render resize grips (after their input handling so we don't have a frame of latency)
        if (handle_borders_and_resize_grips && !(flags & ImGuiWindowFlags_NoResize))
        {
            for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
            {
                const ImU32 col = resize_grip_col[resize_grip_n];
                if ((col & IM_COL32_A_MASK) == 0)
                    continue;
                const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
                const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
                const float border_inner = IM_ROUND(window_border_size * 0.5f);
                window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(border_inner, resize_grip_draw_size) : ImVec2(resize_grip_draw_size, border_inner)));
                window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(resize_grip_draw_size, border_inner) : ImVec2(border_inner, resize_grip_draw_size)));
                window->DrawList->PathArcToFast(ImVec2(corner.x + grip.InnerDir.x * (window_rounding + border_inner), corner.y + grip.InnerDir.y * (window_rounding + border_inner)), window_rounding, grip.AngleMin12, grip.AngleMax12);
                window->DrawList->PathFillConvex(col);
            }
        }

        // Borders (for dock node host they will be rendered over after the tab bar)
        if (handle_borders_and_resize_grips && !window->DockNodeAsHost)
            RenderWindowOuterBorders(window);
    }
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
}

// When inside a dock node, this is handled in DockNodeCalcTabBarLayout() instead.
// Render title text, collapse button, close button
void ImGui::RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    const bool has_close_button = (p_open != NULL);
    const bool has_collapse_button = !(flags & ImGuiWindowFlags_NoCollapse) && (style.WindowMenuButtonPosition != ImGuiDir_None);

    // Close & Collapse button are on the Menu NavLayer and don't default focus (unless there's nothing else on that layer)
    // FIXME-NAV: Might want (or not?) to set the equivalent of ImGuiButtonFlags_NoNavFocus so that mouse clicks on standard title bar items don't necessarily set nav/keyboard ref?
    const ImGuiItemFlags item_flags_backup = g.CurrentItemFlags;
    g.CurrentItemFlags |= ImGuiItemFlags_NoNavDefaultFocus;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    // Layout buttons
    // FIXME: Would be nice to generalize the subtleties expressed here into reusable code.
    float pad_l = style.FramePadding.x;
    float pad_r = style.FramePadding.x;
    float button_sz = g.FontSize;
    ImVec2 close_button_pos;
    ImVec2 collapse_button_pos;
    if (has_close_button)
    {
        close_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        pad_r += button_sz + style.ItemInnerSpacing.x;
    }
    if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Right)
    {
        collapse_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        pad_r += button_sz + style.ItemInnerSpacing.x;
    }
    if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Left)
    {
        collapse_button_pos = ImVec2(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y + style.FramePadding.y);
        pad_l += button_sz + style.ItemInnerSpacing.x;
    }

    // Collapse button (submitting first so it gets priority when choosing a navigation init fallback)
    if (has_collapse_button)
        if (CollapseButton(window->GetID("#COLLAPSE"), collapse_button_pos, NULL))
            window->WantCollapseToggle = true; // Defer actual collapsing to next frame as we are too far in the Begin() function

    // Close button
    if (has_close_button)
    {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_NoFocus;
        if (CloseButton(window->GetID("#CLOSE"), close_button_pos))
            *p_open = false;
        g.CurrentItemFlags = backup_item_flags;
    }

    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    g.CurrentItemFlags = item_flags_backup;

    // Title bar text (with: horizontal alignment, avoiding collapse/close button, optional "unsaved document" marker)
    // FIXME: Refactor text alignment facilities along with RenderText helpers, this is WAY too much messy code..
    const float marker_size_x = (flags & ImGuiWindowFlags_UnsavedDocument) ? button_sz * 0.80f : 0.0f;
    const ImVec2 text_size = CalcTextSize(name, NULL, true) + ImVec2(marker_size_x, 0.0f);

    // As a nice touch we try to ensure that centered title text doesn't get affected by visibility of Close/Collapse button,
    // while uncentered title text will still reach edges correctly.
    if (pad_l > style.FramePadding.x)
        pad_l += g.Style.ItemInnerSpacing.x;
    if (pad_r > style.FramePadding.x)
        pad_r += g.Style.ItemInnerSpacing.x;
    if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f)
    {
        float centerness = ImSaturate(1.0f - ImFabs(style.WindowTitleAlign.x - 0.5f) * 2.0f); // 0.0f on either edges, 1.0f on center
        float pad_extend = ImMin(ImMax(pad_l, pad_r), title_bar_rect.GetWidth() - pad_l - pad_r - text_size.x);
        pad_l = ImMax(pad_l, pad_extend * centerness);
        pad_r = ImMax(pad_r, pad_extend * centerness);
    }

    ImRect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y, title_bar_rect.Max.x - pad_r, title_bar_rect.Max.y);
    ImRect clip_r(layout_r.Min.x, layout_r.Min.y, ImMin(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x), layout_r.Max.y);
    if (flags & ImGuiWindowFlags_UnsavedDocument)
    {
        ImVec2 marker_pos;
        marker_pos.x = ImClamp(layout_r.Min.x + (layout_r.GetWidth() - text_size.x) * style.WindowTitleAlign.x + text_size.x, layout_r.Min.x, layout_r.Max.x);
        marker_pos.y = (layout_r.Min.y + layout_r.Max.y) * 0.5f;
        if (marker_pos.x > layout_r.Min.x)
        {
            RenderBullet(window->DrawList, marker_pos, GetColorU32(ImGuiCol_UnsavedMarker));
            clip_r.Max.x = ImMin(clip_r.Max.x, marker_pos.x - (int)(marker_size_x * 0.5f));
        }
    }
    //if (g.IO.KeyShift) window->DrawList->AddRect(layout_r.Min, layout_r.Max, IM_COL32(255, 128, 0, 255)); // [DEBUG]
    //if (g.IO.KeyCtrl) window->DrawList->AddRect(clip_r.Min, clip_r.Max, IM_COL32(255, 128, 0, 255)); // [DEBUG]
    RenderTextClipped(layout_r.Min, layout_r.Max, name, NULL, &text_size, style.WindowTitleAlign, &clip_r);
}

void ImGui::UpdateWindowParentAndRootLinks(ImGuiWindow* window, ImGuiWindowFlags flags, ImGuiWindow* parent_window)
{
    window->ParentWindow = parent_window;
    window->RootWindow = window->RootWindowPopupTree = window->RootWindowDockTree = window->RootWindowForTitleBarHighlight = window->RootWindowForNav = window;
    if (parent_window && (flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Tooltip))
    {
        window->RootWindowDockTree = parent_window->RootWindowDockTree;
        if (!window->DockIsActive && !(parent_window->Flags & ImGuiWindowFlags_DockNodeHost))
            window->RootWindow = parent_window->RootWindow;
    }
    if (parent_window && (flags & ImGuiWindowFlags_Popup))
        window->RootWindowPopupTree = parent_window->RootWindowPopupTree;
    if (parent_window && !(flags & ImGuiWindowFlags_Modal) && (flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip))) // FIXME: simply use _NoTitleBar ?
        window->RootWindowForTitleBarHighlight = parent_window->RootWindowForTitleBarHighlight;
    while (window->RootWindowForNav->ChildFlags & ImGuiChildFlags_NavFlattened)
    {
        IM_ASSERT(window->RootWindowForNav->ParentWindow != NULL);
        window->RootWindowForNav = window->RootWindowForNav->ParentWindow;
    }
}

// [EXPERIMENTAL] Called by Begin(). NextWindowData is valid at this point.
// This is designed as a toy/test-bed for
void ImGui::UpdateWindowSkipRefresh(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    window->SkipRefresh = false;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasRefreshPolicy) == 0)
        return;
    if (g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_TryToAvoidRefresh)
    {
        // FIXME-IDLE: Tests for e.g. mouse clicks or keyboard while focused.
        if (window->Appearing) // If currently appearing
            return;
        if (window->Hidden) // If was hidden (previous frame)
            return;
        if ((g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_RefreshOnHover) && g.HoveredWindow)
            if (window->RootWindow == g.HoveredWindow->RootWindow || IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, window))
                return;
        if ((g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_RefreshOnFocus) && g.NavWindow)
            if (window->RootWindow == g.NavWindow->RootWindow || IsWindowWithinBeginStackOf(g.NavWindow->RootWindow, window))
                return;
        window->DrawList = NULL;
        window->SkipRefresh = true;
    }
}

static void SetWindowActiveForSkipRefresh(ImGuiWindow* window)
{
    window->Active = true;
    for (ImGuiWindow* child : window->DC.ChildWindows)
        if (!child->Hidden)
        {
            child->Active = child->SkipRefresh = true;
            SetWindowActiveForSkipRefresh(child);
        }
}

// Push a new Dear ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    IM_ASSERT(name != NULL && name[0] != '\0');     // Window name required
    IM_ASSERT(g.WithinFrameScope);                  // Forgot to call ImGui::NewFrame()
    IM_ASSERT(g.FrameCountEnded != g.FrameCount);   // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

    // Find or create
    ImGuiWindow* window = FindWindowByName(name);
    const bool window_just_created = (window == NULL);
    if (window_just_created)
        window = CreateNewWindow(name, flags);

    // [DEBUG] Debug break requested by user
    if (g.DebugBreakInWindow == window->ID)
        IM_DEBUG_BREAK();

    // Automatically disable manual moving/resizing when NoInputs is set
    if ((flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    const int current_frame = g.FrameCount;
    const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
    window->IsFallbackWindow = (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

    // Update the Appearing flag (note: the BeginDocked() path may also set this to true later)
    bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1); // Not using !WasActive because the implicit "Debug" window would always toggle off->on
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
        window_just_activated_by_user |= (window != popup_ref.Window);
    }

    // Update Flags, LastFrameActive, BeginOrderXXX fields
    const bool window_was_appearing = window->Appearing;
    if (first_begin_of_the_frame)
    {
        UpdateWindowInFocusOrderList(window, window_just_created, flags);
        window->Appearing = window_just_activated_by_user;
        if (window->Appearing)
            SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);
        window->FlagsPreviousFrame = window->Flags;
        window->Flags = (ImGuiWindowFlags)flags;
        window->ChildFlags = (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags) ? g.NextWindowData.ChildFlags : 0;
        window->LastFrameActive = current_frame;
        window->LastTimeActive = (float)g.Time;
        window->BeginOrderWithinParent = 0;
        window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
    }
    else
    {
        flags = window->Flags;
    }

    // Docking
    // (NB: during the frame dock nodes are created, it is possible that (window->DockIsActive == false) even though (window->DockNode->Windows.Size > 1)
    IM_ASSERT(window->DockNode == NULL || window->DockNodeAsHost == NULL); // Cannot be both
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasDock)
        SetWindowDock(window, g.NextWindowData.DockId, g.NextWindowData.DockCond);
    if (first_begin_of_the_frame)
    {
        const bool has_dock_node = (window->DockId != 0 || window->DockNode != NULL);
        const bool new_auto_dock_node = !has_dock_node && GetWindowAlwaysWantOwnTabBar(window);
        const bool dock_node_was_visible = window->DockNodeIsVisible;
        const bool dock_tab_was_visible = window->DockTabIsVisible;
        window->DockIsActive = window->DockNodeIsVisible = window->DockTabIsVisible = false;

        if (has_dock_node || new_auto_dock_node)
        {
            BeginDocked(window, p_open);
            flags = window->Flags;
            if (window->DockIsActive)
            {
                IM_ASSERT(window->DockNode != NULL);
                g.NextWindowData.HasFlags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint; // Docking currently override constraints
            }

            // Amend the Appearing flag
            if (window->DockTabIsVisible && !dock_tab_was_visible && dock_node_was_visible && !window->Appearing && !window_was_appearing)
            {
                window->Appearing = true;
                SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);
            }
        }
    }

    // Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from a different window stack
    ImGuiWindow* parent_window_in_stack = (window->DockIsActive && window->DockNode->HostWindow) ? window->DockNode->HostWindow : g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back().Window;
    ImGuiWindow* parent_window = first_begin_of_the_frame ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) ? parent_window_in_stack : NULL) : window->ParentWindow;
    IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

    // We allow window memory to be compacted so recreate the base stack when needed.
    if (window->IDStack.Size == 0)
        window->IDStack.push_back(window->ID);

    // Add to stack
    g.CurrentWindow = window;
    g.CurrentWindowStack.resize(g.CurrentWindowStack.Size + 1);
    ImGuiWindowStackData& window_stack_data = g.CurrentWindowStack.back();
    window_stack_data.Window = window;
    window_stack_data.ParentLastItemDataBackup = g.LastItemData;
    window_stack_data.DisabledOverrideReenable = (flags & ImGuiWindowFlags_Tooltip) && (g.CurrentItemFlags & ImGuiItemFlags_Disabled);
    window_stack_data.DisabledOverrideReenableAlphaBackup = 0.0f;
    ErrorRecoveryStoreState(&window_stack_data.StackSizesInBegin);
    g.StackSizesInBeginForCurrentWindow = &window_stack_data.StackSizesInBegin;
    if (flags & ImGuiWindowFlags_ChildMenu)
        g.BeginMenuDepth++;

    // Update ->RootWindow and others pointers (before any possible call to FocusWindow)
    if (first_begin_of_the_frame)
    {
        UpdateWindowParentAndRootLinks(window, flags, parent_window);
        window->ParentWindowInBeginStack = parent_window_in_stack;

        // Focus route
        // There's little point to expose a flag to set this: because the interesting cases won't be using parent_window_in_stack,
        // Use for e.g. linking a tool window in a standalone viewport to a document window, regardless of their Begin() stack parenting. (#6798)
        window->ParentWindowForFocusRoute = (window->RootWindow != window) ? parent_window_in_stack : NULL;
        if (window->ParentWindowForFocusRoute == NULL && window->DockNode != NULL)
            if (window->DockNode->MergedFlags & ImGuiDockNodeFlags_DockedWindowsInFocusRoute)
                window->ParentWindowForFocusRoute = window->DockNode->HostWindow;

        // Override with SetNextWindowClass() field or direct call to SetWindowParentWindowForFocusRoute()
        if (window->WindowClass.FocusRouteParentWindowId != 0)
        {
            window->ParentWindowForFocusRoute = FindWindowByID(window->WindowClass.FocusRouteParentWindowId);
            IM_ASSERT(window->ParentWindowForFocusRoute != 0); // Invalid value for FocusRouteParentWindowId.
        }

        // Inherit SetWindowFontScale() from parent until we fix this system...
        window->FontWindowScaleParents = parent_window ? parent_window->FontWindowScaleParents * parent_window->FontWindowScale : 1.0f;
    }

    // Add to focus scope stack
    PushFocusScope((window->ChildFlags & ImGuiChildFlags_NavFlattened) ? g.CurrentFocusScopeId : window->ID);
    window->NavRootFocusScopeId = g.CurrentFocusScopeId;

    // Add to popup stacks: update OpenPopupStack[] data, push to BeginPopupStack[]
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        popup_ref.Window = window;
        popup_ref.ParentNavLayer = parent_window_in_stack->DC.NavLayerCurrent;
        g.BeginPopupStack.push_back(popup_ref);
        window->PopupId = popup_ref.PopupId;
    }

    // Process SetNextWindow***() calls
    // (FIXME: Consider splitting the HasXXX flags into X/Y components)
    bool window_pos_set_by_api = false;
    bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos)
    {
        window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
        if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
        {
            // May be processed on the next frame if this is our first frame and we are measuring size
            // FIXME: Look into removing the branch so everything can go through this same code path for consistency.
            window->SetWindowPosVal = g.NextWindowData.PosVal;
            window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
            window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
        }
        else
        {
            SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
        }
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize)
    {
        window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
        window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
        if ((window->ChildFlags & ImGuiChildFlags_ResizeX) && (window->SetWindowSizeAllowFlags & ImGuiCond_FirstUseEver) == 0) // Axis-specific conditions for BeginChild()
            g.NextWindowData.SizeVal.x = window->SizeFull.x;
        if ((window->ChildFlags & ImGuiChildFlags_ResizeY) && (window->SetWindowSizeAllowFlags & ImGuiCond_FirstUseEver) == 0)
            g.NextWindowData.SizeVal.y = window->SizeFull.y;
        SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasScroll)
    {
        if (g.NextWindowData.ScrollVal.x >= 0.0f)
        {
            window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
            window->ScrollTargetCenterRatio.x = 0.0f;
        }
        if (g.NextWindowData.ScrollVal.y >= 0.0f)
        {
            window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
            window->ScrollTargetCenterRatio.y = 0.0f;
        }
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasContentSize)
        window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
    else if (first_begin_of_the_frame)
        window->ContentSizeExplicit = ImVec2(0.0f, 0.0f);
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasWindowClass)
        window->WindowClass = g.NextWindowData.WindowClass;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasCollapsed)
        SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasFocus)
        FocusWindow(window);
    if (window->Appearing)
        SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

    // [EXPERIMENTAL] Skip Refresh mode
    UpdateWindowSkipRefresh(window);

    // Nested root windows (typically tooltips) override disabled state
    if (window_stack_data.DisabledOverrideReenable && window->RootWindow == window)
        BeginDisabledOverrideReenable();

    // We intentionally set g.CurrentWindow to NULL to prevent usage until when the viewport is set, then will call SetCurrentWindow()
    g.CurrentWindow = NULL;

    // When reusing window again multiple times a frame, just append content (don't need to setup again)
    if (first_begin_of_the_frame && !window->SkipRefresh)
    {
        // Initialize
        const bool window_is_child_tooltip = (flags & ImGuiWindowFlags_ChildWindow) && (flags & ImGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)
        const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
        window->Active = true;
        window->HasCloseButton = (p_open != NULL);
        window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
        window->IDStack.resize(1);
        window->DrawList->_ResetForNewFrame();
        window->DC.CurrentTableIdx = -1;
        if (flags & ImGuiWindowFlags_DockNodeHost)
        {
            window->DrawList->ChannelsSplit(2);
            window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG); // Render decorations on channel 1 as we will render the backgrounds manually later
        }

        // Restore buffer capacity when woken from a compacted state, to avoid
        if (window->MemoryCompacted)
            GcAwakeTransientWindowBuffers(window);

        // Update stored window name when it changes (which can _only_ happen with the "###" operator, so the ID would stay unchanged).
        // The title bar always display the 'name' parameter, so we only update the string storage if it needs to be visible to the end-user elsewhere.
        bool window_title_visible_elsewhere = false;
        if ((window->Viewport && window->Viewport->Window == window) || (window->DockIsActive))
            window_title_visible_elsewhere = true;
        else if (g.NavWindowingListWindow != NULL && g.NavWindowingListWindow->WasActive && (flags & ImGuiWindowFlags_NoNavFocus) == 0)   // Window titles visible when using Ctrl+Tab
            window_title_visible_elsewhere = true;
        else if (flags & ImGuiWindowFlags_ChildMenu)
            window_title_visible_elsewhere = true;
        if ((window_title_visible_elsewhere || window_just_activated_by_user) && !window_just_created && strcmp(name, window->Name) != 0)
        {
            size_t buf_len = (size_t)window->NameBufLen;
            window->Name = ImStrdupcpy(window->Name, &buf_len, name);
            window->NameBufLen = (int)buf_len;
        }

        // UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

        // Update contents size from last frame for auto-fitting (or use explicit size)
        CalcWindowContentSizes(window, &window->ContentSize, &window->ContentSizeIdeal);

        // FIXME: These flags are decremented before they are used. This means that in order to have these fields produce their intended behaviors
        // for one frame we must set them to at least 2, which is counter-intuitive. HiddenFramesCannotSkipItems is a more complicated case because
        // it has a single usage before this code block and may be set below before it is finally checked.
        if (window->HiddenFramesCanSkipItems > 0)
            window->HiddenFramesCanSkipItems--;
        if (window->HiddenFramesCannotSkipItems > 0)
            window->HiddenFramesCannotSkipItems--;
        if (window->HiddenFramesForRenderOnly > 0)
            window->HiddenFramesForRenderOnly--;

        // Hide new windows for one frame until they calculate their size
        if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
            window->HiddenFramesCannotSkipItems = 1;

        // Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
        // We reset Size/ContentSize for reappearing popups/tooltips early in this function, so further code won't be tempted to use the old size.
        if (window_just_activated_by_user && (flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0)
        {
            window->HiddenFramesCannotSkipItems = 1;
            if (flags & ImGuiWindowFlags_AlwaysAutoResize)
            {
                if (!window_size_x_set_by_api)
                    window->Size.x = window->SizeFull.x = 0.f;
                if (!window_size_y_set_by_api)
                    window->Size.y = window->SizeFull.y = 0.f;
                window->ContentSize = window->ContentSizeIdeal = ImVec2(0.f, 0.f);
            }
        }

        // SELECT VIEWPORT
        // We need to do this before using any style/font sizes, as viewport with a different DPI may affect font sizes.

        WindowSelectViewport(window);
        SetCurrentViewport(window, window->Viewport);
        SetCurrentWindow(window);
        flags = window->Flags;

        // LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't cause inconsistencies)
        // We read Style data after the call to UpdateSelectWindowViewport() which might be swapping the style.

        if (!window->DockIsActive && (flags & ImGuiWindowFlags_ChildWindow))
            window->WindowBorderSize = style.ChildBorderSize;
        else
            window->WindowBorderSize = ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
        window->WindowPadding = style.WindowPadding;
        if (!window->DockIsActive && (flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !(window->ChildFlags & ImGuiChildFlags_AlwaysUseWindowPadding) && window->WindowBorderSize == 0.0f)
            window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

        // Lock menu offset so size calculation can use it as menu-bar windows need a minimum size.
        window->DC.MenuBarOffset.x = ImMax(ImMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
        window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;
        window->TitleBarHeight = (flags & ImGuiWindowFlags_NoTitleBar) ? 0.0f : g.FontSize + g.Style.FramePadding.y * 2.0f;
        window->MenuBarHeight = (flags & ImGuiWindowFlags_MenuBar) ? window->DC.MenuBarOffset.y + g.FontSize + g.Style.FramePadding.y * 2.0f : 0.0f;
        window->FontRefSize = g.FontSize; // Lock this to discourage calling window->CalcFontSize() outside of current window.

        // Depending on condition we use previous or current window size to compare against contents size to decide if a scrollbar should be visible.
        // Those flags will be altered further down in the function depending on more conditions.
        bool use_current_size_for_scrollbar_x = window_just_created;
        bool use_current_size_for_scrollbar_y = window_just_created;
        if (window_size_x_set_by_api && window->ContentSizeExplicit.x != 0.0f)
            use_current_size_for_scrollbar_x = true;
        if (window_size_y_set_by_api && window->ContentSizeExplicit.y != 0.0f) // #7252
            use_current_size_for_scrollbar_y = true;

        // Collapse window by double-clicking on title bar
        // At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse) && !window->DockIsActive)
        {
            // We don't use a regular button+id to test for double-click on title bar (mostly due to legacy reason, could be fixed),
            // so verify that we don't have items over the title bar.
            ImRect title_bar_rect = window->TitleBarRect();
            if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 && g.ActiveId == 0 && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max))
                if (g.IO.MouseClickedCount[0] == 2 && GetKeyOwner(ImGuiKey_MouseLeft) == ImGuiKeyOwner_NoOwner)
                    window->WantCollapseToggle = true;
            if (window->WantCollapseToggle)
            {
                window->Collapsed = !window->Collapsed;
                if (!window->Collapsed)
                    use_current_size_for_scrollbar_y = true;
                MarkIniSettingsDirty(window);
            }
        }
        else
        {
            window->Collapsed = false;
        }
        window->WantCollapseToggle = false;

        // SIZE

        // Outer Decoration Sizes
        // (we need to clear ScrollbarSize immediately as CalcWindowAutoFitSize() needs it and can be called from other locations).
        const ImVec2 scrollbar_sizes_from_last_frame = window->ScrollbarSizes;
        window->DecoOuterSizeX1 = 0.0f;
        window->DecoOuterSizeX2 = 0.0f;
        window->DecoOuterSizeY1 = window->TitleBarHeight + window->MenuBarHeight;
        window->DecoOuterSizeY2 = 0.0f;
        window->ScrollbarSizes = ImVec2(0.0f, 0.0f);

        // Calculate auto-fit size, handle automatic resize
        // - Using SetNextWindowSize() overrides ImGuiWindowFlags_AlwaysAutoResize, so it can be used on tooltips/popups, etc.
        // - We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
        // - Auto-fit may only grow window during the first few frames.
        {
            const bool size_auto_fit_x_always = !window_size_x_set_by_api && (flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed;
            const bool size_auto_fit_y_always = !window_size_y_set_by_api && (flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed;
            const bool size_auto_fit_x_current = !window_size_x_set_by_api && (window->AutoFitFramesX > 0);
            const bool size_auto_fit_y_current = !window_size_y_set_by_api && (window->AutoFitFramesY > 0);
            int size_auto_fit_mask = 0;
            if (size_auto_fit_x_always || size_auto_fit_x_current)
                size_auto_fit_mask |= (1 << ImGuiAxis_X);
            if (size_auto_fit_y_always || size_auto_fit_y_current)
                size_auto_fit_mask |= (1 << ImGuiAxis_Y);
            const ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, size_auto_fit_mask);

            const ImVec2 old_size = window->SizeFull;
            if (size_auto_fit_x_always || size_auto_fit_x_current)
            {
                if (size_auto_fit_x_always)
                    window->SizeFull.x = size_auto_fit.x;
                else
                    window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
                use_current_size_for_scrollbar_x = true;
            }
            if (size_auto_fit_y_always || size_auto_fit_y_current)
            {
                if (size_auto_fit_y_always)
                    window->SizeFull.y = size_auto_fit.y;
                else
                    window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
                use_current_size_for_scrollbar_y = true;
            }
            if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
                MarkIniSettingsDirty(window);
        }

        // Apply minimum/maximum window size constraints and final size
        window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
        window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

        // POSITION

        // Popup latch its initial position, will position itself when it appears next frame
        if (window_just_activated_by_user)
        {
            window->AutoPosLastDirection = ImGuiDir_None;
            if ((flags & ImGuiWindowFlags_Popup) != 0 && !(flags & ImGuiWindowFlags_Modal) && !window_pos_set_by_api) // FIXME: BeginPopup() could use SetNextWindowPos()
                window->Pos = g.BeginPopupStack.back().OpenPopupPos;
        }

        // Position child window
        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            IM_ASSERT(parent_window && parent_window->Active);
            window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
            parent_window->DC.ChildWindows.push_back(window);
            if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
                window->Pos = parent_window->DC.CursorPos;
        }

        const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
        if (window_pos_with_pivot)
            SetWindowPos(window, window->SetWindowPosVal - window->Size * window->SetWindowPosPivot, 0); // Position given a pivot (e.g. for centering)
        else if ((flags & ImGuiWindowFlags_ChildMenu) != 0)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
            window->Pos = FindBestWindowPosForPopup(window);

        // Late create viewport if we don't fit within our current host viewport.
        if (window->ViewportAllowPlatformMonitorExtend >= 0 && !window->ViewportOwned && !(window->Viewport->Flags & ImGuiViewportFlags_IsMinimized))
            if (!window->Viewport->GetMainRect().Contains(window->Rect()))
            {
                // This is based on the assumption that the DPI will be known ahead (same as the DPI of the selection done in UpdateSelectWindowViewport)
                //ImGuiViewport* old_viewport = window->Viewport;
                window->Viewport = AddUpdateViewport(window, window->ID, window->Pos, window->Size, ImGuiViewportFlags_NoFocusOnAppearing);

                // FIXME-DPI
                //IM_ASSERT(old_viewport->DpiScale == window->Viewport->DpiScale); // FIXME-DPI: Something went wrong
                SetCurrentViewport(window, window->Viewport);
                SetCurrentWindow(window);
            }

        if (window->ViewportOwned)
            WindowSyncOwnedViewport(window, parent_window_in_stack);

        // Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
        // When clamping to stay visible, we will enforce that window->Pos stays inside of visibility_rect.
        ImRect viewport_rect(window->Viewport->GetMainRect());
        ImRect viewport_work_rect(window->Viewport->GetWorkRect());
        ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
        ImRect visibility_rect(viewport_work_rect.Min + visibility_padding, viewport_work_rect.Max - visibility_padding);

        // Clamp position/size so window stays visible within its viewport or monitor
        // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
        // FIXME: Similar to code in GetWindowAllowedExtentRect()
        if (!window_pos_set_by_api && !(flags & ImGuiWindowFlags_ChildWindow))
        {
            if (!window->ViewportOwned && viewport_rect.GetWidth() > 0 && viewport_rect.GetHeight() > 0.0f)
            {
                ClampWindowPos(window, visibility_rect);
            }
            else if (window->ViewportOwned && g.PlatformIO.Monitors.Size > 0)
            {
                if (g.MovingWindow != NULL && window->RootWindowDockTree == g.MovingWindow->RootWindowDockTree)
                {
                    // While moving windows we allow them to straddle monitors (#7299, #3071)
                    visibility_rect = g.PlatformMonitorsFullWorkRect;
                }
                else
                {
                    // When not moving ensure visible in its monitor
                    // Lost windows (e.g. a monitor disconnected) will naturally moved to the fallback/dummy monitor aka the main viewport.
                    const ImGuiPlatformMonitor* monitor = GetViewportPlatformMonitor(window->Viewport);
                    visibility_rect = ImRect(monitor->WorkPos, monitor->WorkPos + monitor->WorkSize);
                }
                visibility_rect.Expand(-visibility_padding);
                ClampWindowPos(window, visibility_rect);
            }
        }
        window->Pos = ImTrunc(window->Pos);

        // Lock window rounding for the frame (so that altering them doesn't cause inconsistencies)
        // Large values tend to lead to variety of artifacts and are not recommended.
        if ((flags & ImGuiWindowFlags_ChildWindow) && !window->DockIsActive)
            window->WindowRounding = style.ChildRounding;
        else if (window->RootWindowDockTree->ViewportOwned)
            window->WindowRounding = 0.0f;
        else
            window->WindowRounding = ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupRounding : style.WindowRounding;

        // For windows with title bar or menu bar, we clamp to FrameHeight(FontSize + FramePadding.y * 2.0f) to completely hide artifacts.
        //if ((window->Flags & ImGuiWindowFlags_MenuBar) || !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        //    window->WindowRounding = ImMin(window->WindowRounding, g.FontSize + style.FramePadding.y * 2.0f);

        // Apply window focus (new and reactivated windows are moved to front)
        bool want_focus = false;
        if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
        {
            if (flags & ImGuiWindowFlags_Popup)
                want_focus = true;
            else if ((window->DockIsActive || (flags & ImGuiWindowFlags_ChildWindow) == 0) && !(flags & ImGuiWindowFlags_Tooltip))
                want_focus = true;
        }

        // [Test Engine] Register whole window in the item system (before submitting further decorations)
#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (g.TestEngineHookItems)
        {
            IM_ASSERT(window->IDStack.Size == 1);
            window->IDStack.Size = 0; // As window->IDStack[0] == window->ID here, make sure TestEngine doesn't erroneously see window as parent of itself.
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            IMGUI_TEST_ENGINE_ITEM_ADD(window->ID, window->Rect(), NULL);
            IMGUI_TEST_ENGINE_ITEM_INFO(window->ID, window->Name, (g.HoveredWindow == window) ? ImGuiItemStatusFlags_HoveredRect : 0);
            window->IDStack.Size = 1;
            window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

        }
#endif

        // Decide if we are going to handle borders and resize grips
        // 'window->SkipItems' is not updated yet so for child windows we rely on ParentWindow to avoid submitting decorations. (#8815)
        // Whenever we add support for full decorated child windows we will likely make this logic more general.
        bool handle_borders_and_resize_grips = (window->DockNodeAsHost || !window->DockIsActive);
        if ((flags & ImGuiWindowFlags_ChildWindow) && window->ParentWindow->SkipItems)
            handle_borders_and_resize_grips = false;

        // Handle manual resize: Resize Grips, Borders, Gamepad
        // Child windows can only be resized when they have the flags set. The resize grip allows resizing in both directions, so it should appear only if both flags are set.
        int border_hovered = -1, border_held = -1;
        ImU32 resize_grip_col[4] = {};
        int resize_grip_count;
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup))
            resize_grip_count = ((window->ChildFlags & ImGuiChildFlags_ResizeX) && (window->ChildFlags & ImGuiChildFlags_ResizeY)) ? 1 : 0;
        else
            resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.

        const float resize_grip_draw_size = IM_TRUNC(ImMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
        if (handle_borders_and_resize_grips && !window->Collapsed)
            if (int auto_fit_mask = UpdateWindowManualResize(window, &border_hovered, &border_held, resize_grip_count, &resize_grip_col[0], visibility_rect))
            {
                if (auto_fit_mask & (1 << ImGuiAxis_X))
                    use_current_size_for_scrollbar_x = true;
                if (auto_fit_mask & (1 << ImGuiAxis_Y))
                    use_current_size_for_scrollbar_y = true;
            }
        window->ResizeBorderHovered = (signed char)border_hovered;
        window->ResizeBorderHeld = (signed char)border_held;

        // Synchronize window --> viewport again and one last time (clamping and manual resize may have affected either)
        if (window->ViewportOwned)
        {
            if (!window->Viewport->PlatformRequestMove)
                window->Viewport->Pos = window->Pos;
            if (!window->Viewport->PlatformRequestResize)
                window->Viewport->Size = window->Size;
            window->Viewport->UpdateWorkRect();
            viewport_rect = window->Viewport->GetMainRect();
        }

        // Save last known viewport position within the window itself (so it can be saved in .ini file and restored)
        window->ViewportPos = window->Viewport->Pos;

        // SCROLLBAR VISIBILITY

        // Update scrollbar visibility (based on the Size that was effective during last frame or the auto-resized Size).
        if (!window->Collapsed)
        {
            // When reading the current size we need to read it after size constraints have been applied.
            // Intentionally use previous frame values for InnerRect and ScrollbarSizes.
            // And when we use window->DecorationUp here it doesn't have ScrollbarSizes.y applied yet.
            ImVec2 avail_size_from_current_frame = ImVec2(window->SizeFull.x, window->SizeFull.y - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
            ImVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + scrollbar_sizes_from_last_frame;
            ImVec2 needed_size_from_last_frame = window_just_created ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
            float size_for_scrollbars_x = use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
            float size_for_scrollbars_y = use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
            bool scrollbar_x_prev = window->ScrollbarX;
            //bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May want to use that in the ScrollbarX expression? How many pros vs cons?
            window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((needed_size_from_last_frame.y > size_for_scrollbars_y) && !(flags & ImGuiWindowFlags_NoScrollbar));
            window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((needed_size_from_last_frame.x > size_for_scrollbars_x - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));

            // Track when ScrollbarX visibility keeps toggling, which is a sign of a feedback loop, and stabilize by enforcing visibility (#3285, #8488)
            // (Feedback loops of this sort can manifest in various situations, but combining horizontal + vertical scrollbar + using a clipper with varying width items is one frequent cause.
            //  The better solution is to, either (1) enforce visibility by using ImGuiWindowFlags_AlwaysHorizontalScrollbar or (2) declare stable contents width with SetNextWindowContentSize(), if you can compute it)
            window->ScrollbarXStabilizeToggledHistory <<= 1;
            window->ScrollbarXStabilizeToggledHistory |= (scrollbar_x_prev != window->ScrollbarX) ? 0x01 : 0x00;
            const bool scrollbar_x_stabilize = (window->ScrollbarXStabilizeToggledHistory != 0) && ImCountSetBits(window->ScrollbarXStabilizeToggledHistory) >= 4; // 4 == half of bits in our U8 history.
            if (scrollbar_x_stabilize)
                window->ScrollbarX = true;
            //if (scrollbar_x_stabilize && !window->ScrollbarXStabilizeEnabled)
            //    IMGUI_DEBUG_LOG("[scroll] Stabilize ScrollbarX for Window '%s'\n", window->Name);
            window->ScrollbarXStabilizeEnabled = scrollbar_x_stabilize;

            if (window->ScrollbarX && !window->ScrollbarY)
                window->ScrollbarY = (needed_size_from_last_frame.y > size_for_scrollbars_y - style.ScrollbarSize) && !(flags & ImGuiWindowFlags_NoScrollbar);
            window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);

            // Amend the partially filled window->DecorationXXX values.
            window->DecoOuterSizeX2 += window->ScrollbarSizes.x;
            window->DecoOuterSizeY2 += window->ScrollbarSizes.y;
        }

        // UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
        // Update various regions. Variables they depend on should be set above in this function.
        // We set this up after processing the resize grip so that our rectangles doesn't lag by a frame.

        // Outer rectangle
        // Not affected by window border size. Used by:
        // - FindHoveredWindow() (w/ extra padding when border resize is enabled)
        // - Begin() initial clipping rect for drawing window background and borders.
        // - Begin() clipping whole child
        const ImRect host_rect = ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip) ? parent_window->ClipRect : viewport_rect;
        const ImRect outer_rect = window->Rect();
        const ImRect title_bar_rect = window->TitleBarRect();
        window->OuterRectClipped = outer_rect;
        if (window->DockIsActive)
            window->OuterRectClipped.Min.y += window->TitleBarHeight;
        window->OuterRectClipped.ClipWith(host_rect);

        // Inner rectangle
        // Not affected by window border size. Used by:
        // - InnerClipRect
        // - ScrollToRectEx()
        // - NavUpdatePageUpPageDown()
        // - Scrollbar()
        window->InnerRect.Min.x = window->Pos.x + window->DecoOuterSizeX1;
        window->InnerRect.Min.y = window->Pos.y + window->DecoOuterSizeY1;
        window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->DecoOuterSizeX2;
        window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->DecoOuterSizeY2;

        // Inner clipping rectangle.
        // - Extend a outside of normal work region up to borders.
        // - This is to allow e.g. Selectable or CollapsingHeader or some separators to cover that space.
        // - It also makes clipped items be more noticeable.
        // - And is consistent on both axis (prior to 2024/05/03 ClipRect used WindowPadding.x * 0.5f on left and right edge), see #3312
        // - Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
        // Note that if our window is collapsed we will end up with an inverted (~null) clipping rectangle which is the correct behavior.
        // Affected by window/frame border size. Used by:
        // - Begin() initial clip rect
        float top_border_size = (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);

        // Try to match the fact that our border is drawn centered over the window rectangle, rather than inner.
        // This is why we do a *0.5f here. We don't currently even technically support large values for WindowBorderSize,
        // see e.g #7887 #7888, but may do after we move the window border to become an inner border (and then we can remove the 0.5f here).
        window->InnerClipRect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x + window->WindowBorderSize * 0.5f);
        window->InnerClipRect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y + top_border_size * 0.5f);
        window->InnerClipRect.Max.x = ImFloor(window->InnerRect.Max.x - window->WindowBorderSize * 0.5f);
        window->InnerClipRect.Max.y = ImFloor(window->InnerRect.Max.y - window->WindowBorderSize * 0.5f);
        window->InnerClipRect.ClipWithFull(host_rect);

        // SCROLLING

        // Lock down maximum scrolling
        // The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is intentionally using InnerRect from previous rect in order to accommodate
        // for right/bottom aligned items without creating a scrollbar.
        window->ScrollMax.x = ImMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
        window->ScrollMax.y = ImMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

        // Apply scrolling
        window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
        window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
        window->DecoInnerSizeX1 = window->DecoInnerSizeY1 = 0.0f;

        // DRAWING

        // Setup draw list and outer clipping rectangle
        IM_ASSERT(window->DrawList->CmdBuffer.Size == 1 && window->DrawList->CmdBuffer[0].ElemCount == 0);
        window->DrawList->PushTexture(g.Font->OwnerAtlas->TexRef);
        PushClipRect(host_rect.Min, host_rect.Max, false);

        // Child windows can render their decoration (bg color, border, scrollbars, etc.) within their parent to save a draw call (since 1.71)
        // When using overlapping child windows, this will break the assumption that child z-order is mapped to submission order.
        // FIXME: User code may rely on explicit sorting of overlapping child window and would need to disable this somehow. Please get in contact if you are affected (github #4493)
        const bool is_undocked_or_docked_visible = !window->DockIsActive || window->DockTabIsVisible;
        if (is_undocked_or_docked_visible)
        {
            bool render_decorations_in_parent = false;
            if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
            {
                // - We test overlap with the previous child window only (testing all would end up being O(log N) not a good investment here)
                // - We disable this when the parent window has zero vertices, which is a common pattern leading to laying out multiple overlapping childs
                ImGuiWindow* previous_child = parent_window->DC.ChildWindows.Size >= 2 ? parent_window->DC.ChildWindows[parent_window->DC.ChildWindows.Size - 2] : NULL;
                bool previous_child_overlapping = previous_child ? previous_child->Rect().Overlaps(window->Rect()) : false;
                bool parent_is_empty = (parent_window->DrawList->VtxBuffer.Size == 0);
                if (window->DrawList->CmdBuffer.back().ElemCount == 0 && !parent_is_empty && !previous_child_overlapping)
                    render_decorations_in_parent = true;
            }
            if (render_decorations_in_parent)
                window->DrawList = parent_window->DrawList;

            // Handle title bar, scrollbar, resize grips and resize borders
            const ImGuiWindow* window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
            const bool title_bar_is_highlight = want_focus || (window_to_highlight && (window->RootWindowForTitleBarHighlight == window_to_highlight->RootWindowForTitleBarHighlight || (window->DockNode && window->DockNode == window_to_highlight->DockNode)));
            RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight, handle_borders_and_resize_grips, resize_grip_count, resize_grip_col, resize_grip_draw_size);

            if (render_decorations_in_parent)
                window->DrawList = &window->DrawListInst;
        }

        // UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

        // Work rectangle.
        // Affected by window padding and border size. Used by:
        // - Columns() for right-most edge
        // - TreeNode(), CollapsingHeader() for right-most edge
        // - BeginTabBar() for right-most edge
        const bool allow_scrollbar_x = !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar);
        const bool allow_scrollbar_y = !(flags & ImGuiWindowFlags_NoScrollbar);
        const float work_rect_size_x = (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : ImMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f, window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
        const float work_rect_size_y = (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : ImMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f, window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));
        window->WorkRect.Min.x = ImTrunc(window->InnerRect.Min.x - window->Scroll.x + ImMax(window->WindowPadding.x, window->WindowBorderSize));
        window->WorkRect.Min.y = ImTrunc(window->InnerRect.Min.y - window->Scroll.y + ImMax(window->WindowPadding.y, window->WindowBorderSize));
        window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
        window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
        window->ParentWorkRect = window->WorkRect;

        // [LEGACY] Content Region
        // FIXME-OBSOLETE: window->ContentRegionRect.Max is currently very misleading / partly faulty, but some BeginChild() patterns relies on it.
        // Unless explicit content size is specified by user, this currently represent the region leading to no scrolling.
        // Used by:
        // - Mouse wheel scrolling + many other things
        window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x + window->DecoOuterSizeX1;
        window->ContentRegionRect.Min.y = window->Pos.y - window->Scroll.y + window->WindowPadding.y + window->DecoOuterSizeY1;
        window->ContentRegionRect.Max.x = window->ContentRegionRect.Min.x + (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : (window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
        window->ContentRegionRect.Max.y = window->ContentRegionRect.Min.y + (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : (window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));

        // Setup drawing context
        // (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient data only. Nowadays difference between window-> and window->DC-> is dubious.)
        window->DC.Indent.x = window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
        window->DC.GroupOffset.x = 0.0f;
        window->DC.ColumnsOffset.x = 0.0f;

        // Record the loss of precision of CursorStartPos which can happen due to really large scrolling amount.
        // This is used by clipper to compensate and fix the most common use case of large scroll area. Easy and cheap, next best thing compared to switching everything to double or ImU64.
        double start_pos_highp_x = (double)window->Pos.x + window->WindowPadding.x - (double)window->Scroll.x + window->DecoOuterSizeX1 + window->DC.ColumnsOffset.x;
        double start_pos_highp_y = (double)window->Pos.y + window->WindowPadding.y - (double)window->Scroll.y + window->DecoOuterSizeY1;
        window->DC.CursorStartPos  = ImVec2((float)start_pos_highp_x, (float)start_pos_highp_y);
        window->DC.CursorStartPosLossyness = ImVec2((float)(start_pos_highp_x - window->DC.CursorStartPos.x), (float)(start_pos_highp_y - window->DC.CursorStartPos.y));
        window->DC.CursorPos = window->DC.CursorStartPos;
        window->DC.CursorPosPrevLine = window->DC.CursorPos;
        window->DC.CursorMaxPos = window->DC.CursorStartPos;
        window->DC.IdealMaxPos = window->DC.CursorStartPos;
        window->DC.CurrLineSize = window->DC.PrevLineSize = ImVec2(0.0f, 0.0f);
        window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
        window->DC.IsSameLine = window->DC.IsSetPos = false;

        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
        window->DC.NavLayersActiveMaskNext = 0x00;
        window->DC.NavIsScrollPushableX = true;
        window->DC.NavHideHighlightOneFrame = false;
        window->DC.NavWindowHasScrollY = (window->ScrollMax.y > 0.0f);

        window->DC.MenuBarAppending = false;
        window->DC.MenuColumns.Update(style.ItemSpacing.x, window_just_activated_by_user);
        window->DC.TreeDepth = 0;
        window->DC.TreeHasStackDataDepthMask = window->DC.TreeRecordsClippedNodesY2Mask = 0x00;
        window->DC.ChildWindows.resize(0);
        window->DC.StateStorage = &window->StateStorage;
        window->DC.CurrentColumns = NULL;
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;

        // Default item width. Make it proportional to window size if window can be manually resized.
        // (we cannot use AutoFitFramesX/AutoFitFramesY which is a temporary state)
        bool is_resizable_width;
        if (flags & ImGuiWindowFlags_ChildWindow)
            is_resizable_width = (window->Size.x > 0.0f) && !(window->ChildFlags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysAutoResize));
        else
            is_resizable_width = (window->Size.x > 0.0f) && !(flags & ImGuiWindowFlags_AlwaysAutoResize);
        if (is_resizable_width)
            window->DC.ItemWidthDefault = ImTrunc(window->Size.x * 0.65f);
        else
            window->DC.ItemWidthDefault = ImTrunc(g.FontSize * 16.0f);
        window->DC.ItemWidth = window->DC.ItemWidthDefault;
        window->DC.ItemWidthStack.resize(0);
        window->DC.TextWrapPos = -1.0f; // Disabled
        window->DC.TextWrapPosStack.resize(0);
        if (flags & ImGuiWindowFlags_Modal)
            window->DC.ModalDimBgColor = ColorConvertFloat4ToU32(GetStyleColorVec4(ImGuiCol_ModalWindowDimBg));

        if (window->AutoFitFramesX > 0)
            window->AutoFitFramesX--;
        if (window->AutoFitFramesY > 0)
            window->AutoFitFramesY--;

        // Clear SetNextWindowXXX data (can aim to move this higher in the function)
        g.NextWindowData.ClearFlags();

        // Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
        // We ImGuiFocusRequestFlags_UnlessBelowModal to:
        // - Avoid focusing a window that is created outside of a modal. This will prevent active modal from being closed.
        // - Position window behind the modal that is not a begin-parent of this window.
        if (want_focus)
            FocusWindow(window, ImGuiFocusRequestFlags_UnlessBelowModal);
        if (want_focus && window == g.NavWindow)
            NavInitWindow(window, false); // <-- this is in the way for us to be able to defer and sort reappearing FocusWindow() calls

        // Close requested by platform window (apply to all windows in this viewport)
        // FIXME: Investigate removing the 'window->Viewport != GetMainViewport()' test, which seems superfluous.
        if (p_open != NULL && window->Viewport->PlatformRequestClose && window->Viewport != GetMainViewport())
            if (window->DockNode == NULL || (window->DockNode->MergedFlags & ImGuiDockNodeFlags_DockSpace) == 0)
            {
                IMGUI_DEBUG_LOG_VIEWPORT("[viewport] Window '%s' closed by PlatformRequestClose\n", window->Name);
                *p_open = false;
                g.NavWindowingToggleLayer = false; // Assume user mapped PlatformRequestClose on Alt-F4 so we disable Alt for menu toggle. False positive not an issue. // FIXME-NAV: Try removing.
            }

        // Pressing Ctrl+C copy window content into the clipboard
        // [EXPERIMENTAL] Breaks on nested Begin/End pairs. We need to work that out and add better logging scope.
        // [EXPERIMENTAL] Text outputs has many issues.
        if (g.IO.ConfigWindowsCopyContentsWithCtrlC)
            if (g.NavWindow && g.NavWindow->RootWindow == window && g.ActiveId == 0 && Shortcut(ImGuiMod_Ctrl | ImGuiKey_C))
                LogToClipboard(0);

        // Title bar
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
            RenderWindowTitleBarContents(window, ImRect(title_bar_rect.Min.x + window->WindowBorderSize, title_bar_rect.Min.y, title_bar_rect.Max.x - window->WindowBorderSize, title_bar_rect.Max.y), name, p_open);
        else if (!(flags & ImGuiWindowFlags_NoTitleBar) && window->DockIsActive)
            LogText("%.*s\n", (int)(FindRenderedTextEnd(window->Name) - window->Name), window->Name);

        // Clear hit test shape every frame
        window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

        if (flags & ImGuiWindowFlags_Tooltip)
            g.TooltipPreviousWindow = window;

        if (g.IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            // Docking: Dragging a dockable window (or any of its child) turns it into a drag and drop source.
            // We need to do this _before_ we overwrite window->DC.LastItemId below because BeginDockableDragDropSource() also overwrites it.
            if (g.MovingWindow == window && (window->RootWindowDockTree->Flags & ImGuiWindowFlags_NoDocking) == 0)
                BeginDockableDragDropSource(window);

            // Docking: Any dockable window can act as a target. For dock node hosts we call BeginDockableDragDropTarget() in DockNodeUpdate() instead.
            if (g.DragDropActive && !(flags & ImGuiWindowFlags_NoDocking))
                if (g.MovingWindow == NULL || g.MovingWindow->RootWindowDockTree != window)
                    if ((window == window->RootWindowDockTree) && !(window->Flags & ImGuiWindowFlags_DockNodeHost))
                        BeginDockableDragDropTarget(window);
        }

        // Set default BgClickFlags
        // This is set at the end of this function, so UpdateWindowManualResize()/ClampWindowPos() may use last-frame value if overridden by user code.
        // FIXME: The general intent is that we will later expose config options to default to enable scrolling + select scrolling mouse button.
        window->BgClickFlags = (flags & ImGuiWindowFlags_ChildWindow) ? parent_window->BgClickFlags : (g.IO.ConfigWindowsMoveFromTitleBarOnly ? ImGuiWindowBgClickFlags_None : ImGuiWindowBgClickFlags_Move);

        // We fill last item data based on Title Bar/Tab, in order for IsItemHovered() and IsItemActive() to be usable after Begin().
        // This is useful to allow creating context menus on title bar only, etc.
        window->DC.WindowItemStatusFlags = ImGuiItemStatusFlags_None;
        window->DC.WindowItemStatusFlags |= IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? ImGuiItemStatusFlags_HoveredRect : 0;
        SetLastItemDataForWindow(window, title_bar_rect);

        // [DEBUG]
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
        if (g.DebugLocateId != 0 && (window->ID == g.DebugLocateId || window->MoveId == g.DebugLocateId))
            DebugLocateItemResolveWithLastItem();
#endif

        // [Test Engine] Register title bar / tab with MoveId.
#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
        {
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            IMGUI_TEST_ENGINE_ITEM_ADD(g.LastItemData.ID, g.LastItemData.Rect, &g.LastItemData);
            window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        }
#endif
    }
    else
    {
        // Skip refresh always mark active
        if (window->SkipRefresh)
            SetWindowActiveForSkipRefresh(window);

        // Append
        SetCurrentViewport(window, window->Viewport);
        SetCurrentWindow(window);
        g.NextWindowData.ClearFlags();
        SetLastItemDataForWindow(window, window->TitleBarRect());
    }

    if (!(flags & ImGuiWindowFlags_DockNodeHost) && !window->SkipRefresh)
        PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

    // Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when the default "Debug" window is unused)
    window->WriteAccessed = false;
    window->BeginCount++;

    // Update visibility
    if (first_begin_of_the_frame && !window->SkipRefresh)
    {
        // When we are about to select this tab (which will only be visible on the _next frame_), flag it with a non-zero HiddenFramesCannotSkipItems.
        // This will have the important effect of actually returning true in Begin() and not setting SkipItems, allowing an earlier submission of the window contents.
        // This is analogous to regular windows being hidden from one frame.
        // It is especially important as e.g. nested TabBars would otherwise generate flicker in the form of one empty frame, or focus requests won't be processed.
        if (window->DockIsActive && !window->DockTabIsVisible)
        {
            if (window->LastFrameJustFocused == g.FrameCount)
                window->HiddenFramesCannotSkipItems = 1;
            else
                window->HiddenFramesCanSkipItems = 1;
        }

        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_ChildMenu))
        {
            // Child window can be out of sight and have "negative" clip windows.
            // Mark them as collapsed so commands are skipped earlier (we can't manually collapse them because they have no title bar).
            IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0 || window->DockIsActive);
            const bool nav_request = (window->ChildFlags & ImGuiChildFlags_NavFlattened) && (g.NavAnyRequest && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
            if (!g.LogEnabled && !nav_request)
                if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x || window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
                {
                    if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
                        window->HiddenFramesCannotSkipItems = 1;
                    else
                        window->HiddenFramesCanSkipItems = 1;
                }

            // Hide along with parent or if parent is collapsed
            if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
                window->HiddenFramesCanSkipItems = 1;
            if (parent_window && parent_window->HiddenFramesCannotSkipItems > 0)
                window->HiddenFramesCannotSkipItems = 1;
        }

        // Don't render if style alpha is 0.0 at the time of Begin(). This is arbitrary and inconsistent but has been there for a long while (may remove at some point)
        if (style.Alpha <= 0.0f)
            window->HiddenFramesCanSkipItems = 1;

        // Update the Hidden flag
        bool hidden_regular = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0);
        window->Hidden = hidden_regular || (window->HiddenFramesForRenderOnly > 0);

        // Disable inputs for requested number of frames
        if (window->DisableInputsFrames > 0)
        {
            window->DisableInputsFrames--;
            window->Flags |= ImGuiWindowFlags_NoInputs;
        }

        // Update the SkipItems flag, used to early out of all items functions (no layout required)
        bool skip_items = false;
        if (window->Collapsed || !window->Active || hidden_regular)
            if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
                skip_items = true;
        window->SkipItems = skip_items;

        // Restore NavLayersActiveMaskNext to previous value when not visible, so a CTRL+Tab back can use a safe value.
        if (window->SkipItems)
            window->DC.NavLayersActiveMaskNext = window->DC.NavLayersActiveMask;

        // Sanity check: there are two spots which can set Appearing = true
        // - when 'window_just_activated_by_user' is set -> HiddenFramesCannotSkipItems is set -> SkipItems always false
        // - in BeginDocked() path when DockNodeIsVisible == DockTabIsVisible == true -> hidden _should_ be all zero // FIXME: Not formally proven, hence the assert.
        if (window->SkipItems && !window->Appearing)
            IM_ASSERT(window->Appearing == false); // Please report on GitHub if this triggers: https://github.com/ocornut/imgui/issues/4177
    }
    else if (first_begin_of_the_frame)
    {
        // Skip refresh mode
        window->SkipItems = true;
    }

    // [DEBUG] io.ConfigDebugBeginReturnValue override return value to test Begin/End and BeginChild/EndChild behaviors.
    // (The implicit fallback window is NOT automatically ended allowing it to always be able to receive commands without crashing)
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (!window->IsFallbackWindow)
        if ((g.IO.ConfigDebugBeginReturnValueOnce && window_just_created) || (g.IO.ConfigDebugBeginReturnValueLoop && g.DebugBeginReturnValueCullDepth == g.CurrentWindowStack.Size))
        {
            if (window->AutoFitFramesX > 0) { window->AutoFitFramesX++; }
            if (window->AutoFitFramesY > 0) { window->AutoFitFramesY++; }
            return false;
        }
#endif

    return !window->SkipItems;
}

void ImGui::End()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Error checking: verify that user hasn't called End() too many times!
    if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow)
    {
        IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size > 1, "Calling End() too many times!");
        return;
    }
    ImGuiWindowStackData& window_stack_data = g.CurrentWindowStack.back();

    // Error checking: verify that user doesn't directly call End() on a child window.
    if (window->Flags & ImGuiWindowFlags_Popup)
        IM_ASSERT_USER_ERROR(g.WithinEndPopupID == window->ID, "Must call EndPopup() and not End()!");
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_DockNodeHost) && !window->DockIsActive)
        IM_ASSERT_USER_ERROR(g.WithinEndChildID == window->ID, "Must call EndChild() and not End()!");

    // Close anything that is open
    if (window->DC.CurrentColumns)
        EndColumns();
    if (!(window->Flags & ImGuiWindowFlags_DockNodeHost) && !window->SkipRefresh)   // Pop inner window clip rectangle
        PopClipRect();
    PopFocusScope();
    if (window_stack_data.DisabledOverrideReenable && window->RootWindow == window)
        EndDisabledOverrideReenable();

    if (window->SkipRefresh)
    {
        IM_ASSERT(window->DrawList == NULL);
        window->DrawList = &window->DrawListInst;
    }

    // Stop logging
    if (g.LogWindow == window) // FIXME: add more options for scope of logging
        LogFinish();

    if (window->DC.IsSetPos)
        ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

    // Docking: report contents sizes to parent to allow for auto-resize
    if (window->DockNode && window->DockTabIsVisible)
        if (ImGuiWindow* host_window = window->DockNode->HostWindow)         // FIXME-DOCK
            host_window->DC.CursorMaxPos = window->DC.CursorMaxPos + window->WindowPadding - host_window->WindowPadding;

    // Pop from window stack
    g.LastItemData = window_stack_data.ParentLastItemDataBackup;
    if (window->Flags & ImGuiWindowFlags_ChildMenu)
        g.BeginMenuDepth--;
    if (window->Flags & ImGuiWindowFlags_Popup)
        g.BeginPopupStack.pop_back();

    // Error handling, state recovery
    if (g.IO.ConfigErrorRecovery)
        ErrorRecoveryTryToRecoverWindowState(&window_stack_data.StackSizesInBegin);

    g.CurrentWindowStack.pop_back();
    SetCurrentWindow(g.CurrentWindowStack.Size == 0 ? NULL : g.CurrentWindowStack.back().Window);
    if (g.CurrentWindow)
        SetCurrentViewport(g.CurrentWindow, g.CurrentWindow->Viewport);
}

void ImGui::PushItemFlag(ImGuiItemFlags option, bool enabled)
{
    ImGuiContext& g = *GImGui;
    ImGuiItemFlags item_flags = g.CurrentItemFlags;
    IM_ASSERT(item_flags == g.ItemFlagsStack.back());
    if (enabled)
        item_flags |= option;
    else
        item_flags &= ~option;
    g.CurrentItemFlags = item_flags;
    g.ItemFlagsStack.push_back(item_flags);
}

void ImGui::PopItemFlag()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.ItemFlagsStack.Size > 1, "Calling PopItemFlag() too many times!");
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
}

// BeginDisabled()/EndDisabled()
// - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
// - Visually this is currently altering alpha, but it is expected that in a future styling system this would work differently.
// - Feedback welcome at https://github.com/ocornut/imgui/issues/211
// - BeginDisabled(false)/EndDisabled() essentially does nothing but is provided to facilitate use of boolean expressions.
//   (as a micro-optimization: if you have tens of thousands of BeginDisabled(false)/EndDisabled() pairs, you might want to reformulate your code to avoid making those calls)
// - Note: mixing up BeginDisabled() and PushItemFlag(ImGuiItemFlags_Disabled) is currently NOT SUPPORTED.
void ImGui::BeginDisabled(bool disabled)
{
    ImGuiContext& g = *GImGui;
    bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (!was_disabled && disabled)
    {
        g.DisabledAlphaBackup = g.Style.Alpha;
        g.Style.Alpha *= g.Style.DisabledAlpha; // PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * g.Style.DisabledAlpha);
    }
    if (was_disabled || disabled)
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled;
    g.ItemFlagsStack.push_back(g.CurrentItemFlags); // FIXME-OPT: can we simply skip this and use DisabledStackSize?
    g.DisabledStackSize++;
}

void ImGui::EndDisabled()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.DisabledStackSize > 0, "Calling EndDisabled() too many times!");
    g.DisabledStackSize--;
    bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    //PopItemFlag();
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    if (was_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
        g.Style.Alpha = g.DisabledAlphaBackup; //PopStyleVar();
}

// Could have been called BeginDisabledDisable() but it didn't want to be award nominated for most awkward function name.
// Ideally we would use a shared e.g. BeginDisabled()->BeginDisabledEx() but earlier needs to be optimal.
// The whole code for this is awkward, will reevaluate if we find a way to implement SetNextItemDisabled().
void ImGui::BeginDisabledOverrideReenable()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentItemFlags & ImGuiItemFlags_Disabled);
    g.CurrentWindowStack.back().DisabledOverrideReenableAlphaBackup = g.Style.Alpha;
    g.Style.Alpha = g.DisabledAlphaBackup;
    g.CurrentItemFlags &= ~ImGuiItemFlags_Disabled;
    g.ItemFlagsStack.push_back(g.CurrentItemFlags);
    g.DisabledStackSize++;
}

void ImGui::EndDisabledOverrideReenable()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DisabledStackSize > 0);
    g.DisabledStackSize--;
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    g.Style.Alpha = g.CurrentWindowStack.back().DisabledOverrideReenableAlphaBackup;
}

// ATTENTION THIS IS IN LEGACY LOCAL SPACE.
void ImGui::PushTextWrapPos(float wrap_local_pos_x)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DC.TextWrapPosStack.push_back(window->DC.TextWrapPos);
    window->DC.TextWrapPos = wrap_local_pos_x;
}

void ImGui::PopTextWrapPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR_RET(window->DC.TextWrapPosStack.Size > 0, "Calling PopTextWrapPos() too many times!");
    window->DC.TextWrapPos = window->DC.TextWrapPosStack.back();
    window->DC.TextWrapPosStack.pop_back();
}

static ImGuiWindow* GetCombinedRootWindow(ImGuiWindow* window, bool popup_hierarchy, bool dock_hierarchy)
{
    ImGuiWindow* last_window = NULL;
    while (last_window != window)
    {
        last_window = window;
        window = window->RootWindow;
        if (popup_hierarchy)
            window = window->RootWindowPopupTree;
		if (dock_hierarchy)
			window = window->RootWindowDockTree;
	}
    return window;
}

bool ImGui::IsWindowChildOf(ImGuiWindow* window, ImGuiWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy)
{
    ImGuiWindow* window_root = GetCombinedRootWindow(window, popup_hierarchy, dock_hierarchy);
    if (window_root == potential_parent)
        return true;
    while (window != NULL)
    {
        if (window == potential_parent)
            return true;
        if (window == window_root) // end of chain
            return false;
        window = window->ParentWindow;
    }
    return false;
}

bool ImGui::IsWindowInBeginStack(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    for (int n = g.CurrentWindowStack.Size - 1; n >= 0; n--)
        if (g.CurrentWindowStack[n].Window == window)
            return true;
    return false;
}

bool ImGui::IsWindowWithinBeginStackOf(ImGuiWindow* window, ImGuiWindow* potential_parent)
{
    if (window->RootWindow == potential_parent)
        return true;
    while (window != NULL)
    {
        if (window == potential_parent)
            return true;
        window = window->ParentWindowInBeginStack;
    }
    return false;
}

bool ImGui::IsWindowAbove(ImGuiWindow* potential_above, ImGuiWindow* potential_below)
{
    ImGuiContext& g = *GImGui;

    // It would be saner to ensure that display layer is always reflected in the g.Windows[] order, which would likely requires altering all manipulations of that array
    const int display_layer_delta = GetWindowDisplayLayer(potential_above) - GetWindowDisplayLayer(potential_below);
    if (display_layer_delta != 0)
        return display_layer_delta > 0;

    for (int i = g.Windows.Size - 1; i >= 0; i--)
    {
        ImGuiWindow* candidate_window = g.Windows[i];
        if (candidate_window == potential_above)
            return true;
        if (candidate_window == potential_below)
            return false;
    }
    return false;
}

// Is current window hovered and hoverable (e.g. not blocked by a popup/modal)? See ImGuiHoveredFlags_ for options.
// IMPORTANT: If you are trying to check whether your mouse should be dispatched to Dear ImGui or to your underlying app,
// you should not use this function! Use the 'io.WantCaptureMouse' boolean for that!
// Refer to FAQ entry "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?" for details.
bool ImGui::IsWindowHovered(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR((flags & ~ImGuiHoveredFlags_AllowedMaskForIsWindowHovered) == 0, "Invalid flags for IsWindowHovered()!");

    ImGuiWindow* ref_window = g.HoveredWindow;
    ImGuiWindow* cur_window = g.CurrentWindow;
    if (ref_window == NULL)
        return false;

    if ((flags & ImGuiHoveredFlags_AnyWindow) == 0)
    {
        IM_ASSERT(cur_window); // Not inside a Begin()/End()
        const bool popup_hierarchy = (flags & ImGuiHoveredFlags_NoPopupHierarchy) == 0;
        const bool dock_hierarchy = (flags & ImGuiHoveredFlags_DockHierarchy) != 0;
        if (flags & ImGuiHoveredFlags_RootWindow)
            cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

        bool result;
        if (flags & ImGuiHoveredFlags_ChildWindows)
            result = IsWindowChildOf(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
        else
            result = (ref_window == cur_window);
        if (!result)
            return false;
    }

    if (!IsWindowContentHoverable(ref_window, flags))
        return false;
    if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != ref_window->MoveId)
            return false;

    // When changing hovered window we requires a bit of stationary delay before activating hover timer.
    // FIXME: We don't support delay other than stationary one for now, other delay would need a way
    // to fulfill the possibility that multiple IsWindowHovered() with varying flag could return true
    // for different windows of the hierarchy. Possibly need a Hash(Current+Flags) ==> (Timer) cache.
    // We can implement this for _Stationary because the data is linked to HoveredWindow rather than CurrentWindow.
    if (flags & ImGuiHoveredFlags_ForTooltip)
        flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);
    if ((flags & ImGuiHoveredFlags_Stationary) != 0 && g.HoverWindowUnlockedStationaryId != ref_window->ID)
        return false;

    return true;
}

ImGuiID ImGui::GetWindowDockID()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentWindow->DockId;
}

bool ImGui::IsWindowDocked()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentWindow->DockIsActive;
}

float ImGui::GetWindowWidth()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.x;
}

float ImGui::GetWindowHeight()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.y;
}

ImVec2 ImGui::GetWindowPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    return window->Pos;
}

void ImGui::SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
    window->SetWindowPosVal = ImVec2(FLT_MAX, FLT_MAX);

    // Set
    const ImVec2 old_pos = window->Pos;
    window->Pos = ImTrunc(pos);
    ImVec2 offset = window->Pos - old_pos;
    if (offset.x == 0.0f && offset.y == 0.0f)
        return;
    MarkIniSettingsDirty(window);
    // FIXME: share code with TranslateWindow(), need to confirm whether the 3 rect modified by TranslateWindow() are desirable here.
    window->DC.CursorPos += offset;         // As we happen to move the window while it is being appended to (which is a bad idea - will smear) let's at least offset the cursor
    window->DC.CursorMaxPos += offset;      // And more importantly we need to offset CursorMaxPos/CursorStartPos this so ContentSize calculation doesn't get affected.
    window->DC.IdealMaxPos += offset;
    window->DC.CursorStartPos += offset;
}

void ImGui::SetWindowPos(const ImVec2& pos, ImGuiCond cond)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    SetWindowPos(window, pos, cond);
}

void ImGui::SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowPos(window, pos, cond);
}

ImVec2 ImGui::GetWindowSize()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Size;
}

void ImGui::SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    window->SetWindowSizeAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Enable auto-fit (not done in BeginChild() path unless appearing or combined with ImGuiChildFlags_AlwaysAutoResize)
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || window->Appearing || (window->ChildFlags & ImGuiChildFlags_AlwaysAutoResize) != 0)
        window->AutoFitFramesX = (size.x <= 0.0f) ? 2 : 0;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || window->Appearing || (window->ChildFlags & ImGuiChildFlags_AlwaysAutoResize) != 0)
        window->AutoFitFramesY = (size.y <= 0.0f) ? 2 : 0;

    // Set
    ImVec2 old_size = window->SizeFull;
    if (size.x <= 0.0f)
        window->AutoFitOnlyGrows = false;
    else
        window->SizeFull.x = IM_TRUNC(size.x);
    if (size.y <= 0.0f)
        window->AutoFitOnlyGrows = false;
    else
        window->SizeFull.y = IM_TRUNC(size.y);
    if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
        MarkIniSettingsDirty(window);
}

void ImGui::SetWindowSize(const ImVec2& size, ImGuiCond cond)
{
    SetWindowSize(GImGui->CurrentWindow, size, cond);
}

void ImGui::SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowSize(window, size, cond);
}

void ImGui::SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
        return;
    window->SetWindowCollapsedAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Queue applying in Begin()
    if (window->WantCollapseToggle)
        window->Collapsed ^= 1;
    window->WantCollapseToggle = (window->Collapsed != collapsed);
}

void ImGui::SetWindowHitTestHole(ImGuiWindow* window, const ImVec2& pos, const ImVec2& size)
{
    IM_ASSERT(window->HitTestHoleSize.x == 0);     // We don't support multiple holes/hit test filters
    window->HitTestHoleSize = ImVec2ih(size);
    window->HitTestHoleOffset = ImVec2ih(pos - window->Pos);
}

void ImGui::SetWindowHiddenAndSkipItemsForCurrentFrame(ImGuiWindow* window)
{
    window->Hidden = window->SkipItems = true;
    window->HiddenFramesCanSkipItems = 1;
}

void ImGui::SetWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    SetWindowCollapsed(GImGui->CurrentWindow, collapsed, cond);
}

bool ImGui::IsWindowCollapsed()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Collapsed;
}

bool ImGui::IsWindowAppearing()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Appearing;
}

void ImGui::SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowCollapsed(window, collapsed, cond);
}

void ImGui::SetNextWindowPos(const ImVec2& pos, ImGuiCond cond, const ImVec2& pivot)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasPos;
    g.NextWindowData.PosVal = pos;
    g.NextWindowData.PosPivotVal = pivot;
    g.NextWindowData.PosCond = cond ? cond : ImGuiCond_Always;
    g.NextWindowData.PosUndock = true;
}

void ImGui::SetNextWindowSize(const ImVec2& size, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasSize;
    g.NextWindowData.SizeVal = size;
    g.NextWindowData.SizeCond = cond ? cond : ImGuiCond_Always;
}

// For each axis:
// - Use 0.0f as min or FLT_MAX as max if you don't want limits, e.g. size_min = (500.0f, 0.0f), size_max = (FLT_MAX, FLT_MAX) sets a minimum width.
// - Use -1 for both min and max of same axis to preserve current size which itself is a constraint.
// - See "Demo->Examples->Constrained-resizing window" for examples.
void ImGui::SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback, void* custom_callback_user_data)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
    g.NextWindowData.SizeConstraintRect = ImRect(size_min, size_max);
    g.NextWindowData.SizeCallback = custom_callback;
    g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

// Content size = inner scrollable rectangle, padded with WindowPadding.
// SetNextWindowContentSize(ImVec2(100,100)) + ImGuiWindowFlags_AlwaysAutoResize will always allow submitting a 100x100 item.
void ImGui::SetNextWindowContentSize(const ImVec2& size)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasContentSize;
    g.NextWindowData.ContentSizeVal = ImTrunc(size);
}

void ImGui::SetNextWindowScroll(const ImVec2& scroll)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasScroll;
    g.NextWindowData.ScrollVal = scroll;
}

void ImGui::SetNextWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasCollapsed;
    g.NextWindowData.CollapsedVal = collapsed;
    g.NextWindowData.CollapsedCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowBgAlpha(float alpha)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasBgAlpha;
    g.NextWindowData.BgAlphaVal = alpha;
}

void ImGui::SetNextWindowViewport(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasViewport;
    g.NextWindowData.ViewportId = id;
}

void ImGui::SetNextWindowDockID(ImGuiID id, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasDock;
    g.NextWindowData.DockCond = cond ? cond : ImGuiCond_Always;
    g.NextWindowData.DockId = id;
}

void ImGui::SetNextWindowClass(const ImGuiWindowClass* window_class)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT((window_class->ViewportFlagsOverrideSet & window_class->ViewportFlagsOverrideClear) == 0); // Cannot set both set and clear for the same bit
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasWindowClass;
    g.NextWindowData.WindowClass = *window_class;
}

// This is experimental and meant to be a toy for exploring a future/wider range of features.
void ImGui::SetNextWindowRefreshPolicy(ImGuiWindowRefreshFlags flags)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasRefreshPolicy;
    g.NextWindowData.RefreshFlagsVal = flags;
}

ImDrawList* ImGui::GetWindowDrawList()
{
    ImGuiWindow* window = GetCurrentWindow();
    return window->DrawList;
}

float ImGui::GetWindowDpiScale()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentDpiScale;
}

ImGuiViewport* ImGui::GetWindowViewport()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentViewport != NULL && g.CurrentViewport == g.CurrentWindow->Viewport);
    return g.CurrentViewport;
}

ImFont* ImGui::GetFont()
{
    return GImGui->Font;
}

ImFontBaked* ImGui::GetFontBaked()
{
    return GImGui->FontBaked;
}

// Get current font size (= height in pixels) of current font, with global scale factors applied.
// - Use style.FontSizeBase to get value before global scale factors.
// - recap: ImGui::GetFontSize() == style.FontSizeBase * (style.FontScaleMain * style.FontScaleDpi * other_scaling_factors)
float ImGui::GetFontSize()
{
    return GImGui->FontSize;
}

ImVec2 ImGui::GetFontTexUvWhitePixel()
{
    return GImGui->DrawListSharedData.TexUvWhitePixel;
}

// Prefer using PushFont(NULL, style.FontSizeBase * factor), or use style.FontScaleMain to scale all windows.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
void ImGui::SetWindowFontScale(float scale)
{
    IM_ASSERT(scale > 0.0f);
    ImGuiWindow* window = GetCurrentWindow();
    window->FontWindowScale = scale;
    UpdateCurrentFontSize(0.0f);
}
#endif

void ImGui::PushFocusScope(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiFocusScopeData data;
    data.ID = id;
    data.WindowID = g.CurrentWindow->ID;
    g.FocusScopeStack.push_back(data);
    g.CurrentFocusScopeId = id;
}

void ImGui::PopFocusScope()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.FocusScopeStack.Size > g.StackSizesInBeginForCurrentWindow->SizeOfFocusScopeStack, "Calling PopFocusScope() too many times!");
    g.FocusScopeStack.pop_back();
    g.CurrentFocusScopeId = g.FocusScopeStack.Size ? g.FocusScopeStack.back().ID : 0;
}

bool ImGui::IsInNavFocusRoute(ImGuiID focus_scope_id)
{
    ImGuiContext& g = *GImGui;
    if (g.NavFocusScopeId == focus_scope_id)
        return true;
    for (const ImGuiFocusScopeData& focus_scope : g.NavFocusRoute)
        if (focus_scope.ID == focus_scope_id)
            return true;
    return false;
}

void ImGui::SetNavFocusScope(ImGuiID focus_scope_id)
{
    ImGuiContext& g = *GImGui;
    g.NavFocusScopeId = focus_scope_id;
    g.NavFocusRoute.resize(0); // Invalidate
    if (focus_scope_id == 0)
        return;
    IM_ASSERT(g.NavWindow != NULL);

    // Store current path (in reverse order)
    if (focus_scope_id == g.CurrentFocusScopeId)
    {
        // Top of focus stack contains local focus scopes inside current window
        for (int n = g.FocusScopeStack.Size - 1; n >= 0 && g.FocusScopeStack.Data[n].WindowID == g.CurrentWindow->ID; n--)
            g.NavFocusRoute.push_back(g.FocusScopeStack.Data[n]);
    }
    else if (focus_scope_id == g.NavWindow->NavRootFocusScopeId)
        g.NavFocusRoute.push_back({ focus_scope_id, g.NavWindow->ID });
    else
        return;

    // Then follow on manually set ParentWindowForFocusRoute field (#6798)
    for (ImGuiWindow* window = g.NavWindow->ParentWindowForFocusRoute; window != NULL; window = window->ParentWindowForFocusRoute)
        g.NavFocusRoute.push_back({ window->NavRootFocusScopeId, window->ID });
    IM_ASSERT(g.NavFocusRoute.Size < 100); // Maximum depth is technically 251 as per CalcRoutingScore(): 254 - 3
}

// Focus = move navigation cursor, set scrolling, set focus window.
void ImGui::FocusItem()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IMGUI_DEBUG_LOG_FOCUS("FocusItem(0x%08x) in window \"%s\"\n", g.LastItemData.ID, window->Name);
    if (g.DragDropActive || g.MovingWindow != NULL) // FIXME: Opt-in flags for this?
    {
        IMGUI_DEBUG_LOG_FOCUS("FocusItem() ignored while DragDropActive!\n");
        return;
    }

    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi | ImGuiNavMoveFlags_NoSetNavCursorVisible | ImGuiNavMoveFlags_NoSelect;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    SetNavWindow(window);
    NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_Up, move_flags, scroll_flags);
    NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
}

void ImGui::ActivateItemByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.NavNextActivateId = id;
    g.NavNextActivateFlags = ImGuiActivateFlags_None;
}

// Note: this will likely be called ActivateItem() once we rework our Focus/Activation system!
// But ActivateItem() should function without altering scroll/focus?
void ImGui::SetKeyboardFocusHere(int offset)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(offset >= -1);    // -1 is allowed but not below
    IMGUI_DEBUG_LOG_FOCUS("SetKeyboardFocusHere(%d) in window \"%s\"\n", offset, window->Name);

    // It makes sense in the vast majority of cases to never interrupt a drag and drop.
    // When we refactor this function into ActivateItem() we may want to make this an option.
    // MovingWindow is protected from most user inputs using SetActiveIdUsingNavAndKeys(), but
    // is also automatically dropped in the event g.ActiveId is stolen.
    if (g.DragDropActive || g.MovingWindow != NULL)
    {
        IMGUI_DEBUG_LOG_FOCUS("SetKeyboardFocusHere() ignored while DragDropActive!\n");
        return;
    }

    SetNavWindow(window);

    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_Activate | ImGuiNavMoveFlags_FocusApi | ImGuiNavMoveFlags_NoSetNavCursorVisible;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    NavMoveRequestSubmit(ImGuiDir_None, offset < 0 ? ImGuiDir_Up : ImGuiDir_Down, move_flags, scroll_flags); // FIXME-NAV: Once we refactor tabbing, add LegacyApi flag to not activate non-inputable.
    if (offset == -1)
    {
        NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
    }
    else
    {
        g.NavTabbingDir = 1;
        g.NavTabbingCounter = offset + 1;
    }
}

void ImGui::SetItemDefaultFocus()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!window->Appearing)
        return;
    if (g.NavWindow != window->RootWindowForNav || (!g.NavInitRequest && g.NavInitResult.ID == 0) || g.NavLayer != window->DC.NavLayerCurrent)
        return;

    g.NavInitRequest = false;
    NavApplyItemToResult(&g.NavInitResult);
    NavUpdateAnyRequestFlag();

    // Scroll could be done in NavInitRequestApplyResult() via an opt-in flag (we however don't want regular init requests to scroll)
    if (!window->ClipRect.Contains(g.LastItemData.Rect))
        ScrollToRectEx(window, g.LastItemData.Rect, ImGuiScrollFlags_None);
}

void ImGui::SetStateStorage(ImGuiStorage* tree)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

ImGuiStorage* ImGui::GetStateStorage()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->DC.StateStorage;
}

bool ImGui::IsRectVisible(const ImVec2& size)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ClipRect.Overlaps(ImRect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool ImGui::IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ClipRect.Overlaps(ImRect(rect_min, rect_max));
}

//-----------------------------------------------------------------------------
// [SECTION] FONTS, TEXTURES
//-----------------------------------------------------------------------------
// Most of the relevant font logic is in imgui_draw.cpp.
// Those are high-level support functions.
//-----------------------------------------------------------------------------
// - UpdateTexturesNewFrame() [Internal]
// - UpdateTexturesEndFrame() [Internal]
// - UpdateFontsNewFrame() [Internal]
// - UpdateFontsEndFrame() [Internal]
// - GetDefaultFont() [Internal]
// - RegisterUserTexture() [Internal]
// - UnregisterUserTexture() [Internal]
// - RegisterFontAtlas() [Internal]
// - UnregisterFontAtlas() [Internal]
// - SetCurrentFont() [Internal]
// - UpdateCurrentFontSize() [Internal]
// - SetFontRasterizerDensity() [Internal]
// - PushFont()
// - PopFont()
//-----------------------------------------------------------------------------

static void ImGui::UpdateTexturesNewFrame()
{
    // Cannot update every atlases based on atlas's FrameCount < g.FrameCount, because an atlas may be shared by multiple contexts with different frame count.
    ImGuiContext& g = *GImGui;
    const bool has_textures = (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures) != 0;
    for (ImFontAtlas* atlas : g.FontAtlases)
    {
        if (atlas->OwnerContext == &g)
        {
            ImFontAtlasUpdateNewFrame(atlas, g.FrameCount, has_textures);
        }
        else
        {
            // (1) If you manage font atlases yourself, e.g. create a ImFontAtlas yourself you need to call ImFontAtlasUpdateNewFrame() on it.
            // Otherwise, calling ImGui::CreateContext() without parameter will create an atlas owned by the context.
            // (2) If you have multiple font atlases, make sure the 'atlas->RendererHasTextures' as specified in the ImFontAtlasUpdateNewFrame() call matches for that.
            // (3) If you have multiple imgui contexts, they also need to have a matching value for ImGuiBackendFlags_RendererHasTextures.
            IM_ASSERT(atlas->Builder != NULL && atlas->Builder->FrameCount != -1);
            IM_ASSERT(atlas->RendererHasTextures == has_textures);
        }
    }
}

// Build a single texture list
static void ImGui::UpdateTexturesEndFrame()
{
    ImGuiContext& g = *GImGui;
    g.PlatformIO.Textures.resize(0);
    for (ImFontAtlas* atlas : g.FontAtlases)
        for (ImTextureData* tex : atlas->TexList)
        {
            // We provide this information so backends can decide whether to destroy textures.
            // This means in practice that if N imgui contexts are created with a shared atlas, we assume all of them have a backend initialized.
            tex->RefCount = (unsigned short)atlas->RefCount;
            g.PlatformIO.Textures.push_back(tex);
        }
    for (ImTextureData* tex : g.UserTextures)
        g.PlatformIO.Textures.push_back(tex);
}

void ImGui::UpdateFontsNewFrame()
{
    ImGuiContext& g = *GImGui;
    if ((g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0)
        for (ImFontAtlas* atlas : g.FontAtlases)
            atlas->Locked = true;

    if (g.Style._NextFrameFontSizeBase != 0.0f)
    {
        g.Style.FontSizeBase = g.Style._NextFrameFontSizeBase;
        g.Style._NextFrameFontSizeBase = 0.0f;
    }

    // Apply default font size the first time
    ImFont* font = ImGui::GetDefaultFont();
    if (g.Style.FontSizeBase <= 0.0f)
        g.Style.FontSizeBase = (font->LegacySize > 0.0f ? font->LegacySize : FONT_DEFAULT_SIZE_BASE);

    // Set initial font
    g.Font = font;
    g.FontSizeBase = g.Style.FontSizeBase;
    g.FontSize = 0.0f;
    ImFontStackData font_stack_data = { font, g.Style.FontSizeBase, g.Style.FontSizeBase };           // <--- Will restore FontSize
    SetCurrentFont(font_stack_data.Font, font_stack_data.FontSizeBeforeScaling, 0.0f); // <--- but use 0.0f to enable scale
    g.FontStack.push_back(font_stack_data);
    IM_ASSERT(g.Font->IsLoaded());
}

void ImGui::UpdateFontsEndFrame()
{
    PopFont();
}

ImFont* ImGui::GetDefaultFont()
{
    ImGuiContext& g = *GImGui;
    ImFontAtlas* atlas = g.IO.Fonts;
    if (atlas->Builder == NULL || atlas->Fonts.Size == 0)
        ImFontAtlasBuildMain(atlas);
    return g.IO.FontDefault ? g.IO.FontDefault : atlas->Fonts[0];
}

// EXPERIMENTAL: DO NOT USE YET.
void ImGui::RegisterUserTexture(ImTextureData* tex)
{
    ImGuiContext& g = *GImGui;
    tex->RefCount++;
    g.UserTextures.push_back(tex);
}

void ImGui::UnregisterUserTexture(ImTextureData* tex)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(tex->RefCount > 0);
    tex->RefCount--;
    g.UserTextures.find_erase(tex);
}

void ImGui::RegisterFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    if (g.FontAtlases.Size == 0)
        IM_ASSERT(atlas == g.IO.Fonts);
    atlas->RefCount++;
    g.FontAtlases.push_back(atlas);
    ImFontAtlasAddDrawListSharedData(atlas, &g.DrawListSharedData);
    for (ImTextureData* tex : atlas->TexList)
        tex->RefCount = (unsigned short)atlas->RefCount;
}

void ImGui::UnregisterFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(atlas->RefCount > 0);
    ImFontAtlasRemoveDrawListSharedData(atlas, &g.DrawListSharedData);
    g.FontAtlases.find_erase(atlas);
    atlas->RefCount--;
    for (ImTextureData* tex : atlas->TexList)
        tex->RefCount = (unsigned short)atlas->RefCount;
}

// Use ImDrawList::_SetTexture(), making our shared g.FontStack[] authoritative against window-local ImDrawList.
// - Whereas ImDrawList::PushTexture()/PopTexture() is not to be used across Begin() calls.
// - Note that we don't propagate current texture id when e.g. Begin()-ing into a new window, we never really did...
//   - Some code paths never really fully worked with multiple atlas textures.
//   - The right-ish solution may be to remove _SetTexture() and make AddText/RenderText lazily call PushTexture()/PopTexture()
//     the same way AddImage() does, but then all other primitives would also need to? I don't think we should tackle this problem
//     because we have a concrete need and a test bed for multiple atlas textures.
// FIXME-NEWATLAS-V2: perhaps we can now leverage ImFontAtlasUpdateDrawListsTextures() ?
void ImGui::SetCurrentFont(ImFont* font, float font_size_before_scaling, float font_size_after_scaling)
{
    ImGuiContext& g = *GImGui;
    g.Font = font;
    g.FontSizeBase = font_size_before_scaling;
    UpdateCurrentFontSize(font_size_after_scaling);

    if (font != NULL)
    {
        IM_ASSERT(font && font->IsLoaded());    // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        IM_ASSERT(font->Scale > 0.0f);
#endif
        ImFontAtlas* atlas = font->OwnerAtlas;
        g.DrawListSharedData.FontAtlas = atlas;
        g.DrawListSharedData.Font = font;
        ImFontAtlasUpdateDrawListsSharedData(atlas);
        if (g.CurrentWindow != NULL)
            g.CurrentWindow->DrawList->_SetTexture(atlas->TexRef);
    }
}

void ImGui::UpdateCurrentFontSize(float restore_font_size_after_scaling)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    g.Style.FontSizeBase = g.FontSizeBase;

    // Restoring is pretty much only used by PopFont()
    float final_size = (restore_font_size_after_scaling > 0.0f) ? restore_font_size_after_scaling : 0.0f;
    if (final_size == 0.0f)
    {
        final_size = g.FontSizeBase;

        // Global scale factors
        final_size *= g.Style.FontScaleMain;    // Main global scale factor
        final_size *= g.Style.FontScaleDpi;     // Per-monitor/viewport DPI scale factor (in docking branch: automatically updated when io.ConfigDpiScaleFonts is enabled).

        // Window scale (mostly obsolete now)
        if (window != NULL)
            final_size *= window->FontWindowScale;

        // Legacy scale factors
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        final_size *= g.IO.FontGlobalScale; // Use style.FontScaleMain instead!
        if (g.Font != NULL)
            final_size *= g.Font->Scale;    // Was never really useful.
#endif
    }

    // Round font size
    // - We started rounding in 1.90 WIP (18991) as our layout system currently doesn't support non-rounded font size well yet.
    // - We may support it better later and remove this rounding.
    final_size = GetRoundedFontSize(final_size);
    final_size = ImClamp(final_size, 1.0f, IMGUI_FONT_SIZE_MAX);
    if (g.Font != NULL && (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures))
        g.Font->CurrentRasterizerDensity = g.FontRasterizerDensity;

    g.FontSize = final_size;
    g.DrawListSharedData.FontSize = g.FontSize;

    // Early out to avoid hidden window keeping bakes referenced and out of GC reach.
    // - However this leave a pretty subtle and damning error surface area if g.FontBaked was mismatching.
    //   Probably needs to be reevaluated into e.g. setting g.FontBaked = nullptr to mark it as dirty.
    // - Note that 'PushFont(); Begin(); End(); PopFont()' from within any collapsed window is not compromised, because Begin() calls SetCurrentWindow()->...->UpdateCurrentSize()
    if (window != NULL && window->SkipItems)
    {
        ImGuiTable* table = g.CurrentTable;
        const bool allow_early_out = table == NULL || (table->CurrentColumn != -1 && table->Columns[table->CurrentColumn].IsSkipItems == false); // See 8465#issuecomment-2951509561 and #8865. Ideally the SkipItems=true in tables would be amended with extra data.
        if (allow_early_out)
            return;
    }

    g.FontBaked = (g.Font != NULL && window != NULL) ? g.Font->GetFontBaked(final_size) : NULL;
    g.FontBakedScale = (g.FontBaked != NULL) ? (g.FontSize / g.FontBaked->Size) : 0.0f;
    g.DrawListSharedData.FontScale = g.FontBakedScale;
}

// Exposed in case user may want to override setting density.
// IMPORTANT: Begin()/End() is overriding density. Be considerate of this you change it.
void ImGui::SetFontRasterizerDensity(float rasterizer_density)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures);
    if (g.FontRasterizerDensity == rasterizer_density)
        return;
    g.FontRasterizerDensity = rasterizer_density;
    UpdateCurrentFontSize(0.0f);
}

// If you want to scale an existing font size! Read comments in imgui.h!
void ImGui::PushFont(ImFont* font, float font_size_base)
{
    ImGuiContext& g = *GImGui;
    if (font == NULL) // Before 1.92 (June 2025), PushFont(NULL) == PushFont(GetDefaultFont())
        font = g.Font;
    IM_ASSERT(font != NULL);
    IM_ASSERT(font_size_base >= 0.0f);

    g.FontStack.push_back({ g.Font, g.FontSizeBase, g.FontSize });
    if (font_size_base == 0.0f)
        font_size_base = g.FontSizeBase; // Keep current font size
    SetCurrentFont(font, font_size_base, 0.0f);
}

void  ImGui::PopFont()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.FontStack.Size > 0, "Calling PopFont() too many times!");
    ImFontStackData* font_stack_data = &g.FontStack.back();
    SetCurrentFont(font_stack_data->Font, font_stack_data->FontSizeBeforeScaling, font_stack_data->FontSizeAfterScaling);
    g.FontStack.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] ID STACK
//-----------------------------------------------------------------------------

// This is one of the very rare legacy case where we use ImGuiWindow methods,
// it should ideally be flattened at some point but it's been used a lots by widgets.
IM_MSVC_RUNTIME_CHECKS_OFF
ImGuiID ImGuiWindow::GetID(const char* str, const char* str_end)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashStr(str, str_end ? (str_end - str) : 0, seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_String, str, str_end);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetID(const void* ptr)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashData(&ptr, sizeof(void*), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_Pointer, ptr, NULL);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetID(int n)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashData(&n, sizeof(n), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_S32, (void*)(intptr_t)n, NULL);
#endif
    return id;
}

// This is only used in rare/specific situations to manufacture an ID out of nowhere.
// FIXME: Consider instead storing last non-zero ID + count of successive zero-ID, and combine those?
ImGuiID ImGuiWindow::GetIDFromPos(const ImVec2& p_abs)
{
    ImGuiID seed = IDStack.back();
    ImVec2 p_rel = ImGui::WindowPosAbsToRel(this, p_abs);
    ImGuiID id = ImHashData(&p_rel, sizeof(p_rel), seed);
    return id;
}

// "
ImGuiID ImGuiWindow::GetIDFromRectangle(const ImRect& r_abs)
{
    ImGuiID seed = IDStack.back();
    ImRect r_rel = ImGui::WindowRectAbsToRel(this, r_abs);
    ImGuiID id = ImHashData(&r_rel, sizeof(r_rel), seed);
    return id;
}

void ImGui::PushID(const char* str_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(str_id);
    window->IDStack.push_back(id);
}

void ImGui::PushID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(str_id_begin, str_id_end);
    window->IDStack.push_back(id);
}

void ImGui::PushID(const void* ptr_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(ptr_id);
    window->IDStack.push_back(id);
}

void ImGui::PushID(int int_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(int_id);
    window->IDStack.push_back(id);
}

// Push a given id value ignoring the ID stack as a seed.
void ImGui::PushOverrideID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_ID, NULL, NULL);
#endif
    window->IDStack.push_back(id);
}

// Helper to avoid a common series of PushOverrideID -> GetID() -> PopID() call
// (note that when using this pattern, ID Stack Tool will tend to not display the intermediate stack level.
//  for that to work we would need to do PushOverrideID() -> ItemAdd() -> PopID() which would alter widget code a little more)
ImGuiID ImGui::GetIDWithSeed(const char* str, const char* str_end, ImGuiID seed)
{
    ImGuiID id = ImHashStr(str, str_end ? (str_end - str) : 0, seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_String, str, str_end);
#endif
    return id;
}

ImGuiID ImGui::GetIDWithSeed(int n, ImGuiID seed)
{
    ImGuiID id = ImHashData(&n, sizeof(n), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_S32, (void*)(intptr_t)n, NULL);
#endif
    return id;
}

void ImGui::PopID()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    IM_ASSERT_USER_ERROR_RET(window->IDStack.Size > 1, "Calling PopID() too many times!");
    window->IDStack.pop_back();
}

ImGuiID ImGui::GetID(const char* str_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(str_id);
}

ImGuiID ImGui::GetID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(str_id_begin, str_id_end);
}

ImGuiID ImGui::GetID(const void* ptr_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(ptr_id);
}

ImGuiID ImGui::GetID(int int_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(int_id);
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] WINDOW FOCUS
//----------------------------------------------------------------------------
// - SetWindowFocus()
// - SetNextWindowFocus()
// - IsWindowFocused()
// - UpdateWindowInFocusOrderList() [Internal]
// - BringWindowToFocusFront() [Internal]
// - BringWindowToDisplayFront() [Internal]
// - BringWindowToDisplayBack() [Internal]
// - BringWindowToDisplayBehind() [Internal]
// - FindWindowDisplayIndex() [Internal]
// - FocusWindow() [Internal]
// - FocusTopMostWindowUnderOne() [Internal]
//-----------------------------------------------------------------------------

void ImGui::SetWindowFocus()
{
    FocusWindow(GImGui->CurrentWindow);
}

void ImGui::SetWindowFocus(const char* name)
{
    if (name)
    {
        if (ImGuiWindow* window = FindWindowByName(name))
            FocusWindow(window);
    }
    else
    {
        FocusWindow(NULL);
    }
}

void ImGui::SetNextWindowFocus()
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasFocus;
}

// Similar to IsWindowHovered()
bool ImGui::IsWindowFocused(ImGuiFocusedFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* ref_window = g.NavWindow;
    ImGuiWindow* cur_window = g.CurrentWindow;

    if (ref_window == NULL)
        return false;
    if (flags & ImGuiFocusedFlags_AnyWindow)
        return true;

    IM_ASSERT(cur_window); // Not inside a Begin()/End()
    const bool popup_hierarchy = (flags & ImGuiFocusedFlags_NoPopupHierarchy) == 0;
    const bool dock_hierarchy = (flags & ImGuiFocusedFlags_DockHierarchy) != 0;
    if (flags & ImGuiFocusedFlags_RootWindow)
        cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

    if (flags & ImGuiFocusedFlags_ChildWindows)
        return IsWindowChildOf(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
    else
        return ref_window == cur_window;
}

int ImGui::FindWindowFocusIndex(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(g);
    int order = window->FocusOrder;
    IM_ASSERT(window->RootWindow == window); // No child window (not testing _ChildWindow because of docking)
    IM_ASSERT(g.WindowsFocusOrder[order] == window);
    return order;
}

static void ImGui::UpdateWindowInFocusOrderList(ImGuiWindow* window, bool just_created, ImGuiWindowFlags new_flags)
{
    ImGuiContext& g = *GImGui;

    const bool new_is_explicit_child = (new_flags & ImGuiWindowFlags_ChildWindow) != 0 && ((new_flags & ImGuiWindowFlags_Popup) == 0 || (new_flags & ImGuiWindowFlags_ChildMenu) != 0);
    const bool child_flag_changed = new_is_explicit_child != window->IsExplicitChild;
    if ((just_created || child_flag_changed) && !new_is_explicit_child)
    {
        IM_ASSERT(!g.WindowsFocusOrder.contains(window));
        g.WindowsFocusOrder.push_back(window);
        window->FocusOrder = (short)(g.WindowsFocusOrder.Size - 1);
    }
    else if (!just_created && child_flag_changed && new_is_explicit_child)
    {
        IM_ASSERT(g.WindowsFocusOrder[window->FocusOrder] == window);
        for (int n = window->FocusOrder + 1; n < g.WindowsFocusOrder.Size; n++)
            g.WindowsFocusOrder[n]->FocusOrder--;
        g.WindowsFocusOrder.erase(g.WindowsFocusOrder.Data + window->FocusOrder);
        window->FocusOrder = -1;
    }
    window->IsExplicitChild = new_is_explicit_child;
}

void ImGui::BringWindowToFocusFront(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(window == window->RootWindow);

    const int cur_order = window->FocusOrder;
    IM_ASSERT(g.WindowsFocusOrder[cur_order] == window);
    if (g.WindowsFocusOrder.back() == window)
        return;

    const int new_order = g.WindowsFocusOrder.Size - 1;
    for (int n = cur_order; n < new_order; n++)
    {
        g.WindowsFocusOrder[n] = g.WindowsFocusOrder[n + 1];
        g.WindowsFocusOrder[n]->FocusOrder--;
        IM_ASSERT(g.WindowsFocusOrder[n]->FocusOrder == n);
    }
    g.WindowsFocusOrder[new_order] = window;
    window->FocusOrder = (short)new_order;
}

// Note technically focus related but rather adjacent and close to BringWindowToFocusFront()
// FIXME-FOCUS: Could opt-in/opt-out enable modal check like in FocusWindow().
void ImGui::BringWindowToDisplayFront(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* current_front_window = g.Windows.back();
    if (current_front_window == window || current_front_window->RootWindowDockTree == window) // Cheap early out (could be better)
        return;
    for (int i = g.Windows.Size - 2; i >= 0; i--) // We can ignore the top-most window
        if (g.Windows[i] == window)
        {
            memmove(&g.Windows[i], &g.Windows[i + 1], (size_t)(g.Windows.Size - i - 1) * sizeof(ImGuiWindow*));
            g.Windows[g.Windows.Size - 1] = window;
            break;
        }
}

void ImGui::BringWindowToDisplayBack(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (g.Windows[0] == window)
        return;
    for (int i = 0; i < g.Windows.Size; i++)
        if (g.Windows[i] == window)
        {
            memmove(&g.Windows[1], &g.Windows[0], (size_t)i * sizeof(ImGuiWindow*));
            g.Windows[0] = window;
            break;
        }
}

void ImGui::BringWindowToDisplayBehind(ImGuiWindow* window, ImGuiWindow* behind_window)
{
    IM_ASSERT(window != NULL && behind_window != NULL);
    ImGuiContext& g = *GImGui;
    window = window->RootWindow;
    behind_window = behind_window->RootWindow;
    int pos_wnd = FindWindowDisplayIndex(window);
    int pos_beh = FindWindowDisplayIndex(behind_window);
    if (pos_wnd < pos_beh)
    {
        size_t copy_bytes = (pos_beh - pos_wnd - 1) * sizeof(ImGuiWindow*);
        memmove(&g.Windows.Data[pos_wnd], &g.Windows.Data[pos_wnd + 1], copy_bytes);
        g.Windows[pos_beh - 1] = window;
    }
    else
    {
        size_t copy_bytes = (pos_wnd - pos_beh) * sizeof(ImGuiWindow*);
        memmove(&g.Windows.Data[pos_beh + 1], &g.Windows.Data[pos_beh], copy_bytes);
        g.Windows[pos_beh] = window;
    }
}

int ImGui::FindWindowDisplayIndex(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    return g.Windows.index_from_ptr(g.Windows.find(window));
}

// Moving window to front of display and set focus (which happens to be back of our sorted list)
void ImGui::FocusWindow(ImGuiWindow* window, ImGuiFocusRequestFlags flags)
{
    ImGuiContext& g = *GImGui;

    // Modal check?
    if ((flags & ImGuiFocusRequestFlags_UnlessBelowModal) && (g.NavWindow != window)) // Early out in common case.
        if (ImGuiWindow* blocking_modal = FindBlockingModal(window))
        {
            // This block would typically be reached in two situations:
            // - API call to FocusWindow() with a window under a modal and ImGuiFocusRequestFlags_UnlessBelowModal flag.
            // - User clicking on void or anything behind a modal while a modal is open (window == NULL)
            IMGUI_DEBUG_LOG_FOCUS("[focus] FocusWindow(\"%s\", UnlessBelowModal): prevented by \"%s\".\n", window ? window->Name : "<NULL>", blocking_modal->Name);
            if (window && window == window->RootWindow && (window->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus) == 0)
                BringWindowToDisplayBehind(window, blocking_modal); // Still bring right under modal. (FIXME: Could move in focus list too?)
            ClosePopupsOverWindow(GetTopMostPopupModal(), false); // Note how we need to use GetTopMostPopupModal() aad NOT blocking_modal, to handle nested modals
            return;
        }

    // Find last focused child (if any) and focus it instead.
    if ((flags & ImGuiFocusRequestFlags_RestoreFocusedChild) && window != NULL)
        window = NavRestoreLastChildNavWindow(window);

    // Apply focus
    if (g.NavWindow != window)
    {
        SetNavWindow(window);
        if (window && g.NavHighlightItemUnderNav)
            g.NavMousePosDirty = true;
        g.NavId = window ? window->NavLastIds[0] : 0; // Restore NavId
        g.NavLayer = ImGuiNavLayer_Main;
        SetNavFocusScope(window ? window->NavRootFocusScopeId : 0);
        g.NavIdIsAlive = false;
        g.NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;

        // Close popups if any
        ClosePopupsOverWindow(window, false);
    }

    // Move the root window to the top of the pile
    IM_ASSERT(window == NULL || window->RootWindowDockTree != NULL);
    ImGuiWindow* focus_front_window = window ? window->RootWindow : NULL;
    ImGuiWindow* display_front_window = window ? window->RootWindowDockTree : NULL;
    ImGuiDockNode* dock_node = window ? window->DockNode : NULL;
    bool active_id_window_is_dock_node_host = (g.ActiveIdWindow && dock_node && dock_node->HostWindow == g.ActiveIdWindow);

    // Steal active widgets. Some of the cases it triggers includes:
    // - Focus a window while an InputText in another window is active, if focus happens before the old InputText can run.
    // - When using Nav to activate menu items (due to timing of activating on press->new window appears->losing ActiveId)
    // - Using dock host items (tab, collapse button) can trigger this before we redirect the ActiveIdWindow toward the child window.
    if (g.ActiveId != 0 && g.ActiveIdWindow && g.ActiveIdWindow->RootWindow != focus_front_window)
        if (!g.ActiveIdNoClearOnFocusLoss && !active_id_window_is_dock_node_host)
            ClearActiveID();

    // Passing NULL allow to disable keyboard focus
    if (!window)
        return;
    window->LastFrameJustFocused = g.FrameCount;

    // Select in dock node
    // For #2304 we avoid applying focus immediately before the tabbar is visible.
    //if (dock_node && dock_node->TabBar)
    //    dock_node->TabBar->SelectedTabId = dock_node->TabBar->NextSelectedTabId = window->TabId;

    // Bring to front
    BringWindowToFocusFront(focus_front_window);
    if (((window->Flags | focus_front_window->Flags | display_front_window->Flags) & ImGuiWindowFlags_NoBringToFrontOnFocus) == 0)
        BringWindowToDisplayFront(display_front_window);
}

void ImGui::FocusTopMostWindowUnderOne(ImGuiWindow* under_this_window, ImGuiWindow* ignore_window, ImGuiViewport* filter_viewport, ImGuiFocusRequestFlags flags)
{
    ImGuiContext& g = *GImGui;
    int start_idx = g.WindowsFocusOrder.Size - 1;
    if (under_this_window != NULL)
    {
        // Aim at root window behind us, if we are in a child window that's our own root (see #4640)
        int offset = -1;
        while (under_this_window->Flags & ImGuiWindowFlags_ChildWindow)
        {
            under_this_window = under_this_window->ParentWindow;
            offset = 0;
        }
        start_idx = FindWindowFocusIndex(under_this_window) + offset;
    }
    for (int i = start_idx; i >= 0; i--)
    {
        // We may later decide to test for different NoXXXInputs based on the active navigation input (mouse vs nav) but that may feel more confusing to the user.
        ImGuiWindow* window = g.WindowsFocusOrder[i];
        if (window == ignore_window || !window->WasActive)
            continue;
        if (filter_viewport != NULL && window->Viewport != filter_viewport)
            continue;
        if ((window->Flags & (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs)) != (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs))
        {
            // FIXME-DOCK: When ImGuiFocusRequestFlags_RestoreFocusedChild is set...
            // This is failing (lagging by one frame) for docked windows.
            // If A and B are docked into window and B disappear, at the NewFrame() call site window->NavLastChildNavWindow will still point to B.
            // We might leverage the tab order implicitly stored in window->DockNodeAsHost->TabBar (essentially the 'most_recently_selected_tab' code in tab bar will do that but on next update)
            // to tell which is the "previous" window. Or we may leverage 'LastFrameFocused/LastFrameJustFocused' and have this function handle child window itself?
            FocusWindow(window, flags);
            return;
        }
    }
    FocusWindow(NULL, flags);
}

//-----------------------------------------------------------------------------
// [SECTION] LOGGING/CAPTURING
//-----------------------------------------------------------------------------
// All text output from the interface can be captured into tty/file/clipboard.
// By default, tree nodes are automatically opened during logging.
//-----------------------------------------------------------------------------

// Pass text data straight to log (without being displayed)
static inline void LogTextV(ImGuiContext& g, const char* fmt, va_list args)
{
    if (g.LogFile)
    {
        g.LogBuffer.Buf.resize(0);
        g.LogBuffer.appendfv(fmt, args);
        ImFileWrite(g.LogBuffer.c_str(), sizeof(char), (ImU64)g.LogBuffer.size(), g.LogFile);
    }
    else
    {
        g.LogBuffer.appendfv(fmt, args);
    }
}

void ImGui::LogText(const char* fmt, ...)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    va_list args;
    va_start(args, fmt);
    LogTextV(g, fmt, args);
    va_end(args);
}

void ImGui::LogTextV(const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogTextV(g, fmt, args);
}

// Internal version that takes a position to decide on newline placement and pad items according to their depth.
// We split text into individual lines to add current tree level padding
// FIXME: This code is a little complicated perhaps, considering simplifying the whole system.
void ImGui::LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const char* prefix = g.LogNextPrefix;
    const char* suffix = g.LogNextSuffix;
    g.LogNextPrefix = g.LogNextSuffix = NULL;

    if (!text_end)
        text_end = FindRenderedTextEnd(text, text_end);

    const bool log_new_line = ref_pos && (ref_pos->y > g.LogLinePosY + ImMax(g.Style.FramePadding.y, g.Style.ItemSpacing.y) + 1);
    if (ref_pos)
        g.LogLinePosY = ref_pos->y;
    if (log_new_line)
    {
        LogText(IM_NEWLINE);
        g.LogLineFirstItem = true;
    }

    if (prefix)
        LogRenderedText(ref_pos, prefix, prefix + ImStrlen(prefix)); // Calculate end ourself to ensure "##" are included here.

    // Re-adjust padding if we have popped out of our starting depth
    if (g.LogDepthRef > window->DC.TreeDepth)
        g.LogDepthRef = window->DC.TreeDepth;
    const int tree_depth = (window->DC.TreeDepth - g.LogDepthRef);

    const char* text_remaining = text;
    for (;;)
    {
        // Split the string. Each new line (after a '\n') is followed by indentation corresponding to the current depth of our log entry.
        // We don't add a trailing \n yet to allow a subsequent item on the same line to be captured.
        const char* line_start = text_remaining;
        const char* line_end = ImStreolRange(line_start, text_end);
        const bool is_last_line = (line_end == text_end);
        if (line_start != line_end || !is_last_line)
        {
            const int line_length = (int)(line_end - line_start);
            const int indentation = g.LogLineFirstItem ? tree_depth * 4 : 1;
            LogText("%*s%.*s", indentation, "", line_length, line_start);
            g.LogLineFirstItem = false;
            if (*line_end == '\n')
            {
                LogText(IM_NEWLINE);
                g.LogLineFirstItem = true;
            }
        }
        if (is_last_line)
            break;
        text_remaining = line_end + 1;
    }

    if (suffix)
        LogRenderedText(ref_pos, suffix, suffix + ImStrlen(suffix));
}

// Start logging/capturing text output
void ImGui::LogBegin(ImGuiLogFlags flags, int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(g.LogEnabled == false);
    IM_ASSERT(g.LogFile == NULL && g.LogBuffer.empty());
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiLogFlags_OutputMask_)); // Check that only 1 type flag is used

    g.LogEnabled = g.ItemUnclipByLog = true;
    g.LogFlags = flags;
    g.LogWindow = window;
    g.LogNextPrefix = g.LogNextSuffix = NULL;
    g.LogDepthRef = window->DC.TreeDepth;
    g.LogDepthToExpand = ((auto_open_depth >= 0) ? auto_open_depth : g.LogDepthToExpandDefault);
    g.LogLinePosY = FLT_MAX;
    g.LogLineFirstItem = true;
}

// Important: doesn't copy underlying data, use carefully (prefix/suffix must be in scope at the time of the next LogRenderedText)
void ImGui::LogSetNextTextDecoration(const char* prefix, const char* suffix)
{
    ImGuiContext& g = *GImGui;
    g.LogNextPrefix = prefix;
    g.LogNextSuffix = suffix;
}

void ImGui::LogToTTY(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    IM_UNUSED(auto_open_depth);
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    LogBegin(ImGuiLogFlags_OutputTTY, auto_open_depth);
    g.LogFile = stdout;
#endif
}

// Start logging/capturing text output to given file
void ImGui::LogToFile(int auto_open_depth, const char* filename)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;

    // FIXME: We could probably open the file in text mode "at", however note that clipboard/buffer logging will still
    // be subject to outputting OS-incompatible carriage return if within strings the user doesn't use IM_NEWLINE.
    // By opening the file in binary mode "ab" we have consistent output everywhere.
    if (!filename)
        filename = g.IO.LogFilename;
    if (!filename || !filename[0])
        return;
    ImFileHandle f = ImFileOpen(filename, "ab");
    if (!f)
    {
        IM_ASSERT(0);
        return;
    }

    LogBegin(ImGuiLogFlags_OutputFile, auto_open_depth);
    g.LogFile = f;
}

// Start logging/capturing text output to clipboard
void ImGui::LogToClipboard(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    LogBegin(ImGuiLogFlags_OutputClipboard, auto_open_depth);
}

void ImGui::LogToBuffer(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    LogBegin(ImGuiLogFlags_OutputBuffer, auto_open_depth);
}

void ImGui::LogFinish()
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogText(IM_NEWLINE);
    switch (g.LogFlags & ImGuiLogFlags_OutputMask_)
    {
    case ImGuiLogFlags_OutputTTY:
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
        fflush(g.LogFile);
#endif
        break;
    case ImGuiLogFlags_OutputFile:
        ImFileClose(g.LogFile);
        break;
    case ImGuiLogFlags_OutputBuffer:
        break;
    case ImGuiLogFlags_OutputClipboard:
        if (!g.LogBuffer.empty())
            SetClipboardText(g.LogBuffer.begin());
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    g.LogEnabled = g.ItemUnclipByLog = false;
    g.LogFlags = ImGuiLogFlags_None;
    g.LogFile = NULL;
    g.LogBuffer.clear();
}

// Helper to display logging buttons
// FIXME-OBSOLETE: We should probably obsolete this and let the user have their own helper (this is one of the oldest function alive!)
void ImGui::LogButtons()
{
    ImGuiContext& g = *GImGui;

    PushID("LogButtons");
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    const bool log_to_tty = Button("Log To TTY"); SameLine();
#else
    const bool log_to_tty = false;
#endif
    const bool log_to_file = Button("Log To File"); SameLine();
    const bool log_to_clipboard = Button("Log To Clipboard"); SameLine();
    PushItemFlag(ImGuiItemFlags_NoTabStop, true);
    SetNextItemWidth(CalcTextSize("999").x);
    SliderInt("Default Depth", &g.LogDepthToExpandDefault, 0, 9, NULL);
    PopItemFlag();
    PopID();

    // Start logging at the end of the function so that the buttons don't appear in the log
    if (log_to_tty)
        LogToTTY();
    if (log_to_file)
        LogToFile();
    if (log_to_clipboard)
        LogToClipboard();
}

//-----------------------------------------------------------------------------
// [SECTION] SETTINGS
//-----------------------------------------------------------------------------
// - UpdateSettings() [Internal]
// - MarkIniSettingsDirty() [Internal]
// - FindSettingsHandler() [Internal]
// - ClearIniSettings() [Internal]
// - LoadIniSettingsFromDisk()
// - LoadIniSettingsFromMemory()
// - SaveIniSettingsToDisk()
// - SaveIniSettingsToMemory()
//-----------------------------------------------------------------------------
// - CreateNewWindowSettings() [Internal]
// - FindWindowSettingsByID() [Internal]
// - FindWindowSettingsByWindow() [Internal]
// - ClearWindowSettings() [Internal]
// - WindowSettingsHandler_***() [Internal]
//-----------------------------------------------------------------------------

// Called by NewFrame()
void ImGui::UpdateSettings()
{
    // Load settings on first frame (if not explicitly loaded manually before)
    ImGuiContext& g = *GImGui;
    if (!g.SettingsLoaded)
    {
        IM_ASSERT(g.SettingsWindows.empty());
        if (g.IO.IniFilename)
            LoadIniSettingsFromDisk(g.IO.IniFilename);
        g.SettingsLoaded = true;
    }

    // Save settings (with a delay after the last modification, so we don't spam disk too much)
    if (g.SettingsDirtyTimer > 0.0f)
    {
        g.SettingsDirtyTimer -= g.IO.DeltaTime;
        if (g.SettingsDirtyTimer <= 0.0f)
        {
            if (g.IO.IniFilename != NULL)
                SaveIniSettingsToDisk(g.IO.IniFilename);
            else
                g.IO.WantSaveIniSettings = true;  // Let user know they can call SaveIniSettingsToMemory(). user will need to clear io.WantSaveIniSettings themselves.
            g.SettingsDirtyTimer = 0.0f;
        }
    }
}

void ImGui::MarkIniSettingsDirty()
{
    ImGuiContext& g = *GImGui;
    if (g.SettingsDirtyTimer <= 0.0f)
        g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void ImGui::MarkIniSettingsDirty(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
        if (g.SettingsDirtyTimer <= 0.0f)
            g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void ImGui::AddSettingsHandler(const ImGuiSettingsHandler* handler)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(FindSettingsHandler(handler->TypeName) == NULL);
    g.SettingsHandlers.push_back(*handler);
}

void ImGui::RemoveSettingsHandler(const char* type_name)
{
    ImGuiContext& g = *GImGui;
    if (ImGuiSettingsHandler* handler = FindSettingsHandler(type_name))
        g.SettingsHandlers.erase(handler);
}

ImGuiSettingsHandler* ImGui::FindSettingsHandler(const char* type_name)
{
    ImGuiContext& g = *GImGui;
    const ImGuiID type_hash = ImHashStr(type_name);
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.TypeHash == type_hash)
            return &handler;
    return NULL;
}

// Clear all settings (windows, tables, docking etc.)
void ImGui::ClearIniSettings()
{
    ImGuiContext& g = *GImGui;
    g.SettingsIniData.clear();
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ClearAllFn != NULL)
            handler.ClearAllFn(&g, &handler);
}

void ImGui::LoadIniSettingsFromDisk(const char* ini_filename)
{
    size_t file_data_size = 0;
    IO::path target = ENGINE_CACHE / ini_filename;
    char* file_data = (char*)ImFileLoadToMemory(target.string().c_str(), "rb", &file_data_size);
    if (!file_data)
        return;
    if (file_data_size > 0)
        LoadIniSettingsFromMemory(file_data, (size_t)file_data_size);
    IM_FREE(file_data);
}

// Zero-tolerance, no error reporting, cheap .ini parsing
// Set ini_size==0 to let us use strlen(ini_data). Do not call this function with a 0 if your buffer is actually empty!
void ImGui::LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);
    //IM_ASSERT(!g.WithinFrameScope && "Cannot be called between NewFrame() and EndFrame()");
    //IM_ASSERT(g.SettingsLoaded == false && g.FrameCount == 0);

    // For user convenience, we allow passing a non zero-terminated string (hence the ini_size parameter).
    // For our convenience and to make the code simpler, we'll also write zero-terminators within the buffer. So let's create a writable copy..
    if (ini_size == 0)
        ini_size = ImStrlen(ini_data);
    g.SettingsIniData.Buf.resize((int)ini_size + 1);
    char* const buf = g.SettingsIniData.Buf.Data;
    char* const buf_end = buf + ini_size;
    memcpy(buf, ini_data, ini_size);
    buf_end[0] = 0;

    // Call pre-read handlers
    // Some types will clear their data (e.g. dock information) some types will allow merge/override (window)
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ReadInitFn != NULL)
            handler.ReadInitFn(&g, &handler);

    void* entry_data = NULL;
    ImGuiSettingsHandler* entry_handler = NULL;

    char* line_end = NULL;
    for (char* line = buf; line < buf_end; line = line_end + 1)
    {
        // Skip new lines markers, then find end of the line
        while (*line == '\n' || *line == '\r')
            line++;
        line_end = line;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
            line_end++;
        line_end[0] = 0;
        if (line[0] == ';')
            continue;
        if (line[0] == '[' && line_end > line && line_end[-1] == ']')
        {
            // Parse "[Type][Name]". Note that 'Name' can itself contains [] characters, which is acceptable with the current format and parsing code.
            line_end[-1] = 0;
            const char* name_end = line_end - 1;
            const char* type_start = line + 1;
            char* type_end = (char*)(void*)ImStrchrRange(type_start, name_end, ']');
            const char* name_start = type_end ? ImStrchrRange(type_end + 1, name_end, '[') : NULL;
            if (!type_end || !name_start)
                continue;
            *type_end = 0; // Overwrite first ']'
            name_start++;  // Skip second '['
            entry_handler = FindSettingsHandler(type_start);
            entry_data = entry_handler ? entry_handler->ReadOpenFn(&g, entry_handler, name_start) : NULL;
        }
        else if (entry_handler != NULL && entry_data != NULL)
        {
            // Let type handler parse the line
            entry_handler->ReadLineFn(&g, entry_handler, entry_data, line);
        }
    }
    g.SettingsLoaded = true;

    // [DEBUG] Restore untouched copy so it can be browsed in Metrics (not strictly necessary)
    memcpy(buf, ini_data, ini_size);

    // Call post-read handlers
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ApplyAllFn != NULL)
            handler.ApplyAllFn(&g, &handler);
}

void ImGui::SaveIniSettingsToDisk(const char* ini_filename)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    if (!ini_filename)
        return;

    size_t ini_data_size = 0;
    const char* ini_data = SaveIniSettingsToMemory(&ini_data_size);
    IO::path target = ENGINE_CACHE / ini_filename;
    // use Program Path
    ImFileHandle f = ImFileOpen(target.string().c_str(), "wt");
    if (!f)
        return;
    ImFileWrite(ini_data, sizeof(char), ini_data_size, f);
    ImFileClose(f);
}

// Call registered handlers (e.g. SettingsHandlerWindow_WriteAll() + custom handlers) to write their stuff into a text buffer
const char* ImGui::SaveIniSettingsToMemory(size_t* out_size)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    g.SettingsIniData.Buf.resize(0);
    g.SettingsIniData.Buf.push_back(0);
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        handler.WriteAllFn(&g, &handler, &g.SettingsIniData);
    if (out_size)
        *out_size = (size_t)g.SettingsIniData.size();
    return g.SettingsIniData.c_str();
}

ImGuiWindowSettings* ImGui::CreateNewWindowSettings(const char* name)
{
    ImGuiContext& g = *GImGui;

    // Preserve the full string when ConfigDebugVerboseIniSettings is set to make .ini inspection easier.
    if (g.IO.ConfigDebugIniSettings == false)
        name = ImHashSkipUncontributingPrefix(name);
    const size_t name_len = ImStrlen(name);

    // Allocate chunk
    const size_t chunk_size = sizeof(ImGuiWindowSettings) + name_len + 1;
    ImGuiWindowSettings* settings = g.SettingsWindows.alloc_chunk(chunk_size);
    IM_PLACEMENT_NEW(settings) ImGuiWindowSettings();
    settings->ID = ImHashStr(name, name_len);
    memcpy(settings->GetName(), name, name_len + 1);   // Store with zero terminator

    return settings;
}

// We don't provide a FindWindowSettingsByName() because Docking system doesn't always hold on names.
// This is called once per window .ini entry + once per newly instantiated window.
ImGuiWindowSettings* ImGui::FindWindowSettingsByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->ID == id && !settings->WantDelete)
            return settings;
    return NULL;
}

// This is faster if you are holding on a Window already as we don't need to perform a search.
ImGuiWindowSettings* ImGui::FindWindowSettingsByWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (window->SettingsOffset != -1)
        return g.SettingsWindows.ptr_from_offset(window->SettingsOffset);
    return FindWindowSettingsByID(window->ID);
}

// This will revert window to its initial state, including enabling the ImGuiCond_FirstUseEver/ImGuiCond_Once conditions once more.
void ImGui::ClearWindowSettings(const char* name)
{
    //IMGUI_DEBUG_LOG("ClearWindowSettings('%s')\n", name);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = FindWindowByName(name);
    if (window != NULL)
    {
        window->Flags |= ImGuiWindowFlags_NoSavedSettings;
        InitOrLoadWindowSettings(window, NULL);
        if (window->DockId != 0)
            DockContextProcessUndockWindow(&g, window, true);
    }
    if (ImGuiWindowSettings* settings = window ? FindWindowSettingsByWindow(window) : FindWindowSettingsByID(ImHashStr(name)))
        settings->WantDelete = true;
}

static void WindowSettingsHandler_ClearAll(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
    ImGuiContext& g = *ctx;
    for (ImGuiWindow* window : g.Windows)
        window->SettingsOffset = -1;
    g.SettingsWindows.clear();
}

static void* WindowSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
{
    ImGuiID id = ImHashStr(name);
    ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByID(id);
    if (settings)
        *settings = ImGuiWindowSettings(); // Clear existing if recycling previous entry
    else
        settings = ImGui::CreateNewWindowSettings(name);
    settings->ID = id;
    settings->WantApply = true;
    return (void*)settings;
}

static void WindowSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
    ImGuiWindowSettings* settings = (ImGuiWindowSettings*)entry;
    int x, y;
    int i;
    ImU32 u1;
    if (sscanf(line, "Pos=%i,%i", &x, &y) == 2)             { settings->Pos = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "Size=%i,%i", &x, &y) == 2)       { settings->Size = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "ViewportId=0x%08X", &u1) == 1)   { settings->ViewportId = u1; }
    else if (sscanf(line, "ViewportPos=%i,%i", &x, &y) == 2){ settings->ViewportPos = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "Collapsed=%d", &i) == 1)         { settings->Collapsed = (i != 0); }
    else if (sscanf(line, "IsChild=%d", &i) == 1)           { settings->IsChild = (i != 0); }
    else if (sscanf(line, "DockId=0x%X,%d", &u1, &i) == 2)  { settings->DockId = u1; settings->DockOrder = (short)i; }
    else if (sscanf(line, "DockId=0x%X", &u1) == 1)         { settings->DockId = u1; settings->DockOrder = -1; }
    else if (sscanf(line, "ClassId=0x%X", &u1) == 1)        { settings->ClassId = u1; }
}

// Apply to existing windows (if any)
static void WindowSettingsHandler_ApplyAll(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
    ImGuiContext& g = *ctx;
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->WantApply)
        {
            if (ImGuiWindow* window = ImGui::FindWindowByID(settings->ID))
                ApplyWindowSettings(window, settings);
            settings->WantApply = false;
        }
}

static void WindowSettingsHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
    // Gather data from windows that were active during this session
    // (if a window wasn't opened in this session we preserve its settings)
    ImGuiContext& g = *ctx;
    for (ImGuiWindow* window : g.Windows)
    {
        if (window->Flags & ImGuiWindowFlags_NoSavedSettings)
            continue;

        ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByWindow(window);
        if (!settings)
        {
            settings = ImGui::CreateNewWindowSettings(window->Name);
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
        }
        IM_ASSERT(settings->ID == window->ID);
        settings->Pos = ImVec2ih(window->Pos - window->ViewportPos);
        settings->Size = ImVec2ih(window->SizeFull);
        settings->ViewportId = window->ViewportId;
        settings->ViewportPos = ImVec2ih(window->ViewportPos);
        IM_ASSERT(window->DockNode == NULL || window->DockNode->ID == window->DockId);
        settings->DockId = window->DockId;
        settings->ClassId = window->WindowClass.ClassId;
        settings->DockOrder = window->DockOrder;
        settings->Collapsed = window->Collapsed;
        settings->IsChild = (window->RootWindow != window); // Cannot rely on ImGuiWindowFlags_ChildWindow here as docked windows have this set.
        settings->WantDelete = false;
    }

    // Write to text buffer
    buf->reserve(buf->size() + g.SettingsWindows.size() * 6); // ballpark reserve
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
    {
        if (settings->WantDelete)
            continue;
        const char* settings_name = settings->GetName();
        buf->appendf("[%s][%s]\n", handler->TypeName, settings_name);
        if (settings->IsChild)
        {
            buf->appendf("IsChild=1\n");
            buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
        }
        else
        {
            if (settings->ViewportId != 0 && settings->ViewportId != ImGui::IMGUI_VIEWPORT_DEFAULT_ID)
            {
                buf->appendf("ViewportPos=%d,%d\n", settings->ViewportPos.x, settings->ViewportPos.y);
                buf->appendf("ViewportId=0x%08X\n", settings->ViewportId);
            }
            if (settings->Pos.x != 0 || settings->Pos.y != 0 || settings->ViewportId == ImGui::IMGUI_VIEWPORT_DEFAULT_ID)
                buf->appendf("Pos=%d,%d\n", settings->Pos.x, settings->Pos.y);
            if (settings->Size.x != 0 || settings->Size.y != 0)
                buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
            buf->appendf("Collapsed=%d\n", settings->Collapsed);
            if (settings->DockId != 0)
            {
                //buf->appendf("TabId=0x%08X\n", ImHashStr("#TAB", 4, settings->ID)); // window->TabId: this is not read back but writing it makes "debugging" the .ini data easier.
                if (settings->DockOrder == -1)
                    buf->appendf("DockId=0x%08X\n", settings->DockId);
                else
                    buf->appendf("DockId=0x%08X,%d\n", settings->DockId, settings->DockOrder);
                if (settings->ClassId != 0)
                    buf->appendf("ClassId=0x%08X\n", settings->ClassId);
            }
        }
        buf->append("\n");
    }
}

//-----------------------------------------------------------------------------
// [SECTION] LOCALIZATION
//-----------------------------------------------------------------------------

void ImGui::LocalizeRegisterEntries(const ImGuiLocEntry* entries, int count)
{
    ImGuiContext& g = *GImGui;
    for (int n = 0; n < count; n++)
        g.LocalizationTable[entries[n].Key] = entries[n].Text;
}

//-----------------------------------------------------------------------------

// Include imgui_user.inl at the end of imgui.cpp to access private data/functions that aren't exposed.
// Prefer just including imgui_internal.h from your code rather than using this define. If a declaration is missing from imgui_internal.h add it or request it on the github.
#ifdef IMGUI_INCLUDE_IMGUI_USER_INL
#include "imgui_user.inl"
#endif

//-----------------------------------------------------------------------------

#endif // #ifndef IMGUI_DISABLE
