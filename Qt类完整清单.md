# Qt 类完整清单 — PDP 学业发展规划系统（桌面端）

---

## 一、自定义类继承关系（项目中继承了 Qt 类的所有子类）

### 1.1 继承自 `QMainWindow`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `MainWindow` | `client/MainWindow.h` | 主窗口，持有 QStackedWidget、侧边栏、AI 面板、系统托盘等 |

### 1.2 继承自 `QWidget`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `BasePage` | `client/pages/BasePage.h` | 页面基类，纯虚函数 `refresh()` + `createMetricCard()` 工厂方法 |
| `ToastNotification` | `client/widgets/ToastNotification.h` | 消息提示组件，QPropertyAnimation 滑入滑出 |

### 1.3 继承自 `QFrame`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `SidebarWidget` | `client/widgets/SidebarWidget.h` | 侧边栏，折叠动画 |
| `AiPanelWidget` | `client/widgets/AiPanelWidget.h` | AI 面板（状态 + 对话 + 建议） |
| `AiConversationWidget` | `client/widgets/AiConversationWidget.h` | AI 对话消息列表 |
| `AiStatusBar` | `client/widgets/AiStatusBar.h` | AI 状态指示栏 |
| `CrudPageShell` | `client/widgets/CrudPageShell.h` | CRUD 页面外壳（搜索 + 表格 + 分页） |
| `MetricGridWidget` | `client/widgets/MetricGridWidget.h` | 指标卡片网格 |
| `PeerBenchmarkWidget` | `client/widgets/PeerBenchmarkWidget.h` | 同学对照组件 |
| `ResumeCandidatePanel` | `client/widgets/ResumeCandidatePanel.h` | 简历候选素材面板 |
| `ResumeEditorPanel` | `client/widgets/ResumeEditorPanel.h` | 简历编辑面板（18 个表单字段） |
| `SemesterAnalysisWidget` | `client/widgets/SemesterAnalysisWidget.h` | 学期分析组件 |
| `SuggestionListWidget` | `client/widgets/SuggestionListWidget.h` | AI 建议列表 |
| `TimeInfoCard` | `client/widgets/TimeInfoCard.h` | 时间信息卡片 |

### 1.4 继承自 `QDialog`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `ProfileEditorDialog` | `client/dialogs/ProfileEditorDialog.h` | 个人信息编辑 |
| `CourseEditorDialog` | `client/dialogs/CourseEditorDialog.h` | 课程编辑 |
| `RoleEditorDialog` | `client/dialogs/RoleEditorDialog.h` | 角色编辑 |
| `AchievementEditorDialog` | `client/dialogs/AchievementEditorDialog.h` | 成就编辑 |
| `ExperienceEditorDialog` | `client/dialogs/ExperienceEditorDialog.h` | 经历编辑 |
| `ActivityEditorDialog` | `client/dialogs/ActivityEditorDialog.h` | 活动编辑 |
| `GoalEditorDialog` | `client/dialogs/GoalEditorDialog.h` | 目标编辑 |
| `JobEditorDialog` | `client/dialogs/JobEditorDialog.h` | 岗位编辑 |
| `PeerEditorDialog` | `client/dialogs/PeerEditorDialog.h` | 同学数据编辑 |
| `ResumePreviewDialog` | `client/widgets/ResumePreviewDialog.h` | 简历预览弹窗 |

### 1.5 继承自 `QObject`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `AppShellController` | `client/core/AppShellController.h` | 页面切换控制器 |
| `DataRefreshCoordinator` | `client/core/DataRefreshCoordinator.h` | 数据刷新协调器 |
| `BackendRuntimeController` | `client/core/BackendRuntimeController.h` | 后端进程管理 |
| `AiContextMediator` | `client/core/AiContextMediator.h` | AI 上下文中介器（eventFilter） |
| `CrudPageController` | `client/core/CrudPageController.h` | CRUD 通用控制器（模板） |

### 1.6 继承自 `QListWidget`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `NavigationListWidget` | `client/widgets/NavigationListWidget.h` | 侧边栏导航列表 |

### 1.7 继承自 `QPushButton`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `StudentInfoCard` | `client/widgets/StudentInfoCard.h` | 学生信息卡片（可点击） |

### 1.8 继承自 `QTextBrowser`

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `ResumePreviewWidget` | `client/widgets/ResumePreviewWidget.h` | 简历 HTML 预览（setHtml 渲染） |

