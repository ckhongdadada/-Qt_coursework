#include "SidebarWidget.h"
#include "NavigationListWidget.h"
#include "TimeInfoCard.h"
#include "StudentInfoCard.h"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QHBoxLayout>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QVBoxLayout>

SidebarWidget::SidebarWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("sidebar");
    setFixedWidth(kSidebarExpandedWidth);
    setupUi();
    setupToggleAnimation();
}

void SidebarWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    QWidget* topRow = new QWidget(this);
    QHBoxLayout* topRowLayout = new QHBoxLayout(topRow);
    topRowLayout->setContentsMargins(0, 0, 0, 0);
    topRowLayout->setSpacing(8);

    m_brandMark = new QLabel("P", topRow);
    m_brandMark->setObjectName("brandMark");
    topRowLayout->addWidget(m_brandMark, 0, Qt::AlignTop);

    m_brandText = new QWidget(topRow);
    QVBoxLayout* brandTextLayout = new QVBoxLayout(m_brandText);
    brandTextLayout->setContentsMargins(0, 0, 0, 0);
    brandTextLayout->setSpacing(2);

    QLabel* title = new QLabel("Personal Planner", m_brandText);
    title->setObjectName("sidebarTitle");
    brandTextLayout->addWidget(title);

    QLabel* subtitle = new QLabel("个人发展知识库", m_brandText);
    subtitle->setObjectName("sidebarSubtitle");
    brandTextLayout->addWidget(subtitle);
    topRowLayout->addWidget(m_brandText, 1);

    m_toggleButton = new QPushButton(QString(QChar(0x2261)), topRow);
    m_toggleButton->setObjectName("sidebarToggle");
    m_toggleButton->setFlat(true);
    m_toggleButton->setToolTip("收起导航栏");
    topRowLayout->addWidget(m_toggleButton, 0, Qt::AlignTop);
    layout->addWidget(topRow);

    m_navList = new NavigationListWidget(this);
    layout->addWidget(m_navList, 1);

    layout->addStretch(0);

    m_timeCard = new TimeInfoCard(this);
    layout->addWidget(m_timeCard);

    m_studentCard = new StudentInfoCard(this);
    layout->addWidget(m_studentCard);

    connect(m_navList, &NavigationListWidget::navigationRequested, this, &SidebarWidget::navigationRequested);
}

void SidebarWidget::setupToggleAnimation()
{
    connect(m_toggleButton, &QPushButton::clicked, this, [this]() {
        bool isExpanded = (width() > kSidebarCollapsedWidth);
        auto* group = new QParallelAnimationGroup(this);
        auto* anim1 = new QPropertyAnimation(this, "minimumWidth");
        auto* anim2 = new QPropertyAnimation(this, "maximumWidth");
        anim1->setDuration(220);
        anim2->setDuration(220);
        anim1->setEasingCurve(QEasingCurve::InOutQuad);
        anim2->setEasingCurve(QEasingCurve::InOutQuad);
        group->addAnimation(anim1);
        group->addAnimation(anim2);

        if (isExpanded) {
            anim1->setStartValue(kSidebarExpandedWidth); anim1->setEndValue(kSidebarCollapsedWidth);
            anim2->setStartValue(kSidebarExpandedWidth); anim2->setEndValue(kSidebarCollapsedWidth);
            m_navList->setCollapsed(true);
            m_timeCard->setCollapsed(true);
            m_studentCard->setCollapsed(true);
            m_toggleButton->setText(QString(QChar(0x2630)));
            m_toggleButton->setToolTip("展开导航栏");
            connect(group, &QParallelAnimationGroup::finished, this, [this, group]() {
                m_brandMark->hide();
                m_brandText->hide();
                group->deleteLater();
            });
        } else {
            anim1->setStartValue(kSidebarCollapsedWidth); anim1->setEndValue(kSidebarExpandedWidth);
            anim2->setStartValue(kSidebarCollapsedWidth); anim2->setEndValue(kSidebarExpandedWidth);
            m_navList->setCollapsed(false);
            m_timeCard->setCollapsed(false);
            m_studentCard->setCollapsed(false);
            m_toggleButton->setText(QString(QChar(0x2261)));
            m_toggleButton->setToolTip("收起导航栏");
            m_brandMark->show();
            m_brandText->show();
            connect(group, &QParallelAnimationGroup::finished, group, &QObject::deleteLater);
        }
        group->start();
    });
}

void SidebarWidget::refreshData()
{
    if (m_timeCard) m_timeCard->refreshNow();
    if (m_studentCard) m_studentCard->loadFromSettings();
}
