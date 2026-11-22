# Dear ImGui 1.92.8 WIP —— imgui.cpp 头部注释（中文版）

> **来源**：`Engine/ThirdParty/IMGUI/Private/imgui.cpp` 顶部的原始注释（上游 `imgui.cpp` 的 "main code and documentation" 文件头），共 1,216 行。
> **现状**：这部分内容已从 `imgui.cpp` 移出（该文件头只保留许可证信息和指向本文的说明），源码不再被上千行文档注释淹没。
> **英文原文**：[imgui.cpp-notes.en.md](imgui.cpp-notes.en.md)（逐字保留上游结构，是核对时的基准）。
> **翻译约定**：正文译成中文；代码示例、宏名、API 名、`[SECTION]` 标记保持原样；**API 破坏性变更日志保留英文原文**（条目全是 API 名、版本号与 diff 说明，翻译反而妨碍检索，见英文版对应章节）。
> **维护**：上游升级时重新提取新版 `imgui.cpp` 的头部注释整理，不要在这份译文上手工追改。
> **版本**：Dear ImGui v1.92.8 WIP（`IMGUI_VERSION_NUM 19275`，docking 分支），MIT 协议，版权归 Omar Cornut 及所有贡献者。

## 相关链接

| 资源 | 地址 | 说明 |
| --- | --- | --- |
| FAQ | <https://dearimgui.com/faq> | 上游 `docs/FAQ.md`，本仓库未收录 |
| 主页 / 仓库 | <https://github.com/ocornut/imgui> | |
| 版本与 Changelog | <https://github.com/ocornut/imgui/releases> | |
| Gallery | <https://github.com/ocornut/imgui/issues?q=label%3Agallery> | 截图/视频投稿处 |
| Wiki | <https://github.com/ocornut/imgui/wiki> | 实践资料很多 |
| Getting Started | <https://github.com/ocornut/imgui/wiki/Getting-Started> | 约 25 行代码接入已有程序 |
| 第三方扩展 | <https://github.com/ocornut/imgui/wiki/Useful-Extensions> | ImPlot 等 |
| 绑定 / 后端 | <https://github.com/ocornut/imgui/wiki/Bindings> | 各语言绑定、各技术栈后端 |
| 调试工具 | <https://github.com/ocornut/imgui/wiki/Debug-Tools> | |
| 术语表 | <https://github.com/ocornut/imgui/wiki/Glossary> | |
| 已知用例 | <https://github.com/ocornut/imgui/wiki/Software-using-dear-imgui> | |
| Issues / 支持 | <https://github.com/ocornut/imgui/issues> | 交叉链接的知识库 |
| 测试引擎 | <https://github.com/ocornut/imgui_test_engine> | 自动化测试套件 |
| Web 版 Demo | <https://pthom.github.io/imgui_explorer> | 带源码浏览器 |

首次编译/链接/运行出问题，先去 <https://github.com/ocornut/imgui/discussions>；其余问题走 `Issues`。自 1.92 起，字体加载问题也建议发 `Issues`。

## 上游的几条提醒

- 不要改 `imgui.cpp`：改了以后升级会非常痛苦。
- `ImGui::` 是命名空间，可以在自己的源文件里往这个命名空间加函数，不必动 `imgui.h` / `imgui.cpp`。
- 包含 `imgui_internal.h` 可以访问内部数据结构，但不保证向前兼容；把改动讨论到 GitHub Issue Tracker 上，通常能得到更好的方案或官方支持。
- 保证断言是开着的（`IM_ASSERT` 默认转 `assert()`，可重定向）；命中断言时读断言附近的注释。

## 本文件索引

### 文档目录

- 使命声明（MISSION STATEMENT）
- 操作指南（CONTROLS GUIDE）
- 程序员指南（PROGRAMMER GUIDE）
  - 先读这些（READ FIRST）
  - 如何升级到新版 Dear ImGui
  - 接入到自己的代码 / 引擎
  - 一个简单程序长什么样
  - 自定义后端 / 自定义引擎
- API 破坏性变更（升级时必读）
- 常见问题（FAQ）

### 代码分区（`[SECTION]` 标记）

```cpp
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
```

> 本地拆分后，这些 `[SECTION]` 已分散到 `Private/*.cpp`，逐文件对应关系见 [08-ImGui拆分复核报告.md](../../../../docs/08-ImGui拆分复核报告.md) 第 2.1 节。
> 注意：部分拆分出来的文件里还留着复制时带过来的旧 `[SECTION]` 横幅（对应内容已迁走），定位代码请以实际函数为准。

***

## 使命声明（MISSION STATEMENT）

