# AI 助手改进方案

**版本**: v1.0  
**创建日期**: 2026-04-29  
**优先级**: 中等  
**预计工时**: 2-3天

---

## 改进概述

基于当前AI助手的使用体验分析，提出5个核心改进点和具体实现方案。

---

## 改进1：明确模式切换 🔥 高优先级

### 问题描述

当前点击 **[对话]** 和 **[建议]** 标签后，界面没有明显变化，用户不清楚当前处于哪种模式。

### 改进方案

**方案A：隐藏/显示不同功能区（推荐）**

```cpp
// AiPanelWidget.cpp - setupUi()

// 保存组件引用
QWidget* m_quickActionsArea = nullptr;  // 快捷按钮区域
QWidget* m_chatInputArea = nullptr;     // 输入框区域

// 建议模式
connect(suggTabBtn, &QPushButton::clicked, this, [=]() {
    // 样式切换
    suggTabBtn->setStyleSheet(activeTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
    chatTabBtn->setStyleSheet(inactiveTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
    
    // 显示/隐藏功能区
    m_quickActionsArea->show();
    m_chatInputArea->hide();
    
    // 更新提示
    setOutput("💡 建议模式\n\n点击上方快捷按钮获取针对性建议：\n\n"
              "• 综合 - 全面学业分析\n"
              "• 课程 - 课程规划建议\n"
              "• 经历 - 实践经历提升\n"
              "• 目标 - 目标完成策略");
});

// 对话模式
connect(chatTabBtn, &QPushButton::clicked, this, [=]() {
    // 样式切换
    chatTabBtn->setStyleSheet(activeTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
    suggTabBtn->setStyleSheet(inactiveTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
    
    // 显示/隐藏功能区
    m_quickActionsArea->hide();
    m_chatInputArea->show();
    
    // 更新提示
    setOutput("💬 对话模式\n\n"
              "AI 助手已就绪，可以在下方输入问题进行对话。\n\n"
              "示例问题：\n"
              "• 我应该如何提高GPA？\n"
              "• 推荐一些适合我的实习岗位\n"
              "• 如何平衡学习和课外活动？");
});
```

**方案B：添加模式指示器**

```cpp
// 添加模式指示标签
QLabel* m_modeIndicator = new QLabel(this);
m_modeIndicator->setStyleSheet(
    "background: #e3f2fd; color: #1976d2; "
    "padding: 4px 12px; border-radius: 12px; "
    "font-size: 11px; font-weight: bold;");
m_modeIndicator->setText("📋 建议模式");

// 模式切换时更新指示器
connect(chatTabBtn, &QPushButton::clicked, this, [=]() {
    m_modeIndicator->setText("💬 对话模式");
    m_modeIndicator->setStyleSheet(
        "background: #f3e5f5; color: #7b1fa2; "
        "padding: 4px 12px; border-radius: 12px; "
        "font-size: 11px; font-weight: bold;");
});
```

### 实现步骤

1. 修改 `AiPanelWidget.h`，添加成员变量
2. 修改 `AiPanelWidget.cpp` 的 `setupUi()`
3. 测试模式切换效果
4. 更新用户文档

### 预计工时

- 开发：2小时
- 测试：1小时
- 文档：0.5小时

---

## 改进2：优化上下文捕获 🔥 高优先级

### 问题描述

上下文捕获过于敏感，用户在任何地方选中文本都会触发，可能捕获到不相关的内容。

### 改进方案

**方案A：添加确认机制（推荐）**

