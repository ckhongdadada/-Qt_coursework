# 开发者指南

本指南帮助开发者快速上手项目开发，了解开发流程、代码规范和最佳实践。

---

## 目录

1. [环境搭建](#1-环境搭建)
2. [项目结构](#2-项目结构)
3. [开发流程](#3-开发流程)
4. [代码规范](#4-代码规范)
5. [常见任务](#5-常见任务)
6. [调试技巧](#6-调试技巧)
7. [测试指南](#7-测试指南)
8. [发布流程](#8-发布流程)

---

## 1. 环境搭建

### 1.1 必需工具

**基础环境：**
- Qt 6.5+ (包含 Qt Creator)
- CMake 3.25+
- C++17 编译器
  - Windows: MinGW 13.1+ 或 MSVC 2019+
  - macOS: Xcode 14+
  - Linux: GCC 11+ 或 Clang 14+

**可选工具：**
- Git (版本控制)
- Qt Designer (UI 设计)
- Valgrind (内存检查，Linux)
- Doxygen (文档生成)

### 1.2 Qt 安装

**Windows:**
```bash
# 下载 Qt 在线安装器
# https://www.qt.io/download-qt-installer

# 安装时选择组件：
- Qt 6.5.x
  - MinGW 13.1.0 64-bit
  - Qt HTTP Server
  - Qt SQL
  - Qt Widgets
  - Qt PrintSupport
  - Qt Test
- Qt Creator
- CMake
- Ninja
```

**macOS:**
```bash
# 使用 Homebrew
brew install qt@6
brew install cmake

# 或使用官方安装器
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential
sudo apt install libqt6sql6-sqlite libqt6httpserver6-dev
```

### 1.3 克隆项目

```bash
git clone <repository-url>
cd cpp_project
```

### 1.4 构建项目

**使用 Qt Creator:**
1. 打开 Qt Creator
2. File → Open File or Project
3. 选择 `CMakeLists.txt`
4. 配置 Kit (选择 Qt 6.5+ 和编译器)
5. 点击 Build 按钮

**使用命令行:**

```bash
# Windows (MinGW)
mkdir build_desktop
cd build_desktop
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.5.3/mingw_64
mingw32-make

# macOS/Linux
mkdir build_desktop
cd build_desktop
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
make

# 运行
./pdp_desktop  # Linux/macOS
pdp_desktop.exe  # Windows
```

**使用批处理脚本 (Windows):**
```bash
.\build_desktop.bat
```

### 1.5 验证安装

```bash
# 检查 Qt 版本
qmake --version

# 检查 CMake 版本
cmake --version

# 运行测试
cd build_desktop
ctest
```

---

## 2. 项目结构

### 2.1 源码组织

```
src/
├── api/           # HTTP API 路由
├── client/        # 客户端代码
│   ├── core/     # 核心协调器
│   ├── pages/    # 页面组件
│   ├── widgets/  # UI 组件
│   ├── dialogs/  # 对话框
│   └── utils/    # 工具函数
├── dao/           # 数据访问层
├── model/         # 数据模型
├── service/       # 业务逻辑
├── server/        # HTTP 服务器
└── util/          # 通用工具
```

### 2.2 命名约定

**文件命名：**
- 类文件：`ClassName.h` / `ClassName.cpp`
- 头文件保护：`CLASSNAME_H`

**类命名：**
- 类名：`PascalCase` (如 `MainWindow`)
- 成员变量：`m_camelCase` (如 `m_sidebar`)
- 函数：`camelCase` (如 `refreshData()`)
- 常量：`kPascalCase` (如 `kMaxWidth`)

**信号和槽：**
- 信号：动词过去式 (如 `dataChanged`)
- 槽：`on` + 动作 (如 `onDataChanged`)

### 2.3 代码组织

**头文件结构：**
```cpp
#ifndef CLASSNAME_H
#define CLASSNAME_H

// Qt 头文件
#include <QWidget>
#include <QString>

// 项目头文件
#include "model/Course.h"

class ClassName : public QWidget {
    Q_OBJECT  // 如果使用信号槽

public:
    // 构造函数
    explicit ClassName(QWidget* parent = nullptr);
    
    // 析构函数
    ~ClassName() override = default;
    
    // 公共方法
    void publicMethod();

signals:
    // 信号
    void dataChanged();

public slots:
    // 公共槽
    void onDataChanged();

private slots:
    // 私有槽
    void onButtonClicked();

private:
    // 私有方法
    void setupUi();
    void privateMethod();
    
    // 成员变量
    QWidget* m_widget = nullptr;
    QString m_data;
};

#endif // CLASSNAME_H
```

**实现文件结构：**
```cpp
#include "ClassName.h"

#include <QVBoxLayout>
#include <QLabel>

ClassName::ClassName(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void ClassName::setupUi()
{
    // UI 初始化
}

void ClassName::publicMethod()
{
    // 实现
}

void ClassName::privateMethod()
{
    // 实现
}
```

---

## 3. 开发流程

### 3.1 Git 工作流

**分支策略：**
```
main          # 主分支，稳定版本
├── develop   # 开发分支
├── feature/* # 功能分支
├── bugfix/*  # 修复分支
└── release/* # 发布分支
```

**开发流程：**
```bash
# 1. 创建功能分支
git checkout develop
git pull origin develop
git checkout -b feature/new-feature

# 2. 开发和提交
git add .
git commit -m "feat: add new feature"

# 3. 推送到远程
git push origin feature/new-feature

# 4. 创建 Pull Request
# 在 GitHub/GitLab 上创建 PR

# 5. 代码审查通过后合并
git checkout develop
git merge feature/new-feature
git push origin develop

# 6. 删除功能分支
git branch -d feature/new-feature
```

### 3.2 提交信息规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

```
<type>(<scope>): <subject>

<body>

<footer>
```

**类型 (type):**
- `feat`: 新功能
- `fix`: 修复 bug
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 重构
- `test`: 测试相关
- `chore`: 构建/工具相关

**示例：**
```bash
feat(courses): add course import functionality

- Add CSV import support
- Add validation for course data
- Update UI to show import progress

Closes #123
```

### 3.3 代码审查清单

**提交前检查：**
- [ ] 代码编译通过
- [ ] 所有测试通过
- [ ] 代码符合规范
- [ ] 添加了必要的注释
- [ ] 更新了相关文档
- [ ] 没有调试代码（如 `qDebug()`）
- [ ] 没有未使用的变量/函数
- [ ] 内存泄漏检查

---

## 4. 代码规范

### 4.1 C++ 规范

**基本原则：**
- 遵循 [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- 使用 C++17 特性
- 优先使用 Qt 容器和工具

**示例：**

```cpp
// ✅ 好的做法
QString name = "John";
QVector<int> numbers = {1, 2, 3};
auto result = calculateResult();

// ❌ 避免
char* name = "John";  // 使用 QString
std::vector<int> numbers;  // 使用 QVector
int result = calculateResult();  // 使用 auto
```

### 4.2 Qt 规范

**信号槽连接：**
```cpp
// ✅ 推荐：新式连接
connect(button, &QPushButton::clicked,
        this, &MainWindow::onButtonClicked);

// ❌ 避免：旧式连接
connect(button, SIGNAL(clicked()),
        this, SLOT(onButtonClicked()));
```

**内存管理：**
```cpp
// ✅ 好的做法：使用父子关系
QWidget* widget = new QWidget(parent);

// ✅ 好的做法：使用智能指针
auto ptr = std::make_unique<MyClass>();

// ❌ 避免：裸指针
QWidget* widget = new QWidget();
// ... 忘记 delete
```

**字符串处理：**
```cpp
// ✅ 好的做法
QString text = QString("Hello %1").arg(name);

// ❌ 避免
QString text = "Hello " + name;  // 效率低
```

### 4.3 代码格式

**缩进：**
- 使用 4 个空格
- 不使用 Tab

**大括号：**
```cpp
// ✅ 推荐
if (condition) {
    doSomething();
}

// ❌ 避免
if (condition)
{
    doSomething();
}
```

**空格：**
```cpp
// ✅ 好的做法
int x = 5;
if (x > 0) {
    func(a, b, c);
}

// ❌ 避免
int x=5;
if(x>0){
    func(a,b,c);
}
```

### 4.4 注释规范

**文件头注释：**
```cpp
/**
 * @file ClassName.h
 * @brief 类的简要描述
 * @author 作者名
 * @date 2026-04-29
 */
```

**类注释：**
```cpp
/**
 * @brief 课程管理页面
 * 
 * 提供课程的增删改查功能，支持搜索、筛选和排序。
 */
class CoursesPage : public BasePage {
    // ...
};
```

**函数注释：**
```cpp
/**
 * @brief 刷新课程列表
 * 
 * 从数据库重新加载所有课程并更新 UI。
 * 
 * @note 此方法会清空当前选择
 */
void refresh();
```

**行内注释：**
```cpp
// 计算 GPA（保留两位小数）
double gpa = totalScore / courseCount;

// TODO: 添加异常处理
// FIXME: 修复内存泄漏
// NOTE: 此处逻辑较复杂，需要重构
```

---

## 5. 常见任务

### 5.1 添加新页面

**步骤：**

1. **创建页面类**

```cpp
// src/client/pages/NewPage.h
#ifndef NEWPAGE_H
#define NEWPAGE_H

#include "BasePage.h"

class NewPage : public BasePage {
    Q_OBJECT

public:
    explicit NewPage(QWidget* parent = nullptr);
    void refresh() override;

private:
    void setupUi();
};

#endif
```

```cpp
// src/client/pages/NewPage.cpp
#include "NewPage.h"
#include <QVBoxLayout>

NewPage::NewPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
}

void NewPage::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    // 添加 UI 组件
}

void NewPage::refresh()
{
    // 刷新数据
}
```

2. **在 CMakeLists.txt 中添加**

```cmake
qt_add_executable(pdp_desktop
    # ...
    src/client/pages/NewPage.cpp
    # ...
)
```

3. **在 MainWindow 中集成**

```cpp
// MainWindow.h
#include "client/pages/NewPage.h"

// MainWindow.cpp
void MainWindow::setupUi()
{
    // ...
    m_stack->addWidget(new NewPage(this));
    // ...
}
```

4. **添加导航入口**

在 `SidebarWidget.cpp` 的 `navBaseLabels()` 中添加标签。

5. **配置刷新逻辑**

在 `DataRefreshCoordinator` 中添加刷新逻辑。

### 5.2 添加新对话框

**步骤：**

1. **创建对话框类**

```cpp
// src/client/dialogs/NewDialog.h
#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
#include "model/NewModel.h"

class QLineEdit;
class QTextEdit;

class NewDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewDialog(QWidget* parent = nullptr);
    
    void setData(const NewModel& data);
    NewModel data() const;
    void save();
    void showNear(QWidget* anchor);

private:
    void setupUi();
    
    QLineEdit* m_nameEdit = nullptr;
    QTextEdit* m_descEdit = nullptr;
};

#endif
```

2. **实现对话框**

```cpp
#include "NewDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

NewDialog::NewDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

void NewDialog::setupUi()
{
    setWindowTitle("编辑");
    resize(500, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();
    
    m_nameEdit = new QLineEdit(this);
    form->addRow("名称:", m_nameEdit);
    
    m_descEdit = new QTextEdit(this);
    form->addRow("描述:", m_descEdit);
    
    layout->addLayout(form);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void NewDialog::setData(const NewModel& data)
{
    m_nameEdit->setText(data.name);
    m_descEdit->setPlainText(data.description);
}

NewModel NewDialog::data() const
{
    NewModel model;
    model.name = m_nameEdit->text();
    model.description = m_descEdit->toPlainText();
    return model;
}

void NewDialog::save()
{
    // 保存到 QSettings 或其他地方
}

void NewDialog::showNear(QWidget* anchor)
{
    if (anchor) {
        QPoint pos = anchor->mapToGlobal(QPoint(0, anchor->height()));
        move(pos);
    }
    show();
}
```

### 5.3 添加新数据模型

**步骤：**

1. **定义模型**

```cpp
// src/model/NewModel.h
#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <QString>

struct NewModel {
    int id = 0;
    QString name;
    QString description;
    QString createdAt;
    QString updatedAt;
};

#endif
```

2. **创建 DAO**

```cpp
// src/dao/NewModelDao.h
#ifndef NEWMODELDAO_H
#define NEWMODELDAO_H

#include "DaoBase.h"
#include "model/NewModel.h"
#include <QVector>

class NewModelDao : public DaoBase {
public:
    static QVector<NewModel> selectAll();
    static NewModel selectById(int id);
    static int insert(const NewModel& model);
    static bool update(int id, const NewModel& model);
    static bool deleteById(int id);
};

#endif
```

3. **创建 Service**

```cpp
// src/service/NewModelService.h
#ifndef NEWMODELSERVICE_H
#define NEWMODELSERVICE_H

#include "model/NewModel.h"
#include <QVector>
#include <QJsonObject>

class NewModelService {
public:
    static QVector<NewModel> getAll();
    static NewModel getById(int id);
    static NewModel create(const NewModel& model);
    static NewModel update(int id, const NewModel& model);
    static bool remove(int id);
    static QJsonObject getStatistics();
};

#endif
```

4. **更新数据库架构**

在 `resources/schema.sql` 中添加表定义：

```sql
CREATE TABLE IF NOT EXISTS new_models (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP
);
```

### 5.4 添加新 API 端点

```cpp
// src/api/NewModelApi.h
#ifndef NEWMODELAPI_H
#define NEWMODELAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>

class NewModelApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest& request);
    static QHttpServerResponse getById(int id, const QHttpServerRequest& request);
    static QHttpServerResponse create(const QHttpServerRequest& request);
    static QHttpServerResponse update(int id, const QHttpServerRequest& request);
    static QHttpServerResponse remove(int id, const QHttpServerRequest& request);
};

#endif
```

---

## 6. 调试技巧

### 6.1 使用 qDebug()

```cpp
#include <QDebug>

void MyClass::myMethod()
{
    qDebug() << "Method called";
    qDebug() << "Value:" << value;
    qDebug() << "Object:" << object;
}
```

### 6.2 Qt Creator 调试器

**设置断点：**
1. 在代码行号左侧点击
2. F5 启动调试
3. F10 单步执行
4. F11 进入函数

**查看变量：**
- Locals 窗口：查看局部变量
- Expressions 窗口：添加监视表达式

### 6.3 内存泄漏检测

**Windows (Visual Studio):**
```cpp
#include <crtdbg.h>

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // ...
}
```

**Linux (Valgrind):**
```bash
valgrind --leak-check=full ./pdp_desktop
```

### 6.4 性能分析

**Qt Creator Profiler:**
1. Analyze → QML Profiler
2. 运行应用
3. 查看性能报告

**自定义计时：**
```cpp
#include <QElapsedTimer>

QElapsedTimer timer;
timer.start();

// 执行操作

qDebug() << "Elapsed:" << timer.elapsed() << "ms";
```

---

## 7. 测试指南

### 7.1 单元测试

**创建测试：**

```cpp
// tests/CourseServiceTest.cpp
#include <QtTest>
#include "service/CourseService.h"

class CourseServiceTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testGetAll();
    void testCreate();
    void testUpdate();
    void testDelete();
};

void CourseServiceTest::initTestCase()
{
    // 初始化测试数据库
}

void CourseServiceTest::testGetAll()
{
    QVector<Course> courses = CourseService::getAll();
    QVERIFY(!courses.isEmpty());
}

void CourseServiceTest::testCreate()
{
    Course course;
    course.name = "Test Course";
    course.credits = 3.0;
    
    Course created = CourseService::create(course);
    QVERIFY(created.id > 0);
    QCOMPARE(created.name, course.name);
}

QTEST_MAIN(CourseServiceTest)
#include "CourseServiceTest.moc"
```

**运行测试：**
```bash
cd build_desktop
ctest
```

### 7.2 集成测试

```cpp
// tests/MainWindowIntegrationTest.cpp
void MainWindowIntegrationTest::testNavigationFlow()
{
    MainWindow window;
    window.show();
    
    // 模拟导航
    QTest::mouseClick(navigationButton, Qt::LeftButton);
    
    // 验证页面切换
    QCOMPARE(window.currentPageIndex(), 1);
}
```

### 7.3 回归测试

参考 `docs/REGRESSION_CHECKLIST.md` 进行手动回归测试。

---

## 8. 发布流程

### 8.1 版本号管理

版本号格式：`MAJOR.MINOR.PATCH`

- MAJOR: 重大变更
- MINOR: 新功能
- PATCH: Bug 修复

**更新版本：**
```bash
# 编辑 VERSION 文件
echo "0.4.0" > VERSION

# 更新 CHANGELOG.md
```

### 8.2 构建发布版本

**Windows:**
```bash
.\build_desktop.bat
cd build_desktop
windeployqt pdp_desktop.exe
```

**macOS:**
```bash
./build_desktop.sh
cd build_desktop
macdeployqt pdp_desktop.app
```

**Linux:**
```bash
./build_desktop.sh
cd build_desktop
# 使用 linuxdeployqt 或打包为 AppImage
```

### 8.3 打包

**Windows (Inno Setup):**
```iss
[Setup]
AppName=学业发展规划系统
AppVersion=0.4.0
DefaultDirName={pf}\PDP
OutputDir=installer
OutputBaseFilename=pdp-setup-0.4.0

[Files]
Source: "build_desktop\*"; DestDir: "{app}"; Flags: recursesubdirs
```

**macOS (DMG):**
```bash
hdiutil create -volname "PDP" -srcfolder build_desktop/pdp_desktop.app -ov -format UDZO pdp-0.4.0.dmg
```

### 8.4 发布清单

- [ ] 所有测试通过
- [ ] 更新版本号
- [ ] 更新 CHANGELOG
- [ ] 构建发布版本
- [ ] 测试安装包
- [ ] 创建 Git tag
- [ ] 推送到远程仓库
- [ ] 发布 Release Notes

---

## 9. 常见问题

### Q1: 编译错误：找不到 Qt 头文件

**解决：**
```bash
# 设置 CMAKE_PREFIX_PATH
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
```

### Q2: 运行时错误：找不到 Qt DLL

**解决：**
```bash
# Windows
windeployqt pdp_desktop.exe

# 或添加 Qt bin 目录到 PATH
set PATH=%PATH%;C:\Qt\6.5.3\mingw_64\bin
```

### Q3: 中文乱码

**解决：**
- 确保源文件使用 UTF-8 编码
- CMakeLists.txt 中已添加编译选项：
  ```cmake
  add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8)
  ```

### Q4: 数据库锁定错误

**解决：**
```cpp
// 确保及时关闭数据库连接
QSqlQuery query = dao.executeQuery(sql);
// 使用完毕后
query.finish();
```

---

## 10. 资源链接

**官方文档：**
- [Qt Documentation](https://doc.qt.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [SQLite Documentation](https://www.sqlite.org/docs.html)

**学习资源：**
- [Qt for Beginners](https://wiki.qt.io/Qt_for_Beginners)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)

**社区：**
- [Qt Forum](https://forum.qt.io/)
- [Stack Overflow - Qt](https://stackoverflow.com/questions/tagged/qt)
- [Qt Discord](https://discord.gg/qt)

---

**文档版本**: v1.0  
**最后更新**: 2026-04-29  
**维护者**: 项目团队

**贡献者：**
欢迎提交 Issue 和 Pull Request！
