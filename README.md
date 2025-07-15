# 智能超市收银系统 (Smart Supermarket POS System)

一个基于C++/Qt的现代化智能超市收银系统，集成条形码识别、AI推荐、商品管理等功能。

## 🚀 项目特性

- **现代化UI界面**: 基于Qt Widgets的直观用户界面
- **智能条码识别**: 支持摄像头实时扫描和手动输入
- **AI智能推荐**: 基于购物车内容的商品推荐算法
- **完整的商品管理**: 增删改查、库存管理、双击添加功能
- **销售流程管理**: 完整的收银、支付、打印流程
- **数据持久化**: SQLite数据库存储
- **模块化架构**: 清晰的MVC架构设计
- **自动化测试**: 完整的单元测试和集成测试

## 📋 系统要求

### 开发环境
- **操作系统**: Windows 10+, macOS 10.14+, Ubuntu 18.04+
- **编译器**: 
  - GCC 8.0+ 或 Clang 8.0+ (Linux/macOS)
  - MSVC 2019+ (Windows)
- **Qt版本**: Qt 6.2+
- **CMake**: 3.16+
- **C++标准**: C++17

### 依赖库
- Qt6 Core
- Qt6 Widgets  
- Qt6 Sql
- Qt6 Network
- Qt6 PrintSupport
- Qt6 Test (测试模块)

## 🛠️ 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/your-username/smart-pos-system.git
cd smart-pos-system
```

### 2. 一键编译 (推荐)

```bash
# Linux/macOS
./scripts/build.sh

# Windows
scripts\build.bat
```

### 3. 手动编译

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译 (使用多核编译加速)
make -j4                # Linux/macOS
# 或者
cmake --build . -j 4    # 跨平台方式
```

### 4. 运行应用

```bash
# 运行主程序
./SmartPOS

# 运行测试
make test
# 或者单独运行测试
./tests/TestProduct
./tests/TestGUIBasic
```

## 📁 项目结构

```
SmartPOS/
├── 📁 src/                         # 源代码
│   ├── main.cpp                    # 程序入口点
│   ├── 📁 ui/                      # 用户界面层
│   │   ├── MainWindow.cpp/h/ui     # 主窗口
│   │   ├── ProductDialog.cpp/h     # 商品对话框
│   │   └── PaymentDialog.cpp/h     # 支付对话框
│   ├── 📁 models/                  # 数据模型层
│   │   ├── Product.cpp/h           # 商品模型
│   │   ├── Customer.cpp/h          # 客户模型  
│   │   ├── Sale.cpp/h              # 销售模型
│   │   └── SaleItem.cpp/h          # 销售项目模型
│   ├── 📁 controllers/             # 控制器层
│   │   ├── ProductManager.cpp/h    # 商品管理控制器
│   │   └── CheckoutController.cpp/h # 收银流程控制器
│   ├── 📁 database/                # 数据持久层
│   │   └── DatabaseManager.cpp/h  # 数据库管理器
│   ├── 📁 ai/                      # AI推荐系统
│   │   └── AIRecommender.cpp/h     # 智能推荐引擎
│   ├── 📁 barcode/                 # 条码识别
│   │   └── BarcodeScanner.cpp/h    # 条码扫描器
│   └── 📁 utils/                   # 工具类
│       └── ReceiptPrinter.cpp/h    # 票据打印
├── 📁 tests/                       # 测试代码
│   ├── 📁 unit/                    # 单元测试
│   └── 📁 integration/             # 集成测试
├── 📁 resources/                   # 资源文件
│   ├── icons.qrc                   # Qt资源文件
│   └── *.png                       # 图标文件
├── 📁 scripts/                     # 自动化脚本
│   ├── build.sh                    # Linux/macOS构建脚本
│   ├── build.bat                   # Windows构建脚本
│   └── setup-dev.sh                # 开发环境配置
├── 📁 .github/workflows/           # CI/CD配置
│   └── ci-cd.yml                   # GitHub Actions
```

## 🏗️ 技术架构
│   │   └── TestDatabaseManager.cpp # 数据库测试
│   └── 📁 integration/             # 集成测试
│       ├── TestSmartPOSWorkflow.cpp # 完整工作流测试
│       ├── TestGUIBasic.cpp        # GUI基础测试
│       ├── TestGUIAutomation.cpp   # GUI自动化测试
│       └── TestGUIInterface.cpp    # GUI接口测试
├── 📁 resources/                   # 资源文件
│   ├── icons.qrc                   # Qt资源配置
│   └── 📁 icons/                   # 应用图标资源
├── 📁 scripts/                     # 自动化脚本
│   ├── build.sh/.bat               # 跨平台构建脚本
│   ├── setup-dev.sh                # 开发环境设置
│   └── init-git.sh                 # Git仓库初始化
├── 📁 .github/                     # GitHub配置
│   └── workflows/ci-cd.yml         # CI/CD自动化流水线
├── CMakeLists.txt                  # CMake主配置
├── README.md                       # 项目说明
├── BUILD.md                        # 编译指南
├── CONTRIBUTING.md                 # 贡献指南
├── CHANGELOG.md                    # 更新日志
├── TEST_REPORT.md                  # 测试报告
├── LICENSE                         # MIT许可证
└── .gitignore                      # Git忽略配置
```
├── resources/             # 资源文件
├── tests/                # 测试文件
├── docs/                 # 文档
├── CMakeLists.txt        # CMake构建配置
└── README.md
```

## 快速开始

### 环境要求
- Qt 6.0 或更高版本
- CMake 3.16 或更高版本
- C++17 兼容编译器
- SQLite 3
- 支持的摄像头设备

### 构建步骤
```bash
mkdir build
cd build
cmake ..
make
```

### 运行系统
```bash
./SmartPOS
```

## 核心模块说明

### 1. 商品管理 (ProductManager)
- 商品信息的增删改查
- 条码管理和验证
- 库存自动更新

### 2. 销售控制器 (CheckoutController)
- 管理销售流程
- 处理支付逻辑
- 集成条形码扫描

### 3. 数据库管理 (DatabaseManager)
- 单例模式设计
- 事务安全保证
- 数据持久化

### 4. 条形码识别 (BarcodeScanner)
- QZXing集成
- 实时视频处理
- 性能优化

### 5. AI推荐系统 (AIRecommender)
- 协同过滤算法
- 冷启动问题处理
- 实时推荐生成

## 数据库设计

### 主要表结构
- **Products**: 商品信息
- **Customers**: 客户信息
- **Transactions**: 交易记录
- **TransactionItems**: 交易明细

## 开发指南

### 编码规范
- 遵循C++核心指导原则
- 使用Qt的信号与槽机制
- 实现RAII内存管理
- 保持代码模块化

### 测试策略
- 单元测试覆盖核心逻辑
- 集成测试验证模块交互
- 性能测试保证响应速度

## 部署说明

### 硬件要求
- 最小2GB RAM
- 支持Qt的操作系统
- USB摄像头（用于条形码识别）
- 票据打印机（可选）

### 软件依赖
- Qt运行时库
- SQLite数据库
- 相关硬件驱动

## 贡献指南

欢迎提交Bug报告和功能请求。请遵循项目的编码规范和提交流程。

## 许可证

本项目采用MIT许可证。详见LICENSE文件。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 项目Issues页面
- 开发团队邮箱

---

**注意**: 这是一个教学实训项目，展示了现代C++/Qt开发的最佳实践和创新技术的集成应用。
