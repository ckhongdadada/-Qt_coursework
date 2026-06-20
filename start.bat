@echo off
title PDP Server - C++ Backend

echo ========================================
echo   PDP Server - C++ Backend Launcher
echo ========================================
echo.

cd /d "%~dp0"

echo [1/3] Checking Python environment...
python --version >nul 2>&1
if errorlevel 1 (
    echo [Warning] Python not installed, AI service unavailable
    echo           Big model features will be unavailable
    echo.
    goto :start_cpp_server
)

echo [2/3] Starting AI service...
echo        Port: 8001
echo        Model: Qwen2.5-1.5B-Instruct
echo        Model root: %~dp0qwen_models
echo.

if exist "ai_server.py" (
    set "AI_MODEL_PATH=%~dp0qwen_models"
    start "AI Service" cmd /k "python ai_server.py"
    timeout /t 5 /nobreak >nul
) else (
    echo [Warning] ai_server.py not found, skipping AI service
    echo.
)

:start_cpp_server
echo [3/3] Starting C++ backend service...
echo        Port: 5000
echo        Database: SQLite (pdp.db)
echo.

if exist "build_desktop\pdp_server.exe" (
    echo Starting: build_desktop\pdp_server.exe
    cd build_desktop
    pdp_server.exe
) else if exist "build\pdp_server.exe" (
    echo Starting fallback: build\pdp_server.exe
    cd build
    pdp_server.exe
) else if exist "pdp_server.exe" (
    echo Starting: pdp_server.exe
    pdp_server.exe
) else (
    echo [Error] pdp_server.exe not found
    echo         Please build the project first:
    echo         cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
    echo         cmake --build build
    echo.
    pause
    exit /b 1
)

pause
