# 个人发展规划系统 - Stage 2

本仓库记录了一个 C++ 个人发展规划系统的分阶段开发过程。

Stage 2 在后端基础架构之上扩展了 Qt 桌面应用外壳和导航布局。UI 聚焦于主窗口框架、侧边栏导航和信息卡片，而业务 CRUD 页面将在后续阶段添加。

## Stage 2 范围

- Qt 桌面应用入口 (`main_desktop.cpp`)
- 主窗口外壳和布局框架
- 侧边栏导航列表组件
- 顶部栏（包含学生信息卡和时间信息卡）
- 概览页面与指标网格
- 嵌入式服务器运行控制器
- Toast 通知系统

## Stage 1 -> Stage 2 变化

Stage 1 交付了独立的后端服务器 (`pdp_server`)。Stage 2 引入了桌面应用外壳，嵌入相同的后端服务并添加主要导航结构。

## 项目结构

```text
src/
  client/
    core/
      AppShellController.*   应用壳层编排
      BackendRuntimeController.* 嵌入式服务器生命周期管理
      DataDomain.h           数据域常量定义
    pages/
      BasePage.*             页面基类
      OverviewPage.*         仪表板概览页面
    widgets/
      MetricGridWidget.*     统计数据网格展示
      NavigationListWidget.* 侧边栏导航列表
      SidebarWidget.*        主侧边栏容器
      StudentInfoCard.*      学生信息卡片
      TimeInfoCard.*         当前学期/时间卡片
      ToastNotification.*    临时通知弹窗
    utils/
      UiHelpers.*            UI 工具函数
    MainWindow.*             主窗口框架
  main_desktop.cpp           桌面应用入口
```

## 构建

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.0\mingw_64
cmake --build build --target pdp_desktop
```

## 运行

```powershell
.\build\pdp_desktop.exe
```

桌面应用会在端口 8080 启动一个嵌入式后端服务器，并打开 Qt 主窗口。

## 后续阶段

- **Stage 3**: 业务页面与 CRUD 对话框（课程、角色、成果、经历、活动、目标、岗位、时间轴）
- **Stage 4**: 简历编辑器、分析页面、数据导入工具和 AI 助手
- **Stage 5**: 报告、文档、演示材料和最终提交包