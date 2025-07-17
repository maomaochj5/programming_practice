#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
更新产品ID为8位随机数的脚本
同时更新数据库和AI模型映射文件
"""

import sqlite3
import json
import random
import sys
from pathlib import Path

def generate_8_digit_id():
    """生成8位随机数ID"""
    return random.randint(10000000, 99999999)

def get_existing_products(db_path):
    """获取所有现有产品"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    cursor.execute("SELECT product_id, barcode FROM Products")
    products = cursor.fetchall()
    
    conn.close()
    return products

def create_id_mapping(products):
    """为每个产品创建新的8位ID映射"""
    mapping = {}
    used_ids = set()
    
    for old_id, barcode in products:
        while True:
            new_id = generate_8_digit_id()
            if new_id not in used_ids:
                used_ids.add(new_id)
                mapping[old_id] = new_id
                break
    
    return mapping

def backup_database(db_path):
    """备份数据库"""
    backup_path = f"{db_path}.backup"
    import shutil
    shutil.copy2(db_path, backup_path)
    print(f"✓ 数据库已备份到: {backup_path}")

def update_database(db_path, id_mapping):
    """更新数据库中的product_id"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    try:
        # 开始事务
        cursor.execute("BEGIN TRANSACTION")
        
        # 1. 创建临时表来存储新的产品数据
        cursor.execute("""
        CREATE TEMPORARY TABLE Products_temp AS 
        SELECT * FROM Products WHERE 1=0
        """)
        
        # 2. 插入更新后的产品数据
        for old_id, new_id in id_mapping.items():
            cursor.execute("""
            INSERT INTO Products_temp 
            SELECT ?, barcode, name, description, price, stock_quantity, 
                   category, image_path, created_at, updated_at
            FROM Products WHERE product_id = ?
            """, (new_id, old_id))
        
        # 3. 创建临时的TransactionItems表
        cursor.execute("""
        CREATE TEMPORARY TABLE TransactionItems_temp AS 
        SELECT * FROM TransactionItems WHERE 1=0
        """)
        
        # 4. 更新TransactionItems表中的product_id引用
        for old_id, new_id in id_mapping.items():
            cursor.execute("""
            INSERT INTO TransactionItems_temp
            SELECT transaction_item_id, transaction_id, ?, 
                   quantity, unit_price, subtotal
            FROM TransactionItems WHERE product_id = ?
            """, (new_id, old_id))
        
        # 5. 删除原始数据
        cursor.execute("DELETE FROM TransactionItems")
        cursor.execute("DELETE FROM Products")
        
        # 6. 从临时表恢复数据
        cursor.execute("""
        INSERT INTO Products 
        SELECT * FROM Products_temp
        """)
        
        cursor.execute("""
        INSERT INTO TransactionItems 
        SELECT * FROM TransactionItems_temp
        """)
        
        # 7. 删除临时表
        cursor.execute("DROP TABLE Products_temp")
        cursor.execute("DROP TABLE TransactionItems_temp")
        
        # 提交事务
        conn.commit()
        print("✓ 数据库更新成功")
        
    except Exception as e:
        conn.rollback()
        print(f"❌ 数据库更新失败: {e}")
        raise
    finally:
        conn.close()

def update_ai_mappings(id_mapping):
    """更新AI模型映射文件"""
    
    # 读取现有映射
    with open("Ai_model/ai_to_db_id_mapping.json", "r") as f:
        ai_to_db = json.load(f)
    
    with open("Ai_model/db_to_ai_id_mapping.json", "r") as f:
        db_to_ai = json.load(f)
    
    # 创建新的映射
    new_ai_to_db = {}
    new_db_to_ai = {}
    
    for ai_id, old_db_id in ai_to_db.items():
        if old_db_id in id_mapping:
            new_db_id = id_mapping[old_db_id]
            new_ai_to_db[ai_id] = new_db_id
            new_db_to_ai[str(new_db_id)] = ai_id
    
    # 备份原始文件
    import shutil
    shutil.copy2("Ai_model/ai_to_db_id_mapping.json", "Ai_model/ai_to_db_id_mapping.json.backup")
    shutil.copy2("Ai_model/db_to_ai_id_mapping.json", "Ai_model/db_to_ai_id_mapping.json.backup")
    
    # 写入新的映射文件
    with open("Ai_model/ai_to_db_id_mapping.json", "w") as f:
        json.dump(new_ai_to_db, f, indent=2, ensure_ascii=False)
    
    with open("Ai_model/db_to_ai_id_mapping.json", "w") as f:
        json.dump(new_db_to_ai, f, indent=2, ensure_ascii=False)
    
    print("✓ AI映射文件更新成功")

def verify_update(db_path, id_mapping):
    """验证更新结果"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # 检查产品ID是否为8位数
    cursor.execute("SELECT product_id FROM Products")
    product_ids = [row[0] for row in cursor.fetchall()]
    
    eight_digit_count = sum(1 for pid in product_ids if 10000000 <= pid <= 99999999)
    
    print(f"✓ 验证结果:")
    print(f"  - 总产品数: {len(product_ids)}")
    print(f"  - 8位数ID: {eight_digit_count}")
    print(f"  - 映射关系: {len(id_mapping)} 个")
    
    # 检查外键约束
    cursor.execute("""
    SELECT COUNT(*) FROM TransactionItems t
    LEFT JOIN Products p ON t.product_id = p.product_id
    WHERE p.product_id IS NULL
    """)
    orphaned = cursor.fetchone()[0]
    
    if orphaned == 0:
        print("  - 外键约束: ✓ 正常")
    else:
        print(f"  - 外键约束: ❌ 有{orphaned}个孤立记录")
    
    conn.close()

