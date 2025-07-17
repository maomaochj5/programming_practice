#!/usr/bin/env python3
"""
智能POS系统一键启动脚本
启动AI服务器和POS程序
"""

import os
import sys
import time
import subprocess
import signal
import threading
import requests
from pathlib import Path

class SystemLauncher:
    def __init__(self):
        self.ai_server_process = None
        self.pos_process = None
        self.ai_server_url = "http://localhost:5001"
        
    def check_dependencies(self):
        """检查必要的依赖"""
        print("🔍 检查系统依赖...")
        
        # 检查Python依赖
        try:
            import torch
            import transformers
            import sentence_transformers
            print("✅ Python AI依赖已安装")
        except ImportError as e:
            print(f"❌ 缺少Python AI依赖: {e}")
            print("请运行: pip install torch transformers sentence-transformers")
            return False
            
        # 检查Qt
        try:
            import PyQt5
            print("✅ PyQt5已安装")
        except ImportError:
            print("❌ 缺少PyQt5")
            print("请运行: pip install PyQt5")
            return False
            
        return True
    
    def start_ai_server(self):
        """启动AI服务器"""
        print("🚀 启动AI服务器...")
        
        ai_server_path = Path("Ai_model/start_ai_server.py")
        if not ai_server_path.exists():
            print(f"❌ AI服务器文件不存在: {ai_server_path}")
            return False
            
        try:
            # 切换到AI模型目录
            os.chdir("Ai_model")
            
            # 启动AI服务器
            self.ai_server_process = subprocess.Popen(
                [sys.executable, "start_ai_server.py"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            # 等待服务器启动
            print("⏳ 等待AI服务器启动...")
            for i in range(30):  # 最多等待30秒
                try:
                    response = requests.get(f"{self.ai_server_url}/health", timeout=2)
                    if response.status_code == 200:
                        print("✅ AI服务器启动成功")
                        return True
                except requests.exceptions.RequestException:
                    pass
                time.sleep(1)
                print(f"  等待中... ({i+1}/30)")
                
            print("❌ AI服务器启动超时")
            return False
            
        except Exception as e:
            print(f"❌ 启动AI服务器失败: {e}")
            return False
        finally:
            # 切换回原目录
            os.chdir("..")
    
    def build_pos_system(self):
        """编译POS系统"""
        print("🔨 编译POS系统...")
        
        try:
            # 创建build目录
            build_dir = Path("build")
            build_dir.mkdir(exist_ok=True)
            
            # 切换到build目录
            os.chdir("build")
            
            # 运行cmake
            result = subprocess.run(["cmake", ".."], capture_output=True, text=True)
            if result.returncode != 0:
                print(f"❌ CMake配置失败: {result.stderr}")
                return False
                
            # 编译
            result = subprocess.run(["make"], capture_output=True, text=True)
            if result.returncode != 0:
                print(f"❌ 编译失败: {result.stderr}")
                return False
                
            print("✅ POS系统编译成功")
            return True
            
        except Exception as e:
            print(f"❌ 编译POS系统失败: {e}")
            return False
        finally:
            # 切换回原目录
            os.chdir("..")
    
    def start_pos_system(self):
        """启动POS系统"""
        print("🖥️  启动POS系统...")
        
        pos_executable = Path("build/pos_system")
        if not pos_executable.exists():
            print(f"❌ POS可执行文件不存在: {pos_executable}")
            return False
            
        try:
            self.pos_process = subprocess.Popen(
                [str(pos_executable)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            print("✅ POS系统启动成功")
            return True
            
        except Exception as e:
            print(f"❌ 启动POS系统失败: {e}")
            return False
    
    def wait_for_shutdown(self):
        """等待用户关闭程序"""
        print("\n" + "="*50)
        print("🎉 系统启动完成!")
        print("📱 POS系统界面应该已经打开")
        print("🤖 AI服务器运行在: http://localhost:5001")
        print("💡 按 Ctrl+C 关闭所有服务")
        print("="*50)
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n🛑 正在关闭系统...")
            self.cleanup()
    
    def cleanup(self):
        """清理进程"""
        if self.ai_server_process:
            print("🛑 关闭AI服务器...")
            self.ai_server_process.terminate()
            try:
                self.ai_server_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.ai_server_process.kill()
                
        if self.pos_process:
            print("🛑 关闭POS系统...")
            self.pos_process.terminate()
            try:
                self.pos_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.pos_process.kill()
                
        print("✅ 系统已关闭")
    
    def kill_existing_processes(self):
        """杀死可能占用端口的进程"""
        print("🧹 清理可能冲突的进程...")
        
        try:
            # 杀死占用5001端口的进程
            result = subprocess.run(
                ["lsof", "-ti:5001"], 
                capture_output=True, 
                text=True
            )
            if result.stdout.strip():
                pids = result.stdout.strip().split('\n')
                for pid in pids:
                    if pid:
                        subprocess.run(["kill", "-9", pid])
                        print(f"  已杀死进程 {pid}")
        except Exception:
            pass
    
    def run(self):
        """运行整个系统"""
        print("🎯 智能POS系统启动器")
        print("="*50)
        
        # 检查依赖
        if not self.check_dependencies():
            return
            
        # 清理冲突进程
        self.kill_existing_processes()
        
        # 启动AI服务器
        if not self.start_ai_server():
            print("❌ 无法启动AI服务器，退出")
            return
            
        # 编译POS系统
        if not self.build_pos_system():
            print("❌ 无法编译POS系统，退出")
            return
            
        # 启动POS系统
        if not self.start_pos_system():
            print("❌ 无法启动POS系统，退出")
            return
            
        # 等待用户关闭
        self.wait_for_shutdown()

def main():
    launcher = SystemLauncher()
    launcher.run()

if __name__ == "__main__":
    main() 