@echo off
setlocal enabledelayedexpansion

:: ========================================
::   MagicToonEngine - Git LFS Download Tool
:: ========================================
:: Description: Download large files managed by Git LFS (ThirdParty folder)
:: Usage: Double-click to run, or execute from command line
:: ========================================

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo.
echo ========================================
echo   MagicToonEngine - Git LFS Download Tool
echo ========================================
echo.
echo Current directory: %CD%
echo.

:: ========================================
:: 1. Check if Git is installed
:: ========================================
echo [1/4] Checking Git environment...

git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Git not found. Please install Git first.
    echo Download: https://git-scm.com/downloads
    pause
    exit /b 1
)
echo        Git is installed

:: ========================================
:: 2. Check if Git LFS is installed
:: ========================================
echo [2/4] Checking Git LFS environment...

git lfs version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Git LFS not found. Please install Git LFS first.
    echo Download: https://git-lfs.github.com/
    echo.
    echo After installation, run the following command to initialize:
    echo   git lfs install
    pause
    exit /b 1
)
echo        Git LFS is installed

:: ========================================
:: 3. Initialize LFS (if needed)
:: ========================================
echo [3/4] Initializing Git LFS...

git lfs install >nul 2>&1
echo        Git LFS is ready

:: ========================================
:: 4. Download LFS files
:: ========================================
echo [4/4] Downloading LFS files...
echo.
echo Note: First download may take a few minutes. Please be patient.
echo Files to download:
echo   - .dll files under ThirdParty folder
echo   - .lib files under ThirdParty folder
echo   - .bin files under ThirdParty folder
echo.

git lfs pull

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo   Download completed successfully!
    echo ========================================
    echo.
    echo Verify downloaded files:
    git lfs ls-files
    echo.
    echo All LFS files are now available locally. You can proceed to build.
) else (
    echo.
    echo ========================================
    echo   Download failed!
    echo ========================================
    echo.
    echo Possible reasons:
    echo   1. Network connection issue (unstable connection to GitHub)
    echo   2. Authentication failed (private repo requires SSH or Token)
    echo   3. No LFS files to download
    echo.
    echo Troubleshooting:
    echo   1. Check network or configure proxy
    echo   2. Ensure you have repository access permissions
    echo   3. Manually run: git lfs pull
    echo.
    pause
    exit /b 1
)

echo.
pause