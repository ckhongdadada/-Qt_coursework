#include "AiConversationWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

AiConversationWidget::AiConversationWidget(QWidget* parent)
    : QFrame(parent)
{
    setupUi();
}

void AiConversationWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QFrame* outputCard = new QFrame(this);
    outputCard->setObjectName("contentCard");
    QVBoxLayout* outputLayout = new QVBoxLayout(outputCard);
    outputLayout->setContentsMargins(14, 12, 14, 12);
    outputLayout->setSpacing(8);

    QLabel* outputTitle = new QLabel("分析结果", outputCard);
    outputTitle->setObjectName("sectionTitle");
    outputLayout->addWidget(outputTitle);

    m_output = new QTextEdit(outputCard);
    m_output->setReadOnly(true);
    m_output->setAcceptRichText(false);
    m_output->setLineWrapMode(QTextEdit::WidgetWidth);
    m_output->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_output->setMinimumHeight(300);
    m_output->setStyleSheet("border: none; background: #fffdf8; font-size: 13px; color: #241f1b; line-height: 1.6;");
    m_output->setPlainText("AI 助手已就绪。\n可以点击上方的快捷按钮获取建议，或在下方直接向我提问。");
    outputLayout->addWidget(m_output, 1);
    layout->addWidget(outputCard, 1);

    QHBoxLayout* refillLayout = new QHBoxLayout();
    refillLayout->setSpacing(8);
    m_toResumeButton = new QPushButton("回填摘要", this);
    m_toGoalButton = new QPushButton("转为目标", this);
    m_toResumeButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    m_toGoalButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    connect(m_toResumeButton, &QPushButton::clicked, this, [this]() {
        emit applyToResumeRequested(m_output->toPlainText());
    });
    connect(m_toGoalButton, &QPushButton::clicked, this, [this]() {
        emit createGoalRequested("AI 建议目标", m_output->toPlainText());
    });
    refillLayout->addWidget(m_toResumeButton, 1);
    refillLayout->addWidget(m_toGoalButton, 1);
    layout->addLayout(refillLayout);

    QFrame* masterInputCard = new QFrame(this);
    masterInputCard->setStyleSheet("QFrame { background: white; border: 1px solid #dcdfe6; border-radius: 12px; }");
    QVBoxLayout* masterInputLayout = new QVBoxLayout(masterInputCard);
    masterInputLayout->setContentsMargins(12, 12, 12, 12);
    masterInputLayout->setSpacing(8);

    m_contextCard = new QFrame(masterInputCard);
    m_contextCard->setStyleSheet("QFrame { background: #f0f2f5; border: none; border-radius: 6px; }");
    QHBoxLayout* contextLayout = new QHBoxLayout(m_contextCard);
    contextLayout->setContentsMargins(8, 6, 8, 6);
    contextLayout->setSpacing(8);
    
    QFrame* indicator = new QFrame(m_contextCard);
    indicator->setFixedSize(3, 14);
    indicator->setStyleSheet("background: #c0c4cc; border-radius: 1px;");
    contextLayout->addWidget(indicator);

    m_contextLabel = new QLabel(m_contextCard);
    m_contextLabel->setStyleSheet("font-size: 13px; color: #606266; border: none; background: transparent;");
    m_contextLabel->setWordWrap(true);
    m_contextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    contextLayout->addWidget(m_contextLabel, 1);

    QPushButton* closeContextBtn = new QPushButton("×", m_contextCard);
    closeContextBtn->setFixedSize(16, 16);
    closeContextBtn->setCursor(Qt::PointingHandCursor);
    closeContextBtn->setStyleSheet("QPushButton { border: none; color: #909399; font-weight: bold; background: transparent; font-size: 14px; } QPushButton:hover { color: #f56c6c; }");
    connect(closeContextBtn, &QPushButton::clicked, this, &AiConversationWidget::clearSelectedContext);
    contextLayout->addWidget(closeContextBtn);

    m_contextCard->hide();
    masterInputLayout->addWidget(m_contextCard);

    QFrame* quickActionFrame = new QFrame(masterInputCard);
    quickActionFrame->setStyleSheet("border: none; background: transparent;");
    QHBoxLayout* quickActionLayout = new QHBoxLayout(quickActionFrame);
    quickActionLayout->setContentsMargins(0, 0, 0, 0);
    quickActionLayout->setSpacing(8);

    QString quickActionStyle = "QPushButton { background: #f5f7fa; color: #303133; border: 1px solid #ebeef5; border-radius: 6px; padding: 4px 10px; font-size: 12px; } QPushButton:hover { background: #e4e7ed; }";
    QPushButton* qa1 = new QPushButton("解释一下 ->", quickActionFrame);
    QPushButton* qa2 = new QPushButton("详细展开 ->", quickActionFrame);
    QPushButton* qa3 = new QPushButton("深入研究 ->", quickActionFrame);
    qa1->setStyleSheet(quickActionStyle); qa1->setCursor(Qt::PointingHandCursor);
    qa2->setStyleSheet(quickActionStyle); qa2->setCursor(Qt::PointingHandCursor);
    qa3->setStyleSheet(quickActionStyle); qa3->setCursor(Qt::PointingHandCursor);
    
    auto quickActionHandler = [this](const QString& prefix) {
        if (!m_chatInput) return;
        m_chatInput->setText(prefix);
        sendChatMessage();
    };
    connect(qa1, &QPushButton::clicked, this, [quickActionHandler]() { quickActionHandler("请解释一下："); });
    connect(qa2, &QPushButton::clicked, this, [quickActionHandler]() { quickActionHandler("请详细展开："); });
    connect(qa3, &QPushButton::clicked, this, [quickActionHandler]() { quickActionHandler("请深入研究："); });

    quickActionLayout->addWidget(qa1);
    quickActionLayout->addWidget(qa2);
    quickActionLayout->addWidget(qa3);
    quickActionLayout->addStretch();
    masterInputLayout->addWidget(quickActionFrame);

    QFrame* inputArea = new QFrame(masterInputCard);
    inputArea->setStyleSheet("border: none; background: transparent;");
    QHBoxLayout* chatLayout = new QHBoxLayout(inputArea);
    chatLayout->setContentsMargins(0, 4, 0, 0);
    chatLayout->setSpacing(8);

    m_chatInput = new QLineEdit(inputArea);
    m_chatInput->setPlaceholderText("发消息...");
    m_chatInput->setStyleSheet("background: transparent; border: none; font-size: 14px; color: #333;");

    QPushButton* sendButton = new QPushButton("发送", inputArea);
    sendButton->setStyleSheet("background: #88a7aa; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: bold;");
    sendButton->setCursor(Qt::PointingHandCursor);

    connect(sendButton, &QPushButton::clicked, this, &AiConversationWidget::sendChatMessage);
    connect(m_chatInput, &QLineEdit::returnPressed, this, &AiConversationWidget::sendChatMessage);

    chatLayout->addWidget(m_chatInput, 1);
    chatLayout->addWidget(sendButton);
    masterInputLayout->addWidget(inputArea);

    layout->addWidget(masterInputCard);
}

