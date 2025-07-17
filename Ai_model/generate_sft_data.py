import json
import random
import itertools

# --- 配置 ---
PRODUCT_FILE = 'products.json'  # 确保这个文件和脚本在同一目录下
RETRIEVER_DATA_FILE = 'sft_retriever_data.jsonl'
LLM_DATA_FILE = 'sft_llm_data.jsonl'
NUM_RETRIEVER_SAMPLES = 500  # 希望生成的检索器训练数据量
NUM_LLM_SAMPLES = 500  # 希望生成的LLM训练数据量

# --- 加载商品数据 ---
try:
    with open(PRODUCT_FILE, 'r', encoding='utf-8') as f:
        products = json.load(f)
    print(f"成功加载 {len(products)} 个商品。")
except FileNotFoundError:
    print(f"错误: 未找到商品文件 '{PRODUCT_FILE}'。请确保文件存在于正确的位置。")
    exit()

# 将商品按类别分组，方便后续生成负样本
products_by_category = {}
for p in products:
    category = p['category']
    if category not in products_by_category:
        products_by_category[category] = []
    products_by_category[category].append(p)


# --- 1. 为检索器 (Retriever) 生成数据 ---
# 目标：生成三元组 (anchor, positive, negative) 数据
# anchor: 模拟的用户查询
# positive: 与查询高度相关的商品
# negative: 与查询不相关的商品 (最好是“困难负样本”)

def generate_retriever_data():
    print("\n--- 开始生成检索器 (Retriever) SFT 数据 ---")
    retriever_data = []

    # 定义查询模板，增加多样性
    query_templates = [
        "我想买一个{}",
        "有什么好的{}推荐吗？",
        "给我找一下{}",
        "最近在看{}，有什么建议？",
        "有没有适合的{}",
        "寻找一款{}"
    ]

    for i in range(NUM_RETRIEVER_SAMPLES):
        # 1. 随机选择一个商品作为“正样本” (positive)
        positive_product = random.choice(products)
        positive_category = positive_product['category']

        # 2. 根据正样本信息，生成一个自然的用户查询 (anchor)
        # 优先使用商品名中的核心词
        product_name_parts = positive_product['product_name'].split()
        anchor_keyword = random.choice(product_name_parts) if product_name_parts else positive_product['product_name']
        anchor_query = random.choice(query_templates).format(anchor_keyword)

        # 3. 生成一个“困难负样本” (hard negative)
        # 优先从同一类别中选择一个不同的商品，这能让模型学会区分细微差别
        negative_product = None
        if len(products_by_category[positive_category]) > 1:
            negative_product = random.choice(products_by_category[positive_category])
            while negative_product['product_id'] == positive_product['product_id']:
                negative_product = random.choice(products_by_category[positive_category])
        else:
            # 如果该类别只有一个商品，从其他类别随机选一个
            other_categories = list(products_by_category.keys())
            other_categories.remove(positive_category)
            if other_categories:
                negative_category = random.choice(other_categories)
                negative_product = random.choice(products_by_category[negative_category])

        if negative_product:
            # 将正负样本的描述组合起来，用于训练
            positive_text = f"{positive_product['product_name']} {positive_product['description']}"
            negative_text = f"{negative_product['product_name']} {negative_product['description']}"

            retriever_data.append({
                "query": anchor_query,
                "positive": positive_text,
                "negative": negative_text
            })

    # 保存为 JSON Lines (.jsonl) 格式，这是SFT常用的格式
    with open(RETRIEVER_DATA_FILE, 'w', encoding='utf-8') as f:
        for entry in retriever_data:
            f.write(json.dumps(entry, ensure_ascii=False) + '\n')

    print(f"成功生成 {len(retriever_data)} 条检索器数据，已保存到 '{RETRIEVER_DATA_FILE}'")


# --- 2. 为大语言模型 (LLM) 生成数据 ---
# 目标：生成指令（Instruction）格式的数据，包含Reranker和Generator两种任务

