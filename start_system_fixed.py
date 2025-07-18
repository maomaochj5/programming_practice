#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
SmartPOS 系统修复版启动脚本
解决 Connection refused 问题
"""

import os
import subprocess
import time
import signal
import sys
import requests
from pathlib import Path

def start_ai_server():
    """启动AI服务器并等待其完全启动"""
    print("🚀 启动 AI 推荐服务器...")
    
    # 启动AI服务器
    ai_process = subprocess.Popen([
        sys.executable, "api_server.py"
    ], cwd="Ai_model", 
       stdout=subprocess.PIPE, 
       stderr=subprocess.PIPE)
    
    # 等待AI服务器完全启动
    print("⏳ 等待 AI 服务器启动...")
    max_attempts = 30
    for attempt in range(max_attempts):
        try:
            # 尝试连接AI服务器
            response = requests.get("http://127.0.0.1:5001/ask", 
                                  json={"query": "test"}, 
                                  timeout=2)
            if response.status_code in [200, 405]:  # 200正常, 405方法不允许但服务器响应
                print("✅ AI 服务器启动成功!")
                return ai_process
        except requests.exceptions.RequestException:
            pass
        
        time.sleep(1)
        print(f"   等待中... ({attempt + 1}/{max_attempts})")
    
    print("❌ AI 服务器启动超时")
    ai_process.terminate()
    return None

def start_smartpos():
    """启动SmartPOS应用程序"""
    print("🚀 启动 SmartPOS 应用程序...")
    
    try:
        smartpos_process = subprocess.Popen([
            "./SmartPOS"
        ], cwd="build")
        
        print("✅ SmartPOS 应用程序启动成功!")
        return smartpos_process
    except Exception as e:
        print(f"❌ SmartPOS 启动失败: {e}")
        return None

def main():
    """主函数"""
    print("🎯 SmartPOS 系统修复版启动脚本")
    print("=" * 50)
    
    # 检查必要文件
    if not Path("build/SmartPOS").exists():
        print("❌ 错误: build/SmartPOS 不存在")
        print("💡 请先运行: cmake .. && make")
        return 1
    
    if not Path("Ai_model/api_server.py").exists():
        print("❌ 错误: Ai_model/api_server.py 不存在")
        return 1
    
    ai_process = None
    smartpos_process = None
    
    try:
        # 1. 启动AI服务器（等待完全启动）
        ai_process = start_ai_server()
        if not ai_process:
            return 1
        
        # 2. 额外等待2秒确保AI服务器稳定
        print("⏳ 等待 AI 服务器稳定...")
        time.sleep(2)
        
        # 3. 启动SmartPOS应用程序
        smartpos_process = start_smartpos()
        if not smartpos_process:
            ai_process.terminate()
            return 1
        
        print("\n" + "=" * 50)
        print("🎉 系统启动成功!")
        print(f"- AI 服务器 PID: {ai_process.pid}")
        print(f"- SmartPOS 应用程序 PID: {smartpos_process.pid}")
        print("- AI 服务器地址: http://127.0.0.1:5001")
        print("\n💡 使用 Ctrl+C 停止系统")
        print("💡 或运行: python3 stop_system_manual.py")
        print("=" * 50)
        
        # 监控进程
        while True:
            if ai_process.poll() is not None:
                print("\n⚠️  AI 服务器意外退出")
                break
            if smartpos_process.poll() is not None:
                print("\n⚠️  SmartPOS 应用程序已退出")
                break
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\n🛑 收到停止信号...")
    except Exception as e:
        print(f"\n❌ 错误: {e}")
    finally:
        # 清理进程
        if smartpos_process and smartpos_process.poll() is None:
            smartpos_process.terminate()
            print("✅ SmartPOS 应用程序已停止")
        
        if ai_process and ai_process.poll() is None:
            ai_process.terminate()
            print("✅ AI 服务器已停止")
    
    return 0

if __name__ == "__main__":
    sys.exit(main()) 