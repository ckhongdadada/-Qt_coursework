#pragma once

#include <QFrame>
#include <QLabel>

class AiStatusBar : public QFrame {
    Q_OBJECT

public:
    explicit AiStatusBar(QWidget* parent = nullptr);

    void refreshStatus();
    void setContext(const QString& type, const QString& context);
    void clearContext();

private:
    void setupUi();

    QLabel* m_modeValue = nullptr;
    QLabel* m_modelValue = nullptr;
    QLabel* m_statusValue = nullptr;
    QLabel* m_contextLabel = nullptr;
    QString m_selectedContext;
};
