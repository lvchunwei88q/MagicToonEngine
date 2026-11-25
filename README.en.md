# MagicToonEngine

[简体中文](README.md) | [English](README.en.md)

MagicToonEngine is a C++-based cartoon rendering validation engine focused on rendering prototype development and visual verification. The project is organized with CMake; the included `Build.bat` builds with Ninja, and Visual Studio / MSBuild projects can also be generated manually.

`Windows 10/11 x64 only. Other operating systems (including Linux, macOS, Android, iOS) are not supported.`

## Requirements

| Component | Requirement | Notes |
| --- | --- | --- |
| Operating System | Windows 10/11 x64 | The project currently supports Windows only |
| Visual Studio | 2022 or 2026 | Install the Desktop development with C++ workload and Windows SDK to provide the MSVC compiler |
| CMake | 3.20 or newer | Minimum version is defined by the root `CMakeLists.txt` |
| Git | Any recent version | Used to fetch source code |
| Git LFS | Required | Required to download large files in `Engine/ThirdParty` |
| Ninja | Required | `Build.bat` uses the Ninja generator |
| Python | 3.12 or newer | Build helper scripts (environment checks and build automation) are written in Python; must be callable via `py -3` or `python`, 64-bit recommended |

> Before running `Build.bat`, launch `x64 Native Tools Command Prompt for VS` or the corresponding developer PowerShell to ensure `cl` and the MSVC environment variables are available.

> Python 3.12 or newer is required by the repository's build helper scripts (environment checks and build automation). Run `py -3 --version` or `python --version` to confirm the version; upgrade first if only 3.11 or older is installed.

## Build Environment Check

Run the environment-check pipeline before building to check Python, Git, Git LFS, and CMake:

```powershell
py -3 Engine\Source\Programs\MagicBuilder\main.py
```

Use `--print-json` to produce JSON for CI or scripts, or `--no-color` to disable ANSI colors.

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

The script uses the Ninja generator and prompts for a build configuration:

1. `1` for Debug
2. `2` for Release

You can also pass the configuration as an argument to skip the prompt:

```powershell
.\Build.bat Debug
.\Build.bat Release
```

The script configures and builds in the corresponding `out\Ninja_Debug` or `out\Ninja_Release` directory and places outputs in the matching `Binary` directory. It generates `compile_commands.json` and copies it to `out\compile_commands.json` for IDE tools such as clangd; each run updates that file for the selected configuration. To perform a clean rebuild, delete the corresponding Ninja build directory.

The script switches the console code page to 936 and runs CMake configuration and Ninja in the same invocation. MSVC/Ninja header dependency tracking requires the encoding of CMake's recorded dependency prefix to match `cl` output; a mismatch can prevent Ninja from recognizing header dependencies and rebuilding after header changes.

Before running MagicCodegenTool, CMake copies the compilation database from the active build directory to `out\engine_codegen_generate\engine_generate_info\engine_compile_commands.buildmeta`. The tool locates this copy from `--generate-dir` and the filename defined in `MagicConfig.h`; it does not read `out\compile_commands.json` copied for IDE tooling. CMake's Ninja/Makefile generators provide the compilation database. Other outputs are written to the same `engine_generate_info` directory: one `.ast` syntax tree per target header containing `MCLASS(...)`, plus `engine_class_info.log` describing classes, functions, access levels, and members annotated with `MMEMBER(...)`. On Clang or marker-parsing errors, MagicCodegenTool continues checking independent headers/classes, reports the collected diagnostics, and then stops before stages that depend on the failed stage. When writing generated headers, it attempts the other files and reports all write failures.

To use the Visual Studio generator instead of the Ninja generator in `Build.bat`, configure the project manually with CMake:

```powershell
cmake -S . -B out\VS_Debug -G "Visual Studio 17 2022" -A x64
cmake --build out\VS_Debug --config Debug
```

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
Engine/ThirdParty/       # Third-party dependencies (for example Lua, IMGUI, SPIRV-Cross, etc.)
Binary/                  # Build output directory
out/                     # CMake generation directory
CMakeLists.txt           # Top-level CMake entry point
Build.bat                # One-click build script
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
| `magic_enum` | Enum reflection | Converts C++ enums to strings and provides reflection-like utilities for debugging and tooling. |
| `nlohmann` | JSON processing | Reads and writes JSON configuration files and runtime data. |
| `ufbx` | 3D model loading | Imports and processes 3D model assets such as FBX files. |

## Notes

This project is currently focused on Windows local development and debugging, and is suitable for visual evaluation, rendering effect verification, and engine prototype development. If you want to debug or extend rendering features in the IDE, generate the Visual Studio solution first and open the project from `out\VS`.