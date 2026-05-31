# 学业发展规划系统 - 架构文档

## 1. 系统概述

学业发展规划系统是一个基于 Qt 6 的跨平台桌面应用，采用 C++ 开发，提供学业数据管理、目标追踪、简历生成和 AI 辅助分析功能。

### 1.1 技术栈

- **框架**: Qt 6.5+
- **语言**: C++17
- **构建**: CMake 3.25+
- **数据库**: SQLite 3
- **HTTP服务**: Qt HTTP Server
- **测试**: Qt Test

### 1.2 核心特性

- 📚 课程、角色、成果、经历、活动、目标、岗位管理
- 📊 数据分析与可视化
- 📄 简历生成与多格式导出
- 🤖 AI 辅助分析与建议
- 📥 CSV 数据导入
- 🔄 实时数据同步

---

## 2. 整体架构

### 2.1 架构分层

```
┌─────────────────────────────────────────────────────────────┐
│                      Presentation Layer                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  MainWindow  │  │    Pages     │  │   Dialogs    │      │
│  │  (Shell)     │  │  (Business)  │  │   (Input)    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                  │                  │              │
│         └──────────────────┴──────────────────┘              │
│                            │                                 │
├────────────────────────────┼─────────────────────────────────┤
│                      Core Layer                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Coordinators │  │ Controllers  │  │  Mediators   │      │
│  │ (Refresh)    │  │  (Shell)     │  │  (Context)   │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                            │                                 │
├────────────────────────────┼─────────────────────────────────┤
│                     Business Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Services   │  │     APIs     │  │   Models     │      │
│  │  (Logic)     │  │  (Routing)   │  │   (Data)     │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                            │                                 │
├────────────────────────────┼─────────────────────────────────┤
│                      Data Layer                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │     DAOs     │  │   Database   │  │   Schema     │      │
│  │  (Access)    │  │   (SQLite)   │  │   (SQL)      │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 目录结构

```
cpp_project/
├── src/
│   ├── api/                    # API 路由层
│   │   ├── AchievementApi.h
│   │   ├── ActivityApi.h
│   │   ├── AiApi.h
│   │   ├── AnalyticsApi.h
│   │   ├── AuthApi.h
│   │   ├── CourseApi.h
│   │   ├── DashboardApi.h
│   │   ├── ExperienceApi.h
│   │   ├── GoalApi.h
│   │   ├── ImportApi.h
│   │   ├── JobApi.h
│   │   ├── ResumeApi.h
│   │   ├── RoleApi.h
│   │   └── TimelineApi.h
│   │
│   ├── client/                 # 客户端层
│   │   ├── core/              # 核心协调器
│   │   │   ├── AppShellController.h/cpp      # 壳层控制
│   │   │   ├── DataRefreshCoordinator.h/cpp  # 刷新协调
│   │   │   ├── BackendRuntimeController.h/cpp # 后端管理
│   │   │   ├── AiContextMediator.h/cpp       # AI上下文
│   │   │   ├── CrudPageController.h/cpp      # CRUD控制
│   │   │   └── DataDomain.h                  # 数据域定义
│   │   │
│   │   ├── pages/             # 页面层
│   │   │   ├── BasePage.h/cpp              # 页面基类
│   │   │   ├── OverviewPage.h/cpp          # 总览页
│   │   │   ├── CoursesPage.h/cpp           # 课程页
│   │   │   ├── RolesPage.h/cpp             # 角色页
│   │   │   ├── AchievementsPage.h/cpp      # 成果页
│   │   │   ├── ExperiencesPage.h/cpp       # 经历页
│   │   │   ├── ActivitiesPage.h/cpp        # 活动页
│   │   │   ├── GoalsPage.h/cpp             # 目标页
│   │   │   ├── JobsPage.h/cpp              # 岗位页
│   │   │   ├── AnalysisPage.h/cpp          # 分析页
│   │   │   ├── TimelinePage.h/cpp          # 时间轴页
│   │   │   ├── ResumePage.h/cpp            # 简历页
│   │   │   └── ImportsPage.h/cpp           # 导入页
│   │   │
│   │   ├── widgets/           # 组件层
│   │   │   ├── SidebarWidget.h/cpp         # 侧边栏
│   │   │   ├── NavigationListWidget.h/cpp  # 导航列表
│   │   │   ├── TimeInfoCard.h/cpp          # 时间卡片
│   │   │   ├── StudentInfoCard.h/cpp       # 学生卡片
│   │   │   ├── AiPanelWidget.h/cpp         # AI面板
│   │   │   ├── AiStatusBar.h/cpp           # AI状态栏
│   │   │   ├── AiConversationWidget.h/cpp  # AI对话
│   │   │   ├── ResumePreviewWidget.h/cpp   # 简历预览
│   │   │   ├── ResumeEditorPanel.h/cpp     # 简历编辑
│   │   │   ├── ResumeCandidatePanel.h/cpp  # 备选素材
│   │   │   ├── SemesterAnalysisWidget.h/cpp # 学期分析
│   │   │   ├── PeerBenchmarkWidget.h/cpp   # 同学对标
│   │   │   ├── MetricGridWidget.h/cpp      # 指标网格
│   │   │   ├── SuggestionListWidget.h/cpp  # 建议列表
│   │   │   ├── CrudPageShell.h/cpp         # CRUD壳层
│   │   │   └── ToastNotification.h/cpp     # 提示通知
│   │   │
│   │   ├── dialogs/           # 对话框层
│   │   │   ├── ProfileEditorDialog.h/cpp
│   │   │   ├── CourseEditorDialog.h/cpp
│   │   │   ├── GoalEditorDialog.h/cpp
│   │   │   ├── RoleEditorDialog.h/cpp
│   │   │   ├── AchievementEditorDialog.h/cpp
│   │   │   ├── ExperienceEditorDialog.h/cpp
│   │   │   ├── ActivityEditorDialog.h/cpp
│   │   │   ├── JobEditorDialog.h/cpp
│   │   │   └── PeerEditorDialog.h/cpp
│   │   │
│   │   ├── utils/             # 工具层
│   │   │   ├── UiHelpers.h/cpp
│   │   │   └── ResumeHelpers.h/cpp
│   │   │
│   │   └── MainWindow.h/cpp   # 主窗口
│   │
│   ├── service/               # 服务层
│   │   ├── AchievementService.h
│   │   ├── ActivityService.h
│   │   ├── AiService.h/cpp
│   │   ├── AnalyticsService.h
│   │   ├── AuthService.h
│   │   ├── CourseService.h
│   │   ├── DashboardService.h
│   │   ├── ExperienceService.h
│   │   ├── GoalService.h
│   │   ├── ImportService.h
│   │   ├── JobService.h
│   │   ├── ResumeService.h
│   │   └── RoleService.h
│   │
│   ├── dao/                   # 数据访问层
│   │   ├── DaoBase.h/cpp
│   │   ├── AchievementDao.h
│   │   ├── ActivityDao.h
│   │   ├── CourseDao.h
│   │   ├── ExperienceDao.h
│   │   ├── GoalDao.h
│   │   ├── JobDao.h
│   │   ├── PeerBenchmarkDao.h
│   │   ├── RoleDao.h
│   │   └── UserDao.h
│   │
│   ├── model/                 # 数据模型层
│   │   ├── Achievement.h
│   │   ├── Activity.h
│   │   ├── Course.h/cpp
│   │   ├── Experience.h
│   │   ├── Goal.h
│   │   ├── Job.h
│   │   ├── JobRequirement.h
│   │   ├── PeerBenchmark.h
│   │   ├── Role.h
│   │   └── User.h
│   │
│   ├── server/                # 服务器层
│   │   └── HttpServer.h
│   │
│   ├── util/                  # 通用工具
│   │   ├── JsonUtils.h
│   │   └── Logger.h
│   │
│   ├── config/                # 配置
│   │   └── Version.h.in
│   │
│   ├── main.cpp               # 服务器入口
│   └── main_desktop.cpp       # 桌面端入口
│
├── resources/                 # 资源文件
│   ├── schema.sql            # 数据库架构
│   └── resources.qrc         # Qt资源文件
│
├── tests/                     # 测试
│   ├── smoke_test.cpp
│   └── crud_regression.py
│
├── docs/                      # 文档
│   ├── ARCHITECTURE.md       # 架构文档
│   ├── COMPONENTS.md         # 组件文档
│   ├── DEVELOPMENT_GUIDE.md  # 开发指南
│   └── REGRESSION_CHECKLIST.md
│
└── CMakeLists.txt            # 构建配置
```

---

## 3. 核心设计模式

### 3.1 MVC 模式

```
Model (数据模型)
  ↓
