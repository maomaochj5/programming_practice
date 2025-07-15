@echo off
REM 智能超市收银系统 - Windows 快速构建脚本

echo 🚀 智能超市收银系统 - Windows 构建脚本
echo ==================================

REM 检查 CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ CMake 未安装或未在 PATH 中
    echo 💡 请从 https://cmake.org/download/ 下载并安装 CMake
    pause
    exit /b 1
)
echo ✅ 检测到 CMake

REM 检查 Qt
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    where qmake6 >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo ❌ Qt 未安装或未在 PATH 中
        echo 💡 请从 https://www.qt.io/download 下载并安装 Qt 6.x
        echo 💡 确保 Qt 的 bin 目录已添加到系统 PATH
        pause
        exit /b 1
    )
)
echo ✅ 检测到 Qt

REM 清理旧的构建目录
if exist build (
    echo 🧹 清理旧的构建目录...
    rmdir /s /q build
)

REM 创建构建目录
echo 📁 创建构建目录...
mkdir build
cd build

REM 配置项目
echo ⚙️ 配置项目...
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo ❌ CMake 配置失败
    pause
    exit /b 1
)

REM 编译项目
echo 🔨 开始编译...
cmake --build . --config Release -j
if %ERRORLEVEL% NEQ 0 (
    echo ❌ 编译失败
    pause
    exit /b 1
)

REM 检查编译结果
if exist Release\SmartPOS.exe (
    echo ✅ 编译成功！
    echo 📍 可执行文件位置: %CD%\Release\SmartPOS.exe
) else if exist SmartPOS.exe (
    echo ✅ 编译成功！
    echo 📍 可执行文件位置: %CD%\SmartPOS.exe
) else (
    echo ❌ 编译失败，未找到可执行文件
    pause
    exit /b 1
)

REM 运行测试
echo 🧪 运行测试...
ctest --output-on-failure
if %ERRORLEVEL% EQU 0 (
    echo ✅ 所有测试通过！
) else (
    echo ⚠️ 部分测试失败，请检查测试输出
)

echo.
echo 🎉 构建完成！
echo 💡 使用以下命令运行应用:
if exist Release\SmartPOS.exe (
    echo    cd build\Release ^&^& SmartPOS.exe
) else (
    echo    cd build ^&^& SmartPOS.exe
)
echo.
echo 📚 其他有用的命令:
echo    ctest                    # 运行所有测试
echo    ctest --verbose          # 详细测试输出

pause
