#!/bin/bash

# 智能超市收银系统 - Git 仓库初始化脚本

set -e

echo "📚 智能超市收银系统 - Git 仓库准备"
echo "=================================="

# 检查是否在正确的目录
if [[ ! -f "CMakeLists.txt" ]]; then
    echo "❌ 请在项目根目录运行此脚本"
    exit 1
fi

# 检查 git 是否已初始化
if [[ ! -d ".git" ]]; then
    echo "🔧 初始化 Git 仓库..."
    git init
    echo "✅ Git 仓库初始化完成"
else
    echo "✅ Git 仓库已存在"
fi

# 检查 build 目录是否存在并添加到 .gitignore
if [[ -d "build" ]]; then
    echo "🧹 build 目录已存在，将被忽略"
fi

# 添加所有文件到暂存区 (除了 .gitignore 中的文件)
echo "📁 添加文件到 Git..."
git add .

# 检查是否有文件被添加
if git diff --cached --quiet; then
    echo "⚠️  没有文件需要提交"
else
    # 显示将要提交的文件
    echo "📋 将要提交的文件:"
    git diff --cached --name-only

    # 创建初始提交
    echo "💾 创建初始提交..."
    git commit -m "🎉 Initial commit: Smart Supermarket POS System

📦 项目特性:
- 完整的 Qt/C++ 收银系统
- 商品管理和销售功能
- 智能条码识别
- AI 推荐系统
- 完整的测试套件
- 详细的文档和构建脚本

🛠️ 技术栈:
- C++17
- Qt 6.x Widgets
- SQLite 数据库
- CMake 构建系统
- QTest 测试框架

📚 文档:
- README.md - 项目介绍和快速开始
- BUILD.md - 编译指南
- CONTRIBUTING.md - 贡献指南
- CHANGELOG.md - 更新日志

🔧 工具:
- 自动化构建脚本
- CI/CD 配置
- 开发环境设置脚本"

    echo "✅ 初始提交完成"
fi

# 显示当前状态
echo ""
echo "📊 Git 状态:"
git status --short
echo ""
echo "📝 最近的提交:"
git log --oneline -n 3 2>/dev/null || echo "还没有提交"

echo ""
echo "🚀 准备上传到 GitHub:"
echo ""
echo "1. 在 GitHub 上创建新仓库 'smart-pos-system'"
echo "2. 设置远程仓库:"
echo "   git remote add origin https://github.com/YOUR_USERNAME/smart-pos-system.git"
echo ""
echo "3. 推送到 GitHub:"
echo "   git branch -M main"
echo "   git push -u origin main"
echo ""
echo "🎯 建议的分支结构:"
echo "   main     - 稳定发布版本"
echo "   develop  - 开发分支"
echo "   feature/* - 功能分支"
echo "   fix/*    - 修复分支"
echo ""
echo "4. 创建 develop 分支:"
echo "   git checkout -b develop"
echo "   git push -u origin develop"
echo ""
echo "💡 团队协作工作流:"
echo "   1. 从 develop 分支创建功能分支"
echo "   2. 完成开发后提交 PR 到 develop"
echo "   3. 代码审查通过后合并"
echo "   4. 定期从 develop 合并到 main 发布"
echo ""
echo "📖 更多信息请查看 CONTRIBUTING.md"
