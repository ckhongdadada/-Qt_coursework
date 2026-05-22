# 个人发展规划系统 - Stage 4

本仓库记录了一个 C++ 个人发展规划系统的分阶段开发过程。

Stage 4 添加高级功能：简历编辑器、数据分析页面、数据导入工具和 AI 智能助手。

## Stage 4 范围

- 简历编辑器面板 (`ResumeEditorPanel`)
- 简历预览对话框 (`ResumePreviewDialog`)
- 简历候选素材面板 (`ResumeCandidatePanel`)
- 分析页面 (`AnalysisPage`) - 数据分析与可视化
- 数据导入页面 (`ImportsPage`)
- AI 助手面板 (`AiPanelWidget`)
- AI 对话组件 (`AiConversationWidget`)
- AI 状态栏 (`AiStatusBar`)
- AI 上下文中介器 (`AiContextMediator`)
- 同行基准分析组件 (`PeerBenchmarkWidget`)
- 本地 AI 模型服务入口 (`ai_server.py`)

## Stage 3 -> Stage 4 变化

Stage 3 交付了完整的业务 CRUD 功能。Stage 4 添加了高级功能，包括简历生成、数据分析和 AI 助手集成。

## 项目结构

```text
src/
  client/
    core/
      AiContextMediator.*     AI 上下文中介器
    pages/
      AnalysisPage.*          数据分析页面
      ImportsPage.*           数据导入页面
      ResumePage.*            简历页面
    widgets/
      AiConversationWidget.*  AI 对话组件
      AiPanelWidget.*         AI 助手面板
      AiStatusBar.*           AI 状态栏
      PeerBenchmarkWidget.*   同行基准分析组件
      ResumeCandidatePanel.*  简历候选素材面板
      ResumeEditorPanel.*     简历编辑器面板
      ResumePreviewDialog.*   简历预览对话框
      ResumePreviewWidget.*   简历预览组件
resources/
  icons/                      图标资源
  resources.qrc               Qt 资源文件
ai_server.py                 AI 服务端脚本
requirements.txt             Python 依赖
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

如需使用 AI 助手功能，请先启动 Python 服务：

```powershell
pip install -r requirements.txt
python ai_server.py
```

## 后续阶段

- **Stage 5**: 报告、文档、演示材料和最终提交包