```cpp
// AiStatusBar.h
class AiStatusBar : public QFrame {
    // ...
public:
    void showContextPreview(const QString& type, const QString& context);
    void confirmContext();
    void rejectContext();
    
signals:
    void contextConfirmed(const QString& context);
    void contextRejected();
    
private:
    QPushButton* m_useContextButton = nullptr;
    QPushButton* m_ignoreContextButton = nullptr;
    QString m_pendingContext;
};

// AiStatusBar.cpp
void AiStatusBar::showContextPreview(const QString& type, const QString& context) {
    m_pendingContext = context;
    
    // 显示预览
    m_contextLabel->setText(QString("📎 捕获到内容 [%1]：%2")
        .arg(type, context.left(80) + "..."));
    m_contextLabel->show();
    
    // 显示确认按钮
    m_useContextButton->show();
    m_ignoreContextButton->show();
}

void AiStatusBar::confirmContext() {
    m_selectedContext = m_pendingContext;
    m_contextLabel->setText(QString("✅ 已使用上下文：%1")
        .arg(m_selectedContext.left(80) + "..."));
    m_useContextButton->hide();
    m_ignoreContextButton->hide();
    emit contextConfirmed(m_selectedContext);
}

void AiStatusBar::rejectContext() {
    m_pendingContext.clear();
    m_contextLabel->hide();
    m_useContextButton->hide();
    m_ignoreContextButton->hide();
    emit contextRejected();
}

// AiContextMediator.cpp
void AiContextMediator::pushSelectionToPanel(const QString& text) {
    if (m_panel) {
        // 显示预览，等待用户确认
        m_panel->statusBar()->showContextPreview("选中内容", text);
    }
}
```

**方案B：仅在AI面板展开时捕获**

```cpp
// AiContextMediator.cpp
bool AiContextMediator::eventFilter(QObject* watched, QEvent* event) {
    // 仅在AI面板展开时捕获
    if (!m_panel || m_panel->isCollapsed()) {
        return QObject::eventFilter(watched, event);
    }
    
    if (event->type() == QEvent::MouseButtonRelease) {
        // ... 原有逻辑
    }
    return QObject::eventFilter(watched, event);
}

// AiPanelWidget.h
class AiPanelWidget : public QFrame {
public:
    bool isCollapsed() const { return m_isCollapsed; }
    
private:
    bool m_isCollapsed = false;
};
```

**方案C：添加白名单机制**

```cpp
// AiContextMediator.cpp
bool AiContextMediator::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget* focusWidget = QApplication::focusWidget();
        
        // 白名单：仅捕获这些组件的文本
        QStringList allowedClasses = {
            "QTextEdit",
            "QListWidget",
            "QTableWidget",
            "QTreeWidget"
        };
        
        bool isAllowed = false;
        for (const QString& className : allowedClasses) {
            if (focusWidget->inherits(className.toUtf8())) {
                isAllowed = true;
                break;
            }
        }
        
        if (!isAllowed) {
            return QObject::eventFilter(watched, event);
        }
        
        // ... 原有逻辑
    }
    return QObject::eventFilter(watched, event);
}
```

### 推荐方案

**组合方案：方案B + 方案C**
- 仅在AI面板展开时捕获（避免干扰）
- 使用白名单机制（避免捕获无关内容）

### 实现步骤

1. 修改 `AiPanelWidget`，添加 `isCollapsed()` 方法
2. 修改 `AiContextMediator::eventFilter()`，添加展开检查
3. 添加白名单机制
4. 测试各种场景
5. 更新用户文档

### 预计工时

- 开发：3小时
- 测试：2小时
- 文档：0.5小时

---

## 改进3：智能按钮状态管理 🔥 中优先级

### 问题描述

回填按钮始终可见，即使没有生成AI建议，点击后才提示错误。

### 改进方案

