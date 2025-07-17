#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AI导购助手HTTP API服务器
为Qt GUI应用提供AI推荐服务
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
import json
import logging
import threading
import time
from main import RAG_Recommender, setup_environment, build_and_save_index
import os

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

app = Flask(__name__)
CORS(app)  # 允许跨域请求

# 全局变量
recommender = None
init_lock = threading.Lock()

def initialize_recommender():
    """初始化AI推荐系统"""
    global recommender
    
    if recommender is not None:
        return
        
    with init_lock:
        if recommender is not None:
            return
            
        try:
            logger.info("开始初始化AI推荐系统...")
            setup_environment()
            
            script_dir = os.path.dirname(os.path.abspath(__file__))
            RETRIEVER_MODEL_NAME = os.path.join(script_dir, 'fine-tuned-retriever')
            INDEX_FILE = os.path.join(script_dir, 'products.index')
            
            if not os.path.exists(RETRIEVER_MODEL_NAME):
                logger.error(f"微调模型未找到: {RETRIEVER_MODEL_NAME}")
                raise FileNotFoundError(f"请先训练微调模型: {RETRIEVER_MODEL_NAME}")
                
            if not os.path.exists(INDEX_FILE):
                logger.info("索引文件不存在，开始构建...")
                build_and_save_index()
                
            recommender = RAG_Recommender()
            logger.info("AI推荐系统初始化完成")
            
        except Exception as e:
            logger.error(f"初始化AI推荐系统失败: {e}")
            raise

@app.route('/health', methods=['GET'])
def health_check():
    """健康检查接口"""
    return jsonify({
        "status": "healthy",
        "message": "AI导购助手服务正常运行",
        "initialized": recommender is not None
    })

@app.route('/api/recommend/cart', methods=['POST'])
def recommend_by_cart():
    """基于购物车内容进行推荐"""
    try:
        data = request.json
        cart_products = data.get('cart_products', [])
        
        logger.info(f"收到购物车推荐请求: {cart_products}")
        
        if not cart_products:
            return jsonify({
                "success": False,
                "message": "购物车为空",
                "recommendations": []
            })
        
        # 构建查询文本
        product_names = [item.get('name', '') for item in cart_products if item.get('name')]
        query_text = f"购买了{', '.join(product_names)}，还需要什么商品？"
        
        logger.info(f"构建的查询文本: {query_text}")
        
        # 调用AI推荐
        if recommender is None:
            initialize_recommender()
            
        ai_response = recommender.answer(query_text)
        
        # 解析推荐结果
        recommendations = parse_ai_response(ai_response)
        
        return jsonify({
            "success": True,
            "message": "推荐成功",
            "query": query_text,
            "ai_response": ai_response,
            "recommendations": recommendations
        })
        
    except Exception as e:
        logger.error(f"购物车推荐处理失败: {e}")
        return jsonify({
            "success": False,
            "message": f"推荐失败: {str(e)}",
            "recommendations": []
        }), 500

@app.route('/api/recommend/query', methods=['POST'])
def recommend_by_query():
    """基于用户查询进行推荐"""
    try:
        data = request.json
        user_query = data.get('query', '').strip()
        
        logger.info(f"收到查询推荐请求: {user_query}")
        
        if not user_query:
            return jsonify({
                "success": False,
                "message": "查询内容为空",
                "recommendations": []
            })
        
        # 调用AI推荐
        if recommender is None:
            initialize_recommender()
            
        ai_response = recommender.answer(user_query)
        
        # 解析推荐结果
        recommendations = parse_ai_response(ai_response)
        
        return jsonify({
            "success": True,
            "message": "推荐成功", 
            "query": user_query,
            "ai_response": ai_response,
            "recommendations": recommendations
        })
        
    except Exception as e:
        logger.error(f"查询推荐处理失败: {e}")
        return jsonify({
            "success": False,
            "message": f"推荐失败: {str(e)}",
            "recommendations": []
        }), 500

def parse_ai_response(ai_response):
    """解析AI响应，提取推荐的商品信息"""
    recommendations = []
    
    try:
        # 如果AI响应中包含商品信息，尝试解析
        if recommender and hasattr(recommender, 'products_db'):
            # 在响应中查找商品名称
            for product_id, product_info in recommender.products_db.items():
                product_name = product_info['product_name']
                
                # 如果商品名称出现在AI响应中，添加到推荐列表
                if product_name in ai_response:
                    recommendations.append({
                        "product_id": product_id,
                        "product_name": product_name,
                        "category": product_info['category'],
                        "price": product_info['price'],
                        "description": product_info['description']
                    })
        
        # 如果没有找到具体商品，尝试从最近的推荐中获取
        if not recommendations and hasattr(recommender, '_last_matched_products'):
            for product in recommender._last_matched_products:
                recommendations.append({
                    "product_id": product['product_id'],
                    "product_name": product['product_name'],
                    "category": product['category'],
                    "price": product['price'],
                    "description": product['description']
                })
        
        logger.info(f"解析出 {len(recommendations)} 个推荐商品")
        
    except Exception as e:
        logger.error(f"解析AI响应失败: {e}")
    
    return recommendations

@app.route('/api/products', methods=['GET'])
def get_all_products():
    """获取所有商品信息"""
    try:
        if recommender is None:
            initialize_recommender()
            
        products = []
        for product_id, product_info in recommender.products_db.items():
            products.append({
                "product_id": product_id,
                "product_name": product_info['product_name'],
                "category": product_info['category'],
                "price": product_info['price'],
                "description": product_info['description']
            })
            
        return jsonify({
            "success": True,
            "products": products,
            "count": len(products)
        })
        
    except Exception as e:
        logger.error(f"获取商品列表失败: {e}")
        return jsonify({
            "success": False,
            "message": f"获取失败: {str(e)}",
            "products": []
        }), 500

@app.errorhandler(404)
def not_found(error):
    return jsonify({
        "success": False,
        "message": "API接口不存在"
    }), 404

@app.errorhandler(500)
def internal_error(error):
    return jsonify({
        "success": False,
        "message": "服务器内部错误"
    }), 500

if __name__ == '__main__':
    logger.info("启动AI导购助手API服务器...")
    
    # 预热初始化（可选）
    try:
        initialize_recommender()
        logger.info("AI推荐系统预热完成")
    except Exception as e:
        logger.warning(f"预热失败，将在第一次请求时初始化: {e}")
    
    # 启动Flask服务器
    app.run(
        host='127.0.0.1',
        port=5001,
        debug=False,
        threaded=True
    ) 