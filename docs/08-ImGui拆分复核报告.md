# ImGui 源码拆分复核报告

复核对象：`Engine/ThirdParty/IMGUI`（Dear ImGui 1.92.8 WIP，`IMGUI_VERSION_NUM 19275`）
复核时间：2026-11-22
基线提交：`b4e0c91`
当前提交：`db1b0e1`
拆分提交范围：`0ae5073..db1b0e1`，共 **18 个提交**（2026-11-21 21:50 ~ 22:57）

---

## 1. 结论摘要

**拆分本身是干净的，可以用。** 代码总量守恒、公共头零改动、没有跨编译单元的全局可变状态、定制点全部保留、干净重建与冒烟测试均通过。主要问题不在"拆得对不对"，而在"拆完之后的新耦合点没有护栏"——4 个新增私有头完全不在依赖图里，改了不会重编译。

| 结论 | 判定 |
| --- | --- |
| 代码完整性（有无丢代码/重复代码） | ✅ 通过：46,898 行 vs 拆分前 46,824 行，+74 行全是新增的包含与守卫 |
| 公共 API / ABI 兼容 | ✅ 通过：`imgui.h`、`imgui_internal.h`、`imconfig.h` 的 diff 为空 |
| 编译与链接 | ✅ 通过：24 个编译单元全量重建成功，0 warning |
| 运行行为 | ✅ 通过：单视口与多视口冒烟测试各 60 帧全部断言通过 |
| 跨 TU 状态共享 | ✅ 通过：无跨 TU 全局变量，状态仍集中在 `ImGuiContext` / `GImGui` |
| 增量构建可信度 | ❌ **不通过**：私有头改动不触发重编（`#deps 0`） |
| 后续可维护性 | ⚠️ 有隐患：私有头不自包含、杂糅头、缺上游映射表 |

**一句话**：拆分质量本身没问题，但**必须先修 `/showIncludes`（docs P1-19），否则拆分引入的 4 个私有头是"改了不生效"的定时炸弹**。

---

## 2. 复核方法与证据

### 2.1 静态核对

| 项目 | 手段 | 结果 |
| --- | --- | --- |
| 代码总量守恒 | 逐文件统计行数，与 `b4e0c91` 四个原文件对比 | 46,898 vs 46,824，差 +74 |
| 公共头是否被改 | `git diff b4e0c91..HEAD -- Public/imgui.h imgui_internal.h imconfig.h` | 输出为空 |
| 定制点是否保留 | 搜索 `IO.h`、`ENGINE_CACHE`、`IDI_ENGINE_ICON`、`CS_DROPSHADOW`、`IM_ASSERT_USER_ERROR` | 全部在原位置保留 |
| 跨文件同名 static | 脚本提取全部 `^static ...(` 定义并按名字分组 | 15 组重名全部是**同文件重载**，无跨文件重复 |
| 私有头里的全局变量 | 搜索 `Internal/*.h`、`Private/*.h` 中的 `static` 定义 | 只有 1 个 `static const` 数组（见 P2-2） |
| 私有头声明与定义是否一致 | 提取私有头 34 条函数声明，反查是否有 `static` 定义 | 0 处不一致 |

### 2.2 构建实测

```
cmake --build out\Ninja_Debug --target IMGUI     # 24 个 TU 全量重建
```

- 结果：成功，**0 warning**（日志 `out/imgui_rebuild.log`）
- 全工程 `cmake --build out\Ninja_Debug`：成功，退出码 0

耗时（本机，Debug，`/Od /Zi`）：

| 场景 | 耗时 |
| --- | --- |
| 无改动空跑（仍跑 MagicCodegenTool） | 3.6 s |
| 改 1 个 cpp 后增量 | 4.5 s |
| IMGUI 全量重建（24 TU） | 7.0 s |

> 结论：拆分带来的编辑-验证循环收益被"每次构建都跑代码生成 + 重新链接 DLL"抵消了大半，实际体验是 4.5 s 而不是秒级。这符合预期，不是拆分缺陷。

### 2.3 运行实测

