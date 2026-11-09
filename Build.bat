@echo off
setlocal

:: Use the 64-bit VS command prompt when building with Ninja.
set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"

echo %VSCMD_ARG_TGT_ARCH%

:: Ask which build method to use.
echo ========================================
echo Choose build method:
echo [1] Visual Studio (MSBuild) - For debugging in VS
echo [2] Ninja (with compile_commands.json) - For clangd/IDE
echo ========================================
set /p "BUILD_METHOD=Enter 1 or 2: "

:: Ask which build configuration to use.
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

echo.
echo Selected configuration: %CONFIG%

if "%BUILD_METHOD%"=="2" goto :ninja_build
goto :vs_build

:ninja_build
echo Using Ninja + CMake (for compile_commands.json)

:: Ninja is a single-config generator, so the build type must be specified at configuration time.
if "%CONFIG%"=="Debug" (
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

:vs_build
echo Using Visual Studio (for debugging in VS IDE)
set "BUILD_DIR=%PROJECT_ROOT%\out\VS_%CONFIG%"

set "VS_GENERATOR="
set "SOLUTION_EXT="

cmake --help | findstr "Visual Studio 18 2026" >nul
if %errorlevel% equ 0 (
    set "VS_GENERATOR=Visual Studio 18 2026"
    set "SOLUTION_EXT=slnx"
    echo Using Visual Studio 2026
    goto :generate
)

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

echo Generating project files...
cmake "%PROJECT_ROOT%" -G "%VS_GENERATOR%" -A x64
if %errorlevel% neq 0 (
    echo CMake failed!
    pause
    exit /b %errorlevel%
)

echo Building project (%CONFIG%)...
cmake --build . --config %CONFIG%
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build completed successfully!
pause
exit /b 0