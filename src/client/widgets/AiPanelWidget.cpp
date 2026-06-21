#include "AiPanelWidget.h"
#include "AiStatusBar.h"
#include "AiConversationWidget.h"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "service/AiService.h"

AiPanelWidget::AiPanelWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("aiSidebar");
    setStyleSheet("#aiSidebar { background: #faf8f4; border-left: 1px solid #ddd3c6; }");
    setupUi();
    setupAnimations();
}

void AiPanelWidget::setupUi()
{
    QHBoxLayout* rootAiLayout = new QHBoxLayout(this);
    rootAiLayout->setContentsMargins(0, 0, 0, 0);
    rootAiLayout->setSpacing(0);

    m_collapsedStrip = new QWidget(this);
    m_collapsedStrip->setObjectName("aiCollapsedStrip");
    m_collapsedStrip->setFixedWidth(kAiCollapsedWidth);
    m_collapsedStrip->setCursor(Qt::PointingHandCursor);
    QVBoxLayout* stripLayout = new QVBoxLayout(m_collapsedStrip);
    stripLayout->setContentsMargins(0, 16, 0, 16);
    stripLayout->setSpacing(12);
    stripLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_stripButton = new QPushButton("AI", m_collapsedStrip);
    m_stripButton->setFixedSize(32, 32);
    m_stripButton->setCursor(Qt::PointingHandCursor);
    m_stripButton->setToolTip("打开 AI 助手");
    m_stripButton->setStyleSheet(
        "QPushButton { background: #e8e2d6; border: 1px solid #d0c4b4; border-radius: 10px;"
        "font-size: 13px; font-weight: 700; color: #5b5043; }"
        "QPushButton:hover { background: #d8ceb8; border-color: #b8a99a; }"
        "QPushButton:pressed { background: #ccc1ad; }");
    stripLayout->addWidget(m_stripButton, 0, Qt::AlignHCenter);
    stripLayout->addStretch();

    rootAiLayout->addWidget(m_collapsedStrip);

    m_panelContent = new QWidget(this);
    m_panelContent->setObjectName("aiPanelContent");
    QVBoxLayout* layout = new QVBoxLayout(m_panelContent);
    layout->setContentsMargins(18, 20, 18, 18);
    layout->setSpacing(14);

    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* title = new QLabel("AI 建议", m_panelContent);
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #333;");
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    QPushButton* refreshAiButton = new QPushButton("刷新", m_panelContent);
    refreshAiButton->setCursor(Qt::PointingHandCursor);
    refreshAiButton->setFixedHeight(28);
    refreshAiButton->setToolTip("刷新 AI 服务状态");
    refreshAiButton->setStyleSheet(
        "background: transparent; border: 1px solid rgba(67,57,43,0.14); border-radius: 8px;"
        "font-size: 12px; color: #5b4e43; padding: 0 10px;"
        "QPushButton:hover { background: #f4ede2; }");
    connect(refreshAiButton, &QPushButton::clicked, this, [this]() {
        AiService::resetAiServerCheck();
        refreshStatus();
        setOutput("AI 服务状态已刷新。\n\n在下方输入框直接提问即可。");
    });
    titleLayout->addWidget(refreshAiButton, 0, Qt::AlignRight);

    QPushButton* collapseButton = new QPushButton("收起", m_panelContent);
    collapseButton->setCursor(Qt::PointingHandCursor);
    collapseButton->setFixedHeight(28);
    collapseButton->setStyleSheet(
        "background: transparent; border: 1px solid rgba(67,57,43,0.14); border-radius: 8px;"
        "font-size: 12px; color: #5b4e43; padding: 0 10px;"
        "QPushButton:hover { background: #f4ede2; }");
    titleLayout->addWidget(collapseButton, 0, Qt::AlignRight);
    layout->addLayout(titleLayout);

    QLabel* panelSubtitle = new QLabel("基于课程、经历、目标与简历配置生成建议，并支持回填到当前工作流。", m_panelContent);
    panelSubtitle->setObjectName("pageSubtitle");
    panelSubtitle->setWordWrap(true);

    m_statusBar = new AiStatusBar(m_panelContent);

    QHBoxLayout* helperActionLayout = new QHBoxLayout();
    helperActionLayout->setSpacing(8);
    QPushButton* clearContextButton = new QPushButton("清空上下文", m_panelContent);
    QString secondaryStyle = "background: #eef2f6; color: #444; border: none; border-radius: 4px; padding: 6px;";
    clearContextButton->setStyleSheet(secondaryStyle);
    connect(clearContextButton, &QPushButton::clicked, this, [this]() { clearContext(); });
    helperActionLayout->addWidget(clearContextButton);
    helperActionLayout->addStretch();

    m_conversation = new AiConversationWidget(m_panelContent);
    connect(m_conversation, &AiConversationWidget::chatMessageSent, this, &AiPanelWidget::chatMessageSent);

    QScrollArea* scrollArea = new QScrollArea(m_panelContent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(14);
    scrollLayout->addWidget(panelSubtitle);
    scrollLayout->addWidget(m_statusBar);
    scrollLayout->addLayout(helperActionLayout);
    scrollLayout->addWidget(m_conversation, 1);
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);

    rootAiLayout->addWidget(m_panelContent, 1);

    m_panelOpacity = new QGraphicsOpacityEffect(m_panelContent);
    m_panelContent->setGraphicsEffect(m_panelOpacity);
    m_panelOpacity->setOpacity(1.0);

    m_collapsedStrip->hide();
    m_panelContent->show();
    setMinimumWidth(kAiSidebarWidth);
    setMaximumWidth(kAiSidebarWidth);

    connect(collapseButton, &QPushButton::clicked, this, [this]() {
        setMinimumWidth(kAiSidebarWidth);
        setMaximumWidth(kAiSidebarWidth);

        auto* group = new QParallelAnimationGroup(this);
        auto* minAnim = new QPropertyAnimation(this, "minimumWidth", group);
        auto* maxAnim = new QPropertyAnimation(this, "maximumWidth", group);
        auto* opacityAnim = new QPropertyAnimation(m_panelOpacity, "opacity", group);

        minAnim->setDuration(220);
        maxAnim->setDuration(220);
        opacityAnim->setDuration(140);
        minAnim->setStartValue(kAiSidebarWidth);
        minAnim->setEndValue(kAiCollapsedWidth);
        maxAnim->setStartValue(kAiSidebarWidth);
        maxAnim->setEndValue(kAiCollapsedWidth);
        opacityAnim->setStartValue(1.0);
        opacityAnim->setEndValue(0.0);
        minAnim->setEasingCurve(QEasingCurve::InOutQuad);
        maxAnim->setEasingCurve(QEasingCurve::InOutQuad);
        opacityAnim->setEasingCurve(QEasingCurve::InCubic);

        group->addAnimation(minAnim);
        group->addAnimation(maxAnim);
        group->addAnimation(opacityAnim);
        connect(group, &QParallelAnimationGroup::finished, this, [this, group]() {
            m_panelContent->hide();
            m_collapsedStrip->show();
            setMinimumWidth(kAiCollapsedWidth);
            setMaximumWidth(kAiCollapsedWidth);
            m_panelOpacity->setOpacity(1.0);
            group->deleteLater();
        });
        group->start();
    });
}

