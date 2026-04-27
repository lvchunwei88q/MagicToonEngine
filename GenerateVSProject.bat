@echo off
setlocal

set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"
set "BUILD_DIR=%PROJECT_ROOT%\out\VS"

REM 设置 VS 版本和扩展名（自动检测）
set "VS_GENERATOR="
set "SOLUTION_EXT="

REM 检测 VS 2026
cmake --help | findstr "Visual Studio 18 2026" >nul
if %errorlevel% equ 0 (
    set "VS_GENERATOR=Visual Studio 18 2026"
    set "SOLUTION_EXT=slnx"
    echo Using Visual Studio 2026
    goto :generate
)

REM 检测 VS 2022
cmake --help | findstr "Visual Studio 17 2022" >nul
if %errorlevel% equ 0 (
    set "VS_GENERATOR=Visual Studio 17 2022"
    set "SOLUTION_EXT=sln"
    echo Using Visual Studio 2022
    goto :generate
)

echo No suitable VS found!
pause
exit /b 1

:generate
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

cmake "%PROJECT_ROOT%" -G "%VS_GENERATOR%" -A x64
if %errorlevel% neq 0 (
    echo CMake failed!
    pause
    exit /b %errorlevel%
)

REM 打开对应的解决方案
start "" "MagicToonEngine.%SOLUTION_EXT%"
pause