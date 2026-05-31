# AI 助手使用指南

**版本**: v1.0  
**更新日期**: 2026-04-29  
**适用版本**: v0.3+

---

## 一、功能概述

AI 助手是学业发展规划系统的智能分析模块，提供以下功能：

1. **智能分析**：基于课程、经历、目标等数据生成针对性建议
2. **对话交互**：支持自然语言问答
3. **上下文感知**：自动捕获用户选中的文本作为分析上下文
4. **结果回填**：将AI建议直接应用到简历或目标

---

## 二、界面布局

### 2.1 AI 面板位置

AI 面板位于主窗口**右侧**，默认展开状态，宽度 380px。

```
┌─────────────────────────────────────────────────────┐
│  侧边栏  │      主内容区域      │   AI 面板 (右侧)  │
│  (左)    │      (中间)          │   (380px)         │
└─────────────────────────────────────────────────────┘
```

### 2.2 面板组成

```
┌─────────────────────────────────────┐
│  AI 建议            [刷新] [收起]   │  ← 标题栏
├─────────────────────────────────────┤
│  说明文字                            │
├─────────────────────────────────────┤
│  [模式] [状态] [模型]               │  ← 状态栏
├─────────────────────────────────────┤
│  [上下文显示区域]                    │  ← 上下文（可选）
├─────────────────────────────────────┤
│  [对话] [建议]                       │  ← 模式切换
├─────────────────────────────────────┤
│  [综合] [课程] [经历] [目标]        │  ← 快捷按钮
├─────────────────────────────────────┤
│  [重新生成] [清空上下文]            │  ← 辅助按钮
├─────────────────────────────────────┤
│  ┌───────────────────────────────┐  │
│  │  分析结果                      │  │
│  │  (文本显示区域)                │  │  ← 对话区域
│  │                                │  │
│  └───────────────────────────────┘  │
│  [回填摘要] [转为目标]              │  ← 回填按钮
├─────────────────────────────────────┤
│  [输入问题...]           [发送]     │  ← 输入框
└─────────────────────────────────────┘
```

---

## 三、使用方式

### 3.1 快捷分析（建议模式）

**步骤：**

1. 点击顶部的 **[建议]** 标签（默认选中）
2. 点击快捷按钮之一：
   - **综合**：全面学业分析
   - **课程**：课程规划建议
   - **经历**：实践经历提升建议
   - **目标**：目标完成策略
3. 等待分析完成（显示"正在分析中，请稍候..."）
4. 查看分析结果

**示例输出：**
```
=== 综合学业分析 ===

当前状态：
- 已完成课程：15门
- 总学分：45.0
- 平均GPA：3.5
- 活跃目标：3个

建议：
1. 课程方面：建议选修高级算法课程...
2. 实践方面：建议参加更多项目实践...
3. 目标方面：建议设定短期可达成目标...
```

### 3.2 对话交互（对话模式）

**步骤：**

1. 点击顶部的 **[对话]** 标签
2. 在底部输入框输入问题
3. 点击 **[发送]** 或按 **Enter** 键
4. 查看AI回复

**示例对话：**
```
用户：我应该如何提高GPA？

AI：根据您的当前情况，建议：
1. 重点关注核心课程
2. 合理安排学习时间
3. 寻求老师和同学帮助
```

### 3.3 上下文感知分析

**自动捕获上下文：**

1. 在主内容区域**选中任意文本**（5-2000字符）
2. 松开鼠标后，AI面板会自动显示上下文
3. 上下文显示在状态栏下方的绿色框中
4. 点击快捷按钮时，AI会结合上下文生成建议

**示例：**
```
┌─────────────────────────────────────┐
│  上下文 [选中内容]：数据结构课程... │  ← 自动捕获
└─────────────────────────────────────┘
```

**清空上下文：**
- 点击 **[清空上下文]** 按钮

### 3.4 结果回填

**回填到简历：**

1. 生成AI建议后
2. 点击 **[回填摘要]** 按钮
3. 切换到简历页面
4. AI建议会自动填充到"个人总结"字段

**转为目标：**

