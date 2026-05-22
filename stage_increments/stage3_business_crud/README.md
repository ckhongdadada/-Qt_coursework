# 个人发展规划系统 - Stage 3

本仓库记录了一个 C++ 个人发展规划系统的分阶段开发过程。

Stage 3 在桌面外壳基础上添加完整的业务页面和 CRUD（增删改查）功能。

## Stage 3 范围

- 课程管理页面 (`CoursesPage`)
- 角色管理页面 (`RolesPage`)
- 成果管理页面 (`AchievementsPage`)
- 经历管理页面 (`ExperiencesPage`)
- 活动管理页面 (`ActivitiesPage`)
- 目标管理页面 (`GoalsPage`)
- 岗位管理页面 (`JobsPage`)
- 时间轴页面 (`TimelinePage`)
- 对应的编辑对话框（ProfileEditorDialog、CourseEditorDialog 等）
- CRUD 页面控制器
- 数据刷新协调器

## Stage 2 -> Stage 3 变化

Stage 2 交付了桌面应用外壳和基本导航。Stage 3 添加了完整的业务功能，包括所有核心数据模型的增删改查操作。

## 项目结构

```text
src/
  client/
    core/
      CrudPageController.*    CRUD 页面控制器基类
      DataRefreshCoordinator.* 数据刷新协调器
    pages/
      CoursesPage.*           课程管理页面
      RolesPage.*             角色管理页面
      AchievementsPage.*      成果管理页面
      ExperiencesPage.*       经历管理页面
      ActivitiesPage.*        活动管理页面
      GoalsPage.*             目标管理页面
      JobsPage.*              岗位管理页面
      TimelinePage.*          时间轴页面
    dialogs/
      ProfileEditorDialog.*   个人资料编辑对话框
      CourseEditorDialog.*    课程编辑对话框
      RoleEditorDialog.*      角色编辑对话框
      AchievementEditorDialog.* 成果编辑对话框
      ExperienceEditorDialog.* 经历编辑对话框
      ActivityEditorDialog.*  活动编辑对话框
      GoalEditorDialog.*      目标编辑对话框
      JobEditorDialog.*       岗位编辑对话框
      PeerEditorDialog.*      同行基准编辑对话框
    widgets/
      CrudPageShell.*         CRUD 页面壳层组件
      SuggestionListWidget.*  建议列表组件
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

## 后续阶段

- **Stage 4**: 简历编辑器、分析页面、数据导入工具和 AI 助手
- **Stage 5**: 报告、文档、演示材料和最终提交包