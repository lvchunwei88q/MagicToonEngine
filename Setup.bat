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
::  Check if Git LFS is installed
:: ========================================
echo Checking Git LFS environment...

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
::  Initialize Git LFS (if needed)
:: ========================================
echo Initializing Git LFS...

git lfs install >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Failed to initialize Git LFS
    pause
    exit /b 1
)
echo        Git LFS is ready

:: ========================================
:: Download LFS files
:: ========================================
echo.
echo Downloading LFS files...
echo.
echo Note: First download may take a few minutes. Please be patient.
echo.

git lfs pull
set PULL_RESULT=%errorlevel%

if %PULL_RESULT% equ 0 (
    echo.
    echo ========================================
    echo   Download completed successfully!
    echo ========================================
    echo.
    echo Verifying downloaded files:
    git lfs ls-files
    echo.
    echo All LFS files are now available locally. You can proceed to build.
) else (
    echo.
    echo ========================================
    echo   Download failed with error code: %PULL_RESULT%
    echo ========================================
    echo.
    echo Possible reasons:
    echo   1. Network connection issue
    echo   2. Authentication failed
    echo   3. No LFS files to download
    echo.
    echo Troubleshooting:
    echo   1. Check network or configure proxy
    echo   2. Ensure you have repository access
    echo   3. Manually run: git lfs pull
    echo.
    pause
    exit /b 1
)

echo.
pause