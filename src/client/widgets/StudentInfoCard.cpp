#include "StudentInfoCard.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>

#include "client/dialogs/ProfileEditorDialog.h"

StudentInfoCard::StudentInfoCard(QWidget* parent)
    : QPushButton(parent)
{
    setObjectName("sidebarInfoCard");
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(72);
    setStyleSheet(
        "#sidebarInfoCard { border: 1px solid transparent; background: transparent; border-radius: 14px; text-align: left; }"
        " #sidebarInfoCard:hover { background: #f0ddd1; }");
    setupUi();
    loadFromSettings();

    connect(this, &QPushButton::clicked, this, [this]() {
        openEditorNear(this);
    });
}

void StudentInfoCard::setupUi()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(8);

    m_avatar = new QLabel("👤", this);
    m_avatar->setObjectName("sidebarInfoAvatar");
    m_avatar->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_avatar->setToolTip("学生信息");
    layout->addWidget(m_avatar, 0, Qt::AlignTop);

    m_textWidget = new QWidget(this);
    m_textWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout* textLayout = new QVBoxLayout(m_textWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(1);

    QLabel* kicker = new QLabel("Student", m_textWidget);
    kicker->setObjectName("sidebarInfoKicker");
    kicker->setAttribute(Qt::WA_TransparentForMouseEvents);
    textLayout->addWidget(kicker);

    m_nameLabel = new QLabel("请填写姓名", m_textWidget);
    m_nameLabel->setObjectName("sidebarInfoTitle");
    m_nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    textLayout->addWidget(m_nameLabel);

    m_metaLabel = new QLabel("请填写学号 · 请填写院系", m_textWidget);
    m_metaLabel->setObjectName("sidebarInfoDetail");
    m_metaLabel->setWordWrap(false);
    m_metaLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    textLayout->addWidget(m_metaLabel);

    layout->addWidget(m_textWidget, 1);
}

void StudentInfoCard::loadFromSettings()
{
    QSettings settings;
    QString name = settings.value("profile/name", "请点击设置姓名").toString();
    QString sid = settings.value("profile/studentId", "未填写学号").toString();
    QString dept = settings.value("profile/department", "未填写院系").toString();

    if (m_nameLabel) m_nameLabel->setText(name);
    if (m_metaLabel) m_metaLabel->setText(QString("%1 · %2").arg(sid, dept));
}

void StudentInfoCard::saveToSettings()
{
}

void StudentInfoCard::updateDisplay()
{
    loadFromSettings();
}

void StudentInfoCard::openEditorNear(QWidget* anchor)
{
    ProfileEditorDialog dialog(qobject_cast<QWidget*>(parent()));
    if (dialog.showNear(anchor), dialog.exec() == QDialog::Accepted) {
        dialog.save();
        loadFromSettings();
    }
}

void StudentInfoCard::setCollapsed(bool collapsed)
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

bool StudentInfoCard::isCollapsed() const { return m_collapsed; }