def generate_llm_data():
    print("\n--- 开始生成大语言模型 (LLM) SFT 数据 ---")
    llm_data = []

    for i in range(NUM_LLM_SAMPLES):
        # 随机决定本次是生成Reranker任务还是Generator任务
        task_type = random.choice(["reranker", "generator"])

        # --- Reranker 任务数据生成 ---
        if task_type == "reranker" and len(products) >= 3:
            # 1. 随机选一个查询主题
            query_product = random.choice(products)
            query = f"和'{query_product['product_name']}'类似的商品"

            # 2. 构建候选列表，包含相关和不相关商品
            candidates = [query_product]
            # 从同类中找一个相关商品
            if len(products_by_category[query_product['category']]) > 1:
                related_product = random.choice(products_by_category[query_product['category']])
                if related_product not in candidates: candidates.append(related_product)
            # 从其他类别找一个不相关商品
            other_categories = list(products_by_category.keys())
            other_categories.remove(query_product['category'])
            if other_categories:
                unrelated_product = random.choice(products_by_category[random.choice(other_categories)])
                if unrelated_product not in candidates: candidates.append(unrelated_product)

            random.shuffle(candidates)

            # 3. 构建指令和输入
            instruction = f"你是一个商品相关性排序助手。根据查询“{query}”，为以下商品按相关性从高到低排序，并以JSON格式返回商品ID列表。"
            input_text = "\n".join(
                [f"ID:{p['product_id']}, 名称:{p['product_name']}, 描述:{p['description']}" for p in candidates])

            # 4. 构建理想的输出 (排序后的ID列表)
            # 这里用一个简化逻辑：同类商品>其他商品
            candidates.sort(key=lambda p: p['category'] == query_product['category'], reverse=True)
            output_text = json.dumps([p['product_id'] for p in candidates])

            llm_data.append({"instruction": instruction, "input": input_text, "output": output_text})

        # --- Generator 任务数据生成 ---
        else:
            # 1. 随机选择1-2个高度相关的商品作为上下文
            context_product = random.choice(products)
            context = [context_product]
            if len(products_by_category[context_product['category']]) > 1:
                related_product = random.choice(products_by_category[context_product['category']])
                if related_product not in context: context.append(related_product)

            # 2. 构建指令和输入
            query = f"我想买{context_product['category']}类的东西，比如'{context_product['product_name']}'"
            instruction = "你是一个专业的电商导购。请根据以下提供的商品信息，为用户的查询生成一段自然、有吸引力的推荐语。"
            input_text = "\n".join(
                [f"- {p['product_name']}: {p['description']} (价格: ${p['price']})" for p in context])

            # 3. 构建理想的输出 (一段专业的推荐语)
            if len(context) > 1:
                output_text = f"您好！关于{context_product['category']}类的产品，我们这里有几款非常不错的选择。首先是**{context[0]['product_name']}**，它是一款{context[0]['description']}。此外，我们还推荐**{context[1]['product_name']}**，特点是{context[1]['description']}。这两款都非常受欢迎，您可以根据您的具体需求来选择！"
            else:
                output_text = f"当然！为您推荐**{context[0]['product_name']}**。这是一款非常优秀的{context[0]['category']}产品，{context[0]['description']}。它的价格是${context[0]['price']}，性价比很高，绝对值得考虑！"

            llm_data.append({"instruction": instruction, "input": input_text, "output": output_text})

    # 保存为 JSON Lines (.jsonl) 格式
    with open(LLM_DATA_FILE, 'w', encoding='utf-8') as f:
        for entry in llm_data:
            f.write(json.dumps(entry, ensure_ascii=False) + '\n')

    print(f"成功生成 {len(llm_data)} 条LLM数据，已保存到 '{LLM_DATA_FILE}'")


# --- 主函数 ---
if __name__ == '__main__':
    generate_retriever_data()
    generate_llm_data()
    print("\n所有SFT数据生成完毕！")