def main():
    """主函数"""
    print("=" * 50)
    print("   产品ID更新为8位随机数")
    print("=" * 50)
    
    db_path = "build/pos_database.db"
    
    # 检查文件是否存在
    if not Path(db_path).exists():
        print(f"❌ 数据库文件不存在: {db_path}")
        return 1
    
    if not Path("Ai_model/ai_to_db_id_mapping.json").exists():
        print("❌ AI映射文件不存在")
        return 1
    
    try:
        # 1. 获取现有产品
        print("📋 获取现有产品数据...")
        products = get_existing_products(db_path)
        print(f"✓ 找到 {len(products)} 个产品")
        
        # 2. 创建ID映射
        print("🔢 生成8位随机ID映射...")
        id_mapping = create_id_mapping(products)
        
        # 显示映射示例
        print("✓ ID映射示例:")
        for i, (old_id, new_id) in enumerate(list(id_mapping.items())[:5]):
            print(f"   {old_id} -> {new_id}")
        if len(id_mapping) > 5:
            print(f"   ... 还有 {len(id_mapping)-5} 个")
        
        # 3. 备份数据库
        print("💾 备份数据库...")
        backup_database(db_path)
        
        # 4. 更新数据库
        print("🔄 更新数据库...")
        update_database(db_path, id_mapping)
        
        # 5. 更新AI映射文件
        print("🤖 更新AI映射文件...")
        update_ai_mappings(id_mapping)
        
        # 6. 验证更新
        print("✅ 验证更新结果...")
        verify_update(db_path, id_mapping)
        
        print("\n" + "=" * 50)
        print("   🎉 产品ID更新完成!")
        print("=" * 50)
        print("\n📋 更新总结:")
        print(f"  - 已更新 {len(products)} 个产品的ID")
        print(f"  - 数据库备份: {db_path}.backup")
        print(f"  - AI映射备份: Ai_model/*.json.backup")
        print("\n💡 如需恢复，请运行:")
        print(f"  cp {db_path}.backup {db_path}")
        print("  cp Ai_model/*.json.backup Ai_model/")
        
        return 0
        
    except Exception as e:
        print(f"\n❌ 更新失败: {e}")
        print("\n🔧 建议:")
        print("1. 检查数据库文件权限")
        print("2. 确保没有其他程序正在使用数据库")
        print("3. 查看错误日志获取详细信息")
        return 1

if __name__ == "__main__":
    random.seed()  # 使用当前时间作为随机种子
    sys.exit(main())