# SmartPOS 一键启动指南

## 快速开始

### macOS/Linux 用户

1. **启动系统**
   ```bash
   ./start_system.sh
   ```

2. **停止系统**
   ```bash
   ./stop_system.sh
   ```

### Windows 用户

1. **启动系统**
   ```
   双击 start_system.bat
   ```

2. **停止系统**
   ```
   双击 stop_system.bat
   ```

## 启动脚本功能

`start_system.sh` / `start_system.bat` 会自动执行以下操作：

1. ✅ 检查 Python 环境
2. ✅ 安装必要的 Python 依赖
3. ✅ 启动 AI 推荐服务器（端口 5001）
4. ✅ 编译 C++ 应用程序（如需要）
5. ✅ 启动 SmartPOS 主应用程序

## 系统要求

- Python 3.8 或更高版本
- CMake 3.16 或更高版本
- Qt6
- C++ 编译器（macOS: Xcode, Linux: GCC, Windows: MSVC）

## 常见问题

### 1. Python 未找到
请确保已安装 Python 并将其添加到系统 PATH 中。

### 2. 编译失败
- 检查是否安装了所有必要的开发工具
- 查看 `cmake.log` 或 `make.log` 了解详细错误信息

### 3. AI 服务器启动失败
- 检查端口 5001 是否被占用
- 查看 `ai_server.log` 了解错误信息

### 4. 应用程序无法启动
- 确保 Qt6 已正确安装
- 检查 `build` 目录下是否生成了可执行文件

## 日志文件

- `ai_server.log` - AI 服务器日志
- `cmake.log` - CMake 配置日志
- `make.log` / `build.log` - 编译日志

## 手动启动（如果脚本失败）

1. **启动 AI 服务器**
   ```bash
   cd Ai_model
   python api_server.py
   ```

2. **编译并运行应用程序**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ./SmartPOS
   ``` 