```cpp
// AiConversationWidget.h
class AiConversationWidget : public QFrame {
    // ...
private:
    void updateButtonStates();
    bool hasValidContent() const;
};

// AiConversationWidget.cpp
void AiConversationWidget::setOutput(const QString& text) {
    if (m_output) {
        m_output->setPlainText(text);
        updateButtonStates();
    }
}

void AiConversationWidget::updateButtonStates() {
    bool hasContent = hasValidContent();
    
    // 启用/禁用回填按钮
    m_toResumeButton->setEnabled(hasContent);
    m_toGoalButton->setEnabled(hasContent);
    
    // 更新样式（禁用时显示灰色）
    QString enabledStyle = "background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;";
    QString disabledStyle = "background: #f5f5f5; color: #999; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;";
    
    m_toResumeButton->setStyleSheet(hasContent ? enabledStyle : disabledStyle);
    m_toGoalButton->setStyleSheet(hasContent ? enabledStyle : disabledStyle);
    
    // 更新提示
    if (hasContent) {
        m_toResumeButton->setToolTip("将AI建议回填到简历摘要");
        m_toGoalButton->setToolTip("将AI建议转为新目标");
    } else {
        m_toResumeButton->setToolTip("请先生成AI建议");
        m_toGoalButton->setToolTip("请先生成AI建议");
    }
}

bool AiConversationWidget::hasValidContent() const {
    if (!m_output) return false;
    
    QString text = m_output->toPlainText().trimmed();
    
    // 排除提示性文本
    QStringList invalidPrefixes = {
        "AI 助手已就绪",
        "正在分析中",
        "AI 正在思考中",
        "点击上方快捷按钮",
        "对话已清空",
        "上下文已清空"
    };
    
    for (const QString& prefix : invalidPrefixes) {
        if (text.startsWith(prefix)) {
            return false;
        }
    }
    
    // 至少要有20个字符
    return text.length() >= 20;
}
```

### 实现步骤

1. 修改 `AiConversationWidget.h`
2. 实现 `updateButtonStates()` 和 `hasValidContent()`
3. 在 `setOutput()` 中调用更新
4. 测试各种场景
5. 更新用户文档

### 预计工时

- 开发：1.5小时
- 测试：1小时
- 文档：0.5小时

---

## 改进4：记忆上次分析类型 🔥 中优先级

### 问题描述

点击 **[重新生成]** 总是执行"综合"分析，用户期望重新生成上一次的分析类型。

### 改进方案

```cpp
// AiPanelWidget.h
class AiPanelWidget : public QFrame {
    // ...
private:
    QString m_lastAnalysisType = "general";
    QDateTime m_lastAnalysisTime;
};

// AiPanelWidget.cpp
void AiPanelWidget::runAiAnalysis(const QString& type) {
    m_lastAnalysisType = type;
    m_lastAnalysisTime = QDateTime::currentDateTime();
    emit analysisRequested(type);
}

// 重新生成按钮
connect(rerunButton, &QPushButton::clicked, this, [this]() {
    if (m_lastAnalysisType.isEmpty()) {
        setOutput("⚠️ 尚未执行过分析，请先点击上方快捷按钮。");
        return;
    }
    
    // 显示正在重新生成
    setOutput(QString("🔄 正在重新生成 [%1] 分析...\n\n上次分析时间：%2")
        .arg(getAnalysisTypeName(m_lastAnalysisType))
        .arg(m_lastAnalysisTime.toString("yyyy-MM-dd HH:mm:ss")));
    
    // 执行分析
    runAiAnalysis(m_lastAnalysisType);
});

// 辅助方法
QString AiPanelWidget::getAnalysisTypeName(const QString& type) const {
    static QMap<QString, QString> typeNames = {
        {"general", "综合"},
        {"course", "课程"},
        {"career", "经历"},
        {"goal", "目标"}
    };
    return typeNames.value(type, type);
}
```

### 增强功能：显示分析历史

```cpp
// AiPanelWidget.h
class AiPanelWidget : public QFrame {
private:
    struct AnalysisRecord {
        QString type;
        QDateTime time;
        QString result;
    };
    QVector<AnalysisRecord> m_analysisHistory;
    
    void addToHistory(const QString& type, const QString& result);
    void showHistory();
};

// 添加历史按钮
QPushButton* historyButton = new QPushButton("历史", m_panelContent);
connect(historyButton, &QPushButton::clicked, this, &AiPanelWidget::showHistory);

void AiPanelWidget::showHistory() {
    if (m_analysisHistory.isEmpty()) {
        setOutput("📋 暂无分析历史");
        return;
    }
    
    QStringList lines;
    lines << "📋 分析历史\n";
    
    for (int i = m_analysisHistory.size() - 1; i >= 0 && i >= m_analysisHistory.size() - 5; --i) {
        const auto& record = m_analysisHistory[i];
        lines << QString("[%1] %2 - %3")
            .arg(record.time.toString("MM-dd HH:mm"))
            .arg(getAnalysisTypeName(record.type))
            .arg(record.result.left(30) + "...");
    }
    
    setOutput(lines.join("\n"));
}
```

