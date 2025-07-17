#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AI导购助手服务器启动脚本
"""

import sys
import os
import subprocess
import time

def check_dependencies():
    """检查Python依赖是否安装"""
    print("检查Python依赖...")
    
    required_packages = {
        'flask': 'flask', 
        'flask_cors': 'flask_cors', 
        'sentence_transformers': 'sentence_transformers', 
        'faiss_cpu': 'faiss', 
        'transformers': 'transformers', 
        'torch': 'torch', 
        'jieba': 'jieba'
    }
    
    missing_packages = []
    
    for display_name, import_name in required_packages.items():
        try:
            __import__(import_name)
            print(f"✓ {display_name}")
        except ImportError:
            missing_packages.append(display_name)
            print(f"✗ {display_name}")
    
    if missing_packages:
        print(f"\n缺少以下依赖包: {', '.join(missing_packages)}")
        print("请运行: pip install -r requirements.txt")
        return False
    
    print("所有依赖已安装 ✓")
    return True

def check_models():
    """检查AI模型文件"""
    print("\n检查AI模型文件...")
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(script_dir, 'fine-tuned-retriever')
    
    if not os.path.exists(model_path):
        print(f"✗ 微调检索器模型未找到: {model_path}")
        print("请先运行: python train_retriever.py")
        return False
    
    print(f"✓ 微调检索器模型: {model_path}")
    
    # 检查其他必要文件
    files_to_check = [
        'products.json',
        'index_to_id.json'
    ]
    
    for file_name in files_to_check:
        file_path = os.path.join(script_dir, file_name)
        if os.path.exists(file_path):
            print(f"✓ {file_name}")
        else:
            print(f"✗ {file_name}")
            return False
    
    return True

def start_server():
    """启动AI服务器"""
    print("\n启动AI导购助手服务器...")
    
    try:
        # 切换到脚本目录
        script_dir = os.path.dirname(os.path.abspath(__file__))
        os.chdir(script_dir)
        
        # 启动Flask服务器
        print("正在启动Flask API服务器...")
        print("服务器地址: http://127.0.0.1:5000")
        print("健康检查: http://127.0.0.1:5000/health")
        print("\n按 Ctrl+C 停止服务器\n")
        
        subprocess.run([sys.executable, 'api_server.py'], check=True)
        
    except KeyboardInterrupt:
        print("\n服务器已停止")
    except Exception as e:
        print(f"启动服务器时出错: {e}")
        return False
    
    return True

def main():
    """主函数"""
    print("=== AI导购助手服务器启动器 ===")
    
    # 检查依赖
    if not check_dependencies():
        return 1
    
    # 检查模型
    if not check_models():
        return 1
    
    # 启动服务器
    if not start_server():
        return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main()) 