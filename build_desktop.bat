@echo off
setlocal EnableExtensions
title PDP Desktop Build

echo ========================================
echo   PDP Desktop Build
echo ========================================
echo.

cd /d "%~dp0"

if not defined QT6_DIR (
    if exist "C:\Qt\6.11.0\mingw_64" (
        set "QT6_DIR=C:\Qt\6.11.0\mingw_64"
    )
)

if not exist "%QT6_DIR%\bin\qmake.exe" (
    echo [ERROR] Invalid QT6_DIR: %QT6_DIR%
    echo         Example: set QT6_DIR=C:\Qt\6.11.0\mingw_64
    pause
    exit /b 1
)

set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"
if not exist "%MINGW_BIN%\g++.exe" (
    echo [ERROR] MinGW compiler not found: %MINGW_BIN%
    pause
    exit /b 1
)

set "PATH=C:\Program Files\CMake\bin;%QT6_DIR%\bin;%MINGW_BIN%;%PATH%"

echo [1/4] Check toolchain...
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake not found
    pause
    exit /b 1
)

where g++ >nul 2>&1
if errorlevel 1 (
    echo [ERROR] g++ not found
    pause
    exit /b 1
)

echo [2/4] Clean stale desktop cache...
if exist "build_desktop\CMakeCache.txt" del /f /q "build_desktop\CMakeCache.txt" >nul 2>&1
if exist "build_desktop\Makefile" del /f /q "build_desktop\Makefile" >nul 2>&1
if exist "build_desktop\cmake_install.cmake" del /f /q "build_desktop\cmake_install.cmake" >nul 2>&1
if exist "build_desktop\CMakeFiles" rd /s /q "build_desktop\CMakeFiles" >nul 2>&1
if not exist "build_desktop" mkdir build_desktop

echo [3/4] Configure CMake...
cmake -S . -B build_desktop -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH=%QT6_DIR% ^
    -DPDP_BUILD_SERVER=ON ^
    -DPDP_BUILD_DESKTOP=ON
if errorlevel 1 (
    echo [ERROR] CMake configure failed
    pause
    exit /b 1
)

echo [4/4] Build and deploy desktop app...
cmake --build build_desktop --config Release --target pdp_server pdp_desktop
if errorlevel 1 (
    echo [ERROR] Build failed
    pause
    exit /b 1
)

if exist "%QT6_DIR%\bin\windeployqt.exe" (
    "%QT6_DIR%\bin\windeployqt.exe" --no-translations --no-opengl-sw "build_desktop\pdp_desktop.exe" >nul
)

echo.
echo ========================================
echo   Build complete
echo   Server : build_desktop\pdp_server.exe
echo   Desktop: build_desktop\pdp_desktop.exe
echo ========================================
echo.

pause