### 实现步骤

1. 修改 `AiPanelWidget.h`，添加成员变量
2. 修改 `runAiAnalysis()`，记录类型和时间
3. 修改重新生成按钮逻辑
4. （可选）实现历史记录功能
5. 测试各种场景
6. 更新用户文档

### 预计工时

- 开发：2小时
- 测试：1小时
- 文档：0.5小时

---

## 改进5：自动刷新状态 🔥 低优先级

### 问题描述

首次打开AI面板时，状态可能显示不正确，需要手动点击刷新。

### 改进方案

```cpp
// AiPanelWidget.cpp
AiPanelWidget::AiPanelWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("aiSidebar");
    setStyleSheet("#aiSidebar { background: #faf8f4; border-left: 1px solid #ddd3c6; }");
    setupUi();
    setupAnimations();
    
    // 延迟刷新状态（等待UI完全初始化）
    QTimer::singleShot(100, this, [this]() {
        refreshStatus();
        setOutput("AI 助手已就绪。\n\n"
                 "💡 点击上方快捷按钮获取建议\n"
                 "💬 或在下方输入问题进行对话");
    });
}

// 定期刷新状态（可选）
void AiPanelWidget::setupAutoRefresh() {
    QTimer* refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        // 仅在面板展开时刷新
        if (!isCollapsed()) {
            refreshStatus();
        }
    });
    refreshTimer->start(60000);  // 每60秒刷新一次
}
```

### 实现步骤

1. 修改 `AiPanelWidget` 构造函数
2. 添加延迟刷新逻辑
3. （可选）添加定期刷新
4. 测试各种场景
5. 更新用户文档

### 预计工时

- 开发：1小时
- 测试：0.5小时
- 文档：0.5小时

---

## 额外改进建议

### 改进6：添加加载动画

```cpp
// AiConversationWidget.h
class AiConversationWidget : public QFrame {
public:
    void showLoadingAnimation();
    void hideLoadingAnimation();
    
private:
    QLabel* m_loadingLabel = nullptr;
    QMovie* m_loadingMovie = nullptr;
};

// AiConversationWidget.cpp
void AiConversationWidget::showLoadingAnimation() {
    if (!m_loadingLabel) {
        m_loadingLabel = new QLabel(m_output);
        m_loadingMovie = new QMovie(":/icons/loading.gif");
        m_loadingLabel->setMovie(m_loadingMovie);
        m_loadingLabel->setAlignment(Qt::AlignCenter);
    }
    
    m_loadingMovie->start();
    m_loadingLabel->show();
    m_output->setEnabled(false);
}

void AiConversationWidget::hideLoadingAnimation() {
    if (m_loadingMovie) {
        m_loadingMovie->stop();
    }
    if (m_loadingLabel) {
        m_loadingLabel->hide();
    }
    m_output->setEnabled(true);
}
```

### 改进7：添加快捷键支持

```cpp
// AiPanelWidget.cpp
void AiPanelWidget::setupShortcuts() {
    // Ctrl+1-4: 快捷分析
    QShortcut* shortcut1 = new QShortcut(QKeySequence("Ctrl+1"), this);
    connect(shortcut1, &QShortcut::activated, this, [this]() {
        runAiAnalysis("general");
    });
    
    QShortcut* shortcut2 = new QShortcut(QKeySequence("Ctrl+2"), this);
    connect(shortcut2, &QShortcut::activated, this, [this]() {
        runAiAnalysis("course");
    });
    
    // Ctrl+R: 重新生成
    QShortcut* shortcutR = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(shortcutR, &QShortcut::activated, this, [this]() {
        runAiAnalysis(m_lastAnalysisType);
    });
    
    // Ctrl+K: 清空上下文
    QShortcut* shortcutK = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(shortcutK, &QShortcut::activated, this, &AiPanelWidget::clearContext);
}
```

### 改进8：添加结果导出