Service (业务逻辑)
  ↓
Page/Widget (视图)
  ↓
User Interaction
```

**示例：课程管理**
```cpp
// Model
struct Course {
    int id;
    QString name;
    double credits;
    // ...
};

// Service
class CourseService {
    static QVector<Course> getAll();
    static Course create(const Course& course);
    static Course update(int id, const Course& course);
    static bool remove(int id);
};

// View
class CoursesPage : public BasePage {
    void refresh() {
        QVector<Course> courses = CourseService::getAll();
        displayCourses(courses);
    }
};
```

### 3.2 信号槽模式

用于组件间解耦通信：

```cpp
// 发送方
class CoursesPage : public BasePage {
signals:
    void dataChanged(DataDomain domain);
};

// 接收方
class DataRefreshCoordinator {
public slots:
    void refreshByDomain(DataDomain domain);
};

// 连接
connect(coursesPage, &CoursesPage::dataChanged,
        coordinator, &DataRefreshCoordinator::refreshByDomain);
```

### 3.3 协调器模式

用于管理复杂的跨组件交互：

```cpp
class DataRefreshCoordinator {
    // 绑定所有页面
    void bindPages(...);
    
    // 统一刷新逻辑
    void refreshByDomain(DataDomain domain) {
        switch(domain) {
            case DataDomain::Courses:
                m_overview->refresh();
                m_analysis->refresh();
                m_timeline->refresh();
                m_resume->refresh();
                break;
            // ...
        }
    }
};
```

### 3.4 中介者模式

用于 AI 上下文管理：

```cpp
class AiContextMediator {
    // 监听全局文本选择
    bool eventFilter(QObject* watched, QEvent* event) override;
    
