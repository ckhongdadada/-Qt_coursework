#pragma once

#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>

class QPushButton;
class QLabel;

class AiConversationWidget : public QFrame {
    Q_OBJECT

public:
    explicit AiConversationWidget(QWidget* parent = nullptr);

    void setOutput(const QString& text);
    QString output() const;
    void appendMessage(const QString& role, const QString& text);
    void clearConversation();
    void setSelectedContext(const QString& text);
    void clearSelectedContext();

signals:
    void applyToResumeRequested(const QString& summary);
    void createGoalRequested(const QString& title, const QString& description);
    void chatMessageSent(const QString& message);

private:
    void setupUi();
    void sendChatMessage();

    QTextEdit* m_output = nullptr;
    QFrame* m_contextCard = nullptr;
    QLabel* m_contextLabel = nullptr;
    QLineEdit* m_chatInput = nullptr;
    QString m_selectedContext;
    QPushButton* m_toResumeButton = nullptr;
    QPushButton* m_toGoalButton = nullptr;
};
