@echo off
chcp 65001 >nul
title 构建Vue前端

echo ========================================
echo   构建Vue前端用于桌面应用
echo ========================================
echo.

cd /d "%~dp0"

set FRONTEND_DIR=..\frontend
set OUTPUT_DIR=frontend_dist

if not exist "%FRONTEND_DIR%" (
    echo [错误] 未找到前端目录: %FRONTEND_DIR%
    echo        请确保frontend目录存在
    pause
    exit /b 1
)

echo [1/3] 检查Node.js环境...
node --version >nul 2>&1
if errorlevel 1 (
    echo [错误] Node.js未安装
    echo        请先安装Node.js: https://nodejs.org/
    pause
    exit /b 1
)

echo [2/3] 安装前端依赖...
cd /d "%FRONTEND_DIR%"
if not exist "node_modules" (
    echo        正在安装依赖...
    call npm install
    if errorlevel 1 (
        echo [错误] 依赖安装失败
        pause
        exit /b 1
    )
)

echo [3/3] 构建前端...
echo        输出目录: %OUTPUT_DIR%
call npm run build
if errorlevel 1 (
    echo [错误] 前端构建失败
    pause
    exit /b 1
)

cd /d "%~dp0"

if exist "%FRONTEND_DIR%\dist" (
    if exist "%OUTPUT_DIR%" rd /s /q "%OUTPUT_DIR%"
    xcopy "%FRONTEND_DIR%\dist" "%OUTPUT_DIR%\" /E /I /Y
    echo.
    echo ========================================
    echo   构建完成！
    echo   输出目录: %OUTPUT_DIR%
    echo ========================================
) else (
    echo [错误] 构建输出未找到
    pause
    exit /b 1
)

pause
