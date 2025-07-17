#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
SmartPOS 智能收银系统一键启动脚本
包括 AI 推荐服务器和主应用程序的完整启动流程
"""

import os
import sys
import time
import signal
import subprocess
import platform
import requests
import threading
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
        print(message)  # Windows控制台可能不支持颜色
    else:
        print(f"{color}{message}{Colors.NC}")

def print_header():
    """打印启动标题"""
    print_colored("=" * 50, Colors.BLUE)
    print_colored("   SmartPOS 智能收银系统启动脚本", Colors.BOLD)
    print_colored("=" * 50, Colors.BLUE)
    print()

def check_command(command):
    """检查命令是否存在"""
    try:
        subprocess.run([command, "--version"], 
                      capture_output=True, check=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

def check_python_environment():
    """检查Python环境"""
    print_colored("[1/5] 检查 Python 环境...", Colors.YELLOW)
    
    if sys.version_info < (3, 7):
        print_colored("错误: 需要 Python 3.7 或更高版本", Colors.RED)
        return False
    
    print_colored("✓ Python 环境检查通过", Colors.GREEN)
    return True

def install_python_dependencies():
    """安装Python依赖"""
    print_colored("[2/5] 检查并安装 Python 依赖...", Colors.YELLOW)
    
    requirements_file = Path("Ai_model/requirements.txt")
    if not requirements_file.exists():
        print_colored("警告: 未找到 requirements.txt", Colors.YELLOW)
        return True
    
    try:
        subprocess.run([
            sys.executable, "-m", "pip", "install", "-q", 
            "-r", str(requirements_file)
        ], check=True, capture_output=True)
        print_colored("✓ Python 依赖安装完成", Colors.GREEN)
        return True
    except subprocess.CalledProcessError as e:
        print_colored(f"错误: 依赖安装失败 - {e}", Colors.RED)
        return False

def start_ai_server():
    """启动AI推荐服务器"""
    print_colored("[3/5] 启动 AI 推荐服务器...", Colors.YELLOW)
    
    ai_model_dir = Path("Ai_model")
    if not ai_model_dir.exists():
        print_colored("错误: Ai_model 目录不存在", Colors.RED)
        return None
    
    api_server_file = ai_model_dir / "api_server.py"
    if not api_server_file.exists():
        print_colored("错误: api_server.py 不存在", Colors.RED)
        return None
    
    try:
        # 启动AI服务器进程
        with open("ai_server.log", "w") as log_file:
            process = subprocess.Popen([
                sys.executable, str(api_server_file)
            ], cwd=str(ai_model_dir), stdout=log_file, stderr=log_file)
        
        # 等待服务器启动
        print_colored("等待 AI 服务器启动...", Colors.YELLOW)
        for i in range(30):  # 最多等待30秒
            try:
                response = requests.get("http://127.0.0.1:5001", timeout=1)
                break
            except requests.exceptions.RequestException:
                time.sleep(1)
                print(".", end="", flush=True)
        else:
            print_colored("\n错误: AI 服务器启动超时", Colors.RED)
            process.terminate()
            return None
        
        print_colored(f"\n✓ AI 推荐服务器已启动 (PID: {process.pid})", Colors.GREEN)
        return process
        
    except Exception as e:
        print_colored(f"错误: AI 服务器启动失败 - {e}", Colors.RED)
        return None

def check_build_tools():
    """检查构建工具"""
    print_colored("[4/5] 检查构建环境...", Colors.YELLOW)
    
    # 检查cmake
    if not check_command("cmake"):
        print_colored("错误: 未找到 cmake", Colors.RED)
        return False
    
    # 检查make或ninja
    if not (check_command("make") or check_command("ninja")):
        print_colored("错误: 未找到 make 或 ninja", Colors.RED)
        return False
    
    print_colored("✓ 构建工具检查通过", Colors.GREEN)
    return True

def build_and_start_app():
    """编译和启动主应用程序"""
    print_colored("[5/5] 编译和启动 SmartPOS 应用程序...", Colors.YELLOW)
    
    # 创建构建目录
    build_dir = Path("build")
    build_dir.mkdir(exist_ok=True)
    
    try:
        # 配置项目
        if not (build_dir / "Makefile").exists():
            print_colored("配置项目...", Colors.YELLOW)
            with open("cmake.log", "w") as log_file:
                result = subprocess.run([
                    "cmake", ".."
                ], cwd=str(build_dir), stdout=log_file, stderr=log_file)
            
            if result.returncode != 0:
                print_colored("错误: CMake 配置失败，请查看 cmake.log", Colors.RED)
                return None
        
        # 编译项目
        print_colored("编译项目...", Colors.YELLOW)
        with open("make.log", "w") as log_file:
            result = subprocess.run([
                "make", "-j8"
            ], cwd=str(build_dir), stdout=log_file, stderr=log_file)
        
        if result.returncode != 0:
            print_colored("错误: 编译失败，请查看 make.log", Colors.RED)
            return None
        
        # 启动应用程序
        smartpos_executable = build_dir / "SmartPOS"
        if not smartpos_executable.exists():
            print_colored("错误: SmartPOS 可执行文件不存在", Colors.RED)
            return None
        
        print_colored("启动 SmartPOS 应用程序...", Colors.YELLOW)
        process = subprocess.Popen([str(smartpos_executable)], cwd=str(build_dir))
        
        print_colored(f"✓ SmartPOS 应用程序已启动 (PID: {process.pid})", Colors.GREEN)
        return process
        
    except Exception as e:
        print_colored(f"错误: 应用程序启动失败 - {e}", Colors.RED)
        return None

def save_pids(ai_server_process, app_process):
    """保存进程ID到文件"""
    try:
        if ai_server_process:
            with open(".ai_server.pid", "w") as f:
                f.write(str(ai_server_process.pid))
        
        if app_process:
            with open(".app.pid", "w") as f:
                f.write(str(app_process.pid))
    except Exception as e:
        print_colored(f"警告: 无法保存 PID 文件 - {e}", Colors.YELLOW)

def cleanup_processes(ai_server_process, app_process):
    """清理进程"""
    print_colored("\n正在停止服务...", Colors.YELLOW)
    
    if app_process and app_process.poll() is None:
        app_process.terminate()
        try:
            app_process.wait(timeout=5)
            print_colored("✓ SmartPOS 应用程序已停止", Colors.GREEN)
        except subprocess.TimeoutExpired:
            app_process.kill()
            print_colored("✓ SmartPOS 应用程序已强制停止", Colors.YELLOW)
    
    if ai_server_process and ai_server_process.poll() is None:
        ai_server_process.terminate()
        try:
            ai_server_process.wait(timeout=5)
            print_colored("✓ AI 服务器已停止", Colors.GREEN)
        except subprocess.TimeoutExpired:
            ai_server_process.kill()
            print_colored("✓ AI 服务器已强制停止", Colors.YELLOW)
    
    # 清理PID文件
    for pid_file in [".ai_server.pid", ".app.pid"]:
        try:
            os.remove(pid_file)
        except FileNotFoundError:
            pass

def monitor_processes(ai_server_process, app_process):
    """监控进程状态"""
    try:
        while True:
            # 检查AI服务器
            if ai_server_process and ai_server_process.poll() is not None:
                print_colored("警告: AI 服务器意外退出", Colors.RED)
                break
            
            # 检查应用程序
            if app_process and app_process.poll() is not None:
                print_colored("应用程序已退出", Colors.YELLOW)
                break
            
            time.sleep(1)
    except KeyboardInterrupt:
        pass

def main():
    """主函数"""
    print_header()
    
    # 切换到脚本所在目录
    script_dir = Path(__file__).parent
    os.chdir(script_dir)
    
    ai_server_process = None
    app_process = None
    
    def signal_handler(signum, frame):
        cleanup_processes(ai_server_process, app_process)
        sys.exit(0)
    
    # 设置信号处理
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    try:
        # 步骤1: 检查Python环境
        if not check_python_environment():
            return 1
        
        # 步骤2: 安装依赖
        if not install_python_dependencies():
            return 1
        
        # 步骤3: 启动AI服务器
        ai_server_process = start_ai_server()
        if not ai_server_process:
            return 1
        
        # 步骤4: 检查构建工具
        if not check_build_tools():
            cleanup_processes(ai_server_process, None)
            return 1
        
        # 步骤5: 编译和启动应用程序
        app_process = build_and_start_app()
        if not app_process:
            cleanup_processes(ai_server_process, None)
            return 1
        
        # 保存PID
        save_pids(ai_server_process, app_process)
        
        # 显示启动成功信息
        print()
        print_colored("=" * 50, Colors.GREEN)
        print_colored("   系统启动成功！", Colors.BOLD)
        print_colored("=" * 50, Colors.GREEN)
        print()
        print_colored(f"AI 服务器 PID: {ai_server_process.pid}", Colors.BLUE)
        print_colored(f"应用程序 PID: {app_process.pid}", Colors.BLUE)
        print()
        print_colored("AI 服务器地址: http://127.0.0.1:5001", Colors.BLUE)
        print()
        print_colored("使用 Ctrl+C 停止所有服务", Colors.YELLOW)
        print_colored("=" * 50, Colors.GREEN)
        print()
        
        # 监控进程
        monitor_processes(ai_server_process, app_process)
        
    except Exception as e:
        print_colored(f"严重错误: {e}", Colors.RED)
        return 1
    
    finally:
        cleanup_processes(ai_server_process, app_process)
    
    return 0

if __name__ == "__main__":
    sys.exit(main()) 