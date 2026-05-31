# 学业发展规划系统

[![Qt Version](https://img.shields.io/badge/Qt-6.5+-green.svg)](https://www.qt.io/)
[![C++ Standard](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.25+-blue.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

基于 Qt 6 构建的跨平台学业发展规划系统，提供桌面客户端和 HTTP API 服务。

## ✨ 特性

- 📚 **全面的数据管理**：课程、角色、成果、经历、活动、目标、岗位
- 📊 **数据分析与可视化**：GPA 趋势、学期对比、同学对标
- 📄 **智能简历生成**：支持 JSON/HTML/PDF 多格式导出
- 🤖 **AI 辅助分析**：基于规则引擎的智能建议系统
- 📥 **CSV 数据导入**：批量导入各类数据
- 🔄 **实时数据同步**：跨页面自动刷新联动
- 🎨 **现代化 UI**：Material Design 风格，流畅动画

## 🏗️ 架构

### 系统架构

```
┌─────────────────────────────────────────────────────────┐
│                  Qt Desktop Client                       │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │MainWindow│  │  Pages   │  │ Widgets  │              │
│  │  (Shell) │  │(Business)│  │   (UI)   │              │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘              │
│       │             │              │                     │
│  ┌────┴─────────────┴──────────────┴─────┐              │
│  │         Core Coordinators              │              │
│  │  (AppShell, DataRefresh, Backend, AI)  │              │
│  └────────────────┬───────────────────────┘              │
│                   │ 直接调用                              │
└───────────────────┼──────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│                  Service 层                              │
│  (CourseService, GoalService, AiService, etc.)          │
└────────────────────┬────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────┐
│                  DAO 层 + SQLite                         │
└─────────────────────────────────────────────────────────┘
```

### 技术栈

| 技术 | 版本 | 用途 |
|------|------|------|
| **Qt** | 6.5+ | 跨平台框架、UI、网络、数据库 |
| **C++** | 17 | 核心语言 |
| **SQLite** | 3.x | 嵌入式数据库 |
| **CMake** | 3.25+ | 构建系统 |
| **Qt HTTP Server** | 6.5+ | HTTP API 服务（可选） |

## 📁 项目结构

```
cpp_project/
├── src/
│   ├── client/                 # 桌面客户端 (~11,500行)
│   │   ├── core/              # 核心协调器
│   │   │   ├── AppShellController.h/cpp
│   │   │   ├── DataRefreshCoordinator.h/cpp
│   │   │   ├── BackendRuntimeController.h/cpp
│   │   │   ├── AiContextMediator.h/cpp
│   │   │   └── CrudPageController.h/cpp
│   │   ├── pages/             # 业务页面 (12个)
│   │   │   ├── BasePage.h/cpp
│   │   │   ├── OverviewPage.h/cpp
│   │   │   ├── CoursesPage.h/cpp
│   │   │   ├── GoalsPage.h/cpp
│   │   │   ├── ResumePage.h/cpp
│   │   │   └── ...
│   │   ├── widgets/           # UI组件 (23+个)
│   │   │   ├── SidebarWidget.h/cpp
│   │   │   ├── AiPanelWidget.h/cpp
│   │   │   ├── ResumePreviewWidget.h/cpp
│   │   │   └── ...
│   │   ├── dialogs/           # 编辑对话框 (9个)
│   │   │   ├── CourseEditorDialog.h/cpp
│   │   │   ├── GoalEditorDialog.h/cpp
│   │   │   └── ...
│   │   ├── utils/             # UI工具
│   │   │   ├── UiHelpers.h/cpp
│   │   │   └── ResumeHelpers.h/cpp
│   │   └── MainWindow.h/cpp   # 主窗口 (766行)
│   │
│   ├── service/               # 业务逻辑层 (13个)
│   │   ├── CourseService.h
│   │   ├── GoalService.h
│   │   ├── AiService.h/cpp
│   │   ├── ResumeService.h
│   │   └── ...
│   │
│   ├── dao/                   # 数据访问层 (10个)
│   │   ├── DaoBase.h/cpp
│   │   ├── CourseDao.h
│   │   ├── GoalDao.h
│   │   └── ...
│   │
│   ├── model/                 # 数据模型 (11个)
│   │   ├── Course.h/cpp
│   │   ├── Goal.h
│   │   ├── Role.h
│   │   └── ...
│   │
│   ├── api/                   # HTTP API层 (14个)
│   │   ├── CourseApi.h
│   │   ├── GoalApi.h
│   │   ├── AiApi.h
│   │   └── ...
│   │
│   ├── server/                # HTTP服务器
│   │   └── HttpServer.h
│   │
│   ├── util/                  # 通用工具
│   │   ├── JsonUtils.h
│   │   └── Logger.h
│   │
│   ├── main.cpp               # HTTP服务器入口
│   └── main_desktop.cpp       # 桌面客户端入口
│
├── resources/
│   ├── schema.sql             # 数据库Schema
│   ├── resources.qrc          # Qt资源文件
│   └── icons/                 # 图标资源
│
├── docs/                      # 文档
│   ├── ARCHITECTURE.md        # 架构文档
│   ├── COMPONENTS.md          # 组件文档
│   ├── DEVELOPMENT_GUIDE.md   # 开发指南
│   ├── AI_ASSISTANT_GUIDE.md  # AI助手指南
│   └── ...
│
├── tests/                     # 测试
│   ├── smoke_test.cpp
│   └── crud_regression.py
│
├── CMakeLists.txt             # CMake配置
└── README.md
```

**统计数据：**
- 总文件数：~151个
- 总代码行数：~15,200行
- MainWindow：766行（从5000+行重构而来）
- 架构层次：6层（Client/Core/API/Service/DAO/Model）

## 🚀 快速开始

### 环境要求

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| Qt | 6.5+ | 需包含 Core, Widgets, Sql, HttpServer, Network 模块 |
| CMake | 3.25+ | 构建系统 |
| 编译器 | - | 支持 C++17（MSVC 2019+/GCC 9+/Clang 10+） |
| SQLite | 3.x | Qt 内置驱动 |

### 编译与运行

#### 方法1：使用构建脚本（Windows）

```bash
# 构建桌面客户端
.\build_desktop.bat

# 运行
.\build_desktop\pdp_desktop.exe
```

#### 方法2：手动构建

```bash
# 1. 创建构建目录
mkdir build && cd build

# 2. 配置项目（指定Qt路径）
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64

# 3. 编译
cmake --build .

# 4. 运行桌面客户端
./pdp_desktop

# 或运行HTTP服务器
./pdp_server
```

### 首次运行

1. **自动初始化**：首次启动会自动创建 `pdp.db` 数据库
2. **添加数据**：通过界面添加课程、目标等数据
3. **查看分析**：切换到总览页面查看统计数据
4. **生成简历**：在简历页面配置并导出简历

### 数据导入

支持 CSV 格式批量导入：

```bash
# 使用导入页面上传CSV文件
# 或使用Python脚本
python seed_db.py
```

## 📚 核心功能

### 数据管理

| 模块 | 功能 | 页面 |
|------|------|------|
| 📚 **课程** | CRUD、GPA计算、学分统计 | CoursesPage |
| 🎯 **目标** | CRUD、进度跟踪、优先级管理 | GoalsPage |
| 👔 **角色** | CRUD、在职状态管理 | RolesPage |
| 🏆 **成果** | CRUD、级别分类、验证状态 | AchievementsPage |
| 💼 **经历** | CRUD、时间线管理 | ExperiencesPage |
| 🎭 **活动** | CRUD、日期管理 | ActivitiesPage |
| 💻 **岗位** | CRUD、需求匹配、状态跟踪 | JobsPage |

### 数据分析

| 功能 | 说明 | 页面 |
|------|------|------|
| 📊 **总览仪表盘** | GPA、学分、目标进度、快速统计 | OverviewPage |
| 📈 **学期分析** | 学期对比、GPA趋势、成绩分布 | AnalysisPage |
| 👥 **同学对标** | 与同学数据对比、排名分析 | AnalysisPage |
| ⏱️ **时间轴** | 所有事件按时间排序展示 | TimelinePage |

### 简历生成

| 功能 | 说明 | 页面 |
|------|------|------|
| 📝 **实时预览** | 所见即所得的简历预览 | ResumePage |
| ✏️ **可视化编辑** | 拖拽排序、字段配置 | ResumePage |
| 📤 **多格式导出** | JSON、HTML、PDF（规划中） | ResumePage |
| 🎨 **模板选择** | 多种简历模板（规划中） | ResumePage |

### AI 助手

| 功能 | 说明 | 位置 |
|------|------|------|
| 💡 **智能分析** | 综合/课程/经历/目标分析 | 右侧AI面板 |
| 💬 **对话交互** | 自然语言问答 | 右侧AI面板 |
| 📎 **上下文感知** | 自动捕获选中文本 | 全局 |
| 🔄 **结果回填** | 应用到简历或创建目标 | AI面板 |

详见：[AI助手使用指南](docs/AI_ASSISTANT_GUIDE.md)

### 数据导入

| 功能 | 说明 | 页面 |
|------|------|------|
| 📥 **CSV导入** | 批量导入各类数据 | ImportsPage |
| 🔍 **格式验证** | 自动验证数据格式 | ImportsPage |
| 📊 **导入报告** | 显示成功/失败统计 | ImportsPage |

## 🔌 HTTP API（可选）

桌面客户端**直接调用 Service 层**，无需 HTTP API。HTTP API 仅用于 Web 前端访问。

### 主要端点

| 模块 | 端点 | 方法 |
|------|------|------|
| 课程 | `/api/courses` | GET, POST, PUT, DELETE |
| 目标 | `/api/goals` | GET, POST, PUT, DELETE |
| 角色 | `/api/roles` | GET, POST, PUT, DELETE |
| 成果 | `/api/achievements` | GET, POST, PUT, DELETE |
| 经历 | `/api/experiences` | GET, POST, PUT, DELETE |
| 活动 | `/api/activities` | GET, POST, PUT, DELETE |
| 岗位 | `/api/jobs` | GET, POST, PUT, DELETE |
| 仪表盘 | `/api/dashboard/overview` | GET |
| 分析 | `/api/analytics/semester-comparison` | GET |
| 时间轴 | `/api/timeline` | GET |
| 简历 | `/api/resume/generate` | POST |
| AI | `/api/ai/analyze` | POST |
| 导入 | `/api/imports/{entity}` | POST |

### 响应格式

```json
{
    "success": true,
    "code": 200,
    "message": "操作成功",
    "data": { ... }
}
```

详见：[API文档](docs/COMPONENTS.md)

## 💾 数据库

### Schema

系统使用 SQLite 数据库（`pdp.db`），包含10个核心表：

| 表名 | 说明 | 主要字段 |
|------|------|----------|
| `courses` | 课程信息 | name, code, credits, score, grade_point, status |
| `goals` | 目标管理 | title, category, target_value, current_value, status |
| `roles` | 角色信息 | title, type, organization, is_active |
| `achievements` | 成果记录 | title, type, level, date, verified |
| `experiences` | 经历记录 | title, type, organization, start_date, end_date |
| `activities` | 活动记录 | name, description, start_date, end_date |
| `jobs` | 岗位信息 | title, company, location, requirements, status |
| `job_requirements` | 岗位需求 | job_id, requirement, is_met |
| `users` | 用户信息 | username, email, password_hash |
| `peer_benchmarks` | 同学对标 | name, major, semester, gpa, credits |

### 初始化

首次启动时自动执行 `resources/schema.sql` 创建表结构。

## 🛠️ 开发指南

### 添加新页面

1. 创建页面类继承 `BasePage`
2. 实现 `refresh()` 方法
3. 在 `MainWindow::setupUi()` 中添加到 `QStackedWidget`
4. 在 `DataRefreshCoordinator` 中绑定刷新逻辑
5. 在导航列表中添加入口

详见：[开发指南](docs/DEVELOPMENT_GUIDE.md)

### 添加新数据实体

1. 在 `src/model/` 创建模型类（含 `toDict()`/`fromDict()`）
2. 在 `src/dao/` 创建 DAO 类
3. 在 `src/service/` 创建 Service 类
4. 在 `src/api/` 创建 API 类（如需 HTTP 接口）
5. 在 `resources/schema.sql` 添加表结构
6. 创建对应的编辑对话框
7. 创建对应的页面

### 代码规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `CourseService`, `CourseApi` |
| 方法名 | camelCase | `getAll`, `getById`, `create` |
| 成员变量 | camelCase + m_前缀 | `m_server`, `m_database` |
| 常量 | UPPER_SNAKE_CASE | `MAX_RETRY_COUNT` |
| 文件名 | PascalCase | `CourseService.h` |

### 测试

```bash
# 运行单元测试
cd build
ctest --output-on-failure

# 运行回归测试
python tests/crud_regression.py
```

## 📖 文档

| 文档 | 说明 | 适用对象 |
|------|------|----------|
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | 架构设计文档 | 开发者 |
| [COMPONENTS.md](docs/COMPONENTS.md) | 组件使用文档 | 开发者 |
| [DEVELOPMENT_GUIDE.md](docs/DEVELOPMENT_GUIDE.md) | 开发者指南 | 开发者 |
| [AI_ASSISTANT_GUIDE.md](docs/AI_ASSISTANT_GUIDE.md) | AI助手使用指南 | 用户、开发者 |
| [REGRESSION_CHECKLIST.md](docs/REGRESSION_CHECKLIST.md) | 回归测试清单 | 测试工程师 |

## 🎯 项目里程碑

### v0.1 - 单体架构
- ✅ 基础功能实现
- ✅ 单体 MainWindow（5000+行）

### v0.2 - 初步拆分
- ✅ 拆分 Pages 和 Dialogs
- ✅ 引入 Service 层

### v0.3 - 模块化架构（当前）
- ✅ 完整的6层架构
- ✅ MainWindow 精简到 766 行
- ✅ 151个文件，~15,200行代码
- ✅ 核心协调器模式
- ✅ 数据刷新联动
- ✅ AI 助手集成
- ✅ 完整文档

### v0.4 - 体验优化（规划中）
- ⏳ AI 助手交互优化
- ⏳ 性能优化
- ⏳ 测试覆盖率提升至80%+
- ⏳ 用户手册

### v1.0 - 正式版（规划中）
- ⏳ LLM 模式支持
- ⏳ 多用户支持
- ⏳ 云端同步
- ⏳ 移动端适配

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 👥 团队

- **架构设计与管理**：组长
- **API层与路由开发**：组员A
- **后端核心开发**：组员B
- **测试与文档**：组员C

## 🙏 致谢

- [Qt Framework](https://www.qt.io/) - 跨平台应用框架
- [SQLite](https://www.sqlite.org/) - 嵌入式数据库
- [CMake](https://cmake.org/) - 构建系统

---

**项目版本**: v0.3  
**最后更新**: 2026-04-29  
**维护者**: 项目团队