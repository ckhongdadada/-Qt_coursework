#include "client/dialogs/ProfileEditorDialog.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QVBoxLayout>

ProfileEditorDialog::ProfileEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("编辑个人信息"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(340, 360);

    auto* container = new QFrame(this);
    container->setObjectName(QStringLiteral("profilePopup"));
    container->setStyleSheet(
        "#profilePopup {"
        "  background: #ffffff; border: 1px solid rgba(67,57,43,0.12); border-radius: 12px;"
        "  padding: 16px;"
        "}"
    );

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->addWidget(container);

    auto* form = new QVBoxLayout(container);
    form->setContentsMargins(16, 14, 16, 10);
    form->setSpacing(9);

    auto* headerLabel = new QLabel(QStringLiteral("学生信息"), container);
    QFont headerFont = headerLabel->font();
    headerFont.setPointSize(13);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    headerLabel->setStyleSheet(QStringLiteral("color: #24211d; padding-bottom: 6px;"));
    form->addWidget(headerLabel);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(7);

    m_nameEdit = new QLineEdit(container);
    m_nameEdit->setPlaceholderText(QStringLiteral("姓名"));
    m_sidEdit = new QLineEdit(container);
    m_sidEdit->setPlaceholderText(QStringLiteral("学号"));
    m_deptEdit = new QLineEdit(container);
    m_deptEdit->setPlaceholderText(QStringLiteral("院系"));
    m_emailEdit = new QLineEdit(container);
    m_emailEdit->setPlaceholderText(QStringLiteral("邮箱"));
    m_phoneEdit = new QLineEdit(container);
    m_phoneEdit->setPlaceholderText(QStringLiteral("电话"));

    const QString inputStyle =
        QStringLiteral("QLineEdit { border: 1px solid #ddd3c6; border-radius: 6px; padding: 7px 10px; font-size: 12px; background: #faf8f4; } "
                       "QLineEdit:focus { border-color: #2b5c5d; }");
    m_nameEdit->setStyleSheet(inputStyle);
    m_sidEdit->setStyleSheet(inputStyle);
    m_deptEdit->setStyleSheet(inputStyle);
    m_emailEdit->setStyleSheet(inputStyle);
    m_phoneEdit->setStyleSheet(inputStyle);

    grid->addWidget(new QLabel(QStringLiteral("姓名")), 0, 0);
    grid->addWidget(m_nameEdit, 0, 1);
    grid->addWidget(new QLabel(QStringLiteral("学号")), 1, 0);
    grid->addWidget(m_sidEdit, 1, 1);
    grid->addWidget(new QLabel(QStringLiteral("院系")), 2, 0);
    grid->addWidget(m_deptEdit, 2, 1);
    grid->addWidget(new QLabel(QStringLiteral("邮箱")), 3, 0);
    grid->addWidget(m_emailEdit, 3, 1);
    grid->addWidget(new QLabel(QStringLiteral("电话")), 4, 0);
    grid->addWidget(m_phoneEdit, 4, 1);

    for (int i = 0; i < 5; ++i) {
        if (auto* label = qobject_cast<QLabel*>(grid->itemAtPosition(i, 0)->widget())) {
            label->setStyleSheet(QStringLiteral("color: #888; font-size: 11px;"));
        }
    }
    form->addLayout(grid);

    auto* buttonRow = new QHBoxLayout();
    auto* saveButton = new QPushButton(QStringLiteral("保存"), container);
    auto* cancelButton = new QPushButton(QStringLiteral("取消"), container);
    saveButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setCursor(Qt::PointingHandCursor);
    saveButton->setStyleSheet(
        QStringLiteral("QPushButton { background: #2b5c5d; color: white; border-radius: 6px; padding: 7px 20px; font-size: 12px; font-weight: bold; border: none; }"
                       "QPushButton:hover { background: #234a4b; }")
    );
    cancelButton->setStyleSheet(
        QStringLiteral("QPushButton { background: transparent; color: #666; border: 1px solid #ddd3c6; border-radius: 6px; padding: 7px 20px; font-size: 12px; }"
                       "QPushButton:hover { background: #f0eee8; }")
    );
    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonRow->addStretch();
    buttonRow->addWidget(saveButton);
    buttonRow->addWidget(cancelButton);
    form->addLayout(buttonRow);

    QSettings settings;
    m_nameEdit->setText(settings.value(QStringLiteral("profile/name"), QString()).toString());
    m_sidEdit->setText(settings.value(QStringLiteral("profile/studentId"), QString()).toString());
    m_deptEdit->setText(settings.value(QStringLiteral("profile/department"), QString()).toString());
    m_emailEdit->setText(settings.value(QStringLiteral("profile/email"), QString()).toString());
    m_phoneEdit->setText(settings.value(QStringLiteral("profile/phone"), QString()).toString());
}

void ProfileEditorDialog::showNear(QWidget* anchor)
{
    const QRect screenRect = anchor->screen()
        ? anchor->screen()->availableGeometry()
        : QApplication::primaryScreen()->availableGeometry();
    QPoint globalPos = anchor->mapToGlobal(QPoint(anchor->width() - width() - 8, -height() - 8));
    if (globalPos.y() < screenRect.top() + 12) {
        globalPos.setY(anchor->mapToGlobal(QPoint(0, anchor->height() + 8)).y());
    }
    globalPos.setX(qBound(screenRect.left() + 12, globalPos.x(), screenRect.right() - width() - 12));
    globalPos.setY(qBound(screenRect.top() + 12, globalPos.y(), screenRect.bottom() - height() - 12));
    move(globalPos);
}

void ProfileEditorDialog::save()
{
    QSettings settings;
    settings.setValue(QStringLiteral("profile/name"), m_nameEdit->text().trimmed());
    settings.setValue(QStringLiteral("profile/studentId"), m_sidEdit->text().trimmed());
    settings.setValue(QStringLiteral("profile/department"), m_deptEdit->text().trimmed());
    settings.setValue(QStringLiteral("profile/email"), m_emailEdit->text().trimmed());
    settings.setValue(QStringLiteral("profile/phone"), m_phoneEdit->text().trimmed());
}
