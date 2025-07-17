#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
SmartPOS 修复版启动脚本 - 解决常见启动问题
"""

import os
import sys
import time
import signal
import subprocess
import platform
import requests
import shutil
from pathlib import Path

# 颜色定义
class Colors:
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    BOLD = '\033[1m'
    NC = '\033[0m'

def print_colored(message, color=Colors.NC):
    if platform.system() == "Windows":
        print(message)
    else:
        print(f"{color}{message}{Colors.NC}")

def check_and_kill_existing():
    """停止现有进程"""
    print_colored("🔍 检查现有进程...", Colors.YELLOW)
    subprocess.run(["pkill", "-f", "python.*api_server.py"], capture_output=True)
    subprocess.run(["pkill", "-f", "SmartPOS"], capture_output=True)
    time.sleep(2)
    print_colored("✓ 已清理现有进程", Colors.GREEN)

def fix_paths():
    """修复路径问题"""
    print_colored("🔧 修复AI模型路径...", Colors.YELLOW)
    
    build_dir = Path("build")
    build_dir.mkdir(exist_ok=True)
    
    # 创建AI模型目录的符号链接
    ai_link = build_dir / "Ai_model"
    if ai_link.exists():
        if ai_link.is_symlink():
            ai_link.unlink()
        else:
            shutil.rmtree(ai_link)
    
    try:
        ai_link.symlink_to("../Ai_model")
        print_colored("✓ AI模型路径已修复", Colors.GREEN)
    except Exception:
        # 如果符号链接失败，复制必要文件
        ai_dir = build_dir / "Ai_model"
        ai_dir.mkdir(exist_ok=True)
        for file in ["ai_to_db_id_mapping.json", "db_to_ai_id_mapping.json"]:
            src = Path(f"Ai_model/{file}")
            if src.exists():
                shutil.copy2(src, ai_dir / file)
        print_colored("✓ AI模型文件已复制", Colors.GREEN)

def start_ai_server():
    """启动AI服务器"""
    print_colored("🚀 启动 AI 服务器...", Colors.YELLOW)
    
    with open("ai_server.log", "w") as log_file:
        process = subprocess.Popen([
            sys.executable, "api_server.py"
        ], cwd="Ai_model", stdout=log_file, stderr=log_file)
    
    # 等待启动
    print_colored("等待服务器启动", Colors.YELLOW)
    for i in range(20):
        try:
            response = requests.get("http://127.0.0.1:5001", timeout=1)
            break
        except:
            time.sleep(1)
            print(".", end="", flush=True)
    else:
        print_colored("\n❌ AI服务器启动超时", Colors.RED)
        process.terminate()
        return None
    
    print_colored(f"\n✅ AI服务器已启动 (PID: {process.pid})", Colors.GREEN)
    return process

def build_app():
    """编译应用"""
    print_colored("🔨 编译应用程序...", Colors.YELLOW)
    
    build_dir = Path("build")
    
    # 如果没有Makefile，运行cmake
    if not (build_dir / "Makefile").exists():
        print_colored("配置项目...", Colors.YELLOW)
        result = subprocess.run(["cmake", ".."], cwd=build_dir, 
                              capture_output=True, text=True)
        if result.returncode != 0:
            print_colored(f"❌ CMake失败: {result.stderr}", Colors.RED)
            return False
    
    # 编译
    print_colored("编译中...", Colors.YELLOW)
    result = subprocess.run(["make", "-j4"], cwd=build_dir, 
                          capture_output=True, text=True)
    if result.returncode != 0:
        print_colored(f"❌ 编译失败: {result.stderr}", Colors.RED)
        return False
    
    print_colored("✅ 编译完成", Colors.GREEN)
    return True

def start_app():
    """启动应用"""
    print_colored("📱 启动 SmartPOS 应用...", Colors.YELLOW)
    
    build_dir = Path("build")
    executable = build_dir / "SmartPOS"
    
    if not executable.exists():
        print_colored("❌ 可执行文件不存在", Colors.RED)
        return None
    
    # 设置环境变量提高稳定性
    env = os.environ.copy()
    env['QT_LOGGING_RULES'] = '*.debug=false'
    env['MALLOC_CHECK_'] = '0'
    
    # 修复路径问题：在build目录下直接使用./SmartPOS
    process = subprocess.Popen(["./SmartPOS"], cwd=str(build_dir), env=env)
    
    # 检查是否立即崩溃
    time.sleep(2)
    if process.poll() is not None:
        print_colored("❌ 应用程序启动后立即退出", Colors.RED)
        return None
    
    print_colored(f"✅ SmartPOS已启动 (PID: {process.pid})", Colors.GREEN)
    return process

def save_pids(ai_pid, app_pid):
    """保存进程ID"""
    try:
        with open(".ai_server.pid", "w") as f:
            f.write(str(ai_pid))
        with open(".app.pid", "w") as f:
            f.write(str(app_pid))
    except Exception as e:
        print_colored(f"⚠️ 无法保存PID: {e}", Colors.YELLOW)

def cleanup(ai_process, app_process):
    """清理进程"""
    print_colored("\n🛑 停止服务...", Colors.YELLOW)
    
    for process, name in [(app_process, "应用程序"), (ai_process, "AI服务器")]:
        if process and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
                print_colored(f"✅ {name}已停止", Colors.GREEN)
            except subprocess.TimeoutExpired:
                process.kill()
                print_colored(f"⚠️ {name}已强制停止", Colors.YELLOW)

def monitor(ai_process, app_process):
    """监控进程"""
    try:
        while True:
            if ai_process.poll() is not None:
                print_colored("⚠️ AI服务器意外退出", Colors.RED)
                break
            if app_process.poll() is not None:
                print_colored("📱 应用程序已退出", Colors.YELLOW)
                break
            time.sleep(1)
    except KeyboardInterrupt:
        pass

def main():
    """主函数"""
    print_colored("=" * 50, Colors.BLUE)
    print_colored("   SmartPOS 修复版启动脚本", Colors.BOLD)
    print_colored("=" * 50, Colors.BLUE)
    print()
    
    # 切换到脚本目录
    os.chdir(Path(__file__).parent)
    
    ai_process = None
    app_process = None
    
    def signal_handler(signum, frame):
        cleanup(ai_process, app_process)
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    try:
        # 1. 清理现有进程
        check_and_kill_existing()
        
        # 2. 修复路径问题
        fix_paths()
        
        # 3. 启动AI服务器
        ai_process = start_ai_server()
        if not ai_process:
            print_colored("❌ AI服务器启动失败", Colors.RED)
            return 1
        
        # 4. 编译应用
        if not build_app():
            cleanup(ai_process, None)
            return 1
        
        # 5. 启动应用
        app_process = start_app()
        if not app_process:
            cleanup(ai_process, None)
            return 1
        
        # 6. 保存PID
        save_pids(ai_process.pid, app_process.pid)
        
        # 显示成功信息
        print()
        print_colored("🎉 系统启动成功！", Colors.GREEN)
        print_colored(f"AI服务器: http://127.0.0.1:5001 (PID: {ai_process.pid})", Colors.BLUE)
        print_colored(f"SmartPOS应用: PID {app_process.pid}", Colors.BLUE)
        print_colored("按 Ctrl+C 停止所有服务", Colors.YELLOW)
        print()
        
        # 监控进程
        monitor(ai_process, app_process)
        
    except Exception as e:
        print_colored(f"❌ 错误: {e}", Colors.RED)
        return 1
    finally:
        cleanup(ai_process, app_process)
    
    return 0

if __name__ == "__main__":
    sys.exit(main()) 