void AiConversationWidget::setOutput(const QString& text)
{
    if (m_output) {
        m_output->setPlainText(text);
        if (m_output->verticalScrollBar()) {
            m_output->verticalScrollBar()->setValue(0);
        }
    }
}

QString AiConversationWidget::output() const
{
    return m_output ? m_output->toPlainText() : QString();
}

void AiConversationWidget::appendMessage(const QString& role, const QString& text)
{
    if (m_output) {
        m_output->append(QString("\n[%1]: %2").arg(role, text));
        if (m_output->verticalScrollBar()) {
            m_output->verticalScrollBar()->setValue(m_output->verticalScrollBar()->maximum());
        }
    }
}

void AiConversationWidget::clearConversation()
{
    if (m_output) {
        m_output->setPlainText("对话已清空。可以重新开始提问。");
    }
}

void AiConversationWidget::setSelectedContext(const QString& text)
{
    m_selectedContext = text.trimmed();
    if (!m_contextCard || !m_contextLabel) {
        return;
    }

    if (m_selectedContext.isEmpty()) {
        clearSelectedContext();
        return;
    }

    const QString preview = m_selectedContext.length() > 260
        ? m_selectedContext.left(260) + "..."
        : m_selectedContext;
    m_contextLabel->setText(preview);
    m_contextCard->show();
}

void AiConversationWidget::clearSelectedContext()
{
    m_selectedContext.clear();
    if (m_contextLabel) {
        m_contextLabel->clear();
    }
    if (m_contextCard) {
        m_contextCard->hide();
    }
}

void AiConversationWidget::sendChatMessage()
{
    if (!m_chatInput || m_chatInput->text().trimmed().isEmpty()) return;
    const QString message = m_chatInput->text().trimmed();
    m_chatInput->clear();
    if (!m_selectedContext.isEmpty()) {
        emit chatMessageSent(QString("请结合以下选中文本回答我的问题。\n\n选中文本：\n%1\n\n我的问题：\n%2")
            .arg(m_selectedContext, message));
    } else {
        emit chatMessageSent(message);
    }
}