1. 生成AI建议后
2. 点击 **[转为目标]** 按钮
3. 自动打开目标编辑对话框
4. 标题为"AI 建议目标"，描述为AI建议内容
5. 编辑后保存

---

## 四、状态指示

### 4.1 状态栏说明

| 标签 | 含义 | 可能值 |
|------|------|--------|
| **模式** | AI运行模式 | `rule`（规则模式）、`llm`（大模型模式） |
| **状态** | AI服务状态 | `可用`、`不可用` |
| **模型** | 使用的模型 | `local-rule-based`（本地规则）、`gpt-4`等 |

### 4.2 模式说明

#### 规则模式（Rule-based）
- **特点**：本地运行，无需网络
- **优点**：快速、稳定、隐私
- **缺点**：建议相对固定，缺乏灵活性
- **适用场景**：快速获取标准化建议

#### 大模型模式（LLM）
- **特点**：调用远程AI服务
- **优点**：智能、灵活、个性化
- **缺点**：需要网络、可能较慢
- **适用场景**：复杂问题、深度分析

**当前版本**：仅支持规则模式

---

## 五、常见问题

### 5.1 为什么上下文没有自动捕获？

**可能原因：**
1. 选中的文本太短（<5字符）或太长（>2000字符）
2. 选中的不是文本内容（如图片、按钮）
3. 选中的是AI面板内的文本

**解决方法：**
- 确保选中5-2000字符的文本
- 在主内容区域（课程列表、目标列表等）选择文本
- 手动点击 **[清空上下文]** 后重新选择

### 5.2 为什么点击快捷按钮没有反应？

**可能原因：**
1. AI服务未启动
2. 数据库中没有数据

**解决方法：**
1. 点击 **[刷新]** 按钮更新状态
2. 确保已添加课程、目标等数据
3. 查看状态栏是否显示"可用"

### 5.3 为什么回填摘要失败？

**可能原因：**
1. 简历页面未初始化
2. 没有生成AI建议

**解决方法：**
1. 先切换到简历页面，确保页面已加载
2. 生成AI建议后再点击回填
3. 查看Toast通知的错误提示

### 5.4 为什么对话没有回复？

**可能原因：**
1. 当前仅支持规则模式，对话功能有限
2. 输入的问题为空

**解决方法：**
- 使用快捷按钮获取建议（更可靠）
- 输入具体问题（如"如何提高GPA"）
- 等待后续版本支持LLM模式

---

## 六、设计问题与改进建议

### 6.1 当前存在的问题

#### 问题1：模式切换不明确 ⚠️

**现象：**
- 点击 **[对话]** 和 **[建议]** 标签后，界面没有明显变化
- 用户不清楚当前处于哪种模式
- 两种模式的功能区分不明显

**原因：**
- 两个标签只改变了按钮样式，没有改变功能区域
- 快捷按钮和输入框同时显示，容易混淆

**建议改进：**
```cpp
// 建议模式：显示快捷按钮，隐藏输入框
connect(suggTabBtn, &QPushButton::clicked, this, [...]() {
    // 显示快捷按钮区域
    actionGrid->show();
    helperActionLayout->show();
    // 隐藏输入框
    inputCard->hide();
    // 更新提示文本
    setOutput("点击上方快捷按钮获取针对性建议...");
});

// 对话模式：隐藏快捷按钮，显示输入框
connect(chatTabBtn, &QPushButton::clicked, this, [...]() {
    // 隐藏快捷按钮区域
    actionGrid->hide();
    helperActionLayout->hide();
    // 显示输入框
    inputCard->show();
    // 更新提示文本
    setOutput("AI 助手已就绪。可以在下方直接输入问题进行对话。");
});
```

#### 问题2：上下文捕获过于敏感 ⚠️

**现象：**
- 用户在任何地方选中文本都会触发上下文捕获
- 可能捕获到不相关的内容
- 没有明确的"使用上下文"确认步骤

**原因：**
- `AiContextMediator` 监听所有鼠标释放事件
- 自动推送到AI面板，没有用户确认

