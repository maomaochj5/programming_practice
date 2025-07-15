# 编译指南

## 🚀 快速开始

### 一键编译 (推荐)

```bash
# Linux/macOS
./scripts/build.sh

# Windows
scripts\build.bat
```

### 手动编译

```bash
# 1. 创建构建目录
mkdir build && cd build

# 2. 配置项目
cmake ..

# 3. 编译
make -j4                # Linux/macOS
cmake --build . -j 4    # 跨平台

# 4. 运行
./SmartPOS
```

## 📋 系统要求

| 组件 | 最低版本 | 推荐版本 |
|------|----------|----------|
| Qt | 6.2 | 6.5+ |
| CMake | 3.16 | 3.25+ |
| C++ | C++17 | C++20 |

## 🔧 依赖安装

### macOS
```bash
brew install qt@6 cmake
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential
```

### Windows
1. 下载 [Qt 6.x](https://www.qt.io/download)
2. 下载 [CMake](https://cmake.org/download/)
3. 确保添加到系统 PATH

## 🧪 运行测试

```bash
# 所有测试
make test

# 特定测试
./tests/TestProduct
./tests/TestGUIBasic
```

## ❗ 常见问题

### Qt 未找到
```bash
# macOS
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"

# Linux
sudo apt install qt6-base-dev

# Windows
确保 Qt 的 bin 目录在系统 PATH 中
```

### 编译错误
```bash
# 清理重新编译
rm -rf build
mkdir build && cd build
cmake .. && make
```

## 🎯 团队开发

### 首次设置
```bash
# 1. 克隆项目
git clone <repository-url>
cd smart-pos-system

# 2. 设置开发环境
./scripts/setup-dev.sh    # Linux/macOS

# 3. 编译项目
./scripts/build.sh
```

### 日常开发
```bash
# 拉取最新代码
git pull origin develop

# 重新编译
cd build && make -j4

# 运行测试
make test
```

## 💡 开发提示

- 使用 `./scripts/setup-dev.sh` 一键设置开发环境
- VS Code 用户：安装 CMake Tools 扩展
- Qt Creator 用户：直接打开 CMakeLists.txt
- 提交前务必运行 `make test` 确保测试通过
