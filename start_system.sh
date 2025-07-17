#!/bin/bash

# 一键启动 SmartPOS 智能收银系统
# 包括 AI 推荐服务器和主应用程序

echo "=========================================="
echo "   SmartPOS 智能收银系统启动脚本"
echo "=========================================="

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# 检查 Python 环境
echo -e "${YELLOW}[1/4] 检查 Python 环境...${NC}"
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}错误: Python3 未安装${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Python3 已安装${NC}"

# 检查并安装 Python 依赖
echo -e "${YELLOW}[2/4] 检查 Python 依赖...${NC}"
if [ -f "Ai_model/requirements.txt" ]; then
    pip3 install -q -r Ai_model/requirements.txt 2>/dev/null
    echo -e "${GREEN}✓ Python 依赖已安装${NC}"
else
    echo -e "${RED}警告: 未找到 requirements.txt${NC}"
fi

# 启动 AI 推荐服务器
echo -e "${YELLOW}[3/4] 启动 AI 推荐服务器...${NC}"
cd Ai_model
python3 api_server.py > ../ai_server.log 2>&1 &
AI_SERVER_PID=$!
cd ..

# 等待服务器启动
sleep 5

# 检查服务器是否启动成功
if curl -s http://127.0.0.1:5001 > /dev/null 2>&1; then
    echo -e "${GREEN}✓ AI 推荐服务器已启动 (PID: $AI_SERVER_PID)${NC}"
else
    echo -e "${RED}错误: AI 推荐服务器启动失败${NC}"
    exit 1
fi

# 编译和启动主应用程序
echo -e "${YELLOW}[4/4] 启动 SmartPOS 应用程序...${NC}"

# 创建构建目录
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# 如果没有 Makefile，运行 cmake
if [ ! -f "Makefile" ]; then
    echo "配置项目..."
    cmake .. > ../cmake.log 2>&1
    if [ $? -ne 0 ]; then
        echo -e "${RED}错误: CMake 配置失败，请查看 cmake.log${NC}"
        kill $AI_SERVER_PID 2>/dev/null
        exit 1
    fi
fi

# 编译项目
echo "编译项目..."
make -j8 > ../make.log 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}错误: 编译失败，请查看 make.log${NC}"
    kill $AI_SERVER_PID 2>/dev/null
    exit 1
fi

# 启动应用程序
echo -e "${GREEN}✓ 启动 SmartPOS 应用程序...${NC}"
./SmartPOS &
APP_PID=$!

# 保存 PID 到文件以便后续停止
echo $AI_SERVER_PID > .ai_server.pid
echo $APP_PID > .app.pid

echo ""
echo -e "${GREEN}=========================================="
echo "   系统启动成功！"
echo "=========================================="
echo ""
echo "AI 服务器 PID: $AI_SERVER_PID"
echo "应用程序 PID: $APP_PID"
echo ""
echo "AI 服务器地址: http://127.0.0.1:5001"
echo ""
echo "使用 ./stop_system.sh 停止所有服务"
echo "=========================================="${NC}

# 等待用户按键退出
read -p "按回车键退出..."

# 清理
kill $AI_SERVER_PID 2>/dev/null
kill $APP_PID 2>/dev/null
rm -f .ai_server.pid .app.pid 