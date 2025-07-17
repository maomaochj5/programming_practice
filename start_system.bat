@echo off
chcp 65001 >nul
title SmartPOS 智能收银系统

echo ==========================================
echo    SmartPOS 智能收银系统启动脚本
echo ==========================================
echo.

REM 设置变量
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM 检查 Python
echo [1/4] 检查 Python 环境...
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: Python 未安装
    echo 请先安装 Python 3.8 或更高版本
    pause
    exit /b 1
)
echo √ Python 已安装

REM 安装 Python 依赖
echo [2/4] 检查 Python 依赖...
if exist "Ai_model\requirements.txt" (
    pip install -q -r Ai_model\requirements.txt 2>nul
    echo √ Python 依赖已安装
) else (
    echo 警告: 未找到 requirements.txt
)

REM 启动 AI 服务器
echo [3/4] 启动 AI 推荐服务器...
cd Ai_model
start /b python api_server.py > ..\ai_server.log 2>&1
cd ..

REM 等待服务器启动
timeout /t 5 /nobreak >nul

REM 检查服务器是否启动
curl -s http://127.0.0.1:5001 >nul 2>&1
if %errorlevel% equ 0 (
    echo √ AI 推荐服务器已启动
) else (
    echo 错误: AI 推荐服务器启动失败
    pause
    exit /b 1
)

REM 编译和启动主应用
echo [4/4] 启动 SmartPOS 应用程序...

REM 创建构建目录
if not exist "build" mkdir build
cd build

REM 如果没有配置过，运行 cmake
if not exist "CMakeCache.txt" (
    echo 配置项目...
    cmake .. > ..\cmake.log 2>&1
    if %errorlevel% neq 0 (
        echo 错误: CMake 配置失败，请查看 cmake.log
        taskkill /f /im python.exe 2>nul
        pause
        exit /b 1
    )
)

REM 编译项目
echo 编译项目...
cmake --build . --config Release > ..\build.log 2>&1
if %errorlevel% neq 0 (
    echo 错误: 编译失败，请查看 build.log
    taskkill /f /im python.exe 2>nul
    pause
    exit /b 1
)

REM 启动应用程序
echo √ 启动 SmartPOS 应用程序...
if exist "Release\SmartPOS.exe" (
    start Release\SmartPOS.exe
) else if exist "SmartPOS.exe" (
    start SmartPOS.exe
) else (
    echo 错误: 找不到 SmartPOS.exe
    taskkill /f /im python.exe 2>nul
    pause
    exit /b 1
)

cd ..

echo.
echo ==========================================
echo    系统启动成功！
echo ==========================================
echo.
echo AI 服务器地址: http://127.0.0.1:5001
echo.
echo 使用 stop_system.bat 停止所有服务
echo ==========================================
echo.
pause 