```
Binary\Debug\MagicTest-ImGuiSmokeTest.exe --frames 60 --no-vsync
Binary\Debug\MagicTest-ImGuiSmokeTest.exe --frames 60 --no-vsync --viewports
```

两次均 `SMOKE TEST PASSED`，退出码 0：

```
frames=60 fontsBuilt=1 drawLists=6 verts=3868 tableRows=300 storage=1337 dock=1 popupVisibleFrames=13
```

### 2.4 依赖跟踪实测（关键证据）

```
ninja -t deps Engine/ThirdParty/IMGUI/CMakeFiles/IMGUI.dir/Private/imgui_docking.cpp.obj
# => #deps 0, deps mtime ... (VALID)
ninja -t deps .../Private/imgui.cpp.obj
# => #deps 0
```

给 `Internal/imgui_core_private.h` 追加一行注释后：

```
ninja -n <imgui_docking.cpp.obj>
# => 只输出 "Re-checking globbed directories" / "Re-running CMake"，没有编译任务
```

也就是说：**改私有头 → 什么都不重编**。这不是拆分引入的新缺陷，但它让拆分的风险从"1 个耦合点"放大到"4 个耦合点 + 34 条跨文件声明"。

---

## 3. 拆分结果

### 3.1 文件地图

| 文件 | 行数 | 内容来源（上游 `[SECTION]`） |
| --- | ---: | --- |
| `Private/imgui.cpp` | 5,763 | INCLUDES / FORWARD DECL / CONTEXT & MEMORY / STYLE & IO / STYLING / RENDER HELPERS / INIT & SHUTDOWN / MAIN CODE（2682–4677）/ ID STACK / LOGGING / SETTINGS / LOCALIZATION |
| `Private/imgui_window.cpp` | 3,128 | 窗口生命周期、Begin/End、移动缩放、WINDOW FOCUS |
| `Private/imgui_docking.cpp` | 3,982 | DOCKING 全段（含 DockBuilder、Settings） |
| `Private/imgui_viewports.cpp` | 1,156 | VIEWPORTS, PLATFORM WINDOWS |
| `Private/imgui_debug.cpp` | 2,947 | ERROR CHECKING & STATE RECOVERY + METRICS / DEBUG LOG / ITEM PICKER / ID STACK TOOL |
| `Private/imgui_platform.cpp` | 239 | PLATFORM DEPENDENT HELPERS |
| `Private/imgui_input.cpp` | 1,805 | INPUTS（键鼠事件、状态查询） |
| `Private/imgui_navigation.cpp` | 1,761 | KEYBOARD/GAMEPAD NAVIGATION |
| `Private/imgui_dragdrop.cpp` | 412 | DRAG AND DROP |
| `Private/imgui_helpers.cpp` | 879 | 几何 / 字符串 / 哈希 / 文件 / 编码 / 颜色辅助 |
| `Private/imgui_storage.cpp` | 672 | ImGuiStorage / TextFilter / TextBuffer / ListClipper |
| `Private/imgui_layout.cpp` | 804 | ITEM SUBMISSION / LAYOUT / SCROLLING |
| `Private/imgui_popup.cpp` | 783 | TOOLTIPS / POPUPS |
| `Private/imgui_tables.cpp` | 4,219 | 现代表格 |
| `Private/imgui_columns_legacy.cpp` | 459 | 旧 Columns API |
| `Private/imgui_draw.cpp` | 2,543 | ImDrawList / 渲染辅助 |
| `Private/imgui_font.cpp` | 4,232 | 字体图集、字形构建、内置字体、STB 实现 |
| `Private/imgui_widgets.cpp` | 6,607 | 按钮 / 文本 / 菜单 / 标签页 / 选择 / 绘图类控件 |
| `Private/imgui_widgets_textedit.cpp` | 1,791 | InputText 与文本编辑、撤销 |
| `Private/imgui_widgets_scalar.cpp` | 1,767 | 数值格式化、Drag/Slider/Input 数值 |
| `Private/imgui_widgets_color.cpp` | 949 | ColorEdit / ColorPicker |
| **合计** | **46,898** | |

后端三个文件（`imgui_impl_win32/dx11/dx12.cpp`）未拆分，原样保留。

### 3.2 新增私有头

