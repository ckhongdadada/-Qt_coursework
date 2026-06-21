@echo off
setlocal EnableExtensions
title PDP Desktop - Qt Application

echo ========================================
echo   PDP Desktop - Qt Application Launcher
echo ========================================
echo.

cd /d "%~dp0"

if not defined QT6_DIR (
    if exist "C:\Qt\6.11.0\mingw_64" (
        set "QT6_DIR=C:\Qt\6.11.0\mingw_64"
    )
)

set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"
set "PATH=%QT6_DIR%\bin;%MINGW_BIN%;%PATH%"

echo [1/3] Checking Qt environment...
if not defined QT6_DIR (
    echo [Warning] Qt6 not found, application may not run correctly
    echo.
)

echo [2/3] Starting AI service...
echo        Port: 8001
echo        Model: Qwen2.5-1.5B-Instruct
echo        Model root: %~dp0qwen_models
echo.

set "PYTHON_CMD=python"
if exist "C:\Users\28414\anaconda3\envs\qwen_env\python.exe" (
    set "PYTHON_CMD=C:\Users\28414\anaconda3\envs\qwen_env\python.exe"
    echo        [Info] Found conda qwen_env environment
) else if exist "%~dp0venv\Scripts\python.exe" (
    set "PYTHON_CMD=%~dp0venv\Scripts\python.exe"
    echo        [Info] Found virtual environment
)

%PYTHON_CMD% --version >nul 2>&1
if errorlevel 1 (
    echo [Warning] Python not installed, AI service unavailable
    echo           Big model features will be unavailable
    echo.
) else (
    if exist "ai_server.py" (
        echo        [Info] Stopping any existing AI service on port 8001...
        taskkill /F /IM python.exe /FI "WINDOWTITLE eq AI Service" >nul 2>&1
        taskkill /F /FI "WINDOWTITLE eq AI Service" >nul 2>&1
        timeout /t 1 /nobreak >nul
        set "AI_MODEL_PATH=%~dp0qwen_models"
        start "AI Service" cmd /k "%PYTHON_CMD% ai_server.py"
        timeout /t 5 /nobreak >nul
    ) else (
        echo [Warning] ai_server.py not found, skipping AI service
        echo.
    )
)

echo [3/3] Starting desktop application...
echo.

if exist "pdp_desktop.exe" (
    echo Starting: pdp_desktop.exe
    start "" "pdp_desktop.exe"
    exit /b 0
)

echo [Error] pdp_desktop.exe not found
echo.
pause
exit /b 1
