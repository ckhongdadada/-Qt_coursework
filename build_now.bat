@echo off
chcp 65001 >nul
title 构建桌面应用

echo ========================================
echo   学业发展规划系统 - 桌面应用构建
echo ========================================
echo.

cd /d "%~dp0"

set PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.0\mingw_64\bin;%PATH%

echo [1/2] 配置CMake...
if exist build_desktop rd /s /q build_desktop

cmake -B build_desktop -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
if errorlevel 1 (
    echo [错误] CMake配置失败
    pause
    exit /b 1
)

echo [2/2] 编译桌面应用...
cmake --build build_desktop --config Release
if errorlevel 1 (
    echo [错误] 编译失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo   构建完成！
echo   可执行文件: build_desktop\pdp_desktop.exe
echo ========================================

pause