| 头文件 | 行数 | 内容 |
| --- | ---: | --- |
| `Internal/imgui_core_private.h` | 56 | 30 条跨文件声明 + 2 个常量 + 2 个 inline 辅助 |
| `Internal/imgui_docking_private.h` | 17 | `ImGuiDockNodeSettings` 定义（原在 imgui.cpp） |
| `Internal/imgui_platform_private.h` | 44 | Win32 平台包含策略 + 4 条平台回调声明 |
| `Internal/imgui_widgets_private.h` | 6 | `GDefaultRgbaColorMarkers` 常量数组 |

---

## 4. 问题清单

### P1-1 私有头不在依赖图里，改了不会重编译（**必修**）

- **位置**：`Engine/Cmake/common.cmake:8-9`（剥离 `/showIncludes`），受影响面为 `Internal/*.h` 与全部 `Private/*.cpp`
- **现象**：`ninja -t deps` 所有 IMGUI 对象均为 `#deps 0`；修改 `imgui_core_private.h` 后 `ninja -n` 无任何编译任务
- **影响**：
  - 改了私有头里的函数签名 → 旧的 `.obj` 仍按旧声明编译，链接期可能报 `LNK2019`，也可能"链接成功但行为是旧的"；
  - 拆分后这类声明有 34 条、分布在 4 个头里，命中概率远高于拆分前（拆分前只有 1 个 cpp，不存在这个面）；
  - 更隐蔽的是：改完头 + 只改了一个 cpp，看起来"编过了"，实际其他 20 个 TU 还是旧的。
- **修法**：删除 `common.cmake` 第 8–9 行对 `CMAKE_DEPFILE_FLAGS_C` / `CMAKE_DEPFILE_FLAGS_CXX` 的 `/showIncludes` 剥离（即 docs 中的 P1-19），重新 configure。首次会触发全量重编，属预期成本。
- **验收**：`ninja -t deps ...imgui_docking.cpp.obj` 出现 `#deps 40+`；改 `imgui_core_private.h` 后 `ninja -n` 能看到 20+ 个编译任务。

> 建议把这一条排在所有 ImGui 后续改造之前，因为后面任何"改私有头 → 验证"的动作都会踩它。

### P2-1 私有头不自包含，依赖严格的包含顺序

- **位置**：`Internal/imgui_core_private.h:1`、`imgui_docking_private.h:1`、`imgui_widgets_private.h:1`
- **证据**：单独编译三个头（各自一个空 TU）直接失败：

  ```
  imgui_core_private.h(7): error C4430: 缺少类型说明符
  imgui_core_private.h(8): error C2065: “ImGuiWindow”: 未声明的标识符
  imgui_core_private.h(29): error C2065: “ImGuiWindowSettings”: 未声明的标识符
  ...（30+ 条）
  ```

- **影响**：
  - 新文件的 `#include` 必须严格按 `imgui.h → imgui_internal.h → *_private.h` 排列，顺序错了就是几十条报错，且报错位置在头文件里、不在新文件里，不好定位；
  - clangd / IntelliSense 单独打开私有头会满屏红；
  - `imgui_docking_private.h` 用了 `memset` 但没有 `<string.h>`，`ImVec2ih` 也来自 `imgui_internal.h`。
- **修法**：每个私有头自己带上 `#include "imgui.h"` 与 `#include "imgui_internal.h"`（`imgui_internal.h` 不会反向包含私有头，没有循环），`docking_private` 再补 `<string.h>`。私有头一旦自包含，就把"包含顺序"这条隐性规则消掉了。

### P2-2 `imgui_widgets_private.h` 用 `static const` 定义数组 → 两份副本

- **位置**：`Internal/imgui_widgets_private.h:3`

  ```cpp
  static const ImU32 GDefaultRgbaColorMarkers[4] = { ... };
  ```

- **事实**：该头被 `imgui_widgets_color.cpp`、`imgui_widgets_scalar.cpp` 两个 TU 包含，各自生成一份 16 字节的数组（内部链接，不违反 ODR，编译也不报）
- **风险**：目前是 `const`，两份内容一致，无行为问题；但只要有人把 `const` 去掉（比如加一个"标记色可配置"），就会变成两份独立状态，改一处另一处不变，属于典型的静默分叉
- **修法**：改为 `inline constexpr ImU32 GDefaultRgbaColorMarkers[4]`（C++17 起头文件里安全的单一定义方式）