**建议改进：**
```cpp
// 方案1：添加确认按钮
void AiContextMediator::pushSelectionToPanel(const QString& text) {
    if (m_panel) {
        // 显示上下文，但不自动使用
        m_panel->showContextPreview(text);
        // 用户需要点击"使用此上下文"按钮确认
    }
}

// 方案2：仅在AI面板展开时捕获
bool AiContextMediator::eventFilter(QObject* watched, QEvent* event) {
    if (!m_panel || !m_panel->isExpanded()) {
        return QObject::eventFilter(watched, event);
    }
    // ... 原有逻辑
}
```

#### 问题3：回填按钮始终可见 ⚠️

**现象：**
- 即使没有生成AI建议，回填按钮也显示
- 点击后才提示错误，用户体验不佳

**原因：**
- 按钮状态没有根据内容动态更新

**建议改进：**
```cpp
void AiConversationWidget::setOutput(const QString& text) {
    if (m_output) {
        m_output->setPlainText(text);
        
        // 根据内容启用/禁用回填按钮
        bool hasContent = !text.isEmpty() && 
                         !text.startsWith("AI 助手已就绪") &&
                         !text.startsWith("正在分析中");
        m_toResumeButton->setEnabled(hasContent);
        m_toGoalButton->setEnabled(hasContent);
    }
}
```

#### 问题4：重新生成按钮功能不明确 ⚠️

**现象：**
- 点击 **[重新生成]** 总是执行"综合"分析
- 用户期望重新生成上一次的分析类型

**原因：**
- 没有记录上一次的分析类型

**建议改进：**
```cpp
class AiPanelWidget {
private:
    QString m_lastAnalysisType = "general";  // 记录上次类型
    
    void runAiAnalysis(const QString& type) {
        m_lastAnalysisType = type;  // 保存类型
        emit analysisRequested(type);
    }
};

// 重新生成按钮
connect(rerunButton, &QPushButton::clicked, this, [this]() {
    runAiAnalysis(m_lastAnalysisType);  // 使用上次类型
});
```

#### 问题5：状态刷新不及时 ⚠️

**现象：**
- 首次打开AI面板时，状态可能显示不正确
- 需要手动点击刷新

**原因：**
- 状态只在特定时机刷新（页面切换、手动刷新）

**建议改进：**
```cpp
AiPanelWidget::AiPanelWidget(QWidget* parent) {
    // ... 构造函数
    
    // 延迟刷新状态（等待UI完全初始化）
    QTimer::singleShot(100, this, [this]() {
        refreshStatus();
        setOutput("AI 助手已就绪。\n点击上方快捷按钮获取建议，或在下方输入问题。");
    });
}
```

### 6.2 用户体验改进建议

#### 改进1：添加加载动画

```cpp
void AiPanelWidget::runAiAnalysis(const QString& type) {
    // 显示加载动画
    m_conversation->showLoadingAnimation();
    emit analysisRequested(type);
}
```

#### 改进2：添加历史记录

```cpp
class AiPanelWidget {
private:
    QStringList m_analysisHistory;  // 保存历史记录
    
    void addToHistory(const QString& type, const QString& result) {
        m_analysisHistory.append(QString("[%1] %2: %3")
            .arg(QDateTime::currentDateTime().toString("HH:mm"))
            .arg(type)
            .arg(result.left(50)));
    }
};
```

#### 改进3：添加快捷键支持

```cpp
// Ctrl+Enter 发送消息
m_chatInput->installEventFilter(this);

bool AiConversationWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_chatInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return && 
            keyEvent->modifiers() & Qt::ControlModifier) {
            sendChatMessage();
            return true;
        }
    }
    return QFrame::eventFilter(obj, event);
}
```

#### 改进4：添加结果导出

```cpp
// 添加导出按钮
QPushButton* exportButton = new QPushButton("导出", this);
connect(exportButton, &QPushButton::clicked, this, [this]() {
    QString filename = QFileDialog::getSaveFileName(
        this, "导出AI建议", "", "文本文件 (*.txt)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(m_output->toPlainText().toUtf8());
            file.close();
            ToastNotification::display(this, "导出成功");
        }
    }
});
```

