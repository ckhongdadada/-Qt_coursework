#include "AppShellController.h"
#include <QLayout>
#include <QStackedWidget>

AppShellController::AppShellController(QObject* parent)
    : QObject(parent)
{}

void AppShellController::attach(QStackedWidget* stack, QWidget* mainInner,
                                 QSpacerItem* leftSpacer, QSpacerItem* rightSpacer,
                                 QLabel* topbarKicker, QLabel* topbarTitle, QLabel* topbarPill)
{
    m_stack = stack;
    m_mainInner = mainInner;
    m_leftStretchSpacer = leftSpacer;
    m_rightStretchSpacer = rightSpacer;
    m_topbarKicker = topbarKicker;
    m_topbarTitle = topbarTitle;
    m_topbarPill = topbarPill;
}

void AppShellController::onPageChanged(int index)
{
    updateTopbarForPage(index);
    updateContentWidthForPage(index);
    updateShellSpacingForPage(index);
}

void AppShellController::updateTopbarForPage(int index)
{
    if (!m_topbarKicker || !m_topbarTitle || !m_topbarPill) return;

    const bool isResume = (index == kResumeEditorPageIndex);

    if (isResume) {
        m_topbarKicker->setText("Resume editor");
        m_topbarTitle->setText("简历编辑器");
        m_topbarPill->setText("Editor");
    } else {
        m_topbarKicker->setText("Personal development planning website");
        m_topbarTitle->setText("个人发展规划工作台");
        m_topbarPill->setText("Knowledge base");
    }
}

void AppShellController::updateContentWidthForPage(int index)
{
    if (!m_mainInner) return;

    const bool isResume = (index == kResumeEditorPageIndex);

    if (isResume) {
        m_mainInner->setMaximumWidth(16777215);
    } else {
        m_mainInner->setMaximumWidth(kMaxContentWidth);
    }
}

void AppShellController::updateShellSpacingForPage(int index)
{
    const bool isResume = (index == kResumeEditorPageIndex);

    if (m_leftStretchSpacer) {
        m_leftStretchSpacer->changeSize(
            0, 0,
            isResume ? QSizePolicy::Fixed : QSizePolicy::Expanding,
            QSizePolicy::Minimum
        );
    }
    if (m_rightStretchSpacer) {
        m_rightStretchSpacer->changeSize(
            0, 0,
            isResume ? QSizePolicy::Fixed : QSizePolicy::Expanding,
            QSizePolicy::Minimum
        );
    }
    if (m_mainInner && m_mainInner->parentWidget()) {
        m_mainInner->parentWidget()->layout()->invalidate();
    }
}
