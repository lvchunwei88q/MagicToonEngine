# MagicToonEngine

[简体中文](README.md) | [English](README.en)

MagicToonEngine 是一个基于 C++ 的卡通渲染验证引擎，主要用于渲染效果原型开发与可视化验证。项目采用 CMake 组织模块，并支持使用 Visual Studio / MSBuild 和 Ninja 进行本地构建。

`Windows 10/11 x64 only. Other operating systems (including Linux, macOS, Android, iOS) are not supported.`

## 环境要求

| 组件 | 要求 | 说明 |
| --- | --- | --- |
| 操作系统 | Windows 10/11 x64 | 当前仅支持 Windows 平台 |
| Visual Studio | 2022 或 2026 | 需安装“使用 C++ 的桌面开发”工作负载和 Windows SDK |
| CMake | 3.20 或更高 | 由根目录 `CMakeLists.txt` 控制最低版本 |
| Git | 可用版本 | 用于获取源码 |
| Git LFS | 必需 | 用于下载 `ThirdParty` 中的大文件 |
| Ninja | 构建使用 Ninja 时需要 | 与 MSBuild / Visual Studio 构建方式互补 |

> 使用 Ninja 构建时，建议从 `x64 Native Tools Command Prompt for VS` 或等价的开发者 PowerShell 启动，以确保 `cl` 与环境变量可用。

## 获取源码

先安装 Git LFS 并初始化：

```powershell
git lfs install
git clone https://github.com/lvchunwei88q/MagicToonEngine.git
cd MagicToonEngine
```

如果仓库已存在，则可以在项目目录中执行：

```powershell
git lfs pull
```

或者直接使用仓库自带脚本更新：

```powershell
.\Update.bat
```

> `ThirdParty` 目录中包含项目依赖的第三方库和资源；若未安装 Git LFS，可能无法正确获取这些文件。

## 构建

在仓库根目录执行：

```powershell
.\Build.bat
```

脚本会依次询问：

1. 构建方式：`1` 使用 Visual Studio / MSBuild，`2` 使用 Ninja
2. 构建配置：`1` 使用 Debug，`2` 使用 Release

根据选择，脚本会自动生成并构建项目，并输出到对应的 `Binary` 目录。Visual Studio 构建使用 x64 架构，Ninja 构建支持生成 `compile_commands.json`。

也可以手动使用 CMake 配置构建，例如：

```powershell
cmake -S . -B out\VS_Debug -G "Visual Studio 17 2022" -A x64
cmake --build out\VS_Debug --config Debug
```

## 生成项目文件

如果希望在 IDE 中直接查看和编辑代码，可运行：

```powershell
.\GenerateVSProject.bat
```

脚本会在 `out\VS` 目录生成项目文件，并提示是否打开解决方案。

## 构建产物

构建完成后，可执行文件和运行时 DLL 会输出到：

```text
Binary\Debug\
Binary\Release\
```

其中：
- `Binary\Debug\`：调试版本产物
- `Binary\Release\`：发布版本产物

## 项目结构

```text
Engine/                  # 核心引擎、运行时和编辑器模块
ThirdParty/              # 第三方依赖（如 Lua、IMGUI、SPIRV-Cross 等）
Binary/                  # 构建输出目录
out/                     # CMake 生成目录
CMakeLists.txt           # 顶层 CMake 项目入口
Build.bat                # 一键构建脚本
GenerateVSProject.bat    # 生成 Visual Studio 项目文件
Update.bat               # 更新源码和 LFS 依赖
```

## 说明

本项目当前面向 Windows 平台的本地开发与调试，适合在可视化评估、渲染效果验证和引擎原型开发场景中使用。若你希望在 IDE 中进一步调试或扩展渲染功能，可先生成 Visual Studio 解决方案，再从 `out\VS` 处打开项目。