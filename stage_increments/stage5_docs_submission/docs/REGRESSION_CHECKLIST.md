# 回归验证清单

每次代码改动后，请按以下步骤验证功能正常。

## 1. 编译验证

- [ ] CMake 配置成功（无错误）
- [ ] 编译通过（0 errors）
- [ ] 无新增编译警告

```bash
cmake -B build_desktop -S . -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
cmake --build build_desktop
```

## 2. 启动验证

- [ ] 程序启动成功
- [ ] 主窗口正常显示
- [ ] 窗口标题显示版本号
- [ ] 后端服务启动（端口 8080）
- [ ] 浏览器自动弹出前端预览

## 3. 导航验证

- [ ] 左侧导航栏显示正常（NavigationListWidget）
- [ ] 点击各导航项可切换页面
- [ ] 侧边栏展开/收起正常
- [ ] 导航选中态高亮正确
- [ ] 时间信息卡片显示正常（TimeInfoCard）
- [ ] 学生信息卡片显示正常（StudentInfoCard）

## 4. 页面功能验证

### 4.1 总览页（OverviewPage + MetricGridWidget + SuggestionListWidget）
- [ ] 数据统计卡片显示正常（MetricGridWidget）
- [ ] 推荐列表显示正常（SuggestionListWidget）
- [ ] 学期走势显示正常（SuggestionListWidget）

### 4.2 课程库
- [ ] 课程列表显示正常
- [ ] 搜索功能正常
- [ ] 新增/编辑/删除课程正常

### 4.3 角色职责
- [ ] 角色列表显示正常
- [ ] 新增/编辑/删除角色正常

### 4.4 成果记录
- [ ] 成果列表显示正常
- [ ] 新增/编辑/删除成果正常

### 4.5 经历档案
- [ ] 经历列表显示正常
- [ ] 新增/编辑/删除经历正常

### 4.6 课外活动
- [ ] 活动列表显示正常
- [ ] 新增/编辑/删除活动正常

### 4.7 目标追踪
- [ ] 目标列表显示正常
- [ ] 进度显示正常
- [ ] 新增/编辑/删除目标正常

### 4.8 目标岗位
- [ ] 岗位列表显示正常
- [ ] 要求匹配显示正常

### 4.9 分析报告（AnalysisPage + SemesterAnalysisWidget + PeerBenchmarkWidget）
- [ ] 学期趋势表显示正常（SemesterAnalysisWidget）
- [ ] 核心优势/潜在风险/发展建议列表正常
- [ ] 同行对比表显示正常（PeerBenchmarkWidget）
- [ ] 新增/编辑/删除对照同学正常

### 4.10 时间轴
- [ ] 时间轴事件显示正常
- [ ] 建议列表显示正常

### 4.11 简历导出（ResumePage + ResumePreviewWidget + ResumeEditorPanel + ResumeCandidatePanel）
- [ ] 简历预览显示正常（ResumePreviewWidget）
- [ ] 编辑面板正常（ResumeEditorPanel）
- [ ] 候选素材面板正常（ResumeCandidatePanel）
- [ ] 导出 JSON/HTML 正常
- [ ] 分区选中高亮正常
- [ ] 素材插入/追加/替换功能正常

### 4.12 数据导入
- [ ] 文件选择正常
- [ ] 导入功能正常
- [ ] ImportResult 模型正确记录成功/失败/跳过计数

## 5. AI 助手验证（AiPanelWidget + AiStatusBar + AiConversationWidget）

- [ ] AI 助手侧边栏显示正常
- [ ] 展开/收起正常
- [ ] AI 状态显示正确（AiStatusBar：模式/模型/状态/上下文）
- [ ] 发送消息正常（AiConversationWidget）
- [ ] 分析功能正常
- [ ] healthCheck 返回正确状态
- [ ] currentMode 返回 "remote" 或 "rule"
- [ ] isRuleMode / isRemoteMode 互斥正确

## 6. 中文显示验证

- [ ] 所有页面中文显示正常（无乱码）
- [ ] 弹窗标题/按钮文字正常
- [ ] 状态栏文字正常

## 7. 布局验证

- [ ] 无异常横向滚动条
- [ ] 窗口缩放布局正常
- [ ] 最小窗口尺寸限制生效
- [ ] QSplitter 分割比例正常

## 8. 核心控制器验证

- [ ] AppShellController 正确管理壳层布局
- [ ] DataRefreshCoordinator 正确协调数据刷新
- [ ] BackendRuntimeController 正确管理后端进程
- [ ] AiContextMediator 正确管理 AI 上下文
- [ ] CrudPageController 正确执行 CRUD 操作

## 9. 退出验证

- [ ] 点击关闭按钮程序退出
- [ ] 无残留进程

## 10. 自动化测试

### Smoke Tests（C++ Qt Test）
```bash
# 编译并运行 smoke tests
cmake --build build_desktop
./build_desktop/smoke_test
```

### CRUD 回归脚本（Python）
```bash
# 确保后端运行在 http://127.0.0.1:8080
python tests/crud_regression.py --skip-server-check
```

---

## 快速验证脚本

```bash
# 编译并运行
cmake --build build_desktop && ./build_desktop/pdp_desktop.exe
```

## 版本信息

- 验证版本：见 VERSION 文件
- 最后更新：2026-04-29
- 重构阶段：第七阶段完成