    // 推送到 AI 面板
    void pushSelectionToPanel(const QString& text);
};
```

---

## 4. 数据流向

### 4.1 CRUD 操作流程

```
User Action (点击新增)
    ↓
Dialog (输入数据)
    ↓
Service::create() (业务逻辑)
    ↓
DAO::insert() (数据库操作)
    ↓
emit dataChanged(domain) (发送信号)
    ↓
DataRefreshCoordinator (协调刷新)
    ↓
Multiple Pages refresh() (更新UI)
```

### 4.2 页面刷新流程

```
Page A: emit dataChanged(DataDomain::Courses)
    ↓
DataRefreshCoordinator::refreshByDomain(Courses)
    ↓
├─→ OverviewPage::refresh()
├─→ AnalysisPage::refresh()
├─→ TimelinePage::refresh()
└─→ ResumePage::refresh()
```

### 4.3 AI 分析流程

```
User: 点击"综合分析"
    ↓
AiPanelWidget::analysisRequested("general")
    ↓
MainWindow (信号处理)
    ↓
AiService::analyze(payload)
    ↓
├─→ 尝试远程 AI 服务
└─→ 失败则使用本地规则引擎
    ↓
返回分析结果
    ↓
AiPanelWidget::setOutput(result)
```

---

## 5. 关键组件说明

### 5.1 MainWindow (主窗口)

**职责：**
- 应用程序壳层
- 菜单栏、工具栏、状态栏管理
- 页面容器和路由

**关键方法：**
```cpp
void setupUi();              // 初始化UI
void setupMenuBar();         // 设置菜单
void onNavigationChanged(int index);  // 导航切换
```

### 5.2 AppShellController (壳层控制器)

**职责：**
- 管理页面切换时的壳层行为
- 控制顶部栏显示
- 调整内容区宽度

**关键方法：**
```cpp
void onPageChanged(int index);
void updateTopbarForPage(int index);
void updateContentWidthForPage(int index);
```

### 5.3 DataRefreshCoordinator (刷新协调器)

**职责：**
- 统一管理跨页面数据刷新
- 避免循环刷新
- 优化刷新性能

**关键方法：**
```cpp
void bindPages(...);
void refreshByDomain(DataDomain domain);
void refreshAll();
```

### 5.4 BackendRuntimeController (后端控制器)

**职责：**
- 管理后端服务生命周期
- 处理启动、停止、错误
- 管理托盘图标

**关键方法：**
```cpp
void startBackendServer();
void stopBackendServer();
void onBackendStarted();
void onBackendError(const QString& error);
```

### 5.5 AiContextMediator (AI上下文中介)

**职责：**
- 监听用户文本选择
- 推送上下文到 AI 面板
- 管理上下文生命周期

**关键方法：**
```cpp
bool eventFilter(QObject* watched, QEvent* event);
void pushSelectionToPanel(const QString& text);
void clearSelection();
```

---

## 6. 数据库设计

### 6.1 核心表结构

```sql
-- 课程表
CREATE TABLE courses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    code TEXT,
    credits REAL,
    semester TEXT,
    category TEXT,
    score REAL,
    status TEXT,
    teacher TEXT,
    location TEXT,
    description TEXT,
    tags TEXT
);

