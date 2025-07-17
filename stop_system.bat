@echo off
chcp 65001 >nul
title 停止 SmartPOS 系统

echo 正在停止 SmartPOS 系统...

REM 停止 Python 进程
echo 停止 AI 服务器...
taskkill /f /im python.exe 2>nul
if %errorlevel% equ 0 (
    echo √ AI 服务器已停止
) else (
    echo AI 服务器未运行
)

REM 停止 SmartPOS 应用
echo 停止 SmartPOS 应用程序...
taskkill /f /im SmartPOS.exe 2>nul
if %errorlevel% equ 0 (
    echo √ SmartPOS 应用程序已停止
) else (
    echo SmartPOS 应用程序未运行
)

echo.
echo 系统已停止
pause 