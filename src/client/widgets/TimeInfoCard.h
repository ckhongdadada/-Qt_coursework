#pragma once

#include <QFrame>
#include <QLabel>

class TimeInfoCard : public QFrame {
    Q_OBJECT

public:
    explicit TimeInfoCard(QWidget* parent = nullptr);

    void refreshNow();
    QString semesterText() const;
    QString dateTimeText() const;

    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

private:
    void setupUi();

    QLabel* m_avatar = nullptr;
    QLabel* m_semesterLabel = nullptr;
    QLabel* m_detailLabel = nullptr;
    QWidget* m_textWidget = nullptr;

    bool m_collapsed = false;
};