### 1.9 继承自 `BasePage`（间接继承 `QWidget`）

| 自定义类 | 文件 | 说明 |
|---------|------|------|
| `OverviewPage` | `client/pages/OverviewPage.h` | 总览仪表盘 |
| `CoursesPage` | `client/pages/CoursesPage.h` | 课程管理 |
| `RolesPage` | `client/pages/RolesPage.h` | 角色任职 |
| `AchievementsPage` | `client/pages/AchievementsPage.h` | 成就荣誉 |
| `ExperiencesPage` | `client/pages/ExperiencesPage.h` | 实践经历 |
| `ActivitiesPage` | `client/pages/ActivitiesPage.h` | 课外活动 |
| `GoalsPage` | `client/pages/GoalsPage.h` | 目标规划 |
| `JobsPage` | `client/pages/JobsPage.h` | 岗位追踪 |
| `AnalysisPage` | `client/pages/AnalysisPage.h` | 数据分析 |
| `TimelinePage` | `client/pages/TimelinePage.h` | 时间线 |
| `ResumePage` | `client/pages/ResumePage.h` | 简历生成 |
| `ImportsPage` | `client/pages/ImportsPage.h` | 数据导入 |

---

## 二、Qt 内置类使用清单（按模块分类）

### 2.1 Core 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QObject` | 所有 Qt 对象基类，信号槽基础 | client, service |
| `QTimer` | 定时器（防抖 300ms、自动刷新） | client, service |
| `QSettings` | 持久化键值存储（学生信息、窗口状态） | client |
| `QJsonObject` | JSON 对象（Model 序列化、API 通信） | client, model, service, util |
| `QJsonArray` | JSON 数组（tags、milestones） | client, dao, model, service, util |
| `QJsonDocument` | JSON 文档解析/生成 | dao, service, util |
| `QMap` | 有序键值映射（页面注册表） | client, model, service |
| `QList` | 动态数组（数据集合） | client, dao, model, service |
| `QSet` | 无序集合 | client |
| `QString` | 字符串 | 全部模块 |
| `QStringList` | 字符串列表（tags） | client, model, root, service |
| `QVariant` | 通用值类型（DAO 空值处理） | dao |
| `QDate` / `QDateTime` / `QTime` | 日期时间 | client, model, service, util |

### 2.2 Widgets 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QMainWindow` | 主窗口基类 | client |
| `QWidget` | 所有 UI 组件基类 | client |
| `QFrame` | 卡片容器、分割线 | client |
| `QDialog` | 模态对话框基类 | client |
| `QStackedWidget` | 页面切换容器（12 个页面） | client |
| `QScrollArea` | 可滚动区域 | client |
| `QSplitter` | 可拖拽分割面板 | client |
| `QLabel` | 文本/图片显示 | client |
| `QPushButton` | 按钮 | client |
| `QLineEdit` | 单行文本输入 | client |
| `QTextEdit` | 多行文本输入 | client |
| `QTextBrowser` | 富文本显示（简历预览） | client |
| `QComboBox` | 下拉选择框 | client |
| `QCheckBox` | 复选框 | client |
| `QSpinBox` | 整数输入框 | client |
| `QDoubleSpinBox` | 浮点数输入框 | client |
| `QListWidget` | 列表组件 | client |
| `QTableWidget` | 表格组件 | client |
| `QTableWidgetItem` | 表格单元格 | client |
| `QProgressBar` | 进度条 | client |
| `QToolBar` | 工具栏 | client |
| `QMenu` | 右键菜单 | client |
| `QAction` | 菜单/工具栏动作 | client |
| `QMessageBox` | 消息弹窗（确认/警告/错误） | client, root |
| `QInputDialog` | 输入弹窗 | client |
| `QHeaderView` | 表头（列宽、排序） | client |
| `QAbstractItemView` | 列表/表格视图基类（选择行为） | client |

### 2.3 Layout 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QHBoxLayout` | 水平布局 | client |
| `QVBoxLayout` | 垂直布局 | client |
| `QGridLayout` | 网格布局（指标卡片） | client |
| `QFormLayout` | 表单布局（编辑对话框） | client |

### 2.4 Animation 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QPropertyAnimation` | 属性动画（侧边栏折叠、Toast 滑入） | client |
| `QParallelAnimationGroup` | 并行动画组（多属性同时动画） | client |
| `QEasingCurve` | 缓动曲线（InOutCubic） | client |
| `QGraphicsOpacityEffect` | 透明度效果（Toast 淡入淡出） | client |
| `QGraphicsDropShadowEffect` | 阴影效果（悬浮菜单阴影） | client |

