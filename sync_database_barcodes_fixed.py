#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
数据库条形码同步脚本 - 修复版
- 处理条形码冲突问题
- 将数据库中的所有条形码统一为8位随机数
- 同步products.json与数据库的产品ID
"""

import json
import sqlite3
import shutil
import os
from datetime import datetime
from pathlib import Path

class DatabaseBarcodeSyncFixed:
    def __init__(self):
        self.script_dir = Path(__file__).parent
        self.db_path = "build/pos_database.db"
        self.products_json_path = "Ai_model/products.json"
        
    def backup_database(self):
        """备份数据库"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_path = f"{self.db_path}.barcode_sync_fixed_{timestamp}"
        shutil.copy2(self.db_path, backup_path)
        print(f"🔄 数据库已备份: {backup_path}")
        return backup_path
        
    def load_products_json(self):
        """加载products.json中的产品数据"""
        with open(self.products_json_path, 'r', encoding='utf-8') as f:
            products = json.load(f)
        
        # 创建产品名称到新ID的映射
        name_to_new_id = {}
        for product in products:
            name_to_new_id[product['product_name']] = product['product_id']
            
        print(f"📋 从products.json加载了 {len(products)} 个产品映射")
        return name_to_new_id, products
        
    def check_database_status(self):
        """检查数据库当前状态"""
        print("🔍 检查数据库当前状态...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # 检查总产品数
        cursor.execute("SELECT COUNT(*) FROM Products")
        total_count = cursor.fetchone()[0]
        
        # 检查条形码格式
        cursor.execute("SELECT product_id, barcode, name FROM Products")
        products = cursor.fetchall()
        
        numeric_barcodes = 0
        string_barcodes = 0
        mixed_barcodes = 0
        
        for product_id, barcode, name in products:
            if barcode.isdigit():
                if len(barcode) == 8:
                    numeric_barcodes += 1
                else:
                    mixed_barcodes += 1
            else:
                string_barcodes += 1
                
        conn.close()
        
        print(f"   📊 数据库状态:")
        print(f"      总产品数: {total_count}")
        print(f"      8位数字条形码: {numeric_barcodes}")
        print(f"      字符串条形码: {string_barcodes}")
        print(f"      其他数字条形码: {mixed_barcodes}")
        
        return {
            'total': total_count,
            'numeric_8digit': numeric_barcodes,
            'string': string_barcodes,
            'mixed': mixed_barcodes,
            'products': products
        }
        
    def check_barcode_conflicts(self, name_to_new_id):
        """检查条形码冲突"""
        print("🔍 检查潜在的条形码冲突...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # 获取所有当前条形码
        cursor.execute("SELECT barcode FROM Products")
        current_barcodes = set(row[0] for row in cursor.fetchall())
        
        # 获取我们要设置的新条形码
        new_barcodes = set(name_to_new_id.values())
        
        # 找出冲突
        conflicts = current_barcodes.intersection(new_barcodes)
        
        conn.close()
        
        if conflicts:
            print(f"⚠️  发现 {len(conflicts)} 个条形码冲突:")
            for barcode in conflicts:
                print(f"      {barcode}")
        else:
            print("✅ 未发现条形码冲突")
            
        return conflicts
        
    def sync_database_barcodes_safe(self, name_to_new_id, products_data):
        """安全地同步数据库中的条形码"""
        print("🔧 开始安全同步数据库条形码...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        try:
            cursor.execute("BEGIN TRANSACTION")
            
            # 第一步：临时将可能冲突的条形码改为临时值
            print("   🔄 第一阶段：处理条形码冲突...")
            temp_suffix = "_TEMP"
            
            # 获取所有要更新的产品
            cursor.execute("SELECT product_id, barcode, name FROM Products")
            db_products = cursor.fetchall()
            
            # 先将所有需要更新的条形码改为临时值
            temp_updates = []
            for db_product_id, db_barcode, db_name in db_products:
                if db_name in name_to_new_id:
                    new_barcode = name_to_new_id[db_name]
                    if db_barcode != new_barcode:
                        temp_barcode = f"{new_barcode}{temp_suffix}"
                        cursor.execute("""
                            UPDATE Products 
                            SET barcode = ? 
                            WHERE product_id = ?
                        """, (temp_barcode, db_product_id))
                        temp_updates.append((db_product_id, temp_barcode, new_barcode, db_name))
                        
            print(f"   ✓ 临时更新了 {len(temp_updates)} 个条形码")
            
            # 第二步：将临时条形码改为最终值
            print("   🔄 第二阶段：设置最终条形码...")
            final_updates = 0
            
            for db_product_id, temp_barcode, final_barcode, db_name in temp_updates:
                cursor.execute("""
                    UPDATE Products 
                    SET barcode = ?, updated_at = CURRENT_TIMESTAMP 
                    WHERE product_id = ?
                """, (final_barcode, db_product_id))
                final_updates += 1
                print(f"   ✓ 更新: {db_name} - 条形码设置为: {final_barcode}")
                
            # 第三步：处理未匹配的产品
            print("   🔄 第三阶段：处理未匹配的产品...")
            unmatched_count = 0
            
            for db_product_id, db_barcode, db_name in db_products:
                if db_name not in name_to_new_id:
                    # 使用product_id作为条形码
                    new_barcode = str(db_product_id)
                    if db_barcode != new_barcode and not db_barcode.endswith(temp_suffix):
                        cursor.execute("""
                            UPDATE Products 
                            SET barcode = ?, updated_at = CURRENT_TIMESTAMP 
                            WHERE product_id = ?
                        """, (new_barcode, db_product_id))
                        unmatched_count += 1
                        print(f"   🔧 自动修复: {db_name} - 条形码: {db_barcode} → {new_barcode}")
            
            cursor.execute("COMMIT")
            
            print(f"   📊 同步结果:")
            print(f"      匹配的产品更新: {final_updates}")
            print(f"      未匹配产品修复: {unmatched_count}")
            print(f"      总更新数: {final_updates + unmatched_count}")
            
            return final_updates + unmatched_count
            
        except Exception as e:
            cursor.execute("ROLLBACK")
            print(f"❌ 数据库同步失败: {e}")
            raise
        finally:
            conn.close()
        
    def verify_sync_result(self):
        """验证同步结果"""
        print("✅ 验证同步结果...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # 检查是否有临时条形码残留
        cursor.execute("SELECT COUNT(*) FROM Products WHERE barcode LIKE '%_TEMP'")
        temp_count = cursor.fetchone()[0]
        
        if temp_count > 0:
            print(f"⚠️  发现 {temp_count} 个临时条形码残留")
            return False
            
        # 检查所有条形码是否为数字
        cursor.execute("""
            SELECT product_id, barcode, name 
            FROM Products 
            WHERE barcode NOT GLOB '[0-9]*' OR LENGTH(barcode) < 7
        """)
        
        invalid_barcodes = cursor.fetchall()
        
        if invalid_barcodes:
            print("⚠️  发现无效条形码:")
            for product_id, barcode, name in invalid_barcodes:
                print(f"      {name}: {barcode}")
        else:
            print("✅ 所有条形码均为有效数字格式")
            
        # 统计最终状态
        cursor.execute("SELECT COUNT(*) FROM Products WHERE LENGTH(barcode) >= 7 AND barcode GLOB '[0-9]*'")
        valid_numeric_count = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM Products")
        total_count = cursor.fetchone()[0]
        
        print(f"📊 最终状态:")
        print(f"   总产品数: {total_count}")
        print(f"   数字条形码: {valid_numeric_count}")
        print(f"   同步成功率: {valid_numeric_count/total_count*100:.1f}%")
        
        conn.close()
        
        return len(invalid_barcodes) == 0 and temp_count == 0
        
    def generate_sync_report(self, backup_path, updated_count, sync_success):
        """生成同步报告"""
        report_file = "数据库条形码同步报告_修复版.md"
        timestamp = datetime.now().strftime("%Y年%m月%d日 %H:%M:%S")
        
        report_content = f"""# 数据库条形码同步报告 (修复版)

## 同步时间
{timestamp}

## 同步目标
- 统一数据库中所有条形码为8位随机数格式
- 与products.json保持数据一致性
- 安全处理条形码冲突

## 同步结果

### ✅ 同步状态
- **同步状态**: {"成功" if sync_success else "部分成功"}
- **更新条形码数量**: {updated_count}
- **数据库备份**: {backup_path}

### 📊 操作详情
1. **备份原始数据库** - ✅ 完成
2. **检查条形码冲突** - ✅ 完成
3. **安全同步条形码** - ✅ 完成
4. **验证同步结果** - {"✅ 完成" if sync_success else "⚠️ 需要检查"}

### 🔧 技术细节
- **数据库路径**: {self.db_path}
- **Products.json路径**: {self.products_json_path}
- **条形码格式**: 8位随机数 + 自动生成数字
- **同步方式**: 安全的三阶段更新
- **冲突处理**: 临时前缀方法

### 📋 下一步操作
1. **重启系统**: 使用 `python3 start_system_fixed.py`
2. **测试功能**: 验证商品搜索和扫描功能
3. **验证数据**: 检查条形码扫描是否正常

### 🚨 注意事项
- 原始数据库已备份，如有问题可恢复
- 条形码现在使用纯数字格式
- 系统不再需要复杂的映射文件

---
*同步脚本自动生成 - {timestamp}*
"""

        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report_content)
            
        print(f"📊 同步报告已生成: {report_file}")
        
    def run(self):
        """执行完整的同步流程"""
        print("=" * 60)
        print("   数据库条形码同步工具 (修复版)")
        print("   安全统一所有条形码为纯数字")
        print("=" * 60)
        
        try:
            # 1. 检查数据库状态
            status = self.check_database_status()
            
            # 2. 备份数据库
            backup_path = self.backup_database()
            
            # 3. 加载products.json映射
            name_to_new_id, products_data = self.load_products_json()
            
            # 4. 检查条形码冲突
            conflicts = self.check_barcode_conflicts(name_to_new_id)
            
            # 5. 安全同步数据库条形码
            updated_count = self.sync_database_barcodes_safe(name_to_new_id, products_data)
            
            # 6. 验证同步结果
            sync_success = self.verify_sync_result()
            
            # 7. 生成报告
            self.generate_sync_report(backup_path, updated_count, sync_success)
            
            print("\n" + "=" * 60)
            if sync_success:
                print("🎉 数据库条形码同步完成！")
                print("✅ 所有条形码已统一为纯数字格式")
            else:
                print("⚠️  数据库条形码同步部分完成")
                print("💡 请检查报告了解详情")
                
            print("✅ 原始数据库已安全备份")
            print("📊 详细报告已生成")
            print("\n🔄 下一步: 重启系统测试功能")
            print("   python3 start_system_fixed.py")
            print("=" * 60)
            
        except Exception as e:
            print(f"\n❌ 同步过程中发生错误: {e}")
            print("💡 建议：检查备份文件并手动恢复")
            raise

if __name__ == "__main__":
    sync_tool = DatabaseBarcodeSyncFixed()
    sync_tool.run() 