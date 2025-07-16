# 智能超市收银系统 (Smart POS System)

一个基于C++/Qt的现代化、高响应度的智能超市收银系统。本项目从一个不稳定的原型出发，经过了深度重构和调试，现已成为一个功能稳定、性能可靠的应用程序。

## 🚀 项目特性

- **响应式现代化UI**: 基于Qt Widgets，通过集中的样式表管理，提供统一、清晰、高对比度的用户界面。
- **异步数据库操作**: 所有数据库查询均在后台线程中运行 (`QtConcurrent`)，确保UI在数据加载期间绝不冻结。
- **交互式购物车**: 使用自定义委托 (`QStyledItemDelegate`) 实现，支持在表格内直接修改商品数量和移除商品。
- **统一的商品搜索**: 智能识别输入是商品名称还是条形码，简化操作流程。
- **独立的商品管理**: 将商品增、删、改功能封装在独立的对话框中，让主界面专注于销售任务。
- **AI智能推荐**: 基于当前购物车内容，以水平滚动轮播的形式，图文并茂地为用户推荐商品。
- **健壮的错误处理与信号/槽机制**: 确保了应用的稳定性和模块间的可靠通信。
- **完整的销售流程**: 支持收银、多种支付方式模拟、折扣应用和票据打印。

## 📋 系统要求

- **操作系统**: Windows 10+, macOS 10.14+, Ubuntu 18.04+
- **Qt版本**: Qt 6.2+
- **CMake**: 3.16+
- **C++标准**: C++17

## 🛠️ 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/your-username/smart-pos-system.git
cd smart-pos-system
```

### 2. 编译

```bash
# 创建并进入构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译 (建议使用多核加速)
make -j8
```

### 3. 运行

```bash
# 在 build 目录下运行
./SmartPOS
```

## 📁 项目结构

```
smart-pos-system/
├── .github/          # GitHub Actions CI/CD 配置
├── build/            # 编译产物目录 (自动生成)
├── resources/        # 应用资源 (图标, 声音等)
├── src/              # 源代码
│   ├── ai/
│   ├── barcode/
│   ├── controllers/
│   ├── database/
│   ├── models/
│   └── ui/
├── tests/            # 测试代码 (单元测试/集成测试)
├── .gitignore
├── CMakeLists.txt    # 项目主构建文件
├── CONTRIBUTING.md   # 贡献指南
├── LICENSE           # MIT 许可证
└── README.md         # 项目说明文档
```

## 🏗️ 核心技术与架构演进

本项目经历了一次从同步到异步的重大架构重构，以解决UI冻结问题。

- **模型层 (`models`)**: 包括 `Product`, `Sale` 等，负责原始数据结构和业务逻辑。`Sale` 类通过信号 (`totalChanged`, `saleChanged`) 驱动UI更新。
- **数据库层 (`database`)**: `DatabaseManager` 使用 `QtConcurrent::run` 将所有SQL查询移至工作线程，并通过信号 (`productsRead`, `productSaved` 等) 将结果返回给主线程。
- **控制器层 (`controllers`)**: `ProductManager` 和 `CheckoutController` 作为模型和UI之间的桥梁。它们监听来自数据库层的信号，处理数据，并发出自己的信号 (`allProductsChanged`, `saleUpdated`) 来通知UI层。
- **视图层 (`ui`)**: `MainWindow` 和其他对话框完全通过信号和槽与控制器层交互。所有UI更新都是由信号驱动的，实现了高度解耦和响应式设计。

## 许可证

本项目采用MIT许可证。详见 `LICENSE` 文件。
