import json
import faiss
import numpy as np
import torch
from sentence_transformers import SentenceTransformer, CrossEncoder
from transformers import AutoModelForCausalLM, AutoTokenizer
import os
import re
import jieba

# 解决Hugging Face Tokenizers的并行处理警告
os.environ["TOKENIZERS_PARALLELISM"] = "false"

# --- 全局变量和文件名 ---
script_dir = os.path.dirname(os.path.abspath(__file__))
PRODUCT_FILE = os.path.join(script_dir, "products.json")
INDEX_FILE = os.path.join(script_dir, "products.index")
ID_MAP_FILE = os.path.join(script_dir, "index_to_id.json")
RETRIEVER_MODEL_NAME = './fine-tuned-retriever'
LLM_MODEL_NAME = "microsoft/DialoGPT-medium"  # 使用更小的模型


def setup_environment():
    """检查必需文件是否存在。"""
    if not os.path.exists(PRODUCT_FILE):
        print(f"错误: 必需的商品文件 '{PRODUCT_FILE}' 不存在。")
        exit()
    print("确保已安装以下库: sentence-transformers, faiss-cpu, transformers, torch, accelerate")


def build_and_save_index():
    """【离线准备阶段】"""
    print("--- 开始离线准备阶段：建立索引 ---")
    retriever_model = SentenceTransformer(RETRIEVER_MODEL_NAME)
    with open(PRODUCT_FILE, 'r', encoding='utf-8') as f:
        products = json.load(f)
    product_texts = [f"{p['product_name']} {p['category']} {p['description']}" for p in products]
    product_ids = [p['product_id'] for p in products]
    product_embeddings = retriever_model.encode(product_texts, convert_to_tensor=True, show_progress_bar=True)
    index = faiss.IndexFlatL2(product_embeddings.shape[1])
    index.add(product_embeddings.cpu().numpy())
    faiss.write_index(index, INDEX_FILE)
    with open(ID_MAP_FILE, 'w', encoding='utf-8') as f:
        json.dump(product_ids, f)
    print("--- 离线准备阶段完成 ---")


