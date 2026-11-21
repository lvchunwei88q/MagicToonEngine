# ImGui 源码拆分路线图

适用于 `Engine/ThirdParty/IMGUI` 中的 Dear ImGui 1.92.8 WIP（docking）。目标是让开发者按功能定位实现，同时保持现有公共 API、DLL 导出和编辑器行为兼容。

**当前进度**：已完成 imgui_layout.cpp 迁移；下一项为 imgui_popup.cpp。

## 更新约定

每完成并验证一项，在同一次改动中把对应条目改为 `- [x] ~~原任务~~`，在条目后补上实际验证结果，并更新上方的“当前进度”。部分完成的条目保留未勾选状态，注明已迁移范围和剩余工作。下表行号仅用于定位拆分前的版本，后续以函数和源码分区名为准。

## 第一阶段：基线和首批迁移

- [x] ~~建立构建和冒烟测试基线。~~ 在 VS x64 开发环境执行 `cmake --build out/Ninja_Debug --target ImGuiSmokeTest` 成功。从仓库根目录运行 `Binary/Debug/MagicTest-ImGuiSmokeTest.exe --frames 24 --no-vsync` 和默认无参数运行，分别完成 24 帧和 240 帧，所有断言通过（Win32 + DX11，单视口）。
- [x] ~~迁移 `imgui_layout.cpp`。从原 `imgui.cpp` 的 ITEM SUBMISSION、LAYOUT、SCROLLING 三个连续区段开始（11919–12714 行）。验证控件占位、同行排列和滚动。~~ Debug 构建通过，24 帧测试通过，同排、换行及下一帧滚动读回均符合预期。
- [ ] 迁移 `imgui_popup.cpp`。归集 TOOLTIPS 和 POPUPS（12715–13485 行）。验证提示框显示、模态弹窗关闭及焦点恢复。
- [ ] 迁移 `imgui_navigation.cpp`。归集 KEYBOARD/GAMEPAD NAVIGATION（13770–15507 行）。验证 Tab 切换、方向键导航及弹窗内的焦点行为。

每次迁移一个模块后，重新构建并链接 `IMGUI`、运行冒烟测试，再验证该条目列出的交互。测试入口见 [ImGuiSmokeTest.cpp](../Tests/ImGui/ImGuiSmokeTest.cpp)。

## 第二阶段：停靠、视口和调试

- [ ] 迁移 `imgui_docking.cpp`（17669–21651 行）。停靠节点、拆分与合并、DockSpace、DockBuilder 和布局持久化先整体迁移。验证拖出面板、重新停靠、保存布局和重启恢复。
- [ ] 迁移 `imgui_viewports.cpp`（16559–17668 行）。归集视口和平台窗口生命周期，验证多视口配置下窗口创建、移动和关闭。
- [ ] 迁移 `imgui_platform.cpp`（21652–21883 行）。归集平台辅助函数，复测剪贴板操作。平台辅助函数与现有 Win32、DX11、DX12 后端各自保留清晰的归属。
- [ ] 迁移 `imgui_debug.cpp`。归集错误检查与状态恢复（11442–11918 行），以及 21884 行起的 Metrics、Debug Log、Item Picker 和 ID Stack Tool。确认停靠内部类型的访问边界，验证启用与禁用调试工具的构建。

## 第三阶段：其余核心模块和窗口

- [ ] 迁移 `imgui_input.cpp`（9965–11441 行），归集键鼠输入事件及状态查询，复测鼠标点击和键盘输入。
- [ ] 迁移 `imgui_dragdrop.cpp`（15508–15910 行），验证拖放源、目标与载荷传递。
- [ ] 迁移 `imgui_helpers.cpp`（2086–2950 行），归集几何、字符串、哈希、文件、字符编码和颜色辅助函数。
- [ ] 迁移 `imgui_storage.cpp`（2951–3612 行），归集 ImGuiStorage、文本过滤与缓冲、列表裁剪，复测列表滚动和过滤。
- [ ] 迁移 `imgui_window.cpp`。逐函数检查 MAIN CODE（4674–9535 行），将窗口创建、Begin/End、移动、缩放及 WINDOW FOCUS 区段迁出。在 `imgui.cpp` 保留上下文、初始化、销毁、NewFrame、EndFrame 和 Render。验证创建窗口、移动、缩放、聚焦及连续渲染。

样式、IO、字体与纹理接口、ID 栈、设置和日志先留在 `imgui.cpp`，待上述迁移完成后结合实际修改频率复评。

## 第四阶段：控件、绘制和表格

- [ ] 从 `imgui_widgets.cpp` 分出 `imgui_widgets_textedit.cpp`，归集文本输入、选择、光标与撤销。复测单行和多行编辑。
- [ ] 分出 `imgui_widgets_scalar.cpp`，归集数值格式化、拖动、滑条和数值输入，复测拖动与直接输入。
- [ ] 分出 `imgui_widgets_color.cpp`，归集颜色编辑器与选择器。基础按钮、勾选框和文本显示留在原文件；菜单、标签页及多选按后续开发需要复评。
- [ ] 从 `imgui_draw.cpp` 分出 `imgui_font.cpp`，归集字体图集、字形构建和字体实现。核对 STB 实现宏和内置字体数据的唯一归属，验证图集构建与文本绘制。
- [ ] 复评 `imgui_tables.cpp`。如需拆分，优先将旧 Columns API 迁入 `imgui_columns_legacy.cpp`，分别验证旧分栏与现代表格；如决定保留，记录原因后关闭此项。

## 迁移时的实现约束

- 新实现文件放入 `Engine/ThirdParty/IMGUI/Private/`。[CMake 源码收集规则](../Engine/Cmake/macro.cmake)会递归发现新增的 `.cpp`。
- 按完整函数、类型及条件编译块划界。模块独占的静态辅助函数随实现迁移，确有跨文件调用时使用库内私有声明，并检查相关宏和文件级编译设置。
- 每次迁移保留原函数名、命名空间及上游注释。将行为修改另行提交，方便定位回归和对照上游更新。