void AiPanelWidget::setupAnimations()
{
    connect(m_stripButton, &QPushButton::clicked, this, [this]() {
        m_collapsedStrip->hide();
        m_panelContent->show();
        setMinimumWidth(kAiCollapsedWidth);
        setMaximumWidth(kAiCollapsedWidth);

        auto* group = new QParallelAnimationGroup(this);
        auto* minAnim = new QPropertyAnimation(this, "minimumWidth", group);
        auto* maxAnim = new QPropertyAnimation(this, "maximumWidth", group);
        auto* opacityAnim = new QPropertyAnimation(m_panelOpacity, "opacity", group);

        minAnim->setDuration(220);
        maxAnim->setDuration(220);
        opacityAnim->setDuration(180);
        minAnim->setStartValue(kAiCollapsedWidth);
        minAnim->setEndValue(kAiSidebarWidth);
        maxAnim->setStartValue(kAiCollapsedWidth);
        maxAnim->setEndValue(kAiSidebarWidth);
        opacityAnim->setStartValue(0.0);
        opacityAnim->setEndValue(1.0);
        minAnim->setEasingCurve(QEasingCurve::InOutQuad);
        maxAnim->setEasingCurve(QEasingCurve::InOutQuad);
        opacityAnim->setEasingCurve(QEasingCurve::OutCubic);

        group->addAnimation(minAnim);
        group->addAnimation(maxAnim);
        group->addAnimation(opacityAnim);
        connect(group, &QParallelAnimationGroup::finished, this, [this, group]() {
            setMinimumWidth(kAiSidebarWidth);
            setMaximumWidth(kAiSidebarWidth);
            m_panelOpacity->setOpacity(1.0);
            group->deleteLater();
        });
        group->start();
    });
}

void AiPanelWidget::refreshStatus()
{
    if (m_statusBar) m_statusBar->refreshStatus();
}

void AiPanelWidget::setContext(const QString& type, const QString& context)
{
    if (m_statusBar) m_statusBar->setContext(type, context);
    if (m_conversation) m_conversation->setSelectedContext(context);
}

void AiPanelWidget::submitPromptFromSelection(const QString& text)
{
    const QString selectedText = text.trimmed();
    if (selectedText.isEmpty()) {
        return;
    }

    setContext(QString::fromUtf8("选中文本"), selectedText);
    const QString prompt = QString::fromUtf8(
        "请基于以下选中的内容给出可执行的优化建议。如果它适合简历，请压缩成正式简历表达；"
        "如果它适合目标规划，请拆成下一步行动。\n\n选中文本：\n%1")
        .arg(selectedText);
    setOutput(QString::fromUtf8("已将选中文本发送给大模型，正在生成建议..."));
    emit chatMessageSent(prompt);
}

void AiPanelWidget::addContextFromSelection(const QString& text)
{
    const QString selectedText = text.trimmed();
    if (selectedText.isEmpty()) {
        return;
    }

    setContext(QString::fromUtf8("选中文本"), selectedText);
    // Show the context pill inside the conversation input area
    if (m_conversation) {
        m_conversation->setSelectedContext(selectedText);
    }
    // Expand panel if collapsed
    if (m_collapsedStrip->isVisible()) {
        m_stripButton->click();
    }
}

void AiPanelWidget::clearContext()
{
    if (m_statusBar) m_statusBar->clearContext();
    if (m_conversation) m_conversation->clearSelectedContext();
    setOutput("上下文已清空。\n可以重新选择内容，或直接点击上方按钮生成新建议。");
}

void AiPanelWidget::appendMessage(const QString& role, const QString& text)
{
    if (m_conversation) m_conversation->appendMessage(role, text);
}

QString AiPanelWidget::currentOutput() const
{
    return m_conversation ? m_conversation->output() : QString();
}

void AiPanelWidget::setOutput(const QString& text)
{
    if (m_conversation) m_conversation->setOutput(text);
}