- 让"代码驱动 / 数据驱动"的工具好写。
- 让临时的短命工具和长期维护的复杂工具都好写。
- 好改、好改进。
- 把安装与维护成本压到最低。
- 把用户侧的状态存储压到最低。
- 把状态同步压到最低。
- 可移植，依赖最少，能在目标平台上直接跑（主机、手机等）。
- 运行时与内存占用都要省。

它面向的是开发者和内容创作者，不是典型终端用户。因此目前已知的短板包括：

- 默认外观不花哨。
- 布局能力有限，复杂布局通常要靠代码手工编排。

## 操作指南（CONTROLS GUIDE）

### 鼠标

- 滚轮：垂直滚动。
- Shift + 滚轮：水平滚动。
- 点 `[X]`：关闭窗口。仅当给 `ImGui::Begin()` 传了 `bool* p_open` 时才有。
- 点 `^`、双击标题栏：折叠窗口。
- 拖角 / 拖边：缩放窗口（双击可自动适配内容大小）。
- 拖任意空白处：移动窗口（除非 `io.ConfigWindowsMoveFromTitleBarOnly = true`）。
- 左键点弹窗外：关闭整个弹窗栈（在下层弹窗上右键：只关掉栈的一部分）。

### 文本编辑器

- 按住 Shift 或拖动鼠标：选中文本。
- Ctrl + 左/右方向键：按词跳转。
- Ctrl + Shift + 左/右方向键：按词选中。
- Ctrl+A 或双击：全选。
- Ctrl+X / Ctrl+C / Ctrl+V：走系统剪贴板。
- Ctrl+Z：撤销。
- Ctrl+Y 或 Ctrl+Shift+Z：重做。
- ESCAPE：还原成原来的值。
- macOS 上控件会自动适配系统文本编辑习惯（99% 的快捷键里 Ctrl 换成 Cmd）。

### 键盘

基础功能：

- Tab / Shift+Tab：在可编辑文本字段之间循环。
- Ctrl+Tab / Ctrl+Shift+Tab：在窗口之间循环。
- Ctrl + 点击：向 Slider / Drag 控件直接输入数值。

设置 `io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard` 后的扩展功能：

- Tab / Shift+Tab：遍历**所有** item。
- 方向键：方向导航；同时可微调数值。
- 方向键 + Alt / Shift：调得更慢 / 更快（用方向键调节时）。
- Enter：激活 item（尽量优先用文本输入）。
- Space：激活 item（尽量优先用方向键微调）。
- Escape：取消激活、离开子窗口、关闭弹窗。
- Page Up / Page Down：上一页 / 下一页。
- Home / End：滚到顶 / 滚到底。
- Alt：在滚动层与菜单层之间切换。
- Ctrl+Tab 之后按 Ctrl+方向键：移动窗口；按住 Shift 改为缩放。
- Menu 键或 Shift+F10：打开上下文菜单。

设置了 `ImGuiConfigFlags_NavEnableKeyboard` 时的输出：

- `io.WantCaptureKeyboard`：键盘被 ImGui 取走时为 true。
- `io.NavActive`：有窗口聚焦，且该窗口没设 `ImGuiWindowFlags_NoNavInputs`。
- `io.NavVisible`：导航光标可见（通常在用鼠标后转回 false）。

### 手柄

- 通过 `io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad` 启用。
- 在没有鼠标的主机上（PlayStation、Switch、Xbox）尤其有用。
- 键位图（PNG/PSD）可下载：<http://dearimgui.com/controls_sheets>
- 后端需要做两件事：
  - 置 `io.BackendFlags |= ImGuiBackendFlags_HasGamepad`，并用 `ImGuiKey_Gamepad_XXX` 系列键调用 `io.AddKeyEvent` / `io.AddKeyAnalogEvent`。
  - 对模拟量（0.0f 到 1.0f）负责做死区处理与重新标定（例如把原始 0.2..0.9 映射到 0.0..1.0）。
- 如果游戏和 Dear ImGui 界面需要共享输入，最省事的做法是"全有或全无"：用一个组合键切换当前由谁接收输入。

### 远程输入共享与鼠标模拟

