#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class NavigationListWidget;
class TimeInfoCard;
class StudentInfoCard;

class SidebarWidget : public QFrame {
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget* parent = nullptr);
    ~SidebarWidget() override = default;

    void refreshData();
    NavigationListWidget* navigationList() const { return m_navList; }

signals:
    void navigationRequested(int index);

private:
    void setupUi();
    void setupToggleAnimation();

    NavigationListWidget* m_navList = nullptr;
    TimeInfoCard* m_timeCard = nullptr;
    StudentInfoCard* m_studentCard = nullptr;

    QLabel* m_brandMark = nullptr;
    QWidget* m_brandText = nullptr;
    QPushButton* m_toggleButton = nullptr;

    static constexpr int kSidebarExpandedWidth = 260;
    static constexpr int kSidebarCollapsedWidth = 56;
};

#endif // SIDEBARWIDGET_H