### P2-3 `imgui_core_private.h` 是杂糅头，正在长成第二个 `imgui_internal.h`

- **位置**：`Internal/imgui_core_private.h:5-40`
- **事实**：30 条声明覆盖 6 个领域——窗口边框/设置应用、Viewport 生命周期、错误检查与调试工具、键盘鼠标输入更新、导航、滚动计算。`imgui_viewports.cpp`、`imgui_navigation.cpp`、`imgui_input.cpp`、`imgui_window.cpp`、`imgui_popup.cpp`、`imgui_layout.cpp`、`imgui_debug.cpp` 都要包含它
- **影响**：任何新增的跨文件函数都会被顺手塞进这里；半年后它就会变成"谁都能改、改了牵动 7 个文件"的公共垃圾场
- **修法**：按域拆成 `imgui_window_private.h` / `imgui_viewport_private.h` / `imgui_nav_private.h`（或 `imgui_input_private.h`），只保留真正全局的 `WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER` 与 `IMGUI_VIEWPORT_DEFAULT_ID`

### P2-4 上游同步没有映射表，升级会退化成手工考古

- **位置**：`docs/ImGui拆分路线图.md`
- **事实**：路线图用**拆分前的行号**（如 `imgui.cpp 17669–21651`）描述每个模块，拆分后这些行号全部失效；文档里也没有"上游 `[SECTION]` → 当前文件"的反向表
- **影响**：下次把 1.92.x 升到 1.93 时，需要把上游 diff 的每个 hunk 手工映射到 21 个文件之一，成本随拆分粒度线性上升
- **修法**：
  1. 新建 `Engine/ThirdParty/IMGUI/UPSTREAM_MAP.md`，按 `[SECTION]` 名列出 `文件:起始行`（本报告第 3.1 节可直接作为初版）；
  2. 记录上游基线（`v1.92.8-docking` + commit hash）与"本地改造清单"（IO.h、ENGINE_CACHE、图标、错误宏、导出宏）；
  3. 升级时先 diff 上游四文件，再按 map 落到本地文件，最后跑冒烟测试。

### P2-5 构建元数据被拆分放大

- **位置**：`out/engine_codegen_generate/engine_generate_info/engine_headers.buildmeta`
- **事实**：拆分前 IMGUI 占 21 / 240 条，拆分后 **44 / 263 条**（21 个 cpp + 23 个头），其中包含 1.1 MB 的 `Public/front/Roboto_Regular_front.h`
- **影响**：`add_module_sources(IMGUI)` 把所有 `.h` 也写进 header 列表，MagicCodegenTool 每次构建都要读取+哈希这些文件；实测每次构建的代码生成阶段稳定耗时约 2–3 秒（"无改动空跑 3.6 s"里大头就是它）
- **修法**：给 `add_module_sources` 加参数，第三方模块只登记 `.cpp`（或只登记 `Public/` 下的头）；至少把 `Internal/`、`front/` 排除掉。这同时也会降低 P1-1 修复后的全量重编成本。

### P3-1 `IMGUI_DEFINE_MATH_OPERATORS` 的写法不统一

- 有 `#ifndef` 包裹：`imgui.cpp`、`imgui_debug.cpp`、`imgui_dragdrop.cpp`、`imgui_draw.cpp`、`imgui_font.cpp`、`imgui_helpers.cpp`、`imgui_input.cpp`、`imgui_storage.cpp`、`imgui_tables.cpp`、`imgui_widgets_color.cpp`、`imgui_widgets_scalar.cpp`
- 裸 `#define`：`imgui_docking.cpp:2`、`imgui_layout.cpp:2`、`imgui_navigation.cpp:2`、`imgui_popup.cpp:2`、`imgui_viewports.cpp:2`
- 后果：如果哪天把该宏统一挪到 `imconfig.h`（推荐做法，也是之前评审的建议），这 5 个文件会报 `C4005` 宏重定义
- 修法：统一成 `#ifndef ... #define ... #endif`