- PS4/PS5：可以拿 DualShock 触摸板或空闲摇杆模拟鼠标，作为兜底。
- 主机 / 平板 / 手机：可以在 PC 上跑 Synergy 1.x 服务端，并在你的 app 里编译 `examples/libs/synergy/uSynergy.c`，共享 PC 的鼠标键盘。
- 其他远程方案见 <https://github.com/ocornut/imgui/wiki/Useful-Extensions#remoting>。
- 电视 / 主机场景下可读性差、鼠标不好用时，可以设 `io.ConfigNavMoveSetMousePos`：
  - 开启 `io.ConfigNavMoveSetMousePos` + `ImGuiBackendFlags_HasSetMousePos`，导航移动时光标会跟着走。
  - 此时 `NewFrame()` 可能修改 `io.MousePos` 并置 `io.WantSetMousePos`，告诉你要移动光标；**后端必须在下一帧真的移动系统光标**（examples/ 里有些后端做到了）。
  - （如果置了该 flag 却不照做，Dear ImGui 会看到光标来回跳，行为会乱。）
  - （在不好直接控制光标的场景，比如 uSynergy.c 不支持移动远端光标，可以设一个布尔量，在外部输入再次移动之前忽略其他鼠标位置。）

## 程序员指南（PROGRAMMER GUIDE）

### 先读这些（READ FIRST）

- 记得看 Wiki：<https://github.com/ocornut/imgui/wiki>
- UI 是**每帧重建**的：你的代码不跑，UI 就消失。UI 可以高度动态，没有构造/析构步骤，用户侧留存的数据更少、状态重复更少、需要同步的状态更少、bug 更少。
- 调 `ImGui::ShowDemoWindow()` 并读它的源码，里面演示了大部分功能；也可以看 pthom 的在线 imgui_explorer（<https://pthom.github.io/imgui_explorer>，带源码浏览器）。
- 这个库设计成**从源码构建**。别用预编译二进制和打包版本。构建配置看 `imconfig.h`。
- Dear ImGui 是 IMGUI 范式（immediate-mode graphical user interface，Casey Muratori 造的词）的一种实现。原理可读 <http://www.johno.se/book/imgui.html>、<http://mollyrocket.com/861> 以及 Wiki 里更多链接。
- Dear ImGui 是 IMGUI 范式的"单遍"光栅化实现，目标是好用 + 高性能。每个应用帧里你的 UI 代码只被调用一次；对比之下，Unity 的 IMGUI 实现是从一个入口被调用多次（"多遍"）。两种做法各有取舍。
- 坐标原点是左上角。在轴对齐包围盒里，Min = 左上、Max = 右下。
- 请确保断言是开着的（`IM_ASSERT` 默认转 `assert()`，可重定向）。命中断言时读断言附近的注释。
- 这个代码库追求高度优化：
  - 典型的空闲帧里不应该调用 malloc/free。
  - 算法尽量是常数时间或 O(N)，尽量限制搜索/扫描范围。
  - 也刻意保证"典型 Debug 构建"下的性能尚可——所以它并不迷信"零成本抽象"（那玩意根本不是零成本）。
- 这个代码库既高度有主见，又高度灵活：
  - 它能用，正是因为它明确选择"解决什么、不解决什么"。
  - C++ 部分：这是一个务实的 C 风格代码库，不用花哨的 C++ 特性，不包含 C++ 标准头，`ImGui::` 是命名空间。很少用成员函数（用了的地方作者本人多半在后悔）。这样做是为了兼容性、可维护性，以及方便其他语言的绑定。
  - C++ 部分：`ImVec2` / `ImVec4` 默认不导出数学运算符，因为它期望你用**自己的**数学类型。怎么在 `imconfig.h` 里配置，见 FAQ "How can I use my own math types instead of ImVec2/ImVec4?"。也可以用 `IMGUI_DEFINE_MATH_OPERATORS` 导出运算符（库内部就是这么用的）。
  - C++ 部分：注意 `ImVector<>` 操作的是 plain-old-data，不遵守构造/析构（所以别在自己代码里用它）。
  - 构建：主库既不用也不强制某个构建系统。这是为了保证它在真实世界里、在任何古怪的构建环境里都能用；也是因为给主库配构建系统价值不大——构建问题几乎从来不在主库，而在具体后端。

### 如何升级到新版 Dear ImGui

- 更新 submodule，或直接覆盖全部文件。
- 关于 `imconfig.h`：
  - 你可以改自己的 `imconfig.h`，那就别覆盖它；
  - 或者在本地开分支改 `imconfig.h`，再 merge/rebase 上游；
  - 或者在构建系统里全局 `#define IMGUI_USER_CONFIG "my_config_file.h"`，指定你自己的配置头路径，这样默认的 `imconfig.h` 就不用改。
