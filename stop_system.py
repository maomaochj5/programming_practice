#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
SmartPOS 智能收银系统停止脚本
停止 AI 推荐服务器和主应用程序
"""

import os
import sys
import time
import signal
import platform
import subprocess
from pathlib import Path

# 颜色定义
class Colors:
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    BOLD = '\033[1m'
    NC = '\033[0m'  # No Color

def print_colored(message, color=Colors.NC):
    """打印彩色文本"""
    if platform.system() == "Windows":
        print(message)
    else:
        print(f"{color}{message}{Colors.NC}")

def print_header():
    """打印停止标题"""
    print_colored("=" * 50, Colors.BLUE)
    print_colored("   SmartPOS 智能收银系统停止脚本", Colors.BOLD)
    print_colored("=" * 50, Colors.BLUE)
    print()

def stop_process_by_pid(pid, process_name, timeout=5):
    """通过PID停止进程"""
    try:
        # 检查进程是否存在
        os.kill(pid, 0)
        
        # 尝试优雅停止
        os.kill(pid, signal.SIGTERM)
        
        # 等待进程结束
        for _ in range(timeout):
            try:
                os.kill(pid, 0)
                time.sleep(1)
            except OSError:
                print_colored(f"✓ {process_name} 已停止", Colors.GREEN)
                return True
        
        # 如果还在运行，强制终止
        try:
            os.kill(pid, signal.SIGKILL)
            print_colored(f"✓ {process_name} 已强制停止", Colors.YELLOW)
            return True
        except OSError:
            pass
            
    except OSError:
        print_colored(f"{process_name} 未运行", Colors.YELLOW)
        return True
    
    return False

def stop_from_pid_files():
    """从PID文件停止进程"""
    stopped_any = False
    
    # 停止AI服务器
    ai_pid_file = Path(".ai_server.pid")
    if ai_pid_file.exists():
        try:
            with open(ai_pid_file, "r") as f:
                ai_pid = int(f.read().strip())
            
            if stop_process_by_pid(ai_pid, "AI 服务器"):
                stopped_any = True
            
            ai_pid_file.unlink()
            
        except (ValueError, FileNotFoundError) as e:
            print_colored(f"警告: 无法读取 AI 服务器 PID 文件 - {e}", Colors.YELLOW)
    else:
        print_colored("未找到 AI 服务器 PID 文件", Colors.YELLOW)
    
    # 停止应用程序
    app_pid_file = Path(".app.pid")
    if app_pid_file.exists():
        try:
            with open(app_pid_file, "r") as f:
                app_pid = int(f.read().strip())
            
            if stop_process_by_pid(app_pid, "SmartPOS 应用程序"):
                stopped_any = True
            
            app_pid_file.unlink()
            
        except (ValueError, FileNotFoundError) as e:
            print_colored(f"警告: 无法读取应用程序 PID 文件 - {e}", Colors.YELLOW)
    else:
        print_colored("未找到应用程序 PID 文件", Colors.YELLOW)
    
    return stopped_any

def stop_by_process_name():
    """通过进程名停止残留进程"""
    print_colored("检查残留进程...", Colors.YELLOW)
    
    stopped_any = False
    
    # 停止AI服务器进程
    try:
        if platform.system() == "Windows":
            # Windows
            result = subprocess.run([
                "taskkill", "/F", "/IM", "python.exe", "/FI", 
                "WINDOWTITLE eq *api_server.py*"
            ], capture_output=True, text=True)
        else:
            # Unix/Linux/macOS
            result = subprocess.run([
                "pkill", "-f", "python.*api_server.py"
            ], capture_output=True, text=True)
        
        if result.returncode == 0:
            print_colored("✓ 停止了残留的 AI 服务器进程", Colors.GREEN)
            stopped_any = True
            
    except Exception as e:
        pass  # 忽略错误，可能没有残留进程
    
    # 停止SmartPOS进程
    try:
        if platform.system() == "Windows":
            # Windows
            result = subprocess.run([
                "taskkill", "/F", "/IM", "SmartPOS.exe"
            ], capture_output=True, text=True)
        else:
            # Unix/Linux/macOS
            result = subprocess.run([
                "pkill", "-f", "SmartPOS"
            ], capture_output=True, text=True)
        
        if result.returncode == 0:
            print_colored("✓ 停止了残留的 SmartPOS 进程", Colors.GREEN)
            stopped_any = True
            
    except Exception as e:
        pass  # 忽略错误，可能没有残留进程
    
    if not stopped_any:
        print_colored("未发现残留进程", Colors.YELLOW)
    
    return stopped_any

def cleanup_files():
    """清理临时文件"""
    print_colored("清理临时文件...", Colors.YELLOW)
    
    cleanup_files = [
        "ai_server.log",
        "cmake.log", 
        "make.log",
        ".ai_server.pid",
        ".app.pid"
    ]
    
    cleaned = 0
    for file_path in cleanup_files:
        try:
            if Path(file_path).exists():
                Path(file_path).unlink()
                cleaned += 1
        except Exception:
            pass
    
    if cleaned > 0:
        print_colored(f"✓ 清理了 {cleaned} 个临时文件", Colors.GREEN)
    else:
        print_colored("没有需要清理的临时文件", Colors.YELLOW)

def main():
    """主函数"""
    print_header()
    
    # 切换到脚本所在目录
    script_dir = Path(__file__).parent
    os.chdir(script_dir)
    
    print_colored("正在停止 SmartPOS 系统...", Colors.YELLOW)
    print()
    
    stopped_any = False
    
    try:
        # 从PID文件停止
        if stop_from_pid_files():
            stopped_any = True
        
        # 通过进程名停止残留进程
        if stop_by_process_name():
            stopped_any = True
        
        # 清理临时文件
        cleanup_files()
        
        print()
        if stopped_any:
            print_colored("=" * 50, Colors.GREEN)
            print_colored("   系统已停止", Colors.BOLD)
            print_colored("=" * 50, Colors.GREEN)
        else:
            print_colored("=" * 50, Colors.YELLOW)
            print_colored("   没有发现运行中的服务", Colors.BOLD)
            print_colored("=" * 50, Colors.YELLOW)
        
        return 0
        
    except Exception as e:
        print_colored(f"停止过程中发生错误: {e}", Colors.RED)
        return 1

if __name__ == "__main__":
    sys.exit(main()) 