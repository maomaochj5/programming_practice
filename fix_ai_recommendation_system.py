#!/usr/bin/env python3
"""
修复AI推荐系统的产品ID映射问题
将旧的字符串ID格式更新为8位随机数格式
"""

import json
import os

def main():
    print("=== AI推荐系统产品ID修复工具 ===")
    
    # 读取当前产品数据
    products_file = "Ai_model/products.json"
    if not os.path.exists(products_file):
        print(f"错误：找不到产品文件 {products_file}")
        return
    
    with open(products_file, 'r', encoding='utf-8') as f:
        products = json.load(f)
    
    print(f"读取到 {len(products)} 个产品")
    
    # 建立类别到产品ID的映射
    category_mapping = {}
    for product in products:
        category = product['category']
        if category not in category_mapping:
            category_mapping[category] = []
        category_mapping[category].append(product['product_id'])
    
    print("类别映射：")
    for category, ids in category_mapping.items():
        print(f"  {category}: {len(ids)} 个产品")
    
    # 建立关键词到产品ID的精确映射
    keyword_to_id = {}
    
    # 根据产品名称和描述建立关键词映射
    for product in products:
        product_id = product['product_id']
        name = product['product_name'].lower()
        desc = product['description'].lower()
        
        # 耳机类产品
        if any(keyword in name or keyword in desc for keyword in ['耳机', '蓝牙', '运动耳机']):
            keyword_to_id['耳机'] = product_id
            keyword_to_id['蓝牙耳机'] = product_id
            keyword_to_id['运动耳机'] = product_id
        
        # 充电宝类产品
        elif any(keyword in name or keyword in desc for keyword in ['充电宝', '移动电源', '充电']):
            keyword_to_id['充电宝'] = product_id
            keyword_to_id['移动电源'] = product_id
        
        # 书籍类产品 - 历史类
        elif any(keyword in name or keyword in desc for keyword in ['历史', '罗马', '帝国']):
            keyword_to_id['历史书'] = product_id
            keyword_to_id['罗马'] = product_id
            keyword_to_id['历史'] = product_id
        
        # 运动鞋类产品
        elif any(keyword in name or keyword in desc for keyword in ['跑鞋', '运动鞋', '鞋子', '越野']):
            keyword_to_id['鞋子'] = product_id
            keyword_to_id['鞋'] = product_id
            keyword_to_id['跑鞋'] = product_id
            keyword_to_id['越野鞋'] = product_id
            keyword_to_id['运动鞋'] = product_id
    
    # 选择代表性的热门商品ID（耳机、运动鞋、历史书籍）
    popular_products = []
    if '耳机' in keyword_to_id:
        popular_products.append(keyword_to_id['耳机'])
    if '鞋子' in keyword_to_id:
        popular_products.append(keyword_to_id['鞋子'])
    if '历史书' in keyword_to_id:
        popular_products.append(keyword_to_id['历史书'])
    
    # 如果没有找到足够的产品，随机选择一些
    if len(popular_products) < 3:
        available_ids = [p['product_id'] for p in products[:3]]
        for pid in available_ids:
            if pid not in popular_products:
                popular_products.append(pid)
            if len(popular_products) >= 3:
                break
    
    print("\n找到的关键词映射：")
    for keyword, product_id in keyword_to_id.items():
        # 找到对应的产品名称
        product_name = next((p['product_name'] for p in products if p['product_id'] == product_id), "未知")
        print(f"  {keyword} -> {product_id} ({product_name})")
    
    print(f"\n选择的热门产品ID: {popular_products}")
    
    # 读取API服务器文件
    api_server_file = "Ai_model/api_server.py"
    with open(api_server_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 更新热门产品ID
    old_line = 'popular_ids = ["ELEC001", "SPORTS001", "BOOK001"]'
    new_line = f'popular_ids = {popular_products}'
    
    if old_line in content:
        content = content.replace(old_line, new_line)
        print(f"\n✓ 更新热门产品ID: {old_line} -> {new_line}")
    else:
        print("⚠ 警告：未找到热门产品ID的旧代码行")
    
    # 保存修改后的文件
    with open(api_server_file, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("\n=== 修复完成 ===")
    print("AI推荐系统已更新为使用8位随机数产品ID")
    print("请重启AI服务器以应用更改：")
    print("  python3 Ai_model/api_server.py")

if __name__ == "__main__":
    main() 