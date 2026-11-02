# MagicToonEngine

[简体中文](README.md) | [English](README.en.md)

MagicToonEngine is a C++-based cartoon rendering validation engine focused on rendering prototype development and visual verification. The project is organized with CMake and supports local builds through Visual Studio / MSBuild and Ninja.

`Windows 10/11 x64 only. Other operating systems (including Linux, macOS, Android, iOS) are not supported.`

## Requirements

| Component | Requirement | Notes |
| --- | --- | --- |
| Operating System | Windows 10/11 x64 | The project currently supports Windows only |
| Visual Studio | 2022 or 2026 | Install the Desktop development with C++ workload and Windows SDK |
| CMake | 3.20 or newer | Minimum version is defined by the root `CMakeLists.txt` |
| Git | Any recent version | Used to fetch source code |
| Git LFS | Required | Required to download large files in `ThirdParty` |
| Ninja | Required when using Ninja | Not required for Visual Studio / MSBuild builds |

> For Ninja builds, it is recommended to launch the build from `x64 Native Tools Command Prompt for VS` or the corresponding developer PowerShell to ensure `cl` and environment variables are available.

## Get the Source Code

Install Git LFS and initialize it first:

```powershell
git lfs install
git clone https://github.com/lvchunwei88q/MagicToonEngine.git
cd MagicToonEngine
```

If the repository already exists, run:

```powershell
git lfs pull
```

Or update it with the repository script:

```powershell
.\Update.bat
```

> The `ThirdParty` directory contains project dependencies and assets. Without Git LFS, these files may not be fetched correctly.

## Build

Run the build script from the repository root:

```powershell
.\Build.bat
```

The script will prompt for:

1. Build method: `1` for Visual Studio / MSBuild, `2` for Ninja
2. Build configuration: `1` for Debug, `2` for Release

The selected configuration will be generated and built automatically, and the output will be placed in the corresponding `Binary` directory. Visual Studio builds use the x64 architecture, and Ninja builds can generate `compile_commands.json`.

You can also configure the project manually with CMake:

```powershell
cmake -S . -B out\VS_Debug -G "Visual Studio 17 2022" -A x64
cmake --build out\VS_Debug --config Debug
```

## Generate Project Files

To inspect and edit the project in an IDE, run:

```powershell
.\GenerateVSProject.bat
```

The script generates project files under `out\VS` and gives you the option to open the solution.

## Build Artifacts

After a successful build, the executables and runtime DLLs are placed in:

```text
Binary\Debug\
Binary\Release\
```

Where:
- `Binary\Debug\` contains debug build outputs
- `Binary\Release\` contains release build outputs

## Project Structure

```text
Engine/                  # Core engine, runtime, and editor modules
ThirdParty/              # Third-party dependencies (for example Lua, IMGUI, SPIRV-Cross, etc.)
Binary/                  # Build output directory
out/                     # CMake generation directory
CMakeLists.txt           # Top-level CMake entry point
Build.bat                # One-click build script
GenerateVSProject.bat    # Generate Visual Studio project files
Update.bat               # Update source code and LFS dependencies
```

## Third-Party Library Overview

The project depends on several third-party components for rendering, scripting, serialization, and toolchain support. A brief summary is listed below:

| Library | Purpose | Description |
| --- | --- | --- |
| `DirectXShaderCompiler` | Shader compilation | Compiles HLSL / DirectX shaders into executable formats such as DXIL or SPIR-V. |
| `IMGUI` | Debug UI | Provides a lightweight immediate-mode GUI for editor panels, debug windows, and tools. |
| `Lua` | Scripting | Enables script-driven logic, configuration, and data behaviors. |
| `SPIRV-Cross` | Shader cross-compilation | Converts SPIR-V intermediate code to formats usable by other graphics APIs. |
| `WICTextureLoader` | Image loading | Loads common image formats on Windows, such as PNG and JPG textures. |
| `cereal` | Serialization | Provides C++ object serialization and deserialization for config and asset data. |
| `cmdline` | Command-line parsing | Parses command-line options for tools and debugging workflows. |
| `ctre` | Regex matching | Provides high-performance text matching for parsing and filtering tasks. |
| `magic_enum` | Enum reflection | Converts C++ enums to strings and provides reflection-like utilities for debugging and tooling. |
| `nlohmann` | JSON processing | Reads and writes JSON configuration files and runtime data. |
| `ufbx` | 3D model loading | Imports and processes 3D model assets such as FBX files. |
| `Vulkan SDK` | Vulkan API support | Provides Vulkan headers, validation layers, and tools required for Vulkan rendering. |

## Notes

This project is currently focused on Windows local development and debugging, and is suitable for visual evaluation, rendering effect verification, and engine prototype development. If you want to debug or extend rendering features in the IDE, generate the Visual Studio solution first and open the project from `out\VS`.