### P3-2 提交信息与实际动作不符

- `db1b0e1` 标题是 `add private headers for docking, platform, and widgets`，实际内容是把 4 个头从 `Private/` 移到 `Internal/`，`0 insertions, 0 deletions`
- 影响：以后按提交信息检索"私有头是哪次引入的"会找不到（实际引入分散在 `b6bba19` 等提交里）
- 建议：这类纯移动用 `refactor(imgui): move private headers to Internal/`，并在正文写"无内容改动"

### P3-3 路线图只写了"做完了"，没写"现在长什么样"

- `docs/ImGui拆分路线图.md` 开头写"四个阶段的计划项均已完成"，但没有拆分后的文件清单、每文件行数、私有头职责
- 建议：把本报告第 3 节并进去（或直接互链），让路线图从"计划文档"变成"结构文档"

### P3-4 冒烟测试没有进 CTest，18 个提交的验证是人工的

- `Tests/CMakeLists.txt` 里只有 `add_engine_test_library`，没有 `add_test`
- 后果：拆分期间每个提交都要手动跑 exe；回归只能靠人记得
- 修法：

  ```cmake
  add_test(NAME ImGuiSmokeTest COMMAND ImGuiSmokeTest --frames 60 --no-vsync)
  ```

  （注意：`--viewports` 依赖桌面会话，适合单独一个 `LABELS desktop` 的用例，不要塞进默认测试集）

### P3-5 freetype 头残留（裁剪遗留，与拆分无关）

- `Private/imgui_font.cpp` 在 `#ifdef IMGUI_ENABLE_FREETYPE` 下 `#include "misc/freetype/imgui_freetype.h"`，但该目录在裁剪时已删除
- 现状不报错（宏默认关闭），但一旦有人打开 `IMGUI_ENABLE_FREETYPE` 就会立刻 C1083
- 建议：要么补回 freetype 模块，要么在 `imconfig.h` 里显式 `#define IMGUI_DISABLE_FREETYPE` 并删掉这段死引用

### P3-6 下一批拆分候选

单文件仍超过 3,000 行的：

| 文件 | 行数 | 建议 |
| --- | ---: | --- |
| `imgui_widgets.cpp` | 6,607 | 按控件族分：`imgui_widgets_menu.cpp`（Menu/MenuItem/BeginMenu）、`imgui_widgets_tabs.cpp`（TabBar/TabItem）、`imgui_widgets_select.cpp`（Selectable/ListBox/Combo）、`imgui_widgets_plot.cpp`（Plot/ProgressBar） |
| `imgui.cpp` | 5,763 | MAIN CODE（2682–4677，约 2,000 行）可拆 `imgui_frame.cpp`（NewFrame/EndFrame/Render）与 `imgui_item.cpp`（Item 提交与状态） |
| `imgui_font.cpp` | 4,232 | STB 实现（约 2,000 行）可独立成 `imgui_font_stb.cpp`，与字体图集解耦 |
| `imgui_tables.cpp` | 4,219 | 表格可再分"布局计算"与"绘制/交互" |
| `imgui_docking.cpp` | 3,982 | 可拆 `imgui_docknode.cpp`（树操作）与 `imgui_dockbuilder.cpp`（Builder + Settings） |

不建议继续往下切到 500 行以下的碎片——跨文件声明会快速膨胀，收益小于维护成本。

---

## 5. 复核的正面结论（保留证据，便于回归时对照）