class RAG_Recommender:
    """【线上问答阶段】的执行者 - 优化版"""

    def __init__(self):
        print("\n--- 系统初始化开始 ---")
        self.retriever = SentenceTransformer(RETRIEVER_MODEL_NAME)
        self.index = faiss.read_index(INDEX_FILE)
        with open(ID_MAP_FILE, 'r', encoding='utf-8') as f:
            self.index_to_id = json.load(f)
        with open(PRODUCT_FILE, 'r', encoding='utf-8') as f:
            self.products_db = {p['product_id']: p for p in json.load(f)}
        
        # 尝试初始化LLM模型，如果失败则使用检索模式
        try:
            self.tokenizer = AutoTokenizer.from_pretrained(LLM_MODEL_NAME)
            self.llm_model = AutoModelForCausalLM.from_pretrained(LLM_MODEL_NAME, torch_dtype="auto", device_map="auto")
            self.reranker = CrossEncoder('BAAI/bge-reranker-base')
            self.use_llm = True
            print("LLM模型加载成功，启用完整AI推荐功能")
        except Exception as e:
            print(f"LLM模型加载失败，使用检索模式: {e}")
            self.use_llm = False
            self.tokenizer = None
            self.llm_model = None
            self.reranker = None
        
        print("--- 系统初始化完成 ---")

    def answer(self, user_query):
        print(f"\n[用户]: {user_query}")
        intent = self.classify_query_intent(user_query)
        print(f"[系统]: 检测到查询意图: {intent}")
        if intent == "greeting":
            return self.handle_greeting()
        return self.handle_specific_product_query(user_query)

    def classify_query_intent(self, user_query):
        query_lower = user_query.lower()
        if any(keyword in query_lower for keyword in ['你好', '您好', 'hello', 'hi']):
            return "greeting"
        return "specific_product"

    def handle_greeting(self):
        return "您好！欢迎来到我们的商店！我可以帮您推荐商品。"

    def handle_no_match_found(self, user_query):
        return "抱歉，经过仔细筛选，我没有找到特别符合您需求的商品。或许您可以尝试更换一下关键词？"

    def _improved_vector_search(self, user_query):
        """改进的向量检索 - 多策略组合"""

        # 1. 查询扩展和预处理
        expanded_queries = self._expand_query(user_query)
        print(f"[向量检索]: 查询扩展: {expanded_queries}")

        all_candidates = {}  # 使用字典去重

        # 2. 多查询策略检索
        for query in expanded_queries:
            query_vector = self.retriever.encode(query)
            distances, indices = self.index.search(
                np.array([query_vector]).astype('float32'),
                min(20, len(self.index_to_id))  # 获取更多候选
            )

            for dist, idx in zip(distances[0], indices[0]):
                if idx < len(self.index_to_id):  # 确保索引有效
                    prod_id = self.index_to_id[idx]
                    prod_info = self.products_db.get(prod_id)
                    if prod_info:
                        # 使用最小距离（最高相似度）
                        if prod_id not in all_candidates or dist < all_candidates[prod_id][1]:
                            all_candidates[prod_id] = (prod_info, dist)

        # 3. 基于类别的智能过滤
        filtered_candidates = self._category_aware_filter(user_query, list(all_candidates.values()))

        # 4. 基于商品名称相似度的二次排序
        final_candidates = self._semantic_rerank(user_query, filtered_candidates)

        print(f"[向量检索]: 最终返回 {len(final_candidates)} 个高质量候选商品")
        for prod, dist in final_candidates[:5]:
            print(f"  - {prod['product_name']} (距离: {dist:.4f})")

        return final_candidates

    def _expand_query(self, user_query):
        """智能查询扩展"""
        query_lower = user_query.lower()
        expanded_queries = [user_query]  # 原始查询

        # 查询扩展规则
        expansion_rules = {
            # 电子产品类
            '手环': ['智能手环', '运动手环', '健康手环', '小米手环'],
            '耳机': ['蓝牙耳机', '运动耳机', '无线耳机'],
            '充电宝': ['移动电源', '充电器', '电源银行'],
            '键盘': ['机械键盘', '电脑键盘', '办公键盘'],
            '显示器': ['电脑显示器', '液晶显示器', '屏幕'],
            '平板': ['平板电脑', '儿童平板', 'iPad'],
            '路由器': ['无线路由器', 'WiFi路由器', '网络设备'],
            '支架': ['手机支架', '车载支架', '桌面支架'],

            # 运动户外类
            '鞋': ['运动鞋', '跑步鞋', '越野鞋', '户外鞋'],
            '帐篷': ['户外帐篷', '登山帐篷', '露营帐篷'],
            '保温瓶': ['保温杯', '户外水壶', '保温水壶'],
            '球拍': ['羽毛球拍', '网球拍', '运动器材'],
            '篮球': ['运动球', '体育用品'],
            '哑铃': ['健身器材', '力量训练', '运动器械'],
            '头盔': ['骑行头盔', '自行车头盔', '安全头盔'],
            '泳镜': ['游泳镜', '游泳用品', '水上运动'],

            # 书籍类
            '书': ['书籍', '图书', '读物'],
            '小说': ['文学作品', '小说书籍'],
            '历史': ['历史书籍', '史学著作'],
            '投资': ['投资理财', '金融书籍'],
            '科幻': ['科幻小说', '科幻书籍'],
            '食谱': ['烹饪书籍', '菜谱', '料理指南'],
            '指南': ['指导书籍', '参考书'],

            # 办公用品类
            '椅子': ['办公椅', '工作椅', '座椅'],
            '台灯': ['办公台灯', '护眼灯', '桌面灯'],
            '摄像头': ['网络摄像头', '会议摄像头'],
            '钢笔': ['办公笔', '书写用品'],
            '碎纸机': ['办公设备', '文件处理'],
            '标签机': ['标签打印机', '办公设备'],
            '支架': ['笔记本支架', '电脑支架', '桌面支架']
        }

        # 进行查询扩展
        for keyword, expansions in expansion_rules.items():
            if keyword in query_lower:
                expanded_queries.extend(expansions)
                print(f"[查询扩展]: 检测到关键词 '{keyword}', 扩展查询: {expansions}")
                break

        # 去重并限制数量
        expanded_queries = list(dict.fromkeys(expanded_queries))[:5]
        return expanded_queries

    def _category_aware_filter(self, user_query, candidates):
        """基于类别的智能过滤"""
        query_lower = user_query.lower()

        # 查询意图到类别的映射
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

        # 检测查询意图
        detected_intent = None
        for intent, config in intent_to_categories.items():
            if any(keyword in query_lower for keyword in config['keywords']):
                detected_intent = intent
                break

        if not detected_intent:
            print("[类别过滤]: 未检测到明确意图，保持所有候选商品")
            return candidates

        print(f"[类别过滤]: 检测到查询意图: {detected_intent}")
        target_categories = intent_to_categories[detected_intent]['target_categories']
        boost_score = intent_to_categories[detected_intent]['boost_score']

        # 过滤和调整评分
        filtered_candidates = []
        for product, distance in candidates:
            if product['category'] in target_categories:
                # 目标类别的商品，降低距离（提高相似度）
                adjusted_distance = distance - boost_score
                filtered_candidates.append((product, adjusted_distance))
                print(f"[类别过滤]: 保留并提升 '{product['product_name']}' (类别: {product['category']})")
            else:
                # 非目标类别的商品，如果距离很小（非常相似）也保留
                if distance < 0.3:  # 只保留非常相似的跨类别商品
                    filtered_candidates.append((product, distance))
                    print(f"[类别过滤]: 保留高相似度跨类别商品 '{product['product_name']}' (距离: {distance:.4f})")
                else:
                    print(
                        f"[类别过滤]: 过滤掉 '{product['product_name']}' (类别: {product['category']}, 距离: {distance:.4f})")

        return filtered_candidates

    def _semantic_rerank(self, user_query, candidates):
        """基于语义相似度的重排序"""
        if not candidates:
            return candidates

        query_lower = user_query.lower()
        scored_candidates = []

        for product, distance in candidates:
            # 计算语义匹配分数
            semantic_score = 0.0
            product_text = f"{product['product_name']} {product['description']}".lower()

            # 1. 精确关键词匹配
            query_words = [word for word in query_lower.split() if len(word) > 1]
            for word in query_words:
                if word in product_text:
                    semantic_score += 0.1

            # 2. 商品名称匹配
            if any(word in product['product_name'].lower() for word in query_words):
                semantic_score += 0.2

            # 3. 类别匹配奖励
            category_bonuses = {
                'electronics': ['电子产品'],
                'sports': ['运动户外'],
                'books': ['书籍'],
                'office': ['办公用品']
            }

            for intent_keywords, categories in category_bonuses.items():
                if any(keyword in query_lower for keyword in intent_keywords.split()) and product[
                    'category'] in categories:
                    semantic_score += 0.15

            # 4. 综合评分：距离越小越好，语义分数越高越好
            final_score = distance - semantic_score
            scored_candidates.append((product, final_score))

        # 按最终分数排序
        scored_candidates.sort(key=lambda x: x[1])

        return scored_candidates[:10]  # 返回前10个最佳候选

    # 在主类中替换原有的_vector_search方法
    def handle_specific_product_query(self, user_query):
        matched_products = self.improved_keyword_search(user_query)
        
        # 保存匹配的商品，供API服务器使用
        self._last_matched_products = matched_products
        
        if not matched_products:
            return self.handle_no_match_found(user_query)

        candidates = self._improved_vector_search(user_query)
        if not candidates:
            return self.handle_no_match_found(user_query)

        # 更新最后匹配的商品为最终候选商品
        self._last_matched_products = [item[0] for item in candidates[:5]]
        
        # 如果没有LLM模型，使用简单的推荐回复
        if not self.use_llm:
            return self._generate_simple_recommendation(user_query, candidates[:3])
        
        # 使用LLM生成推荐回复
        context_info = ""
        for i, (product, score) in enumerate(candidates[:3]):
            context_info += f"商品{i + 1}:\n- 名称: {product['product_name']}\n- 价格: ¥{product['price']}\n- 描述: {product['description']}\n\n"
        
        prompt = f"""你是一位专业的导购员。用户咨询: {user_query}

根据以下商品信息，为用户提供专业建议:
{context_info}

请提供:
1. 对用户需求的理解
2. 针对性的商品推荐及理由
3. 使用建议或注意事项

保持回答自然、专业、有帮助。"""

        messages = [{"role": "user", "content": prompt}]
        text = self.tokenizer.apply_chat_template(messages, tokenize=False, add_generation_prompt=True)
        model_inputs = self.tokenizer([text], return_tensors="pt").to(self.llm_model.device)

        with torch.no_grad():
            generated_ids = self.llm_model.generate(
                model_inputs.input_ids,
                max_new_tokens=256,
                temperature=0.7,
                do_sample=True,
                pad_token_id=self.tokenizer.eos_token_id
            )
        
        generated_ids = [output_ids[len(input_ids):] for input_ids, output_ids in zip(model_inputs.input_ids, generated_ids)]
        response = self.tokenizer.batch_decode(generated_ids, skip_special_tokens=True)[0]
        
        return response

    def _generate_simple_recommendation(self, user_query, candidates):
        """生成简单的推荐回复（不使用LLM）"""
        if not candidates:
            return self.handle_no_match_found(user_query)
        
        response = f"根据您的查询'{user_query}'，我为您推荐以下商品：\n\n"
        
        for i, (product, score) in enumerate(candidates):
            response += f"{i+1}. {product['product_name']} - ¥{product['price']}\n"
            response += f"   描述: {product['description']}\n"
            response += f"   推荐理由: 与您的需求高度匹配\n\n"
        
        response += "这些商品都是根据您的需求精心挑选的，希望能满足您的购物需求！"
        return response

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

        # 具体商品的精确匹配（添加更多关键词）
        specific_product_mappings = {
            # 露营相关商品
            'SPORTS003': ['帐篷', '登山帐篷', '户外帐篷', '露营帐篷', '野营帐篷'],
            'SPORTS004': ['保温瓶', '水壶', '保温杯', '户外水壶', '露营水壶'],
            'BOOK006': ['步道', '登山', '户外指南', '徒步指南'],

            # 游泳相关商品 - 新增
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

        # 2. 新增：基于description的全文匹配
        if not matched_products:
            print(f"[系统]: 开始基于description的全文匹配...")
            for product in self.products_db.values():
                # 检查商品名称、类别和描述是否包含查询关键词
                full_text = f"{product['product_name']} {product['category']} {product['description']}".lower()

                # 分词处理用户查询
                query_words = jieba.cut(query_lower)

                # 检查是否有匹配的关键词
                for word in query_words:
                    if len(word) >= 2 and word in full_text:  # 只匹配长度>=2的词
                        if product not in matched_products:
                            matched_products.append(product)
                            print(f"[系统]: 基于description匹配到商品: {product['product_name']}")
                            print(f"  - 匹配关键词: '{word}'")
                            print(f"  - 匹配位置: {product['description']}")
                        break

        # 3. 如果仍然没有匹配，进行类别匹配
        if not matched_products:
            matched_categories = []
            for mapping_name, mapping_info in category_mappings.items():
                if any(keyword in query_lower for keyword in mapping_info['keywords']):
                    matched_categories.append(mapping_info['category'])
                    print(f"[系统]: 匹配到类别关键词: {mapping_name} -> {mapping_info['category']}")

            # 去重
            matched_categories = list(set(matched_categories))
            print(f"[系统]: 匹配到类别: {matched_categories}")

            # 根据匹配的类别查找商品
            for product in self.products_db.values():
                if product['category'] in matched_categories:
                    matched_products.append(product)

        print(f"[系统]: 关键词搜索找到 {len(matched_products)} 个商品")
        return [(prod, 0.5) for prod in matched_products]

    def _rerank_with_cross_encoder(self, user_query, candidates):
        """改进的重排序逻辑 - 修复关键词匹配权重"""
        if not candidates:
            return []

        print("[系统]: 启动Cross-Encoder专业重排序器...")

        # 1. 计算Cross-Encoder评分
        sentence_pairs = [[user_query, f"{p['product_name']} {p['description']}"] for p, _ in candidates]
        cross_encoder_scores = self.reranker.predict(sentence_pairs)

        # 2. 改进的关键词匹配权重计算
        query_lower = user_query.lower()
        keyword_weights = []

        for product, _ in candidates:
            weight = 0.0
            product_text = f"{product['product_name']} {product['description']}".lower()

            # 分词处理用户查询
            query_words = [word for word in jieba.cut(query_lower) if len(word) >= 2]
            print(f"[调试] 分词结果: {query_words}")

            # 只有当用户查询词与商品相关时才给权重
            for query_word in query_words:
                # 检查商品名称中是否包含查询词
                if query_word in product['product_name'].lower():
                    weight += 0.3
                    print(f"[关键词匹配]: 商品名称匹配 '{query_word}' -> +0.3")

                # 检查商品描述中是否包含查询词
                elif query_word in product['description'].lower():
                    weight += 0.2
                    print(f"[关键词匹配]: 描述匹配 '{query_word}' -> +0.2")

            # 类别相关性检查（只有当查询暗示特定类别时）
            category_mapping = {
                '运动': ['运动户外'],
                '健身': ['运动户外'],
                '体育': ['运动户外'],
                '器材': ['运动户外'],
                '电子': ['电子产品'],
                '数码': ['电子产品'],
                '书': ['书籍'],
                '小说': ['书籍'],
                '办公': ['办公用品']
            }

            for query_word in query_words:
                if query_word in category_mapping:
                    target_categories = category_mapping[query_word]
                    if product['category'] in target_categories:
                        weight += 0.1
                        print(f"[类别匹配]: 查询词 '{query_word}' 匹配类别 '{product['category']}' -> +0.1")

            keyword_weights.append(weight)

        # 3. 组合评分：Cross-Encoder评分 + 关键词权重
        scored_candidates = []
        for i, (product, _) in enumerate(candidates):
            combined_score = cross_encoder_scores[i] + keyword_weights[i]
            scored_candidates.append({
                'product': product,
                'score': combined_score,
                'cross_encoder_score': cross_encoder_scores[i],
                'keyword_weight': keyword_weights[i]
            })

            print(f"[调试]: 商品 '{product['product_name']}'")
            print(f"  - Cross-Encoder评分: {cross_encoder_scores[i]:.4f}")
            print(f"  - 关键词权重: {keyword_weights[i]:.4f}")
            print(f"  - 组合评分: {combined_score:.4f}")

        # 4. 按组合评分排序
        scored_candidates.sort(key=lambda x: x['score'], reverse=True)

        # 5. 改进的过滤逻辑
        if not scored_candidates:
            return []

        highest_score = scored_candidates[0]['score']
        print(f"[系统决策]: 最高组合评分: {highest_score:.4f}")

        # 如果最高分的商品关键词权重为0，说明没有真正相关的商品
        if scored_candidates[0]['keyword_weight'] == 0 and highest_score < 0.005:
            print("[系统决策]: 没有找到真正相关的商品")
            return []

        # 语义相关性检查
        query_lower = user_query.lower()
        intent_category_mapping = {
            'sports': {
                'keywords': ['运动', '体育', '健身', '锻炼', '球拍', '篮球', '哑铃', '器材', '户外'],
                'relevant_categories': ['运动户外'],
                'irrelevant_categories': ['书籍', '办公用品', '电子产品']
            },
            'electronics': {
                'keywords': ['电子', '数码', '电脑', '手机', '平板', '耳机', '键盘'],
                'relevant_categories': ['电子产品'],
                'irrelevant_categories': ['书籍', '办公用品', '运动户外']
            },
            'books': {
                'keywords': ['书', '小说', '文学', '历史', '投资', '科幻'],
                'relevant_categories': ['书籍'],
                'irrelevant_categories': ['电子产品', '办公用品', '运动户外']
            },
            'office': {
                'keywords': ['办公', '工作', '椅子', '台灯', '笔'],
                'relevant_categories': ['办公用品'],
                'irrelevant_categories': ['书籍', '电子产品', '运动户外']
            }
        }

        # 检测意图
        detected_intent = None
        matched_config = None
        for intent, config in intent_category_mapping.items():
            if any(keyword in query_lower for keyword in config['keywords']):
                detected_intent = intent
                matched_config = config
                break

        # 语义过滤
        if detected_intent and matched_config:
            print(f"[语义检查]: 检测到查询意图: {detected_intent}")

            semantically_valid_candidates = []
            for item in scored_candidates:
                product_category = item['product']['category']

                # 如果有关键词权重，或者类别匹配，才通过
                if (product_category in matched_config['relevant_categories'] or
                        item['keyword_weight'] > 0.1):
                    semantically_valid_candidates.append(item)
                    print(f"[语义检查]: 商品 '{item['product']['product_name']}' 通过语义检查")
                else:
                    print(f"[语义检查]: 商品 '{item['product']['product_name']}' 被过滤")

            scored_candidates = semantically_valid_candidates

        # 6. 最终过滤
        valid_candidates = []
        for item in scored_candidates:
            current_score = item['score']

            # 如果有关键词权重，使用更宽松的阈值
            if item['keyword_weight'] > 0.1:
                threshold = 0.3
            else:
                threshold = 0.7

            if highest_score > 0:
                score_ratio = current_score / highest_score
            else:
                score_ratio = 1.0 if current_score == highest_score else 0.0

            if score_ratio >= threshold:
                valid_candidates.append(item)
                print(f"[系统决策]: 商品 '{item['product']['product_name']}' 通过筛选")
                print(f"  - 组合评分: {current_score:.4f}, 相对比值: {score_ratio:.2f}")
            else:
                print(f"[系统决策]: 商品 '{item['product']['product_name']}' 被过滤")

        print(f"[系统决策]: 最终筛选的商品数量: {len(valid_candidates)}")

        return [(item['product'], -item['score']) for item in valid_candidates[:3]]

if __name__ == '__main__':
    setup_environment()
    if not os.path.exists(RETRIEVER_MODEL_NAME):
        print(f"警告: 未找到微调后的检索器模型 '{RETRIEVER_MODEL_NAME}'。请先运行 'train_retriever.py'。")
        exit()
    if not os.path.exists(INDEX_FILE):
        build_and_save_index()
    recommender = RAG_Recommender()
    print("\n===================================")
    print("你好！我是您的AI导购 (优化版)，请问有什么可以帮您的？")
    print("===================================")
    while True:
        user_input = input("\n你: ")
        if user_input.lower() == '退出':
            print("AI导购: 很高兴为您服务，再见！")
            break
        try:
            ai_response = recommender.answer(user_input)
            print(f"\nAI导购: {ai_response}")
        except Exception as e:
            print(f"AI导购: 抱歉，处理您的请求时出现了问题: {e}")