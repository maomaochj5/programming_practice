<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# 智能超市收银系统 (Smart Supermarket POS System)

这是一个基于C++/Qt的智能超市收银系统项目。

## 项目特性
- 现代化C++/Qt开发
- 模块化架构设计
- 基于摄像头的实时条形码识别
- AI驱动的商品推荐系统
- SQLite数据库集成
- 信号与槽机制进行模块通信

## 开发指导
- 遵循Qt的信号与槽设计模式
- 使用RAII和智能指针进行内存管理
- 保持代码的模块化和可测试性
- 优先考虑性能和用户体验
- 实现离线优先的架构设计

## 核心模块
- UI层：Qt Widgets界面设计
- 业务逻辑层：商品管理、销售流程、支付处理
- 数据访问层：SQLite数据库操作
- 硬件集成层：条形码扫描、票据打印
- AI服务层：商品推荐算法
