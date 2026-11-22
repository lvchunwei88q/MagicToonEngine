# MagicToonEngine 代码缺陷审查报告

本目录是对 MagicToonEngine 现有源代码做静态审查后产出的缺陷记录，按严重程度分级，每条缺陷都给出**文件:行号**、触发条件、影响和修复建议。

## 文档索引

| 文件 | 内容 |
| --- | --- |
| [01-缺陷总览.md](01-缺陷总览.md) | 全部缺陷清单表格（按等级分组），可作为修复排期清单 |
| [02-P0-严重缺陷.md](02-P0-严重缺陷.md) | 内存安全 / 必然崩溃 / 数据损坏类问题，共 1 条 |
| [03-P1-高危缺陷.md](03-P1-高危缺陷.md) | 特定路径崩溃、功能失效、并发与启动失败类问题，共 15 条 |
| [04-P2-中等缺陷.md](04-P2-中等缺陷.md) | 逻辑错误、健壮性不足、性能与可维护性隐患，共 42 条 |
| [05-P3-低危与代码质量.md](05-P3-低危与代码质量.md) | 命名、拼写、死代码、文档与构建脚本细节，共 19 条 |
| [06-架构与工程化建议.md](06-架构与工程化建议.md) | 跨模块的架构级建议与分批次修复路线图 |
| [07-修复复核记录.md](07-修复复核记录.md) | 十轮复核历史，第十二至十四节记录增量生成实测、P2-46 修复验证与第十轮结论 |
| [08-ImGui拆分复核报告.md](08-ImGui拆分复核报告.md) | IMGUI 第三方库 2026-11-21 拆分（18 个提交、21 个 cpp）的完整记录：四阶段路线图、拆分后文件地图、复核结论、问题清单与处理顺序（原 `ImGui拆分路线图.md` 已并入本文档） |

## 当前状态（`614159f`，2026-11-20）

- 活动条目 **77 项**，P0 为 1 项，P1 为 15 项，P2 为 42 项，P3 为 19 项。
- 第九轮（`49c55d3..788c173`）只有 `788c173` 动了代码：`Clang/CompileCommands.cpp` 新增 `FindCompiler`（跳过前置 `sccache`/`ccache` 与可选 `--`）与 `StripGccVersion`（识别 `g++-13` 这类数字后缀），**P2-46 修复并移出清单**。
- 第十轮（`788c173..614159f`，8 个提交，另有 `2b630c4`）：`2b630c4` 把"解析源 / 生成体"两处手写契约收敛成 `Internal/Codegen/ObjectInterfaceContract.h` 单一表（3 处降为 2 处），并修掉成功路径不卸载 → **P2-39 删除**；`220bba3` 把类 ID 从进程内自增换成 `HashFNV1a64(headerPath + "|" + className)`、`ObjectTag` 也从 `std::hash<std::string>` 换成同一套 FNV → **P2-45 删除**；`f47dc50`/`a03a50e` 把逐头解析改成按头并行（每 job 独立 `CXIndex`，结果按输入序回收）；`9969b88`/`614159f` 把 `IO` 全量 API 从 `std::wstring` 迁到 `std::filesystem::path`（`CACHE`/`CONFIG` 宏改名 `ENGINE_CACHE`/`ENGINE_CONFIG`，删除 `SerializeMacro.h`）。新增 **P1-19**、**P2-47 / P2-48**、**P3-19 至 P3-23**。
- **部分修复 3 条**：P2-22（`ObjectType` 仍不校验）、P2-37（同模块同名仍撞，且该处宽字符写法在非 Windows 目标编不过）、P3-13（README / CMake 注释残留）。
- 已修复的 14 条（P0-02、P1-01、P1-02、**P1-03**、P1-17、P2-21、P2-32、**P2-38**、**P2-39**、**P2-45**、**P2-46**、P3-12、P3-14、P3-15）已从缺陷清单移除，修复过程与提交对照见 [07-修复复核记录.md](07-修复复核记录.md)。
- **第七轮（`e2e45d2..5e72262`，13 个提交）修掉 2 条、新增 4 条**：`5ad6ae7` 把增量判据从 mtime 换成**文件内容 FNV-1a 哈希**（`GetFileContentHashesParallel` 并行算），顺带整条修掉 **P2-38**（缓存改为"全部任务成功后落盘"）与 **P1-03**（坏缓存行 `try/catch` 跳过）；`5e72262` 把 `FindEngineClass.cpp`（1256 行）拆进 `Clang/`（`AstClassCollector` / `CompileCommands` / `HeaderMacroScan` / `ClangOwnership` / `ClangPathUtility`），逻辑等价并修掉旧实现"解析失败仍返回半成品 TU 时泄漏"。新增 **P2-42**（libclang 解析靠白名单开关 + `INCLUDE` 兜底，任一 error 诊断即整次生成失败）与 **P3-16 / P3-17 / P3-18**。
- 注意：`MAGIC_DEV` 仍为 `0`，增量路径在线，P1-04 的"增量脱钩"仍在（判据换了、仍不看产物）；一旦把宏改回 `1`，增量整体关闭且 `Run()` 收尾必定报错返回（**P2-41**，现在与磁盘上有没有缓存文件无关，任何 DEV 构建必失败）。
- P1-02 / P2-32 修复后仍留了两处缺口（读文件任务体未自包 `try/catch`、MagicCodegenTool 未注册 `ErrorCapture`），见 [07 第 7.3 节](07-修复复核记录.md)；其中第二条在 `a0dc2d6` 之后**依然成立**（`LOG_OUTPUT` 只在 `_DEBUG` 有输出）。