1. **代码量守恒**：46,898 行 vs 46,824 行，差 +74，全部是每个新文件头部新增的 `#ifndef IMGUI_DEFINE_MATH_OPERATORS` / `#ifndef IMGUI_DISABLE` 守卫与 include；
2. **公共头零改动**：`imgui.h` / `imgui_internal.h` / `imconfig.h` 的 `git diff` 为空 → 外部调用方（RenderUI、RenderEditor、Tests）无需任何改动，DLL 导出集不变；
3. **没有跨 TU 可变状态**：私有头里只有声明和 2 个常量；`imgui.cpp` 剩下的 `static` 变量（`GImAllocatorAllocFunc/FreeFunc/UserData`）仍只被本 TU 使用，`imgui_font.cpp` 的 `g_CharClassifierIsSeparator_*` 同理；
4. **`static` 收敛**：单文件 `static` 定义数从 `imgui.cpp` 的 173 降到最大 49（`imgui_font.cpp`）/ 48（`imgui_docking.cpp`），函数作用域清晰；
5. **定制点全部在位**：

   | 定制 | 位置 |
   | --- | --- |
   | `#include <IO.h>` + INI 落到 `ENGINE_CACHE/Magic-WindowLayout.ini` | `imgui.cpp:1240`、`5452`、`5542` |
   | Win32 图标 + `CS_DROPSHADOW` | `backends/imgui_impl_win32.cpp:105`、`1483`、`1487` |
   | 错误宏改走 `LOG_ERROR` + `NotificationSubsystem` | `imconfig.h:148-154` |
   | DLL 导出宏 | `imgui.h:85-93`（未改动） |

6. **无同名 static 跨文件重复**：15 组重名经脚本核对全部是同文件内的重载/多声明。

---

## 6. 本轮未覆盖的部分

- **未逐个验证 18 个中间提交的可构建性**：只验证了 `db1b0e1` 的干净重建。若要做 `git bisect`，建议补一次 `for c in $(git rev-list 0ae5073^..HEAD); do git checkout $c && build; done`（成本较高，本次未做）。
- **未做拆分前后的像素级/绘制结果对比**：只做了行为断言（绘制列表数、顶点数、表格行数、Storage 值、Dock 节点、Modal 帧数）。若要更严格，可比对连续 N 帧的 `ImDrawData` 顶点缓冲哈希。
- **`IMGUI_DISABLE_DEBUG_TOOLS` 变体**：单独编译 `imgui_debug.cpp`（带该宏）通过，但没有跑完整变体的构建与测试（`out/debug-tools-off` 目录缺 `CMakeCache.txt`，无法直接复用）。
- **旧 `out/debug-tools-off`、`out/build`、`out/debug` 等目录已过期**，不代表当前代码状态。

---

## 7. 建议的处理顺序

| 顺序 | 事项 | 理由 |
| --- | --- | --- |
| 1 | 修 `common.cmake:8-9` 的 `/showIncludes`（docs P1-19） | 不修，后面所有"改私有头再验证"都不可信；且拆分把风险面放大了 4 倍 |
| 2 | 私有头自包含（补 `imgui.h`/`imgui_internal.h`/`<string.h>`） | 成本约 10 分钟，直接消掉"包含顺序"这条隐性规则 |
| 3 | `GDefaultRgbaColorMarkers` 改 `inline constexpr` | 一行改动，避免未来两份状态分叉 |
| 4 | 拆 `imgui_core_private.h`（按域） | 趁声明只有 30 条时拆，越晚越难 |
| 5 | 写 `UPSTREAM_MAP.md` + 记录上游基线 | 决定后续能不能升级上游 |
| 6 | 冒烟测试接 CTest；`add_module_sources` 排除 ThirdParty 的 `.h` | 工程化收尾 |
| 7 | 继续拆 `imgui_widgets.cpp` / `imgui.cpp` 的 MAIN CODE | 按实际修改频率决定，不必一次做完 |

---

## 8. 复现命令

```bash
# 提交范围
git log --oneline b4e0c91..HEAD

# 代码量对比
git show b4e0c91:Engine/ThirdParty/IMGUI/Private/imgui.cpp > /tmp/old.cpp && wc -l /tmp/old.cpp

# 干净重建（需先进入 VS x64 环境）
"D:\VS2026\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake --build out\Ninja_Debug --target IMGUI

# 依赖跟踪（关键）
out\Ninja_Debug> ninja -t deps Engine/ThirdParty/IMGUI/CMakeFiles/IMGUI.dir/Private/imgui_docking.cpp.obj

# 行为验证
Binary\Debug\MagicTest-ImGuiSmokeTest.exe --frames 60 --no-vsync
Binary\Debug\MagicTest-ImGuiSmokeTest.exe --frames 60 --no-vsync --viewports
```
