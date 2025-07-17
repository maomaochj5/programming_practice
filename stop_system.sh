#!/bin/bash

# 停止 SmartPOS 系统

echo "正在停止 SmartPOS 系统..."

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 读取 PID 文件
if [ -f ".ai_server.pid" ]; then
    AI_PID=$(cat .ai_server.pid)
    if kill -0 $AI_PID 2>/dev/null; then
        kill $AI_PID
        echo -e "${GREEN}✓ AI 服务器已停止${NC}"
    else
        echo -e "${YELLOW}AI 服务器未运行${NC}"
    fi
    rm -f .ai_server.pid
else
    echo -e "${YELLOW}未找到 AI 服务器 PID 文件${NC}"
fi

if [ -f ".app.pid" ]; then
    APP_PID=$(cat .app.pid)
    if kill -0 $APP_PID 2>/dev/null; then
        kill $APP_PID
        echo -e "${GREEN}✓ SmartPOS 应用程序已停止${NC}"
    else
        echo -e "${YELLOW}SmartPOS 应用程序未运行${NC}"
    fi
    rm -f .app.pid
else
    echo -e "${YELLOW}未找到应用程序 PID 文件${NC}"
fi

# 尝试通过进程名停止（备用方案）
echo -e "${YELLOW}检查残留进程...${NC}"
pkill -f "python.*api_server.py" 2>/dev/null
pkill -f "SmartPOS" 2>/dev/null

echo -e "${GREEN}系统已停止${NC}" 