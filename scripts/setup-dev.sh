#!/bin/bash

# 智能超市收银系统 - 开发环境设置脚本

set -e

echo "🔧 智能超市收银系统 - 开发环境设置"
echo "================================="

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

# 安装依赖
echo "📦 安装依赖..."

if [[ "$OS" == "macOS" ]]; then
    # macOS 使用 Homebrew
    if ! command -v brew &> /dev/null; then
        echo "❌ Homebrew 未安装"
        echo "💡 请先安装 Homebrew: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    echo "🍺 使用 Homebrew 安装依赖..."
    brew update
    brew install qt@6 cmake
    
    # 添加 Qt 到 PATH
    echo "🔗 配置 Qt 环境变量..."
    if [[ -d "/opt/homebrew/opt/qt@6/bin" ]]; then
        QT_PATH="/opt/homebrew/opt/qt@6/bin"
    elif [[ -d "/usr/local/opt/qt@6/bin" ]]; then
        QT_PATH="/usr/local/opt/qt@6/bin"
    else
        echo "❌ 无法找到 Qt 安装路径"
        exit 1
    fi
    
    # 检查是否已经在 PATH 中
    if [[ ":$PATH:" != *":$QT_PATH:"* ]]; then
        echo "export PATH=\"$QT_PATH:\$PATH\"" >> ~/.zshrc
        echo "export PATH=\"$QT_PATH:\$PATH\"" >> ~/.bash_profile
        echo "✅ Qt 路径已添加到 shell 配置文件"
        echo "💡 请运行 'source ~/.zshrc' 或重新打开终端"
    fi
    
elif [[ "$OS" == "Linux" ]]; then
    # Linux 使用包管理器
    if command -v apt &> /dev/null; then
        echo "📦 使用 apt 安装依赖..."
        sudo apt update
        sudo apt install -y \
            qt6-base-dev \
            qt6-tools-dev \
            cmake \
            build-essential \
            git \
            clang-format \
            cppcheck
    elif command -v yum &> /dev/null; then
        echo "📦 使用 yum 安装依赖..."
        sudo yum update -y
        sudo yum install -y \
            qt6-qtbase-devel \
            cmake \
            gcc-c++ \
            git \
            clang-tools-extra
    elif command -v pacman &> /dev/null; then
        echo "📦 使用 pacman 安装依赖..."
        sudo pacman -Syu --noconfirm \
            qt6-base \
            qt6-tools \
            cmake \
            base-devel \
            git \
            clang
    else
        echo "❌ 无法检测到支持的包管理器"
        exit 1
    fi
fi

# 验证安装
echo "🔍 验证安装..."

# 检查 CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1)
    echo "✅ $CMAKE_VERSION"
else
    echo "❌ CMake 安装失败"
    exit 1
fi

# 检查 Qt
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -query QT_VERSION)
    echo "✅ Qt $QT_VERSION"
elif command -v qmake6 &> /dev/null; then
    QT_VERSION=$(qmake6 -query QT_VERSION)
    echo "✅ Qt $QT_VERSION"
else
    echo "❌ Qt 安装失败或不在 PATH 中"
    if [[ "$OS" == "macOS" ]]; then
        echo "💡 请运行: export PATH=\"$QT_PATH:\$PATH\""
    fi
    exit 1
fi

# 检查编译器
if [[ "$OS" == "macOS" ]]; then
    if command -v clang++ &> /dev/null; then
        COMPILER_VERSION=$(clang++ --version | head -n1)
        echo "✅ $COMPILER_VERSION"
    else
        echo "❌ Clang++ 未找到，请安装 Xcode Command Line Tools"
        echo "💡 运行: xcode-select --install"
        exit 1
    fi
else
    if command -v g++ &> /dev/null; then
        COMPILER_VERSION=$(g++ --version | head -n1)
        echo "✅ $COMPILER_VERSION"
    else
        echo "❌ G++ 安装失败"
        exit 1
    fi
fi

# 设置 Git hooks (可选)
if [[ -d ".git" ]]; then
    echo "🔗 设置 Git hooks..."
    mkdir -p .git/hooks
    
    # 创建 pre-commit hook
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# 运行代码格式检查
echo "Running code format check..."
if command -v clang-format &> /dev/null; then
    find src/ -name "*.cpp" -o -name "*.h" | xargs clang-format -i --style=file
    git add -u
fi
EOF
    chmod +x .git/hooks/pre-commit
    echo "✅ Pre-commit hook 已设置"
fi

# 创建 VS Code 配置 (可选)
if command -v code &> /dev/null; then
    echo "🔧 创建 VS Code 配置..."
    mkdir -p .vscode
    
    # 创建 settings.json
    cat > .vscode/settings.json << 'EOF'
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Unix Makefiles",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "files.associations": {
        "*.h": "cpp",
        "*.cpp": "cpp"
    },
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_style": "file"
}
EOF
    
    # 创建 launch.json
    cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug SmartPOS",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/SmartPOS",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build",
            "environment": [],
            "externalConsole": false,
            "MIMode": "lldb",
            "preLaunchTask": "build"
        }
    ]
}
EOF
    
    # 创建 tasks.json
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "cmake",
            "args": ["--build", "build", "-j"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": "$gcc"
        }
    ]
}
EOF
    
    echo "✅ VS Code 配置已创建"
fi

echo ""
echo "🎉 开发环境设置完成！"
echo ""
echo "📚 下一步:"
echo "1. 运行 './scripts/build.sh' 编译项目"
echo "2. 运行 'cd build && ./SmartPOS' 启动应用"
echo "3. 运行 'make test' 执行测试"
echo ""
echo "🛠️ 开发工具推荐:"
echo "- VS Code 配置已创建 (如果安装了 VS Code)"
echo "- Git hooks 已设置用于代码格式检查"
echo ""
echo "📖 更多信息请查看 README.md 和 CONTRIBUTING.md"
