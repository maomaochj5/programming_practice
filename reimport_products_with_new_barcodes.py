#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
重新导入产品数据脚本
- 清空数据库并重新导入products.json中的所有产品
- 使用纯数字条形码
- 简单直接的解决方案
"""

import json
import sqlite3
import shutil
from datetime import datetime
from pathlib import Path

class ProductReimporter:
    def __init__(self):
        self.db_path = "build/pos_database.db"
        self.products_json_path = "Ai_model/products.json"
        
    def backup_database(self):
        """备份数据库"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_path = f"{self.db_path}.reimport_backup_{timestamp}"
        shutil.copy2(self.db_path, backup_path)
        print(f"🔄 数据库已备份: {backup_path}")
        return backup_path
        
    def load_products_json(self):
        """加载products.json"""
        with open(self.products_json_path, 'r', encoding='utf-8') as f:
            products = json.load(f)
        print(f"📋 从products.json加载了 {len(products)} 个产品")
        return products
        
    def clear_and_reimport_products(self, products):
        """清空数据库并重新导入产品"""
        print("🗑️  清空现有产品数据...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        try:
            cursor.execute("BEGIN TRANSACTION")
            
            # 清空产品表
            cursor.execute("DELETE FROM Products")
            print("   ✓ 已清空Products表")
            
            # 重置自增ID（可选）
            cursor.execute("DELETE FROM sqlite_sequence WHERE name='Products'")
            
            # 重新导入产品
            print("📥 重新导入产品数据...")
            
            insert_query = """
                INSERT INTO Products (barcode, name, description, price, stock_quantity, category, image_path)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            """
            
            imported_count = 0
            for product in products:
                product_id = product['product_id']
                name = product['product_name']
                category = product['category']
                price = product['price']
                description = product['description']
                barcode = product.get('barcode', product_id)  # 使用barcode字段，如果没有则用product_id
                
                cursor.execute(insert_query, (
                    barcode,      # 条形码 (8位随机数)
                    name,         # 商品名称
                    description,  # 描述
                    price,        # 价格
                    100,          # 默认库存100
                    category,     # 分类
                    ""           # 图片路径（空）
                ))
                
                imported_count += 1
                print(f"   ✓ 导入: {name} (条形码: {barcode})")
                
            cursor.execute("COMMIT")
            print(f"✅ 成功导入 {imported_count} 个产品")
            
            return imported_count
            
        except Exception as e:
            cursor.execute("ROLLBACK")
            print(f"❌ 导入失败: {e}")
            raise
        finally:
            conn.close()
            
    def verify_import_result(self):
        """验证导入结果"""
        print("✅ 验证导入结果...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # 检查总数
        cursor.execute("SELECT COUNT(*) FROM Products")
        total_count = cursor.fetchone()[0]
        
        # 检查条形码格式
        cursor.execute("SELECT COUNT(*) FROM Products WHERE barcode GLOB '[0-9]*' AND LENGTH(barcode) >= 7")
        numeric_barcode_count = cursor.fetchone()[0]
        
        # 检查是否有重复的条形码
        cursor.execute("SELECT barcode, COUNT(*) FROM Products GROUP BY barcode HAVING COUNT(*) > 1")
        duplicates = cursor.fetchall()
        
        print(f"📊 验证结果:")
        print(f"   总产品数: {total_count}")
        print(f"   数字条形码: {numeric_barcode_count}")
        print(f"   重复条形码: {len(duplicates)}")
        
        if duplicates:
            print("⚠️  发现重复条形码:")
            for barcode, count in duplicates:
                print(f"      {barcode}: {count}次")
                
        # 显示前5个产品作为示例
        cursor.execute("SELECT product_id, barcode, name FROM Products LIMIT 5")
        sample_products = cursor.fetchall()
        
        print("📋 产品示例:")
        for product_id, barcode, name in sample_products:
            print(f"   ID: {product_id}, 条形码: {barcode}, 名称: {name}")
            
        conn.close()
        
        return len(duplicates) == 0 and numeric_barcode_count == total_count
        
    def generate_report(self, backup_path, imported_count, success):
        """生成导入报告"""
        report_file = "产品重新导入报告.md"
        timestamp = datetime.now().strftime("%Y年%m月%d日 %H:%M:%S")
        
        report_content = f"""# 产品重新导入报告

## 导入时间
{timestamp}

## 导入目标
- 清空数据库中的所有产品数据
- 重新导入products.json中的产品
- 使用统一的纯数字条形码

## 导入结果

### ✅ 导入状态
- **导入状态**: {"成功" if success else "部分成功"}
- **导入产品数量**: {imported_count}
- **数据库备份**: {backup_path}

### 📊 操作详情
1. **备份原始数据库** - ✅ 完成
2. **清空现有产品数据** - ✅ 完成
3. **重新导入产品** - ✅ 完成
4. **验证导入结果** - {"✅ 完成" if success else "⚠️ 需要检查"}

### 🔧 技术细节
- **数据库路径**: {self.db_path}
- **数据源**: {self.products_json_path}
- **条形码格式**: 纯数字 (8位随机数)
- **默认库存**: 100件
- **导入方式**: 清空后重新导入

### 📋 下一步操作
1. **重启系统**: 使用 `python3 start_system_fixed.py`
2. **测试功能**: 验证商品搜索和扫描功能
3. **检查数据**: 确认所有产品正常显示

### 🚨 注意事项
- 原始数据库已备份，如有问题可恢复
- 所有产品使用纯数字条形码
- 产品ID现在是数据库自动生成的
- 条形码与products.json中的product_id对应

---
*重新导入脚本自动生成 - {timestamp}*
"""

        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report_content)
            
        print(f"📊 导入报告已生成: {report_file}")
        
    def run(self):
        """执行完整的重新导入流程"""
        print("=" * 60)
        print("   产品数据重新导入工具")
        print("   清空并重新导入所有产品数据")
        print("=" * 60)
        
        try:
            # 1. 备份数据库
            backup_path = self.backup_database()
            
            # 2. 加载products.json
            products = self.load_products_json()
            
            # 3. 清空并重新导入产品
            imported_count = self.clear_and_reimport_products(products)
            
            # 4. 验证导入结果
            success = self.verify_import_result()
            
            # 5. 生成报告
            self.generate_report(backup_path, imported_count, success)
            
            print("\n" + "=" * 60)
            if success:
                print("🎉 产品数据重新导入完成！")
                print("✅ 所有产品已使用纯数字条形码")
            else:
                print("⚠️  产品数据导入部分完成")
                print("💡 请检查报告了解详情")
                
            print("✅ 原始数据库已备份")
            print("📊 详细报告已生成")
            print("\n🔄 下一步: 重启系统测试功能")
            print("   python3 start_system_fixed.py")
            print("=" * 60)
            
        except Exception as e:
            print(f"\n❌ 导入过程中发生错误: {e}")
            print("💡 建议：检查备份文件并手动恢复")
            raise

if __name__ == "__main__":
    reimporter = ProductReimporter()
    reimporter.run() 