-- 目标表
CREATE TABLE goals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    category TEXT,
    description TEXT,
    target_value REAL,
    current_value REAL,
    unit TEXT,
    deadline TEXT,
    priority TEXT,
    status TEXT,
    milestones TEXT
);

-- 角色表
CREATE TABLE roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    type TEXT,
    organization TEXT,
    description TEXT,
    start_date TEXT,
    end_date TEXT,
    is_active INTEGER,
    achievements TEXT,
    contact TEXT,
    supervisor TEXT
);

-- 成果表
CREATE TABLE achievements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    type TEXT,
    level TEXT,
    date TEXT,
    description TEXT,
    issuer TEXT,
    certificate_number TEXT,
    is_verified INTEGER,
    attachment_url TEXT
);

-- 经历表
CREATE TABLE experiences (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    type TEXT,
    organization TEXT,
    role TEXT,
    description TEXT,
    start_date TEXT,
    end_date TEXT,
    is_ongoing INTEGER,
    technologies TEXT,
    achievements TEXT,
    supervisor TEXT,
    contact TEXT,
    location TEXT,
    url TEXT
);

-- 活动表
CREATE TABLE activities (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    category TEXT,
    start_date TEXT,
    end_date TEXT,
    is_favorite INTEGER,
    is_active INTEGER,
    tags TEXT
);

-- 岗位表
CREATE TABLE jobs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    company TEXT,
    location TEXT,
    salary_range TEXT,
    description TEXT,
    requirements TEXT,
    status TEXT,
    applied_date TEXT,
    deadline TEXT,
    contact TEXT,
    url TEXT,
    notes TEXT
);

-- 岗位要求表
CREATE TABLE job_requirements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    job_id INTEGER,
    requirement TEXT,
    is_met INTEGER,
    notes TEXT,
    FOREIGN KEY (job_id) REFERENCES jobs(id)
);