- 覆盖除 `imconfig.h` 之外的所有源文件（如果你改过自己的 `imconfig.h`）。
- 或者维护自己的分支，把 `imconfig.h` 的改动放在最顶部一个提交，定期在 master 上 rebase。
- 升级时读 "API BREAKING CHANGES"（英文版里有全量原文）。这里是偶尔出现的破坏性变更清单。某个函数/类型被改名或标记废弃时，尽量在被彻底移除之前把名字改掉。找不到某个函数/符号时，在代码里搜它的名字，通常会有相关注释；有问题报到 GitHub。
- 想找出旧 API 的用法，可以在配置里 `#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS`。
- 尽量让本地这份 Dear ImGui 保持比较新。

### 接入到自己的代码 / 引擎（GETTING STARTED）

- 先读 <https://github.com/ocornut/imgui/wiki/Getting-Started>。
- 跑 examples/ 和 `imgui_demo.cpp` 里的 demo，先熟悉这个库。
- 大多数情况下，`backends/` 里现成的后端文件可以原样使用。
- 把 Dear ImGui 源文件 + 选定的后端源文件加进你的工程或你惯用的构建系统。建议把 `.cpp` 静态链接进你的工程，**不要**做成共享库（DLL）。
- 之后可以改 `imconfig.h` 调一些编译期行为，例如把 Dear ImGui 的类型接到你自己的数学类型上。
- 用这个库时，你的 IDE 是好朋友：顺着变量/函数/类型的声明跳转，注释都写在那里。
- Dear ImGui 从不接触、也不知道你的 GPU 状态。唯一"知道 GPU"的函数是你提供的绘制函数。也就是说你可以在代码里任何时候创建控件，不用管当前处于应用的"更新"还是"渲染"阶段；所有渲染信息都存进命令列表，你在 `ImGui::Render()` 之后取走。
- 怎么搭代码，参考 examples/ 里的后端与示例应用。
- 如果跑在标准操作系统 + 常见图形 API 上，`imgui_impl_***` 文件通常可以原样使用。

### 一个简单程序长什么样

用示例后端（也就是 `backends/` 里的 `imgui_impl_XXX.cpp`）。`examples/` 下的子目录都是这个结构：

```cpp
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
```

判断鼠标/键盘事件该给 Dear ImGui 还是给自己的程序，看 `io.WantCaptureMouse`、`io.WantCaptureKeyboard`、`io.WantTextInput` 这三个 flag。详见 FAQ "How can I tell whether to dispatch mouse/keyboard to Dear ImGui or my application?"。

### 自定义后端 / 自定义引擎

实现平台后端：

- 基本说明见 <https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md>
- `imgui_impl_XXX.cpp` 里的平台后端有很多现成实现可以参考。

实现 `RenderDrawData()`：

- 见 <https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md>
- 渲染后端里有很多 `ImGui_ImplXXXX_RenderDrawData()` 的实现可以参考。

实现 `ImGuiBackendFlags_RendererHasTextures` 支持：

- 见 <https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md>
- 渲染后端里有很多 `ImGui_ImplXXXX_UpdateTexture()` 的实现可以参考。

最小应用 / 后端骨架：

```cpp
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
    io.DeltaTime = 1.0f/60.0f;              // 上一帧到这一帧的时间（秒）
    io.DisplaySize.x = 1920.0f;             // 当前显示宽度
    io.DisplaySize.y = 1280.0f;             // 当前显示高度
    io.AddMousePosEvent(mouse_x, mouse_y);  // 更新鼠标位置
    io.AddMouseButtonEvent(0, mouse_b[0]);  // 更新鼠标按键状态
    io.AddMouseButtonEvent(1, mouse_b[1]);  // 更新鼠标按键状态

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
```

## API 破坏性变更（API BREAKING CHANGES）

这里按时间倒序记录"会破坏现有代码"的 API 变更（自 1.92.8 往前一直到 1.x 早期，约 700 行）。升级前把要跨过的版本区间扫一遍，比到处搜 `LNK2019` 省事。

