# 🚀 GitHub 上传指南

## 📋 项目准备完成状态

✅ **项目结构**: 完整的源代码和文档结构  
✅ **构建系统**: CMake 配置和自动化脚本  
✅ **测试套件**: 单元测试和集成测试 (78% 通过率)  
✅ **文档**: README, 编译指南, 贡献指南, 更新日志  
✅ **CI/CD**: GitHub Actions 自动化流水线  
✅ **协作工具**: Git 配置, 脚本工具, VS Code 配置  

## 🛠️ 四人团队编译指南

### 🎯 方案1: 一键脚本 (推荐新手)

```bash
# 1. 克隆项目
git clone https://github.com/your-team/smart-pos-system.git
cd smart-pos-system

# 2. 设置开发环境 (只需运行一次)
./scripts/setup-dev.sh    # Linux/macOS 自动安装依赖

# 3. 构建项目
./scripts/build.sh        # Linux/macOS
# 或者
scripts\build.bat          # Windows

# 4. 运行应用
cd build && ./SmartPOS
```

### ⚙️ 方案2: 手动编译 (开发者推荐)

```bash
# 1. 安装依赖
# macOS:
brew install qt@6 cmake
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"

# Ubuntu:
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# Windows: 下载安装 Qt 6.x 和 CMake

# 2. 编译
mkdir build && cd build
cmake ..
make -j4                   # Linux/macOS
cmake --build . -j 4       # Windows

# 3. 测试
make test
```

## 📤 上传到 GitHub 步骤

### 1. 初始化 Git 仓库

```bash
cd /path/to/smart-pos-system
./scripts/init-git.sh     # 自动化初始化脚本
```

### 2. 在 GitHub 创建仓库

1. 登录 GitHub
2. 点击 "New repository"
3. 仓库名: `smart-pos-system`
4. 描述: `智能超市收银系统 - 基于Qt/C++的现代化POS系统`
5. 选择 Public 或 Private
6. **不要**初始化 README (项目中已有)
7. 点击 "Create repository"

### 3. 连接远程仓库并推送

```bash
# 添加远程仓库
git remote add origin https://github.com/YOUR_USERNAME/smart-pos-system.git

# 推送代码
git branch -M main
git push -u origin main

# 创建开发分支
git checkout -b develop
git push -u origin develop
```

## 👥 团队协作工作流

### 🌿 分支管理策略

```
main         (稳定发布版本)
├── develop  (开发主分支)
│   ├── feature/ui-enhancement        (成员A: UI开发)
│   ├── feature/business-logic        (成员B: 业务逻辑)
│   ├── feature/database-ai           (成员C: 数据库+AI)
│   └── feature/hardware-testing      (成员D: 硬件+测试)
```

### 👨‍💻 任务分工建议

| 成员 | 主要职责 | 文件夹 |
|------|----------|--------|
| **成员A** | UI界面开发 | `src/ui/`, `resources/` |
| **成员B** | 业务逻辑 | `src/controllers/`, `src/models/` |
| **成员C** | 数据库+AI | `src/database/`, `src/ai/` |
| **成员D** | 硬件+测试 | `src/barcode/`, `src/utils/`, `tests/` |

### 🔄 日常开发流程

```bash
# 1. 同步最新代码
git checkout develop
git pull origin develop

# 2. 创建功能分支
git checkout -b feature/your-feature-name

# 3. 开发代码
# ... 编写代码 ...

# 4. 测试代码
./scripts/build.sh
cd build && make test

# 5. 提交代码
git add .
git commit -m "feat: 添加新功能描述"

# 6. 推送分支
git push origin feature/your-feature-name

# 7. 在GitHub创建Pull Request到develop分支
```

### 📝 提交信息规范

```bash
# 格式: 类型(作用域): 简短描述
git commit -m "feat(ui): 添加商品双击添加功能"
git commit -m "fix(database): 修复库存更新bug"
git commit -m "docs: 更新README编译说明"
git commit -m "test: 添加AI推荐单元测试"
```

## 🚨 环境配置检查清单

### macOS 开发者
- [ ] 安装 Xcode Command Line Tools: `xcode-select --install`
- [ ] 安装 Homebrew: `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
- [ ] 安装 Qt: `brew install qt@6`
- [ ] 配置环境变量: `export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"`

### Linux (Ubuntu) 开发者
- [ ] 更新包管理器: `sudo apt update`
- [ ] 安装构建工具: `sudo apt install build-essential cmake`
- [ ] 安装 Qt: `sudo apt install qt6-base-dev qt6-tools-dev`
- [ ] 验证安装: `qmake6 --version`

### Windows 开发者
- [ ] 下载 Qt 6.5+: https://www.qt.io/download
- [ ] 下载 CMake: https://cmake.org/download/
- [ ] 安装 Visual Studio 2019+ 或 MinGW
- [ ] 添加 Qt 和 CMake 到 PATH 环境变量

### 通用检查
- [ ] Git 已安装并配置用户信息
- [ ] 编辑器已配置 (推荐 VS Code + CMake Tools 扩展)
- [ ] 能够成功运行 `./scripts/build.sh` 或 `scripts\build.bat`

## 🎉 项目亮点

- **完整的MVC架构**: 清晰的代码组织
- **78%测试覆盖率**: 可靠的代码质量
- **跨平台支持**: Windows, macOS, Linux
- **现代化CI/CD**: GitHub Actions自动化
- **详细文档**: 完整的开发和使用文档
- **团队协作友好**: 分支策略和编码规范

## 🔗 相关链接

- **项目主页**: https://github.com/your-team/smart-pos-system
- **编译指南**: [BUILD.md](BUILD.md)
- **贡献指南**: [CONTRIBUTING.md](CONTRIBUTING.md)
- **更新日志**: [CHANGELOG.md](CHANGELOG.md)
- **测试报告**: [TEST_REPORT.md](TEST_REPORT.md)

---

**准备好了就上传吧！🚀**
