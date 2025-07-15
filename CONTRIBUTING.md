# 贡献指南 (Contributing Guide)

欢迎为智能超市收银系统项目做出贡献！本文档将指导您如何参与项目开发。

## 🤝 参与方式

### 报告问题
- 使用GitHub Issues报告bug
- 提供详细的复现步骤
- 包含系统环境信息
- 添加相关的错误截图

### 功能建议
- 在Issues中描述新功能需求
- 解释功能的用途和价值
- 提供实现思路（可选）

### 代码贡献
- Fork项目到你的GitHub账户
- 创建功能分支进行开发
- 提交Pull Request

## 🔧 开发环境设置

### 1. Fork和克隆项目
```bash
# Fork项目后克隆到本地
git clone https://github.com/your-username/smart-pos-system.git
cd smart-pos-system

# 添加上游仓库
git remote add upstream https://github.com/original-repo/smart-pos-system.git
```

### 2. 创建开发分支
```bash
# 从develop分支创建功能分支
git checkout develop
git pull upstream develop
git checkout -b feature/your-feature-name
```

### 3. 设置开发环境
```bash
# 安装依赖
# macOS
brew install qt@6 cmake

# Ubuntu
sudo apt install qt6-base-dev qt6-tools-dev cmake

# 编译项目
mkdir build && cd build
cmake ..
make -j4
```

## 📝 编码规范

### C++ 代码风格
- 使用C++17标准
- 遵循Qt编码规范
- 类名使用PascalCase: `ProductManager`
- 函数名使用camelCase: `addProduct()`
- 私有成员变量使用m_前缀: `m_currentSale`
- 常量使用UPPER_CASE: `MAX_ITEMS`

### 代码示例
```cpp
class ProductManager : public QObject
{
    Q_OBJECT

public:
    explicit ProductManager(QObject* parent = nullptr);
    ~ProductManager();

    bool addProduct(Product* product);
    std::unique_ptr<Product> getProduct(int productId);

private slots:
    void onProductUpdated();

private:
    void updateDatabase();
    
    DatabaseManager* m_databaseManager;
    QList<Product*> m_products;
    static const int MAX_PRODUCTS = 10000;
};
```

### 注释规范
```cpp
/**
 * @brief 添加商品到数据库
 * @param product 商品对象指针
 * @return 成功返回true，失败返回false
 */
bool ProductManager::addProduct(Product* product)
{
    // 验证商品数据
    if (!product || product->getName().isEmpty()) {
        qDebug() << "无效的商品数据";
        return false;
    }
    
    // 保存到数据库
    return m_databaseManager->saveProduct(product);
}
```

## 🧪 测试要求

### 编写测试
- 每个新功能都需要对应的测试
- 单元测试覆盖率应达到80%以上
- 使用QTest框架编写测试

### 测试示例
```cpp
class TestProductManager : public QObject
{
    Q_OBJECT

private slots:
    void testAddProduct();
    void testGetProduct();
    void testDeleteProduct();
};

void TestProductManager::testAddProduct()
{
    ProductManager manager;
    Product* product = new Product();
    product->setName("测试商品");
    product->setBarcode("123456789");
    
    QVERIFY(manager.addProduct(product));
    
    delete product;
}
```

### 运行测试
```bash
# 编译测试
make -j4

# 运行所有测试
make test

# 运行特定测试
./tests/TestProductManager
```

## 🚀 提交规范

### 提交信息格式
```
类型(作用域): 简短描述

详细描述（可选）

相关Issue: #123
```

### 提交类型
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `style`: 代码格式化
- `refactor`: 重构代码
- `test`: 添加测试
- `chore`: 构建工具更新

### 提交示例
```bash
git commit -m "feat(product): 添加商品双击添加到购物车功能

- 在ProductTable中添加doubleClicked信号处理
- 通过条码自动添加商品到当前销售
- 添加相应的测试用例

相关Issue: #45"
```

## 📋 Pull Request 流程

### 1. 准备PR
- 确保所有测试通过
- 更新相关文档
- 添加变更日志

### 2. 创建PR
- 提供清晰的标题和描述
- 列出主要变更
- 链接相关Issue
- 添加测试截图（如适用）

### 3. PR模板
```markdown
## 变更描述
简要描述此PR的主要变更

## 变更类型
- [ ] 新功能
- [ ] Bug修复
- [ ] 文档更新
- [ ] 重构
- [ ] 测试

## 测试
- [ ] 所有现有测试通过
- [ ] 添加了新的测试
- [ ] 手动测试通过

## 检查清单
- [ ] 代码遵循项目规范
- [ ] 更新了相关文档
- [ ] 添加了适当的注释

## 相关Issue
关闭 #123
```

## 👥 团队协作

### 分支策略
- `main`: 稳定发布版本
- `develop`: 开发版本
- `feature/*`: 功能开发分支
- `fix/*`: 修复分支
- `release/*`: 发布准备分支

### 工作流程
1. 从`develop`创建`feature`分支
2. 开发并测试功能
3. 提交PR到`develop`分支
4. 代码审查通过后合并
5. 定期从`develop`合并到`main`发布

### 代码审查
- 至少需要一个团队成员审查
- 关注代码质量和规范
- 确保测试覆盖率
- 检查性能影响

## 🏗️ 项目结构

### 目录组织
```
src/
├── controllers/     # 业务逻辑控制器
├── models/         # 数据模型
├── ui/            # 用户界面
├── database/      # 数据库操作
├── barcode/       # 条码识别
├── ai/           # AI推荐
└── utils/        # 工具类

tests/
├── unit/         # 单元测试
├── integration/  # 集成测试
└── TestCommon.*  # 测试通用代码
```

### 命名约定
- 头文件: `ProductManager.h`
- 源文件: `ProductManager.cpp`
- 测试文件: `TestProductManager.cpp`
- UI文件: `MainWindow.ui`

## 📚 学习资源

### Qt文档
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [Qt Widgets](https://doc.qt.io/qt-6/qtwidgets-index.html)
- [QTest Framework](https://doc.qt.io/qt-6/qtest-overview.html)

### C++资源
- [C++17 Reference](https://en.cppreference.com/w/cpp/17)
- [Modern C++ Guidelines](https://isocpp.github.io/CppCoreGuidelines/)

## 🤔 获得帮助

- 📖 查看项目文档和代码注释
- 💬 在Issues中提问
- 📧 联系项目维护者
- 💡 参考现有代码实现

感谢您对智能超市收银系统项目的贡献！🎉