### 2.5 Sql 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QSqlDatabase` | 数据库连接管理 | dao, root |
| `QSqlQuery` | SQL 执行（prepare + bindValue + exec） | dao, root |
| `QSqlError` | 数据库错误信息 | dao, root |

### 2.6 Network 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QNetworkAccessManager` | HTTP 客户端（AI 服务调用） | service |
| `QNetworkRequest` | HTTP 请求对象 | service |
| `QNetworkReply` | HTTP 响应对象 | service |
| `QUrl` | URL 解析 | client, service |

### 2.7 Gui 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QClipboard` | 系统剪贴板（复制简历 HTML） | client |
| `QDesktopServices` | 系统服务（打开浏览器/文件管理器） | client |
| `QFileDialog` | 文件选择对话框（导出/选择头像） | client |
| `QSystemTrayIcon` | 系统托盘图标 | client |
| `QFont` | 字体设置 | client |
| `QColor` | 颜色 | client |
| `QPixmap` / `QImage` | 图片加载（头像） | client |
| `QIcon` | 图标 | client |
| `QMimeData` | 拖拽数据 | client |
| `QApplication` / `QGuiApplication` | 应用实例 | client, root |
| `QScreen` | 屏幕信息（悬浮菜单边界检测） | client |

### 2.8 Event 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QEvent` | 事件基类（eventFilter 拦截） | client |
| `QMouseEvent` | 鼠标事件（文本选中检测） | client |
| `QDragEnterEvent` / `QDropEvent` | 拖拽事件（文件导入） | client |

### 2.9 IO 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QFile` | 文件读写 | client, root, service |
| `QTextStream` | 文本流读写 | root, service |
| `QDir` | 目录操作 | client, root |
| `QFileInfo` | 文件信息查询 | client |

### 2.10 Delegate/Rendering 模块

| Qt 类 | 用途 | 使用位置 |
|-------|------|---------|
| `QStyledItemDelegate` | 自定义表格单元格渲染 | client |
| `QPainter` | 自定义绘制（雷达图、进度条） | client |

---

## 三、继承关系总览图

```
QObject
├── AppShellController
├── DataRefreshCoordinator
├── BackendRuntimeController
├── AiContextMediator
├── CrudPageController
├── QTimer (组合使用)
└── QWidget
    ├── BasePage
    │   ├── OverviewPage
    │   ├── CoursesPage
    │   ├── RolesPage
    │   ├── AchievementsPage
    │   ├── ExperiencesPage
    │   ├── ActivitiesPage
    │   ├── GoalsPage
    │   ├── JobsPage
    │   ├── AnalysisPage
    │   ├── TimelinePage
    │   ├── ResumePage
    │   └── ImportsPage
    ├── ToastNotification
    └── QMainWindow
        └── MainWindow

QFrame
├── SidebarWidget
├── AiPanelWidget
├── AiConversationWidget
├── AiStatusBar
├── CrudPageShell
├── MetricGridWidget
├── PeerBenchmarkWidget
├── ResumeCandidatePanel
├── ResumeEditorPanel
├── SemesterAnalysisWidget
├── SuggestionListWidget
└── TimeInfoCard

QDialog
├── ProfileEditorDialog
├── CourseEditorDialog
├── RoleEditorDialog
├── AchievementEditorDialog
├── ExperienceEditorDialog
├── ActivityEditorDialog
├── GoalEditorDialog
├── JobEditorDialog
├── PeerEditorDialog
└── ResumePreviewDialog

QListWidget
└── NavigationListWidget

QPushButton
└── StudentInfoCard

QTextBrowser
└── ResumePreviewWidget
```

---

## 四、统计汇总

| 类别 | 数量 |
|------|------|
| 继承自 QMainWindow | 1 个 |
| 继承自 QWidget（含 BasePage 子类） | 14 个 |
| 继承自 QFrame | 12 个 |
| 继承自 QDialog | 10 个 |
| 继承自 QObject | 5 个 |
| 继承自 QListWidget | 1 个 |
| 继承自 QPushButton | 1 个 |
| 继承自 QTextBrowser | 1 个 |
| **自定义类总计** | **45 个** |
| 使用的 Qt 内置类 | **60+ 个** |

---

**（文档结束）**
