#include "TimeInfoCard.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>

TimeInfoCard::TimeInfoCard(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("sidebarInfoCard");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(72);
    setupUi();
    refreshNow();
}

void TimeInfoCard::setupUi()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(8);

    m_avatar = new QLabel("⏱", this);
    m_avatar->setObjectName("sidebarInfoAvatar");
    m_avatar->setToolTip("当前学期与时间");
    layout->addWidget(m_avatar, 0, Qt::AlignTop);

    m_textWidget = new QWidget(this);
    QVBoxLayout* textLayout = new QVBoxLayout(m_textWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    QLabel* kicker = new QLabel("Time", m_textWidget);
    kicker->setObjectName("sidebarInfoKicker");
    textLayout->addWidget(kicker);

    m_semesterLabel = new QLabel("--", m_textWidget);
    m_semesterLabel->setObjectName("sidebarInfoTitle");
    textLayout->addWidget(m_semesterLabel);

    m_detailLabel = new QLabel("--", m_textWidget);
    m_detailLabel->setObjectName("sidebarInfoDetail");
    m_detailLabel->setWordWrap(false);
    textLayout->addWidget(m_detailLabel);

    layout->addWidget(m_textWidget, 1);
}

void TimeInfoCard::refreshNow()
{
    const QDateTime now = QDateTime::currentDateTime();
    const int month = now.date().month();
    int year = now.date().year();
    QString semester;
    if (month >= 2 && month <= 7) {
        semester = QString("%1 春季学期").arg(year);
    } else if (month == 8) {
        semester = QString("%1 夏季学期").arg(year);
    } else {
        if (month == 1) year -= 1;
        semester = QString("%1 秋季学期").arg(year);
    }

    if (m_semesterLabel) m_semesterLabel->setText(semester);
    if (m_detailLabel) {
        m_detailLabel->setText(QString("%1 · %2").arg(now.date().toString("yyyy-MM-dd")).arg(now.time().toString("HH:mm")));
    }
}

QString TimeInfoCard::semesterText() const { return m_semesterLabel ? m_semesterLabel->text() : QString(); }
QString TimeInfoCard::dateTimeText() const { return m_detailLabel ? m_detailLabel->text() : QString(); }

void TimeInfoCard::setCollapsed(bool collapsed)
{
    m_collapsed = collapsed;
    if (collapsed) {
        m_textWidget->hide();
        setMinimumHeight(44);
        QHBoxLayout* l = qobject_cast<QHBoxLayout*>(layout());
        if (l) {
            l->setAlignment(m_avatar, Qt::AlignHCenter);
            l->setContentsMargins(0, 6, 0, 6);
        }
    } else {
        m_textWidget->show();
        setMinimumHeight(72);
        QHBoxLayout* l = qobject_cast<QHBoxLayout*>(layout());
        if (l) {
            l->setAlignment(m_avatar, Qt::AlignTop);
            l->setContentsMargins(8, 6, 8, 6);
        }
    }
}

bool TimeInfoCard::isCollapsed() const { return m_collapsed; }