```cpp
// AiConversationWidget.cpp
void AiConversationWidget::setupUi() {
    // ... 原有代码
    
    // 添加导出按钮
    QPushButton* exportButton = new QPushButton("导出", this);
    exportButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    connect(exportButton, &QPushButton::clicked, this, &AiConversationWidget::exportResult);
    refillLayout->addWidget(exportButton);
}

void AiConversationWidget::exportResult() {
    QString filename = QFileDialog::getSaveFileName(
        this, "导出AI建议", 
        QDir::homePath() + "/ai_analysis_" + 
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt",
        "文本文件 (*.txt);;Markdown文件 (*.md)");
    
    if (filename.isEmpty()) return;
    
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        
        // 添加元数据
        out << "# AI 分析结果\n\n";
        out << "生成时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";
        out << "---\n\n";
        out << m_output->toPlainText();
        
        file.close();
        ToastNotification::display(this, "导出成功：" + filename);
    } else {
        ToastNotification::display(this, "导出失败：无法写入文件");
    }
}
```

---

## 实施计划

### 阶段1：核心改进（必须）

| 改进项 | 优先级 | 工时 | 负责人 |
|--------|--------|------|--------|
| 改进1：明确模式切换 | 高 | 3.5h | 前端开发 |
| 改进2：优化上下文捕获 | 高 | 5.5h | 前端开发 |
| 改进3：智能按钮状态 | 中 | 3h | 前端开发 |

**预计总工时**：12小时（1.5天）

### 阶段2：体验优化（推荐）

| 改进项 | 优先级 | 工时 | 负责人 |
|--------|--------|------|--------|
| 改进4：记忆分析类型 | 中 | 3.5h | 前端开发 |
| 改进5：自动刷新状态 | 低 | 2h | 前端开发 |

**预计总工时**：5.5小时（0.7天）

### 阶段3：增强功能（可选）

| 改进项 | 优先级 | 工时 | 负责人 |
|--------|--------|------|--------|
| 改进6：加载动画 | 低 | 2h | 前端开发 |
| 改进7：快捷键支持 | 低 | 1.5h | 前端开发 |
| 改进8：结果导出 | 低 | 2h | 前端开发 |

**预计总工时**：5.5小时（0.7天）

### 总计

- **核心改进**：1.5天
- **体验优化**：0.7天
- **增强功能**：0.7天
- **总计**：2.9天（约3天）

---

## 测试计划

### 功能测试

- [ ] 模式切换正常，界面变化明显
- [ ] 上下文捕获仅在合适时机触发
- [ ] 按钮状态根据内容正确更新
- [ ] 重新生成使用正确的分析类型
- [ ] 状态自动刷新正常

### 回归测试

- [ ] 原有功能不受影响
- [ ] 快捷按钮正常工作
- [ ] 对话功能正常工作
- [ ] 回填功能正常工作
- [ ] 面板折叠/展开正常

### 性能测试

- [ ] 上下文捕获不影响性能
- [ ] 状态刷新不阻塞UI
- [ ] 动画流畅

---

## 文档更新

- [ ] 更新 `docs/AI_ASSISTANT_GUIDE.md`
- [ ] 更新 `docs/COMPONENTS.md`
- [ ] 更新 `CHANGELOG.md`
- [ ] 更新用户手册（如有）

---

## 风险评估

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| 改动影响现有功能 | 高 | 中 | 充分测试，保留回退方案 |
| 用户不适应新交互 | 中 | 低 | 提供文档，保持向后兼容 |
| 性能下降 | 中 | 低 | 性能测试，优化关键路径 |
| 开发时间超预期 | 低 | 中 | 分阶段实施，优先核心改进 |

---

## 总结

本改进方案针对AI助手当前存在的5个核心问题，提供了详细的实现方案和代码示例。建议分3个阶段实施：

1. **阶段1（必须）**：解决核心体验问题，预计1.5天
2. **阶段2（推荐）**：优化用户体验，预计0.7天
3. **阶段3（可选）**：增强功能，预计0.7天

总计约3天工作量，可显著提升AI助手的可用性和用户体验。

---

**文档版本**: v1.0  
**创建日期**: 2026-04-29  
**维护者**: 项目团队

