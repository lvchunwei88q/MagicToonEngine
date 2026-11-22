@echo off
setlocal

:: Pin the console code page. CMake records "msvc_deps_prefix" (注意: 包含文件: ) using the
:: encoding of the console it ran in; Ninja then matches cl's output byte-for-byte. If the
:: build runs under UTF-8 (65001), the bytes differ, Ninja silently records #deps 0, and header
:: changes stop triggering recompiles. Keep configure and build on the same code page (936).
chcp 936 >nul

:: Use the 64-bit VS command prompt when building with Ninja.
set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"

:: Configuration: pass it as an argument for non-interactive builds
:: ("Build.bat Debug" / "Build.bat Release"); otherwise ask.
set "CONFIG=%~1"
if /i "%CONFIG%"=="Debug" goto :config_ready
if /i "%CONFIG%"=="Release" goto :config_ready

echo ========================================
echo Choose build configuration:
echo [1] Debug
echo [2] Release
echo ========================================
set /p "BUILD_CONFIG=Enter 1 or 2: "

if "%BUILD_CONFIG%"=="1" (
    set "CONFIG=Debug"
) else (
    set "CONFIG=Release"
)

:config_ready
echo.
echo Selected configuration: %CONFIG%

:: Ninja is a single-config generator, so the build type must be specified at configuration time.
if /i "%CONFIG%"=="Debug" (
    set "BUILD_TYPE=Debug"
    set "CMAKE_BUILD_TYPE=-DCMAKE_BUILD_TYPE=Debug"
) else (
    set "BUILD_TYPE=Release"
    set "CMAKE_BUILD_TYPE=-DCMAKE_BUILD_TYPE=Release"
)

set "BUILD_DIR=%PROJECT_ROOT%\out\Ninja_%BUILD_TYPE%"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

echo Generating project files with Ninja (%BUILD_TYPE%)...
cmake "%PROJECT_ROOT%" -G Ninja %CMAKE_BUILD_TYPE% -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
if %errorlevel% neq 0 (
    echo CMake failed!
    pause
    exit /b %errorlevel%
)

:: Copy compile_commands.json to the out directory for IDE tooling.
if not exist "%BUILD_DIR%\compile_commands.json" (
    echo Error: compile_commands.json was not generated!
    pause
    exit /b 1
)

copy /Y "%BUILD_DIR%\compile_commands.json" "%PROJECT_ROOT%\out\compile_commands.json"
if errorlevel 1 (
    echo Failed to copy compile_commands.json!
    pause
    exit /b 1
)
echo compile_commands.json generated and copied to the out directory.

echo Building project with Ninja (%BUILD_TYPE%)...
cmake --build .
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build completed successfully!
echo Output directory: %PROJECT_ROOT%\Binary\%BUILD_TYPE%
echo compile_commands.json: %PROJECT_ROOT%\out\compile_commands.json
pause
exit /b 0
