#ifndef AIPANELWIDGET_H
#define AIPANELWIDGET_H

#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPushButton>
#include <QWidget>

class AiStatusBar;
class AiConversationWidget;

class AiPanelWidget : public QFrame {
    Q_OBJECT

public:
    explicit AiPanelWidget(QWidget* parent = nullptr);
    ~AiPanelWidget() override = default;

    void refreshStatus();
    void setContext(const QString& type, const QString& context);
    void submitPromptFromSelection(const QString& text);
    void addContextFromSelection(const QString& text);
    void clearContext();
    void appendMessage(const QString& role, const QString& text);
    QString currentOutput() const;
    void setOutput(const QString& text);

signals:
    void applyToResumeRequested(const QString& summary);
    void createGoalRequested(const QString& title, const QString& description);
    void analysisRequested(const QString& type);
    void chatMessageSent(const QString& message);

private:
    void setupUi();
    void setupAnimations();
    void runAiAnalysis(const QString& type);

    QWidget* m_collapsedStrip = nullptr;
    QPushButton* m_stripButton = nullptr;

    QWidget* m_panelContent = nullptr;
    QGraphicsOpacityEffect* m_panelOpacity = nullptr;

    AiStatusBar* m_statusBar = nullptr;
    AiConversationWidget* m_conversation = nullptr;

    static constexpr int kAiSidebarWidth = 380;
    static constexpr int kAiCollapsedWidth = 50;
};

#endif // AIPANELWIDGET_H
