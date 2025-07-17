#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
将产品ID从8位随机数改回到1-30的简单数字格式
更新所有相关文件和映射
"""

import json
import shutil
from pathlib import Path

def backup_files():
    """备份当前文件"""
    files_to_backup = [
        "Ai_model/products.json",
        "Ai_model/ai_to_db_id_mapping.json", 
        "Ai_model/db_to_ai_id_mapping.json",
        "Ai_model/index_to_id.json"
    ]
    
    print("📦 备份当前文件...")
    for file_path in files_to_backup:
        if Path(file_path).exists():
            backup_path = f"{file_path}.random_backup"
            shutil.copy2(file_path, backup_path)
            print(f"✓ 备份: {file_path} -> {backup_path}")

def create_simple_id_mapping():
    """创建简单ID映射关系"""
    print("📝 读取当前产品并创建新的ID映射...")
    
    # 读取当前产品
    with open("Ai_model/products.json", "r", encoding="utf-8") as f:
        products = json.load(f)
    
    # 创建从8位随机数到简单数字的映射
    random_to_simple_mapping = {}
    
    for i, product in enumerate(products, 1):
        old_random_id = product["product_id"]
        new_simple_id = str(i)  # 1, 2, 3, ..., 30
        random_to_simple_mapping[old_random_id] = new_simple_id
    
    print(f"✓ 创建了 {len(random_to_simple_mapping)} 个ID映射")
    return random_to_simple_mapping

def update_products_json(random_to_simple_mapping):
    """更新products.json文件"""
    print("🔄 更新 products.json...")
    
    with open("Ai_model/products.json", "r", encoding="utf-8") as f:
        products = json.load(f)
    
    # 更新产品ID
    for product in products:
        old_id = product["product_id"]
        if old_id in random_to_simple_mapping:
            new_id = random_to_simple_mapping[old_id]
            product["product_id"] = new_id
            print(f"   {old_id} -> {new_id} ({product['product_name']})")
    
    # 保存更新后的文件
    with open("Ai_model/products.json", "w", encoding="utf-8") as f:
        json.dump(products, f, ensure_ascii=False, indent=4)
    
    print(f"✓ 已更新 {len(products)} 个产品的ID")

def update_mapping_files(random_to_simple_mapping):
    """更新ID映射文件"""
    print("🔗 更新ID映射文件...")
    
    # 1. 更新 ai_to_db_id_mapping.json
    with open("Ai_model/ai_to_db_id_mapping.json", "r", encoding="utf-8") as f:
        ai_to_db = json.load(f)
    
    new_ai_to_db = {}
    for old_ai_id, db_id in ai_to_db.items():
        if old_ai_id in random_to_simple_mapping:
            new_ai_id = random_to_simple_mapping[old_ai_id]
            new_ai_to_db[new_ai_id] = db_id
        else:
            new_ai_to_db[old_ai_id] = db_id
    
    with open("Ai_model/ai_to_db_id_mapping.json", "w", encoding="utf-8") as f:
        json.dump(new_ai_to_db, f, ensure_ascii=False, indent=2)
    
    # 2. 更新 db_to_ai_id_mapping.json
    with open("Ai_model/db_to_ai_id_mapping.json", "r", encoding="utf-8") as f:
        db_to_ai = json.load(f)
    
    new_db_to_ai = {}
    for db_id, old_ai_id in db_to_ai.items():
        if old_ai_id in random_to_simple_mapping:
            new_ai_id = random_to_simple_mapping[old_ai_id]
            new_db_to_ai[db_id] = new_ai_id
        else:
            new_db_to_ai[db_id] = old_ai_id
    
    with open("Ai_model/db_to_ai_id_mapping.json", "w", encoding="utf-8") as f:
        json.dump(new_db_to_ai, f, ensure_ascii=False, indent=2)
    
    # 3. 更新 index_to_id.json
    with open("Ai_model/index_to_id.json", "r", encoding="utf-8") as f:
        index_to_id = json.load(f)
    
    new_index_to_id = []
    for old_id in index_to_id:
        if old_id in random_to_simple_mapping:
            new_index_to_id.append(random_to_simple_mapping[old_id])
        else:
            new_index_to_id.append(old_id)
    
    with open("Ai_model/index_to_id.json", "w", encoding="utf-8") as f:
        json.dump(new_index_to_id, f, ensure_ascii=False, indent=2)
    
    print("✓ 已更新所有映射文件")

def update_api_server_mappings(random_to_simple_mapping):
    """更新API服务器中的硬编码映射"""
    print("🤖 更新API服务器中的产品映射...")
    
    api_server_file = "Ai_model/api_server.py"
    
    # 读取文件内容
    with open(api_server_file, "r", encoding="utf-8") as f:
        content = f.read()
    
    # 替换映射中的ID
    for old_id, new_id in random_to_simple_mapping.items():
        # 替换字典键
        content = content.replace(f"'{old_id}':", f"'{new_id}':")
        # 替换列表中的引用
        content = content.replace(f"'{old_id}'", f"'{new_id}'")
    
    # 保存更新后的文件
    with open(api_server_file, "w", encoding="utf-8") as f:
        f.write(content)
    
    print("✓ 已更新API服务器映射")

def display_conversion_summary(random_to_simple_mapping):
    """显示转换总结"""
    print("\n📊 ID转换总结:")
    print("=" * 60)
    print(f"{'8位随机数ID':<15} -> {'简单数字ID':<10} 序号")
    print("=" * 60)
    
    # 按简单ID排序显示
    sorted_mappings = sorted(random_to_simple_mapping.items(), key=lambda x: int(x[1]))
    
    for old_id, new_id in sorted_mappings:
        print(f"  {old_id:<13} -> {new_id:<8} #{new_id}")

def main():
    """主函数"""
    print("=" * 60)
    print("   产品ID格式转换：8位随机数 -> 简单数字(1-30)")
    print("=" * 60)
    
    # 检查必要文件是否存在
    required_files = [
        "Ai_model/products.json",
        "Ai_model/ai_to_db_id_mapping.json",
        "Ai_model/db_to_ai_id_mapping.json",
        "Ai_model/index_to_id.json",
        "Ai_model/api_server.py"
    ]
    
    missing_files = [f for f in required_files if not Path(f).exists()]
    if missing_files:
        print("❌ 缺少必要文件:")
        for file in missing_files:
            print(f"   - {file}")
        return 1
    
    # 1. 备份文件
    backup_files()
    
    # 2. 创建ID映射
    random_to_simple_mapping = create_simple_id_mapping()
    
    # 3. 显示转换预览
    display_conversion_summary(random_to_simple_mapping)
    
    # 4. 确认更新
    print(f"\n⚠️  即将将 {len(random_to_simple_mapping)} 个产品ID改为简单数字格式(1-30)")
    print("这将影响以下文件:")
    for file in required_files:
        print(f"   - {file}")
    
    response = input("\n是否继续? (y/N): ").strip().lower()
    if response != 'y':
        print("❌ 操作已取消")
        return 0
    
    try:
        # 5. 执行更新
        print("\n🚀 开始转换...")
        update_products_json(random_to_simple_mapping)
        update_mapping_files(random_to_simple_mapping)
        update_api_server_mappings(random_to_simple_mapping)
        
        print("\n🎉 转换完成!")
        print("=" * 60)
        print("📋 转换总结:")
        print(f"   - 已将 {len(random_to_simple_mapping)} 个产品ID改为简单数字(1-30)")
        print(f"   - 已更新所有映射文件")
        print(f"   - 8位随机数格式文件已备份 (*.random_backup)")
        print("\n💡 下次启动系统时将使用简单数字ID格式")
        print("   请重新启动AI服务器以加载新配置")
        
        # 显示最终结果示例
        print("\n📋 新的ID格式示例:")
        print("   1  - 罗马的黄昏：一部帝国衰亡史")
        print("   2  - 力动 Pro 无线运动耳机")
        print("   3  - 野径大师 X4 越野跑鞋")
        print("   ... (总共30个)")
        
        return 0
        
    except Exception as e:
        print(f"❌ 转换过程中发生错误: {e}")
        print("💡 您可以使用备份文件恢复:")
        for file in required_files:
            if Path(f"{file}.random_backup").exists():
                print(f"   cp {file}.random_backup {file}")
        return 1

if __name__ == "__main__":
    import sys
    sys.exit(main()) 