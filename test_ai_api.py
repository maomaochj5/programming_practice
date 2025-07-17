#!/usr/bin/env python3
import requests
import json
import time

# API 端点
base_url = "http://127.0.0.1:5001"

print("=== 测试 AI 推荐系统 ===")

# 测试 1: 基于购物车推荐
print("\n1. 测试基于购物车的推荐功能")
cart_items = ["ELEC001", "ELEC002", "SPORTS001"]  # 耳机、充电宝、运动鞋
print(f"购物车商品: {cart_items}")

response = requests.post(f"{base_url}/recommend", json={"cart_items": cart_items})
if response.status_code == 200:
    result = response.json()
    print(f"AI 回复: {result['response']}")
    print(f"推荐商品ID: {result['products']}")
else:
    print(f"错误: {response.status_code}")

time.sleep(1)

# 测试 2: 用户主动提问
print("\n2. 测试 AI 导购功能")
queries = [
    "我想要一个运动耳机",
    "有什么适合露营的装备吗？",
    "推荐一些办公用品"
]

for query in queries:
    print(f"\n用户提问: {query}")
    response = requests.post(f"{base_url}/ask", json={"query": query})
    if response.status_code == 200:
        result = response.json()
        print(f"AI 回复: {result['response']}")
        print(f"推荐商品ID: {result['products']}")
    else:
        print(f"错误: {response.status_code}")
    time.sleep(1)

print("\n=== 测试完成 ===") 