## 审查范围

已逐文件阅读的部分（约 68 个源文件、16k 行）：

- `Engine/Source/Core/Core`：对象系统、子系统框架、JobSystem、LuaVM、文件与路径、序列化设施、数学库、Checker
- `Engine/Source/Core/Logger`：日志实现与后台刷盘线程
- `Engine/Source/Programs/MagicCodegenTool`：反射代码生成全流程（扫描 / 成员解析 / 宏生成 / 文件写出），以及 `Clang/` 下的 libclang 支持层（`AstClassCollector`、`CompileCommands`、`HeaderMacroScan`、`ClangOwnership`、`ClangPathUtility`）
- `Engine/Source/Runtime/RenderCore`、`RenderRT`：D3D11 设备、缓冲区管理、交换链与 RTV
- `Engine/Source/Editor`：Editor、MagicEditor、MagicLauncher、EngineProject、AssetPipeline、RenderUI、RenderEditor、RenderLauncher
- 根目录 `CMakeLists.txt`、`Engine/Cmake/*.cmake`、`Build.bat`、`clice.toml`、`README.md`、`Tests`

除核对 WIC 输出所有权和 ImGui ID 作用域外，未审查 `Engine/ThirdParty`（第三方库本体，含 libclang 二进制）、`Engine/Content` 资源、ImGui 通知系统的绘制细节（`Tools/ImGuiNotify.h` 属于上游改造代码）。

## 分级标准

| 等级 | 判定标准 | 处理建议 |
| --- | --- | --- |
| **P0 严重** | 确定的未定义行为（悬垂/二次释放/越界）、必然崩溃、不可逆数据丢失。不修则任何后续功能都建立在不稳地基上 | 立即修复，修复前不应继续扩展功能 |
| **P1 高** | 特定但常见的路径下崩溃、功能完全失效、启动失败、并发数据竞争、构建产物不确定 | 当前迭代必修 |
| **P2 中** | 逻辑错误（结果不对但不崩）、异常未捕获的边界情况、性能瓶颈、误导性的 API 语义 | 排入近期迭代 |
| **P3 低** | 命名/拼写、死代码、注释与实现不符、文档与脚本不一致 | 顺手清理 |

## 关于结论的说明

- 本轮在临时目录用已有 Debug 工具复跑了全量生成（删哈希缓存后 238 个头 / 8 个类 / 退出码 0），并实测了 `ninja -t deps`（121 个 obj 全为 `#deps 0`）。其余本轮结论来自静态调用链检查，未运行图形编辑器。
  - P0/P1 中标注了「需要 X 条件才触发」的条目，触发条件已在正文写明；
  - 少数条目（例如「测试断言与实现矛盾」）属于逻辑推演结论，正文给出了推导过程，便于复核。
- 第十轮基准为 `614159f`，覆盖 `2b630c4` 之后 `f47dc50`、`a03a50e`、`dc27caf`、`220bba3`、`74d70c0`、`9969b88`、`06edb21`、`614159f` 八个提交；增量 ID 验证与历史检查点见 [修复复核记录](07-修复复核记录.md)。
- 报告不包含"建议重写"这类空泛结论。`06-架构与工程化建议.md` 中的建议均由具体缺陷归纳而来。

## 修复顺序建议（摘要）

1. **先修 P1-19**（`common.cmake:8-9` 的 `/showIncludes`）：它是唯一让"改了头却不重编"的缺陷，不修则后面所有验证都不可信（改完会触发一次全量重编，属预期成本）。
2. 优先处理 P0-03，覆盖交换链重建失败后的绘制路径。
3. 再修 P1-02 的残留与退出码语义：把 `ErrorCapture` 接到 MagicCodegenTool 的日志、读文件任务体自包 `try/catch`，以及 P3-16 的"空清单被误判失败"。（P2-39 已随 `2b630c4` 修掉，Logger 在成功路径上也会正常刷盘）
4. 然后 P1-04 / P2-36 / P2-41：把增量生成做成"真的正确"——补上"产物是否存在"的判断，并把 `MAGIC_DEV` 拆成"日志"和"增量"两个宏（否则一次 DEV 构建必然失败）。内容哈希那一半 `5ad6ae7` 已经做完，不需要再动。
5. 处理 P2-40、P2-42、P2-48，分别检查模块名匹配、强制包含和"同一头内同名类"的 ID 唯一性（P2-45 的跨头碰撞已随 `220bba3` 修掉）。
6. 之后按模块推进 P1-05 ~ P1-16 与 P2。
