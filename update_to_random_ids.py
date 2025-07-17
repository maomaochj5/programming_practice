#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
将产品ID从BOOK001格式改为8位随机数
更新所有相关文件和映射
"""

import json
import random
import shutil
from pathlib import Path

def generate_8digit_id():
    """生成8位随机数ID"""
    return f"{random.randint(10000000, 99999999)}"

def ensure_unique_ids(existing_ids, count):
    """确保生成的ID是唯一的"""
    new_ids = set()
    while len(new_ids) < count:
        new_id = generate_8digit_id()
        if new_id not in existing_ids and new_id not in new_ids:
            new_ids.add(new_id)
    return list(new_ids)

def backup_files():
    """备份原始文件"""
    files_to_backup = [
        "Ai_model/products.json",
        "Ai_model/ai_to_db_id_mapping.json", 
        "Ai_model/db_to_ai_id_mapping.json",
        "Ai_model/index_to_id.json"
    ]
    
    print("📦 备份原始文件...")
    for file_path in files_to_backup:
        if Path(file_path).exists():
            backup_path = f"{file_path}.backup"
            shutil.copy2(file_path, backup_path)
            print(f"✓ 备份: {file_path} -> {backup_path}")

def update_products_json(old_to_new_mapping):
    """更新products.json文件"""
    print("📝 更新 products.json...")
    
    with open("Ai_model/products.json", "r", encoding="utf-8") as f:
        products = json.load(f)
    
    # 更新产品ID
    for product in products:
        old_id = product["product_id"]
        if old_id in old_to_new_mapping:
            product["product_id"] = old_to_new_mapping[old_id]
            print(f"   {old_id} -> {product['product_id']} ({product['product_name']})")
    
    # 保存更新后的文件
    with open("Ai_model/products.json", "w", encoding="utf-8") as f:
        json.dump(products, f, ensure_ascii=False, indent=4)
    
    print(f"✓ 已更新 {len(products)} 个产品的ID")

def update_mapping_files(old_to_new_mapping):
    """更新ID映射文件"""
    print("🔄 更新ID映射文件...")
    
    # 1. 更新 ai_to_db_id_mapping.json
    with open("Ai_model/ai_to_db_id_mapping.json", "r", encoding="utf-8") as f:
        ai_to_db = json.load(f)
    
    new_ai_to_db = {}
    for old_ai_id, db_id in ai_to_db.items():
        if old_ai_id in old_to_new_mapping:
            new_ai_id = old_to_new_mapping[old_ai_id]
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
        if old_ai_id in old_to_new_mapping:
            new_ai_id = old_to_new_mapping[old_ai_id]
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
        if old_id in old_to_new_mapping:
            new_index_to_id.append(old_to_new_mapping[old_id])
        else:
            new_index_to_id.append(old_id)
    
    with open("Ai_model/index_to_id.json", "w", encoding="utf-8") as f:
        json.dump(new_index_to_id, f, ensure_ascii=False, indent=2)
    
    print("✓ 已更新所有映射文件")

def update_api_server_mappings(old_to_new_mapping):
    """更新API服务器中的硬编码映射"""
    print("🤖 更新API服务器中的产品映射...")
    
    api_server_file = "Ai_model/api_server.py"
    
    # 读取文件内容
    with open(api_server_file, "r", encoding="utf-8") as f:
        content = f.read()
    
    # 替换映射中的ID
    for old_id, new_id in old_to_new_mapping.items():
        # 替换字典键
        content = content.replace(f"'{old_id}':", f"'{new_id}':")
        # 替换列表中的引用
        content = content.replace(f"'{old_id}'", f"'{new_id}'")
    
    # 保存更新后的文件
    with open(api_server_file, "w", encoding="utf-8") as f:
        f.write(content)
    
    print("✓ 已更新API服务器映射")

def display_mapping_summary(old_to_new_mapping):
    """显示映射总结"""
    print("\n📊 ID映射总结:")
    print("=" * 60)
    print(f"{'旧ID':<15} -> {'新ID':<15} 类别")
    print("=" * 60)
    
    # 按类别分组显示
    categories = {}
    for old_id, new_id in old_to_new_mapping.items():
        if old_id.startswith("BOOK"):
            category = "书籍"
        elif old_id.startswith("ELEC"):
            category = "电子产品"
        elif old_id.startswith("SPORTS"):
            category = "运动户外"
        elif old_id.startswith("OFFICE"):
            category = "办公用品"
        else:
            category = "其他"
        
        if category not in categories:
            categories[category] = []
        categories[category].append((old_id, new_id))
    
    for category, mappings in categories.items():
        print(f"\n{category}:")
        for old_id, new_id in mappings:
            print(f"  {old_id:<13} -> {new_id:<13}")

def main():
    """主函数"""
    print("=" * 60)
    print("   产品ID格式转换：字符串 -> 8位随机数")
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
    
    # 2. 读取现有产品
    print("\n📋 读取现有产品...")
    with open("Ai_model/products.json", "r", encoding="utf-8") as f:
        products = json.load(f)
    
    # 提取所有旧ID
    old_ids = [product["product_id"] for product in products]
    print(f"✓ 找到 {len(old_ids)} 个产品")
    
    # 3. 生成新的8位随机数ID
    print("🎲 生成8位随机数ID...")
    new_ids = ensure_unique_ids(set(), len(old_ids))
    
    # 创建映射关系
    old_to_new_mapping = dict(zip(old_ids, new_ids))
    
    # 4. 显示映射预览
    display_mapping_summary(old_to_new_mapping)
    
    # 5. 确认更新
    print(f"\n⚠️  即将更新 {len(old_ids)} 个产品ID")
    print("这将影响以下文件:")
    for file in required_files:
        print(f"   - {file}")
    
    response = input("\n是否继续? (y/N): ").strip().lower()
    if response != 'y':
        print("❌ 操作已取消")
        return 0
    
    try:
        # 6. 执行更新
        print("\n🚀 开始更新...")
        update_products_json(old_to_new_mapping)
        update_mapping_files(old_to_new_mapping)
        update_api_server_mappings(old_to_new_mapping)
        
        print("\n🎉 更新完成!")
        print("=" * 60)
        print("📋 更新总结:")
        print(f"   - 已更新 {len(old_ids)} 个产品ID为8位随机数")
        print(f"   - 已更新所有映射文件")
        print(f"   - 原始文件已备份 (*.backup)")
        print("\n💡 下次启动系统时将使用新的ID格式")
        print("   请重新启动AI服务器以加载新配置")
        
        return 0
        
    except Exception as e:
        print(f"❌ 更新过程中发生错误: {e}")
        print("💡 您可以使用备份文件恢复:")
        for file in required_files:
            if Path(f"{file}.backup").exists():
                print(f"   cp {file}.backup {file}")
        return 1

if __name__ == "__main__":
    import sys
    sys.exit(main()) 