**完整条目保留英文原文**：[imgui.cpp-notes.en.md · API BREAKING CHANGES](imgui.cpp-notes.en.md#api-breaking-changes)。原因很实际——条目里全是 API 名、版本号、旧签名与新签名的对照，翻译过来反而不好检索；需要中文说明时再对具体条目提问。

几条使用方式（中文提示，原文见上链接）：

- 每条格式是 `日期 (版本) - 改动`，同一版本内可能有多条，向下回到更老的版本。
- 形如 `- Before: / - After:` 的缩进行是该改动的行为对比。
- 带 `(#1234)` 的是 GitHub issue/PR 编号，可以顺着编号去看讨论。
- 看到 `Commented out legacy ...` 说明旧签名已经删了，必须改调用点；看到 `Obsoleted ...` 说明还能用但不建议，且可能在下一两个版本被彻底移除。
- 这份日志不含"新增功能"，只列破坏性变更；功能变更去 <https://github.com/ocornut/imgui/releases>。

## 常见问题（FAQ）

完整答案在线看：<https://www.dearimgui.com/faq> 或上游 `docs/FAQ.md`（带 Markdown 阅读器看更舒服）。下面只抄了源码里保留的一小部分，方便在代码里搜索。

### 基础

**Q：文档在哪？**

A：这个库文档不算多，它假定你熟悉 C/C++。可以这样入手：

- 跑 `examples/` 里的应用并动手改。
- 读 Getting Started（<https://github.com/ocornut/imgui/wiki/Getting-Started>）。
- 看 `imgui_demo.cpp`，尤其是 `ImGui::ShowDemoWindow()`。
- 看 pthom 的在线 imgui_explorer（<https://pthom.github.io/imgui_explorer>），它是 demo 的 Web 版，带源码浏览器。
- demo 覆盖了大部分功能，读代码 + 看效果是最快的路径。
- 读本文件（原 `imgui.cpp` 顶部注释）以及 `imgui.h`。
- `examples/` 里有 20 多个独立示例（OpenGL / DirectX 等），演示怎么接进自己的引擎/应用。
- Wiki（<https://github.com/ocornut/imgui/wiki>）资料很多。
- Glossary（<https://github.com/ocornut/imgui/wiki/Glossary>）也有用。
- 你的 IDE 是好朋友：跳到类型或函数声明，注释就在那儿。

**Q：这个库叫什么？和传统 UI 工具包有什么区别？该用哪个版本？**

> 它叫 "Dear ImGui"，别叫 "ImGui" :) 细节见 <https://www.dearimgui.com/faq>。

### 集成

**Q：怎么开始？**

A：读 <https://github.com/ocornut/imgui/wiki/Getting-Started>，读上面的 PROGRAMMER GUIDE，读 `examples/README.txt`。

**Q：怎么判断鼠标/键盘该给 Dear ImGui 还是给我的应用？**

A：看 `io.WantCaptureMouse`、`io.WantCaptureKeyboard`、`io.WantTextInput` 三个 flag。这个问题的完整答案很值得读一遍。

**Q：怎么启用键盘或手柄控制？没有鼠标/键盘/屏幕的机器怎么办（输入共享、远程显示）？集成了但文字显示成小方块怎么办？窗口移动时元素被裁掉/消失怎么办？元素画到了窗口边界之外怎么办？**

> 见 <https://www.dearimgui.com/faq>。

### 用法

**Q：关于 ID 栈系统……**

- 多个控件用同一个标签怎么办（用 `##` 或 `PushID`）。
- 控件要空标签怎么办（用 `##`）。
- 标签要动态怎么办（用 `###`）。
- 标签与 ID 栈系统的总体说明。

**Q：怎么显示图片？`ImTextureID` 是什么，怎么用？怎么用自己的数学类型代替 ImVec2？怎么和 `std::string` / `std::vector` 这类标准 C++ 类型打交道？怎么画自定义图形（用低层 `ImDrawList` API）？**

> 见 <https://www.dearimgui.com/faq>。

### 字体与文本

**Q：应用里怎么处理 DPI？怎么加载默认字体以外的字体？怎么方便地用图标？怎么加载多个字体？怎么显示并输入中文/日文/韩文/西里尔文这类非拉丁字符？**

> 见 <https://www.dearimgui.com/faq> 与 <https://github.com/ocornut/imgui/blob/master/docs/FONTS.md>。

### 顾虑

**Q：谁在用 Dear ImGui？能用它做复杂/正式的工具吗？能换掉它的外观吗？为什么用 C++（而不是 C）？**

> 见 <https://www.dearimgui.com/faq>。

### 社区

**Q：我能怎么帮忙？**

A：

- 企业：如果你所在的公司用 Dear ImGui，欢迎联系 "omar AT dearimgui DOT com"，可以聊技术支持的采购、维护或赞助合同——这是对项目最有帮助的支持方式之一。见 <https://github.com/ocornut/imgui/wiki/Funding>。
- 企业：也可以购买 Dear ImGui 自动化/测试引擎的许可证。
- 如果你熟悉 Dear ImGui 和 C++：去看 GitHub issues 和 Wiki，找你能帮上的地方。
- 公开你的使用：开发博客、推文、截图、在某个地方提一句都行。截图或链接可以发到 gallery 帖子里——有画面最好，能给别人灵感；就算没有画面，公开使用情况也能帮这个库积累可信度，帮其他团队做技术选型。
- 有问题、或者需要深入魔改这个库时，就算你不指望官方支持，也值得把问题分享出来（GitHub 或私下都行）。
