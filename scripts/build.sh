#!/bin/bash

# 智能超市收银系统 - 快速构建脚本
# 适用于 macOS 和 Linux

set -e

echo "🚀 智能超市收银系统 - 快速构建脚本"
echo "=================================="

# 检查操作系统
OS="Unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
else
    echo "❌ 不支持的操作系统: $OSTYPE"
    exit 1
fi

echo "📋 检测到操作系统: $OS"

# 检查依赖
echo "🔍 检查依赖..."

# 检查 CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake 未安装"
    if [[ "$OS" == "macOS" ]]; then
        echo "💡 请运行: brew install cmake"
    else
        echo "💡 请运行: sudo apt install cmake"
    fi
    exit 1
fi

# 检查 Qt
QT_VERSION=""
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -query QT_VERSION)
    echo "✅ 检测到 Qt 版本: $QT_VERSION"
elif command -v qmake6 &> /dev/null; then
    QT_VERSION=$(qmake6 -query QT_VERSION)
    echo "✅ 检测到 Qt 6 版本: $QT_VERSION"
else
    echo "❌ Qt 未安装或未在 PATH 中"
    if [[ "$OS" == "macOS" ]]; then
        echo "💡 请运行: brew install qt@6"
        echo "💡 然后添加到PATH: export PATH=\"/opt/homebrew/opt/qt@6/bin:\$PATH\""
    else
        echo "💡 请运行: sudo apt install qt6-base-dev qt6-tools-dev"
    fi
    exit 1
fi

# 检查编译器
if [[ "$OS" == "macOS" ]]; then
    if ! command -v clang++ &> /dev/null; then
        echo "❌ Clang++ 未安装，请安装 Xcode Command Line Tools"
        exit 1
    fi
    echo "✅ 检测到编译器: $(clang++ --version | head -n1)"
else
    if ! command -v g++ &> /dev/null; then
        echo "❌ G++ 未安装"
        echo "💡 请运行: sudo apt install build-essential"
        exit 1
    fi
    echo "✅ 检测到编译器: $(g++ --version | head -n1)"
fi

# 创建构建目录
echo "📁 创建构建目录..."
if [ -d "build" ]; then
    echo "🧹 清理旧的构建目录..."
    rm -rf build
fi
mkdir build
cd build

# 配置项目
echo "⚙️  配置项目..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 获取CPU核心数
if [[ "$OS" == "macOS" ]]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=$(nproc)
fi

echo "🔨 开始编译 (使用 $CORES 个核心)..."
make -j$CORES

# 检查编译结果
if [ -f "SmartPOS" ]; then
    echo "✅ 编译成功！"
    echo "📍 可执行文件位置: $(pwd)/SmartPOS"
else
    echo "❌ 编译失败！"
    exit 1
fi

# 运行测试
echo "🧪 运行测试..."
if make test; then
    echo "✅ 所有测试通过！"
else
    echo "⚠️  部分测试失败，请检查测试输出"
fi

echo ""
echo "🎉 构建完成！"
echo "💡 使用以下命令运行应用:"
echo "   cd build && ./SmartPOS"
echo ""
echo "📚 其他有用的命令:"
echo "   make test          # 运行所有测试"
echo "   ./tests/TestGUIBasic  # 运行GUI测试"
echo "   ctest --verbose    # 详细测试输出"