---

## 七、最佳实践

### 7.1 获取最佳建议

1. **先添加数据**：确保已添加课程、目标、经历等数据
2. **使用上下文**：选中相关文本后再点击快捷按钮
3. **选择合适类型**：
   - 课程规划 → 点击 **[课程]**
   - 职业发展 → 点击 **[经历]**
   - 目标管理 → 点击 **[目标]**
   - 全面分析 → 点击 **[综合]**

### 7.2 高效使用流程

```
1. 添加数据（课程、目标等）
   ↓
2. 在列表中选中相关项目
   ↓
3. AI面板自动捕获上下文
   ↓
4. 点击对应的快捷按钮
   ↓
5. 查看分析结果
   ↓
6. 回填到简历或创建目标
```

### 7.3 避免常见错误

❌ **错误做法：**
- 没有数据就点击分析
- 选中无关文本作为上下文
- 期望对话模式有复杂推理能力

✅ **正确做法：**
- 先添加足够的数据
- 选中相关文本或清空上下文
- 使用快捷按钮获取标准化建议

---

## 八、技术架构

### 8.1 组件关系

```
MainWindow
  ├── AiPanelWidget (AI面板容器)
  │     ├── AiStatusBar (状态栏)
  │     └── AiConversationWidget (对话区域)
  │
  └── AiContextMediator (上下文中介)
        └── 监听全局文本选择
```

### 8.2 信号流

```
用户操作
  ↓
AiPanelWidget::analysisRequested(type)
  ↓
MainWindow (处理信号)
  ↓
AiService::analyze(payload)
  ↓
返回结果
  ↓
AiPanelWidget::setOutput(result)
```

### 8.3 数据流

```
用户选中文本
  ↓
AiContextMediator::eventFilter
  ↓
AiContextMediator::pushSelectionToPanel
  ↓
AiPanelWidget::setContext
  ↓
AiStatusBar::setContext (显示上下文)
```

---

## 九、开发者指南

### 9.1 添加新的分析类型

```cpp
// 1. 在AiPanelWidget中添加按钮
QPushButton* btn5 = new QPushButton("简历", m_panelContent);
connect(btn5, &QPushButton::clicked, this, [this]() { 
    runAiAnalysis("resume"); 
});

// 2. 在MainWindow中处理新类型
connect(m_aiPanel, &AiPanelWidget::analysisRequested, 
        this, [this](const QString& type) {
    if (type == "resume") {
        // 调用简历分析服务
        QJsonObject result = AiService::analyzeResume(...);
        m_aiPanel->setOutput(result["analysis"].toString());
    }
});

// 3. 在AiService中实现分析逻辑
QJsonObject AiService::analyzeResume(...) {
    // 实现简历分析逻辑
}
```

### 9.2 自定义上下文捕获规则

```cpp
bool AiContextMediator::eventFilter(QObject* watched, QEvent* event) {
    // 添加自定义过滤条件
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget* focusWidget = QApplication::focusWidget();
        
        // 仅捕获特定组件的文本
        if (qobject_cast<QTextEdit*>(focusWidget) ||
            qobject_cast<QListWidget*>(focusWidget)) {
            // ... 原有逻辑
        }
    }
    return QObject::eventFilter(watched, event);
}
```

---

## 十、总结

### 10.1 核心功能

✅ **已实现：**
- 快捷分析（4种类型）
- 对话交互
- 上下文感知
- 结果回填
- 状态显示
- 面板折叠/展开

⚠️ **待改进：**
- 模式切换体验
- 上下文确认机制
- 按钮状态管理
- 重新生成逻辑
- 加载动画
- 历史记录

### 10.2 使用建议

1. **优先使用快捷按钮**：比对话模式更可靠
2. **合理使用上下文**：选中相关文本可获得更精准建议
3. **及时回填结果**：将AI建议应用到实际工作流
4. **定期刷新状态**：确保AI服务正常运行

---

**文档版本**: v1.0  
**最后更新**: 2026-04-29  
**维护者**: 项目团队  
**反馈渠道**: 项目Issue或开发团队

