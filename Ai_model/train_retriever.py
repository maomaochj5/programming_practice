import json
from sentence_transformers import SentenceTransformer, InputExample, losses
from torch.utils.data import DataLoader

# --- 配置 ---
BASE_MODEL_NAME = 'all-MiniLM-L6-v2'
TRAIN_DATA_FILE = 'sft_retriever_data.jsonl'
OUTPUT_MODEL_PATH = './fine-tuned-retriever' # 训练好的模型将保存在这里
EPOCHS = 2 # 训练轮次
BATCH_SIZE = 16 # 批处理大小

print("--- 开始微调检索器模型 ---")

# 1. 加载预训练的基础模型
print(f"加载基础模型: {BASE_MODEL_NAME}")
model = SentenceTransformer(BASE_MODEL_NAME)

# 2. 读取我们生成的SFT数据
print(f"读取训练数据: {TRAIN_DATA_FILE}")
train_examples = []
with open(TRAIN_DATA_FILE, 'r', encoding='utf-8') as f:
    for line in f:
        data = json.loads(line)
        # 将数据转换成模型需要的 InputExample 格式
        example = InputExample(texts=[data['query'], data['positive'], data['negative']])
        train_examples.append(example)
print(f"成功加载 {len(train_examples)} 条训练样本。")

# 3. 定义数据加载器和损失函数
# DataLoader 会在训练时批量提供数据
train_dataloader = DataLoader(train_examples, shuffle=True, batch_size=BATCH_SIZE)

# TripletLoss 是专门用于处理 (anchor, positive, negative) 数据的损失函数
train_loss = losses.TripletLoss(model=model)
print("已定义数据加载器和TripletLoss损失函数。")

# 4. 开始训练
print(f"\n--- 开始训练，共 {EPOCHS} 轮 ---")
model.fit(train_objectives=[(train_dataloader, train_loss)],
          epochs=EPOCHS,
          warmup_steps=100,
          output_path=OUTPUT_MODEL_PATH,
          show_progress_bar=True)

print(f"--- 训练完成！---")
print(f"您微调好的专属模型已保存到: {OUTPUT_MODEL_PATH}")
print(f"\n下一步：请将 main.py 中的 RETRIEVER_MODEL_NAME 变量更新为 '{OUTPUT_MODEL_PATH}'")