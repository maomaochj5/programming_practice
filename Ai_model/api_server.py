import json
import faiss
import numpy as np
import torch
from sentence_transformers import SentenceTransformer, CrossEncoder
from transformers import AutoModelForCausalLM, AutoTokenizer
import os
import re
import jieba
from flask import Flask, request, jsonify

# Environment setup
os.environ["TOKENIZERS_PARALLELISM"] = "false"
script_dir = os.path.dirname(os.path.abspath(__file__))

# --- File Paths ---
PRODUCT_FILE = os.path.join(script_dir, "products.json")
INDEX_FILE = os.path.join(script_dir, "products.index")
ID_MAP_FILE = os.path.join(script_dir, "index_to_id.json")
RETRIEVER_MODEL_NAME = os.path.join(script_dir, 'fine-tuned-retriever')
LLM_MODEL_NAME = "Qwen/Qwen1.5-1.8B-Chat"

class RAG_Recommender:
    """RAG-based Recommendation System - 完整版本，算法逻辑与main.py一致"""
    def __init__(self):
        print("\n--- Initializing System ---")
        self.retriever = SentenceTransformer(RETRIEVER_MODEL_NAME)
        self.index = faiss.read_index(INDEX_FILE)
        with open(ID_MAP_FILE, 'r', encoding='utf-8') as f:
            self.index_to_id = json.load(f)
        with open(PRODUCT_FILE, 'r', encoding='utf-8') as f:
            self.products_db = {p['product_id']: p for p in json.load(f)}
        self.tokenizer = AutoTokenizer.from_pretrained(LLM_MODEL_NAME)
        self.llm_model = AutoModelForCausalLM.from_pretrained(LLM_MODEL_NAME, torch_dtype="auto", device_map="auto")
        self.reranker = CrossEncoder('BAAI/bge-reranker-base')
        print("--- System Initialization Complete ---")

    def search(self, user_query):
        print(f"\n[User Query]: {user_query}")
        
        # 意图分类
        intent = self.classify_query_intent(user_query)
        print(f"[系统]: 检测到查询意图: {intent}")
        
        if intent == "greeting":
            return {"response": self.handle_greeting(), "products": []}

        # 处理具体商品查询
        response_text, recommended_products = self.handle_specific_product_query(user_query)
        
        return {
            "response": response_text,
            "products": [p['product_id'] for p in recommended_products]
        }
    
    def recommend(self, cart_items):
        """Recommend products based on items in the cart."""
        if not cart_items:
            return {"response": "购物车是空的，为您推荐一些热门商品吧！", "products": self._get_popular_products()}

        print(f"\n[Cart Recommendation]: {cart_items}")
        
        # 创建基于购物车商品的查询
        cart_descriptions = []
        for item_id in cart_items:
            if item_id in self.products_db:
                prod = self.products_db[item_id]
                cart_descriptions.append(f"{prod['product_name']} {prod['description']}")
        
        if not cart_descriptions:
            return {"response": "无法识别购物车中的商品。", "products": []}

        combined_query = " ".join(cart_descriptions)
        
        # 使用完整的推荐算法
        response_text, recommended_products = self.handle_specific_product_query(combined_query)
        
        # 过滤掉已在购物车中的商品
        filtered_products = [p for p in recommended_products if p['product_id'] not in cart_items]
        
        if not filtered_products:
            return {"response": "暂时没有找到合适的推荐商品。", "products": []}
            
        final_products = filtered_products[:3]
        final_response = self._generate_llm_response("基于我购物车里的商品进行推荐", final_products)

        return {
            "response": final_response,
            "products": [p['product_id'] for p in final_products]
        }

    def classify_query_intent(self, user_query):
        query_lower = user_query.lower()
        if any(keyword in query_lower for keyword in ['你好', '您好', 'hello', 'hi']):
            return "greeting"
        return "specific_product"

    def handle_greeting(self):
        return "您好！欢迎来到我们的商店！我可以帮您推荐商品。"

    def handle_no_match_found(self, user_query):
        return "抱歉，经过仔细筛选，我没有找到特别符合您需求的商品。或许您可以尝试更换一下关键词？"

    def handle_specific_product_query(self, user_query):
        """处理具体商品查询 - 使用改进的向量检索"""

        # 1. 使用改进的关键词搜索
        keyword_results = self.improved_keyword_search(user_query)
        initial_candidates = keyword_results

        # 2. 如果关键词搜索结果不足，使用改进的向量搜索
        if len(initial_candidates) < 3:
            print("[系统]: 关键词结果不足，启动改进的向量搜索。")
            vector_results = self._improved_vector_search(user_query)

            # 合并结果，避免重复
            combined_results = {prod['product_id']: (prod, dist) for prod, dist in initial_candidates}
            for product, dist in vector_results:
                if product['product_id'] not in combined_results:
                    combined_results[product['product_id']] = (product, dist)

            initial_candidates = list(combined_results.values())
            initial_candidates.sort(key=lambda x: x[1])

        # 3. 使用改进的重排序
        final_candidates = self._rerank_with_cross_encoder(user_query, initial_candidates[:10])

        # 4. 生成回答
        if not final_candidates:
            return self.handle_no_match_found(user_query), []

        top_products = [prod for prod, score in final_candidates[:3]]

        print(f"[系统]: 最终推荐商品:")
        for product in top_products:
            print(f"  - {product['product_name']}")

        response_text = self._generate_llm_response(user_query, top_products)
        return response_text, top_products

    def improved_keyword_search(self, user_query):
        """改进的关键词搜索系统 - 包含description匹配"""
        query_lower = user_query.lower()
        matched_products = []

        # 改进的商品类别和关键词映射
        category_mappings = {
            # 运动户外类
            'sports': {
                'keywords': ['运动', '户外', '体育', '健身', '锻炼'],
                'category': '运动户外'
            },
            'camping': {
                'keywords': ['露营', '野营', '户外装备', '帐篷', '登山', '徒步'],
                'category': '运动户外'
            },
            'shoes': {
                'keywords': ['鞋子', '鞋', '跑鞋', '越野鞋', '运动鞋'],
                'category': '运动户外'
            },
            'sports_equipment': {
                'keywords': ['球拍', '羽毛球拍', '篮球', '哑铃', '泳镜', '头盔', '游泳', '游泳用品'],
                'category': '运动户外'
            },
            'outdoor_gear': {
                'keywords': ['保温瓶', '水壶', '户外'],
                'category': '运动户外'
            },

            # 电子产品类
            'electronics': {
                'keywords': ['电子', '数码', '科技'],
                'category': '电子产品'
            },
            'audio': {
                'keywords': ['耳机', '耳麦', '蓝牙', '音响'],
                'category': '电子产品'
            },
            'computer': {
                'keywords': ['电脑', '键盘', '显示器', '路由器', '平板', '支架'],
                'category': '电子产品'
            },
            'mobile': {
                'keywords': ['手机', '充电宝', '移动电源', '手环', '车载'],
                'category': '电子产品'
            },

            # 书籍类
            'books': {
                'keywords': ['书', '书籍', '小说', '历史', '投资', '科幻', '旅游'],
                'category': '书籍'
            },
            'outdoor_books': {
                'keywords': ['步道', '登山指南', '户外指南'],
                'category': '书籍'
            },
            'novels': {
                'keywords': ['小说', '文学', '村上春树', '卡夫卡', '科幻'],
                'category': '书籍'
            },
            'guides': {
                'keywords': ['指南', '攻略', '食谱', '菜谱'],
                'category': '书籍'
            },

            # 办公用品类
            'office': {
                'keywords': ['办公', '工作', '文具'],
                'category': '办公用品'
            },
            'office_equipment': {
                'keywords': ['摄像头', '钢笔', '碎纸机', '椅子', '台灯', '标签机'],
                'category': '办公用品'
            }
        }

        # 具体商品的精确匹配
        specific_product_mappings = {
            # 露营相关商品
            'SPORTS003': ['帐篷', '登山帐篷', '户外帐篷', '露营帐篷', '野营帐篷'],
            'SPORTS004': ['保温瓶', '水壶', '保温杯', '户外水壶', '露营水壶'],
            'BOOK006': ['步道', '登山', '户外指南', '徒步指南'],

            # 游泳相关商品
            'SPORTS006': ['泳镜', '游泳镜', '游泳用品', '游泳装备', '专业泳镜'],

            # 其他商品
            'SPORTS001': ['鞋子', '鞋', '跑鞋', '越野鞋', '运动鞋'],
            'SPORTS002': ['头盔', '自行车头盔', '骑行头盔'],
            'SPORTS005': ['羽毛球拍', '球拍'],
            'SPORTS007': ['篮球'],
            'SPORTS008': ['哑铃', '健身器材'],
            'ELEC001': ['耳机', '蓝牙耳机', '运动耳机'],
            'ELEC002': ['充电宝', '移动电源'],
            'ELEC003': ['键盘', '机械键盘'],
            'ELEC004': ['显示器', '屏幕'],
            'ELEC005': ['平板', '儿童平板'],
            'ELEC006': ['手环', '智能手环'],
            'ELEC007': ['路由器', 'wifi'],
            'ELEC008': ['车载支架', '手机支架'],
            'BOOK001': ['历史书', '罗马', '历史'],
            'BOOK002': ['食谱', '菜谱', '日本菜', '料理'],
            'BOOK003': ['小说', '村上春树', '挪威的森林'],
            'BOOK004': ['投资', '股票', '理财'],
            'BOOK005': ['科幻', '沙丘'],
            'BOOK007': ['文学', '卡夫卡', '变形记'],
            'OFFICE001': ['摄像头', '会议摄像头'],
            'OFFICE002': ['钢笔', '笔'],
            'OFFICE003': ['碎纸机'],
            'OFFICE004': ['椅子', '办公椅'],
            'OFFICE005': ['标签机'],
            'OFFICE006': ['笔记本支架', '电脑支架'],
            'OFFICE007': ['台灯', '护眼灯']
        }

        # 1. 先进行精确商品匹配
        for product_id, keywords in specific_product_mappings.items():
            if any(keyword in query_lower for keyword in keywords):
                product = self.products_db.get(product_id)
                if product and product not in matched_products:
                    matched_products.append(product)
                    print(f"[系统]: 精确匹配到商品: {product['product_name']}")

        # 2. 基于description的全文匹配
        if not matched_products:
            print(f"[系统]: 开始基于description的全文匹配...")
            for product in self.products_db.values():
                full_text = f"{product['product_name']} {product['category']} {product['description']}".lower()
                query_words = jieba.cut(query_lower)

                for word in query_words:
                    if len(word) >= 2 and word in full_text:
                        if product not in matched_products:
                            matched_products.append(product)
                            print(f"[系统]: 基于description匹配到商品: {product['product_name']}")
                            print(f"  - 匹配关键词: '{word}'")
                        break

        # 3. 类别匹配
        if not matched_products:
            matched_categories = []
            for mapping_name, mapping_info in category_mappings.items():
                if any(keyword in query_lower for keyword in mapping_info['keywords']):
                    matched_categories.append(mapping_info['category'])
                    print(f"[系统]: 匹配到类别关键词: {mapping_name} -> {mapping_info['category']}")

            matched_categories = list(set(matched_categories))
            print(f"[系统]: 匹配到类别: {matched_categories}")

            for product in self.products_db.values():
                if product['category'] in matched_categories:
                    matched_products.append(product)

        print(f"[系统]: 关键词搜索找到 {len(matched_products)} 个商品")
        return [(prod, 0.5) for prod in matched_products]

    def _improved_vector_search(self, user_query):
        """改进的向量检索 - 多策略组合"""
        expanded_queries = self._expand_query(user_query)
        print(f"[向量检索]: 查询扩展: {expanded_queries}")

        all_candidates = {}

        for query in expanded_queries:
            query_vector = self.retriever.encode(query)
            distances, indices = self.index.search(
                np.array([query_vector]).astype('float32'),
                min(20, len(self.index_to_id))
            )

            for dist, idx in zip(distances[0], indices[0]):
                if idx < len(self.index_to_id):
                    prod_id = self.index_to_id[idx]
                    prod_info = self.products_db.get(prod_id)
                    if prod_info:
                        if prod_id not in all_candidates or dist < all_candidates[prod_id][1]:
                            all_candidates[prod_id] = (prod_info, dist)

        filtered_candidates = self._category_aware_filter(user_query, list(all_candidates.values()))
        final_candidates = self._semantic_rerank(user_query, filtered_candidates)

        print(f"[向量检索]: 最终返回 {len(final_candidates)} 个高质量候选商品")
        return final_candidates

    def _expand_query(self, user_query):
        """智能查询扩展"""
        query_lower = user_query.lower()
        expanded_queries = [user_query]

        expansion_rules = {
            '手环': ['智能手环', '运动手环', '健康手环', '小米手环'],
            '耳机': ['蓝牙耳机', '运动耳机', '无线耳机'],
            '充电宝': ['移动电源', '充电器', '电源银行'],
            '键盘': ['机械键盘', '电脑键盘', '办公键盘'],
            '显示器': ['电脑显示器', '液晶显示器', '屏幕'],
            '平板': ['平板电脑', '儿童平板', 'iPad'],
            '路由器': ['无线路由器', 'WiFi路由器', '网络设备'],
            '支架': ['手机支架', '车载支架', '桌面支架'],
            '鞋': ['运动鞋', '跑步鞋', '越野鞋', '户外鞋'],
            '帐篷': ['户外帐篷', '登山帐篷', '露营帐篷'],
            '保温瓶': ['保温杯', '户外水壶', '保温水壶'],
            '球拍': ['羽毛球拍', '网球拍', '运动器材'],
            '篮球': ['运动球', '体育用品'],
            '哑铃': ['健身器材', '力量训练', '运动器械'],
            '头盔': ['骑行头盔', '自行车头盔', '安全头盔'],
            '泳镜': ['游泳镜', '游泳用品', '水上运动'],
            '书': ['书籍', '图书', '读物'],
            '小说': ['文学作品', '小说书籍'],
            '历史': ['历史书籍', '史学著作'],
            '投资': ['投资理财', '金融书籍'],
            '科幻': ['科幻小说', '科幻书籍'],
            '食谱': ['烹饪书籍', '菜谱', '料理指南'],
            '指南': ['指导书籍', '参考书'],
            '椅子': ['办公椅', '工作椅', '座椅'],
            '台灯': ['办公台灯', '护眼灯', '桌面灯'],
            '摄像头': ['网络摄像头', '会议摄像头'],
            '钢笔': ['办公笔', '书写用品'],
            '碎纸机': ['办公设备', '文件处理'],
            '标签机': ['标签打印机', '办公设备']
        }

        for keyword, expansions in expansion_rules.items():
            if keyword in query_lower:
                expanded_queries.extend(expansions)
                print(f"[查询扩展]: 检测到关键词 '{keyword}', 扩展查询: {expansions}")
                break

        return list(dict.fromkeys(expanded_queries))[:5]

    def _category_aware_filter(self, user_query, candidates):
        """基于类别的智能过滤"""
        query_lower = user_query.lower()

        intent_to_categories = {
            'electronics': {
                'keywords': ['手环', '耳机', '充电宝', '键盘', '显示器', '平板', '路由器', '支架', '电子', '数码'],
                'target_categories': ['电子产品'],
                'boost_score': 0.2
            },
            'sports': {
                'keywords': ['鞋', '帐篷', '保温瓶', '球拍', '篮球', '哑铃', '头盔', '泳镜', '运动', '健身', '户外'],
                'target_categories': ['运动户外'],
                'boost_score': 0.2
            },
            'books': {
                'keywords': ['书', '小说', '历史', '投资', '科幻', '食谱', '指南', '书籍'],
                'target_categories': ['书籍'],
                'boost_score': 0.2
            },
            'office': {
                'keywords': ['椅子', '台灯', '摄像头', '钢笔', '碎纸机', '标签机', '办公'],
                'target_categories': ['办公用品'],
                'boost_score': 0.2
            }
        }

        detected_intent = None
        for intent, config in intent_to_categories.items():
            if any(keyword in query_lower for keyword in config['keywords']):
                detected_intent = intent
                break

        if not detected_intent:
            return candidates

        target_categories = intent_to_categories[detected_intent]['target_categories']
        boost_score = intent_to_categories[detected_intent]['boost_score']

        filtered_candidates = []
        for product, distance in candidates:
            if product['category'] in target_categories:
                adjusted_distance = distance - boost_score
                filtered_candidates.append((product, adjusted_distance))
            else:
                if distance < 0.3:
                    filtered_candidates.append((product, distance))

        return filtered_candidates

    def _semantic_rerank(self, user_query, candidates):
        """基于语义相似度的重排序"""
        if not candidates:
            return candidates

        query_lower = user_query.lower()
        scored_candidates = []

        for product, distance in candidates:
            semantic_score = 0.0
            product_text = f"{product['product_name']} {product['description']}".lower()

            query_words = [word for word in query_lower.split() if len(word) > 1]
            for word in query_words:
                if word in product_text:
                    semantic_score += 0.1

            if any(word in product['product_name'].lower() for word in query_words):
                semantic_score += 0.2

            final_score = distance - semantic_score
            scored_candidates.append((product, final_score))

        scored_candidates.sort(key=lambda x: x[1])
        return scored_candidates[:10]

    def _rerank_with_cross_encoder(self, user_query, candidates):
        """改进的重排序逻辑"""
        if not candidates:
            return []

        print("[系统]: 启动Cross-Encoder专业重排序器...")

        sentence_pairs = [[user_query, f"{p['product_name']} {p['description']}"] for p, _ in candidates]
        cross_encoder_scores = self.reranker.predict(sentence_pairs)

        query_lower = user_query.lower()
        keyword_weights = []

        for product, _ in candidates:
            weight = 0.0
            query_words = [word for word in jieba.cut(query_lower) if len(word) >= 2]

            for query_word in query_words:
                if query_word in product['product_name'].lower():
                    weight += 0.3
                elif query_word in product['description'].lower():
                    weight += 0.2

            category_mapping = {
                '运动': ['运动户外'], '健身': ['运动户外'], '体育': ['运动户外'], '器材': ['运动户外'],
                '电子': ['电子产品'], '数码': ['电子产品'],
                '书': ['书籍'], '小说': ['书籍'],
                '办公': ['办公用品']
            }

            for query_word in query_words:
                if query_word in category_mapping:
                    target_categories = category_mapping[query_word]
                    if product['category'] in target_categories:
                        weight += 0.1

            keyword_weights.append(weight)

        scored_candidates = []
        for i, (product, _) in enumerate(candidates):
            combined_score = cross_encoder_scores[i] + keyword_weights[i]
            scored_candidates.append({
                'product': product,
                'score': combined_score,
                'cross_encoder_score': cross_encoder_scores[i],
                'keyword_weight': keyword_weights[i]
            })

        scored_candidates.sort(key=lambda x: x['score'], reverse=True)

        if not scored_candidates:
            return []

        highest_score = scored_candidates[0]['score']
        if scored_candidates[0]['keyword_weight'] == 0 and highest_score < 0.005:
            return []

        valid_candidates = []
        for item in scored_candidates:
            threshold = 0.3 if item['keyword_weight'] > 0.1 else 0.7
            score_ratio = item['score'] / highest_score if highest_score > 0 else 1.0
            
            if score_ratio >= threshold:
                valid_candidates.append(item)

        return [(item['product'], -item['score']) for item in valid_candidates[:3]]

    def _get_popular_products(self, count=3):
        """返回热门商品ID列表"""
        popular_ids = ["ELEC001", "SPORTS001", "BOOK001"]
        return popular_ids[:count]

    def _generate_llm_response(self, query, products):
        """使用LLM生成回复"""
        if not products:
            return "抱歉，没有找到相关商品。"
        
        context = ""
        for i, p in enumerate(products):
            context += f"{i+1}. {p['product_name']} (¥{p['price']}) - {p['description'][:50]}...\n"
        
        prompt = f"简洁推荐以下商品给用户，针对查询'{query}'，用1-2句话说明推荐理由:\n\n{context}"
        messages = [{"role": "user", "content": prompt}]
        text = self.tokenizer.apply_chat_template(messages, tokenize=False, add_generation_prompt=True)
        model_inputs = self.tokenizer([text], return_tensors="pt").to(self.llm_model.device)
        
        generated_ids = self.llm_model.generate(
            model_inputs.input_ids,
            max_new_tokens=80,
            pad_token_id=self.tokenizer.eos_token_id
        )
        response = self.tokenizer.decode(generated_ids[0][model_inputs.input_ids.shape[1]:], skip_special_tokens=True)
        return response.strip()

app = Flask(__name__)

# Initialize the recommender system globally
recommender = RAG_Recommender()

@app.route('/ask', methods=['POST'])
def ask():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON data provided"}), 400
    
    query = data.get('query')
    if not query:
        return jsonify({"error": "Query not provided"}), 400
    
    result = recommender.search(query)
    return jsonify(result)

@app.route('/recommend', methods=['POST'])
def recommend():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON data provided"}), 400
        
    cart_items = data.get('cart_items')
    if not isinstance(cart_items, list):
        return jsonify({"error": "cart_items must be a list"}), 400

    result = recommender.recommend(cart_items)
    return jsonify(result)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001) 