-- 同学对标表
CREATE TABLE peer_benchmarks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    gpa REAL,
    achievements_count INTEGER,
    internships_count INTEGER,
    projects_count INTEGER,
    notes TEXT
);
```

### 6.2 数据访问模式

```cpp
// DAO 基类
class DaoBase {
protected:
    static QSqlDatabase getDatabase();
    static QSqlQuery executeQuery(const QString& sql);
};

// 具体 DAO
class CourseDao {
public:
    static QVector<Course> selectAll();
    static Course selectById(int id);
    static int insert(const Course& course);
    static bool update(int id, const Course& course);
    static bool deleteById(int id);
};
```

---

## 7. 扩展性设计

### 7.1 添加新页面

1. 创建页面类继承 `BasePage`
2. 实现 `refresh()` 方法
3. 在 `MainWindow::setupUi()` 中添加到 `QStackedWidget`
4. 在 `DataRefreshCoordinator` 中绑定刷新逻辑
5. 在导航列表中添加入口

### 7.2 添加新数据实体

1. 在 `src/model/` 创建模型类
2. 在 `src/dao/` 创建 DAO 类
3. 在 `src/service/` 创建 Service 类
4. 在 `src/api/` 创建 API 类（如需HTTP接口）
5. 在 `resources/schema.sql` 添加表结构
6. 创建对应的编辑对话框
7. 创建对应的页面

### 7.3 添加新组件

1. 在 `src/client/widgets/` 创建组件类
2. 继承合适的 Qt 基类（QWidget/QFrame等）
3. 实现 UI 构建和信号槽
4. 在 `CMakeLists.txt` 中添加源文件
5. 在需要的地方引用和使用

---

## 8. 性能优化策略

### 8.1 数据库优化

- 使用索引加速查询
- 批量操作减少事务次数
- 预编译语句提高性能
- 连接池管理

### 8.2 UI 优化

- 延迟加载非可见页面
- 虚拟滚动处理大列表
- 缓存计算结果
- 异步加载数据

### 8.3 内存优化

- 智能指针管理生命周期
- 及时释放不用的资源
- 避免循环引用
- 使用对象池

---

## 9. 安全性考虑

### 9.1 数据安全

- SQL 注入防护（使用参数化查询）
- 输入验证和清理
- 敏感数据加密存储

### 9.2 应用安全

- 权限控制
- 错误信息不泄露敏感信息
- 日志脱敏

---

## 10. 未来演进方向

### 10.1 短期目标

- [ ] 完善测试覆盖
- [ ] 优化性能瓶颈
- [ ] 增强错误处理
- [ ] 完善文档

### 10.2 中期目标

- [ ] 支持多用户
- [ ] 云端同步
- [ ] 移动端适配
- [ ] 插件系统

### 10.3 长期目标

- [ ] 微服务架构
- [ ] 分布式部署
- [ ] 大数据分析
- [ ] AI 深度集成

---

## 附录

### A. 技术选型理由

**为什么选择 Qt？**
- 跨平台支持（Windows/macOS/Linux）
- 成熟的 GUI 框架
- 丰富的组件库
- 优秀的性能
- 活跃的社区

**为什么选择 SQLite？**
- 轻量级、无需配置
- 单文件数据库
- 支持事务
- 跨平台
- 适合桌面应用

**为什么选择 C++？**
- 高性能
- 与 Qt 深度集成
- 丰富的生态
- 学习价值高

### B. 参考资源

- [Qt 官方文档](https://doc.qt.io/)
- [SQLite 文档](https://www.sqlite.org/docs.html)
- [C++ 核心指南](https://isocpp.github.io/CppCoreGuidelines/)
- [设计模式](https://refactoring.guru/design-patterns)

---

**文档版本**: v1.0  
**最后更新**: 2026-04-29  
**维护者**: 项目团队
