#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
SmartPOS 系统停止脚本
用于优雅地停止 SmartPOS 应用程序和 AI 服务器
"""

import os
import signal
import subprocess
import time

def stop_system():
    """停止 SmartPOS 系统"""
    print("🛑 正在停止 SmartPOS 系统...")
    
    # 查找并停止 SmartPOS 进程
    try:
        result = subprocess.run(['pgrep', '-f', 'SmartPOS'], capture_output=True, text=True)
        if result.stdout.strip():
            pids = result.stdout.strip().split('\n')
            for pid in pids:
                if pid.strip():
                    print(f"停止 SmartPOS 进程 (PID: {pid})")
                    os.kill(int(pid), signal.SIGTERM)
                    time.sleep(1)
    except Exception as e:
        print(f"停止 SmartPOS 进程时出错: {e}")
    
    # 查找并停止 AI 服务器进程
    try:
        result = subprocess.run(['pgrep', '-f', 'api_server.py'], capture_output=True, text=True)
        if result.stdout.strip():
            pids = result.stdout.strip().split('\n')
            for pid in pids:
                if pid.strip():
                    print(f"停止 AI 服务器进程 (PID: {pid})")
                    os.kill(int(pid), signal.SIGTERM)
                    time.sleep(1)
    except Exception as e:
        print(f"停止 AI 服务器进程时出错: {e}")
    
    print("✅ 系统已停止")

if __name__ == "__main__":
    stop_system() 