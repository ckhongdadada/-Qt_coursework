#pragma once

#include <QWidget>
#include <QLabel>
#include <QSpacerItem>

class QStackedWidget;

class AppShellController : public QObject {
    Q_OBJECT

public:
    explicit AppShellController(QObject* parent = nullptr);

    void attach(QStackedWidget* stack, QWidget* mainInner,
                QSpacerItem* leftSpacer, QSpacerItem* rightSpacer,
                QLabel* topbarKicker, QLabel* topbarTitle, QLabel* topbarPill);

    void onPageChanged(int index);
    void updateTopbarForPage(int index);
    void updateContentWidthForPage(int index);
    void updateShellSpacingForPage(int index);

private:
    QStackedWidget* m_stack = nullptr;
    QWidget* m_mainInner = nullptr;
    QSpacerItem* m_leftStretchSpacer = nullptr;
    QSpacerItem* m_rightStretchSpacer = nullptr;
    QLabel* m_topbarKicker = nullptr;
    QLabel* m_topbarTitle = nullptr;
    QLabel* m_topbarPill = nullptr;

    static constexpr int kMaxContentWidth = 1080;
    static constexpr int kResumeEditorPageIndex = 10;
};
