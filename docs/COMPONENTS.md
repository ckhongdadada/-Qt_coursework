# 组件使用文档

本文档详细说明系统中各个组件的使用方法、API 接口和最佳实践。

---

## 目录

1. [核心组件](#1-核心组件)
2. [页面组件](#2-页面组件)
3. [UI 组件](#3-ui-组件)
4. [对话框组件](#4-对话框组件)
5. [工具类](#5-工具类)

---

## 1. 核心组件

### 1.1 AppShellController

**功能**: 管理应用程序壳层行为，包括页面切换、顶部栏更新、内容区宽度调整。

**头文件**: `src/client/core/AppShellController.h`

#### API

```cpp
class AppShellController : public QObject {
public:
    explicit AppShellController(QObject* parent = nullptr);
    
    // 绑定主窗口
    void attach(MainWindow* window);
    
    // 页面切换回调
    void onPageChanged(int index);
    
    // 更新顶部栏
    void updateTopbarForPage(int index);
    
    // 更新内容宽度
    void updateContentWidthForPage(int index);
    
    // 更新壳层间距
    void updateShellSpacingForPage(int index);
};
```

#### 使用示例

```cpp
// 在 MainWindow 构造函数中
m_shellController = new AppShellController(this);
m_shellController->attach(this);

// 连接导航信号
connect(m_sidebar->navigationList(), &QListWidget::currentRowChanged,
        m_shellController, &AppShellController::onPageChanged);
```

#### 注意事项

- 必须在 UI 初始化完成后调用 `attach()`
- 页面索引从 0 开始
- 简历页（索引 10）会使用特殊的宽版布局

---

### 1.2 DataRefreshCoordinator

**功能**: 统一管理跨页面数据刷新，避免重复刷新和循环依赖。

**头文件**: `src/client/core/DataRefreshCoordinator.h`

#### API

```cpp
class DataRefreshCoordinator : public QObject {
public:
    explicit DataRefreshCoordinator(QObject* parent = nullptr);
    
    // 绑定所有页面
    void bindPages(
        OverviewPage* overview,
        CoursesPage* courses,
        RolesPage* roles,
        AchievementsPage* achievements,
        ExperiencesPage* experiences,
        ActivitiesPage* activities,
        GoalsPage* goals,
        JobsPage* jobs,
        AnalysisPage* analysis,
        TimelinePage* timeline,
        ResumePage* resume,
        ImportsPage* imports
    );
    
    // 连接信号
    void connectSignals();
    
public slots:
    // 按域刷新
    void refreshByDomain(DataDomain domain);
    
    // 全量刷新
    void refreshAll();
};
```

#### 数据域定义

```cpp
enum class DataDomain {
    Courses,        // 课程
    Goals,          // 目标
    Roles,          // 角色
    Achievements,   // 成果
    Experiences,    // 经历
    Activities,     // 活动
    Jobs,           // 岗位
    Analysis,       // 分析
    Resume,         // 简历
    Timeline,       // 时间轴
    All             // 全部
};
```

#### 刷新联动关系

| 触发域 | 刷新页面 |
|--------|----------|
| Courses | Overview, Analysis, Timeline, Resume |
| Goals | Overview, Analysis, Timeline, Resume |
| Roles | Overview, Analysis, Resume |
| Achievements | Overview, Analysis, Resume |
| Experiences | Overview, Analysis, Timeline, Resume |
| Activities | Overview, Analysis, Timeline, Resume |
| Jobs | Overview, Analysis, Timeline |
| All | 所有页面 |

#### 使用示例

```cpp
// 在页面中发送数据变更信号
class CoursesPage : public BasePage {
signals:
    void dataChanged(DataDomain domain);
    
private slots:
    void onCourseAdded() {
        // 保存课程后
        emit dataChanged(DataDomain::Courses);
    }
};

// 在 MainWindow 中连接
m_refreshCoordinator->bindPages(...);
m_refreshCoordinator->connectSignals();
```

#### 注意事项

- 必须先调用 `bindPages()` 再调用 `connectSignals()`
- 导入完成后应使用 `refreshAll()` 而不是 `refreshByDomain()`
- 避免在刷新方法中再次发送 `dataChanged` 信号

---

### 1.3 BackendRuntimeController

**功能**: 管理后端服务的生命周期，包括启动、停止、状态监控。

**头文件**: `src/client/core/BackendRuntimeController.h`

#### API

```cpp
class BackendRuntimeController : public QObject {
public:
    explicit BackendRuntimeController(QObject* parent = nullptr);
    
    // 启动后端服务
    void startBackendServer();
    
    // 停止后端服务
    void stopBackendServer();
    
    // 检查前端资源
    void checkFrontendExists();
    
    // 打开浏览器
    void openBrowser();
    
    // 插入示例数据
    void insertSampleDataIfNeeded();
    
    // 更新后端状态徽章
    void updateBackendBadge(bool ready, const QString& detail = QString());
    
signals:
    void backendStarted();
    void backendError(const QString& error);
};
```

#### 使用示例

```cpp
// 在 MainWindow 构造函数中
m_backendController = new BackendRuntimeController(this);

connect(m_backendController, &BackendRuntimeController::backendStarted,
        this, &MainWindow::onBackendStarted);
connect(m_backendController, &BackendRuntimeController::backendError,
        this, &MainWindow::onBackendError);

m_backendController->startBackendServer();
```

---

### 1.4 AiContextMediator

**功能**: 监听用户文本选择，自动推送到 AI 面板作为上下文。

**头文件**: `src/client/core/AiContextMediator.h`

#### API

```cpp
class AiContextMediator : public QObject {
public:
    explicit AiContextMediator(QObject* parent = nullptr);
    
    // 绑定根窗口
    void bindRootWidget(QWidget* root);
    
    // 绑定 AI 面板
    void attachPanel(AiPanelWidget* panel);
    
    // 事件过滤器
    bool eventFilter(QObject* watched, QEvent* event) override;
    
    // 推送选择到面板
    void pushSelectionToPanel(const QString& text);
    
    // 清空选择
    void clearSelection();
};
```

#### 使用示例

```cpp
// 在 MainWindow 中
m_aiMediator = new AiContextMediator(this);
m_aiMediator->bindRootWidget(centralWidget());
m_aiMediator->attachPanel(m_aiPanel);
```

#### 注意事项

- 只有长度在 5-2000 字符之间的文本会被捕获
- 支持 QTextEdit 和 QListWidget 的选择
- 选择后会自动显示在 AI 面板的上下文区域

---

### 1.5 CrudPageController

**功能**: 提供 CRUD 页面的通用控制逻辑，减少重复代码。

**头文件**: `src/client/core/CrudPageController.h`

#### API

```cpp
class CrudPageController : public QObject {
public:
    explicit CrudPageController(QObject* parent = nullptr);
    
    // 绑定搜索框
    void bindSearchField(QLineEdit* field);
    
    // 绑定筛选组件
    void bindFilterWidget(QWidget* widget);
    
    // 刷新视图
    void refreshView();
    
    // 恢复选择
    void restoreSelection();
    
    // 编辑当前项
    void requestEditCurrentItem();
    
    // 删除当前项
    void requestRemoveCurrentItem();
    
signals:
    void searchTextChanged(const QString& text);
    void filterChanged();
    void editRequested(int id);
    void removeRequested(int id);
};
```

---

## 2. 页面组件

### 2.1 BasePage

**功能**: 所有页面的基类，提供通用接口。

**头文件**: `src/client/pages/BasePage.h`

#### API

```cpp
class BasePage : public QWidget {
    Q_OBJECT
    
public:
    explicit BasePage(QWidget* parent = nullptr);
    virtual ~BasePage() = default;
    
    // 刷新页面数据（子类必须实现）
    virtual void refresh() = 0;
    
signals:
    // 数据变更信号
    void dataChanged(DataDomain domain);
};
```

#### 使用示例

```cpp
class CoursesPage : public BasePage {
    Q_OBJECT
    
public:
    explicit CoursesPage(QWidget* parent = nullptr);
    
    void refresh() override {
        // 重新加载课程数据
        QVector<Course> courses = CourseService::getAll();
        displayCourses(courses);
    }
    
private:
    void displayCourses(const QVector<Course>& courses);
};
```

---

### 2.2 CoursesPage

**功能**: 课程管理页面，支持 CRUD 操作、搜索、筛选、排序。

**头文件**: `src/client/pages/CoursesPage.h`

#### 主要功能

- 课程列表展示（表格形式）
- 新增/编辑/删除课程
- 按名称、状态、类别搜索
- 按学期、成绩排序
- 统计信息展示（总数、GPA、学分）

#### 使用示例

```cpp
// 创建页面
CoursesPage* coursesPage = new CoursesPage(this);

// 连接数据变更信号
connect(coursesPage, &CoursesPage::dataChanged,
        coordinator, &DataRefreshCoordinator::refreshByDomain);

// 刷新数据
coursesPage->refresh();
```

---

### 2.3 GoalsPage

**功能**: 目标管理页面，支持目标追踪和进度管理。

**头文件**: `src/client/pages/GoalsPage.h`

#### 主要功能

- 目标列表展示
- 进度可视化
- 按状态、优先级筛选
- 里程碑管理
- 统计信息（总数、完成数、平均进度）

---

### 2.4 ResumePage

**功能**: 简历编辑和导出页面。

**头文件**: `src/client/pages/ResumePage.h`

#### 主要功能

- 实时预览
- 多字段编辑
- 备选素材管理
- 多格式导出（JSON/HTML/PDF）
- 模板切换

#### 子组件

- **ResumePreviewWidget**: 简历预览
- **ResumeEditorPanel**: 编辑面板
- **ResumeCandidatePanel**: 备选素材

---

## 3. UI 组件

### 3.1 SidebarWidget

**功能**: 左侧导航栏，支持折叠/展开。

**头文件**: `src/client/widgets/SidebarWidget.h`

#### API

```cpp
class SidebarWidget : public QFrame {
    Q_OBJECT
    
public:
    explicit SidebarWidget(QWidget* parent = nullptr);
    
    // 刷新数据
    void refreshData();
    
    // 获取导航列表
    QListWidget* navigationList() const;
    
signals:
    // 导航请求
    void navigationRequested(int index);
};
```

#### 使用示例

```cpp
SidebarWidget* sidebar = new SidebarWidget(this);
sidebar->refreshData();

connect(sidebar->navigationList(), &QListWidget::currentRowChanged,
        this, &MainWindow::onNavigationChanged);
```

---

### 3.2 AiPanelWidget

**功能**: 右侧 AI 助手面板，支持分析、对话、建议。

**头文件**: `src/client/widgets/AiPanelWidget.h`

#### API

```cpp
class AiPanelWidget : public QFrame {
    Q_OBJECT
    
public:
    explicit AiPanelWidget(QWidget* parent = nullptr);
    
    // 刷新状态
    void refreshStatus();
    
    // 设置上下文
    void setContext(const QString& type, const QString& context);
    
    // 清空上下文
    void clearContext();
    
    // 获取当前输出
    QString currentOutput() const;
    
    // 设置输出
    void setOutput(const QString& text);
    
signals:
    // 应用到简历
    void applyToResumeRequested(const QString& summary);
    
    // 创建目标
    void createGoalRequested(const QString& title, const QString& description);
    
    // 分析请求
    void analysisRequested(const QString& type);
    
    // 聊天消息
    void chatMessageSent(const QString& message);
};
```

#### 使用示例

```cpp
AiPanelWidget* aiPanel = new AiPanelWidget(this);

connect(aiPanel, &AiPanelWidget::analysisRequested,
        this, &MainWindow::onAiAnalysisRequested);

connect(aiPanel, &AiPanelWidget::applyToResumeRequested,
        this, &MainWindow::onApplyToResume);
```

---

### 3.3 ToastNotification

**功能**: 轻量级提示通知。

**头文件**: `src/client/widgets/ToastNotification.h`

#### API

```cpp
class ToastNotification : public QWidget {
public:
    // 显示提示
    static void display(QWidget* parent, const QString& message);
};
```

#### 使用示例

```cpp
// 成功提示
ToastNotification::display(this, "课程已保存");

// 错误提示
ToastNotification::display(this, "保存失败，请重试");

// 警告提示
ToastNotification::display(this, "请先选择一个项目");
```

#### 特性

- 自动居中显示
- 2.5秒后自动消失
- 淡入淡出动画
- 不阻塞用户操作

---

### 3.4 MetricGridWidget

**功能**: 指标卡片网格布局。

**头文件**: `src/client/widgets/MetricGridWidget.h`

#### API

```cpp
class MetricGridWidget : public QWidget {
public:
    explicit MetricGridWidget(QWidget* parent = nullptr);
    
    // 添加指标卡片
    void addMetric(const QString& label, const QString& value, 
                   const QString& helper = QString());
    
    // 更新指标值
    void updateMetric(int index, const QString& value);
    
    // 清空所有指标
    void clear();
};
```

---

### 3.5 NavigationListWidget

**功能**: 导航列表，支持图标和文本模式切换。

**头文件**: `src/client/widgets/NavigationListWidget.h`

#### API

```cpp
class NavigationListWidget : public QListWidget {
public:
    explicit NavigationListWidget(QWidget* parent = nullptr);
    
    // 设置折叠状态
    void setCollapsed(bool collapsed);
    
    // 是否折叠
    bool isCollapsed() const;
    
    // 重建项目
    void rebuildItems();
    
    // 同步选择样式
    void syncSelectionStyle();
};
```

---

## 4. 对话框组件

### 4.1 对话框基本模式

所有编辑对话框遵循统一模式：

```cpp
class XxxEditorDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit XxxEditorDialog(QWidget* parent = nullptr);
    
    // 设置数据（编辑模式）
    void setXxx(const Xxx& xxx);
    
    // 获取数据
    Xxx xxx() const;
    
    // 保存数据
    void save();
    
    // 在指定位置附近显示
    void showNear(QWidget* anchor);
};
```

### 4.2 CourseEditorDialog

**功能**: 课程编辑对话框。

**头文件**: `src/client/dialogs/CourseEditorDialog.h`

#### 使用示例

```cpp
// 新增模式
CourseEditorDialog dialog(this);
dialog.setWindowTitle("新增课程");
if (dialog.exec() == QDialog::Accepted) {
    Course course = dialog.course();
    CourseService::create(course);
}

// 编辑模式
Course existing = CourseService::getById(id);
CourseEditorDialog dialog(this);
dialog.setWindowTitle("编辑课程");
dialog.setCourse(existing);
if (dialog.exec() == QDialog::Accepted) {
    Course updated = dialog.course();
    CourseService::update(id, updated);
}
```

---

### 4.3 GoalEditorDialog

**功能**: 目标编辑对话框，支持进度跟踪。

**头文件**: `src/client/dialogs/GoalEditorDialog.h`

#### 特殊功能

- 进度条可视化
- 里程碑管理
- 优先级选择
- 截止日期选择

---

## 5. 工具类

### 5.1 UiHelpers

**功能**: UI 相关的辅助函数。

**头文件**: `src/client/utils/UiHelpers.h`

#### API

```cpp
namespace UiHelpers {
    // 创建标签
    QLabel* createLabel(const QString& text, QWidget* parent = nullptr);
    
    // 创建按钮
    QPushButton* createButton(const QString& text, QWidget* parent = nullptr);
    
    // 创建输入框
    QLineEdit* createLineEdit(const QString& placeholder, QWidget* parent = nullptr);
    
    // 设置空状态
    void setupEmptyState(QListWidget* list, const QString& hint);
    
    // 格式化日期范围
    QString formatDateRange(const QString& start, const QString& end, 
                           bool active, const QString& activeLabel);
}
```

---

### 5.2 ResumeHelpers

**功能**: 简历相关的辅助函数。

**头文件**: `src/client/utils/ResumeHelpers.h`

#### API

```cpp
namespace ResumeHelpers {
    // HTML 转纯文本
    QString htmlToPlainText(const QString& html);
    
    // 生成简历 HTML
    QString generateResumeHtml(const QJsonObject& options);
    
    // 导出为 PDF
    bool exportToPdf(const QString& html, const QString& filePath);
}
```

---

## 6. 最佳实践

### 6.1 组件通信

**推荐：使用信号槽**
```cpp
// 好的做法
connect(sender, &Sender::dataChanged,
        receiver, &Receiver::onDataChanged);
```

**避免：直接调用**
```cpp
// 不推荐
receiver->onDataChanged();  // 紧耦合
```

### 6.2 内存管理

**推荐：使用父子关系**
```cpp
// 好的做法
QWidget* widget = new QWidget(parent);  // parent 会自动管理
```

**避免：手动 delete**
```cpp
// 不推荐
QWidget* widget = new QWidget();
// ... 使用后需要手动 delete
delete widget;
```

### 6.3 数据刷新

**推荐：使用协调器**
```cpp
// 好的做法
emit dataChanged(DataDomain::Courses);
```

**避免：直接调用多个页面**
```cpp
// 不推荐
overviewPage->refresh();
analysisPage->refresh();
timelinePage->refresh();
// ... 容易遗漏
```

### 6.4 错误处理

**推荐：用户友好的提示**
```cpp
// 好的做法
if (!CourseService::create(course)) {
    ToastNotification::display(this, "保存失败，请检查输入");
    return;
}
```

**避免：静默失败**
```cpp
// 不推荐
CourseService::create(course);  // 失败了用户不知道
```

---

## 7. 常见问题

### Q1: 如何添加新的导航项？

A: 在 `SidebarWidget.cpp` 的 `navBaseLabels()` 函数中添加标签，并在 `MainWindow::setupUi()` 中添加对应页面。

### Q2: 如何自定义 Toast 样式？

A: 修改 `ToastNotification.cpp` 中的 `setStyleSheet()` 调用。

### Q3: 如何添加新的数据域？

A: 在 `DataDomain.h` 中添加枚举值，并在 `DataRefreshCoordinator::refreshByDomain()` 中添加对应逻辑。

### Q4: 如何调试信号槽连接？

A: 使用 `qDebug()` 输出，或在 Qt Creator 中使用调试器的信号槽视图。

---

**文档版本**: v1.0  
**最后更新**: 2026-04-29  
**维护者**: 项目团队
