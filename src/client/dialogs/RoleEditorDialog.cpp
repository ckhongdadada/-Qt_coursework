#include "client/dialogs/RoleEditorDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

RoleEditorDialog::RoleEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("角色编辑"));
    resize(520, 620);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel(QStringLiteral("维护任职、班委、团队职责等角色信息，保存后会同步参与时间轴和简历生成。"), this);
    intro->setObjectName(QStringLiteral("pageSubtitle"));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_titleEdit = new QLineEdit(this);
    m_typeEdit = new QLineEdit(this);
    m_orgEdit = new QLineEdit(this);
    m_startEdit = new QLineEdit(this);
    m_endEdit = new QLineEdit(this);
    m_activeCheck = new QCheckBox(QStringLiteral("当前仍在进行"), this);
    m_achievementEdit = new QLineEdit(this);
    m_achievementEdit->setPlaceholderText(QStringLiteral("用逗号分隔主要成果"));
    m_contactEdit = new QLineEdit(this);
    m_supervisorEdit = new QLineEdit(this);
    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setObjectName(QStringLiteral("richCardText"));
    m_descriptionEdit->setMinimumHeight(120);

    form->addRow(QStringLiteral("角色名称"), m_titleEdit);
    form->addRow(QStringLiteral("角色类型"), m_typeEdit);
    form->addRow(QStringLiteral("所属组织"), m_orgEdit);
    form->addRow(QStringLiteral("开始时间"), m_startEdit);
    form->addRow(QStringLiteral("结束时间"), m_endEdit);
    form->addRow(QString(), m_activeCheck);
    form->addRow(QStringLiteral("成果摘要"), m_achievementEdit);
    form->addRow(QStringLiteral("联系人"), m_contactEdit);
    form->addRow(QStringLiteral("指导老师"), m_supervisorEdit);
    form->addRow(QStringLiteral("职责说明"), m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void RoleEditorDialog::setRole(const Role& role)
{
    m_roleId = role.id;
    m_titleEdit->setText(role.title);
    m_typeEdit->setText(role.type);
    m_orgEdit->setText(role.organization);
    m_startEdit->setText(role.startDate);
    m_endEdit->setText(role.endDate);
    m_activeCheck->setChecked(role.isActive);
    m_achievementEdit->setText(role.achievements);
    m_contactEdit->setText(role.contact);
    m_supervisorEdit->setText(role.supervisor);
    m_descriptionEdit->setPlainText(role.description);
}

Role RoleEditorDialog::role() const
{
    Role role;
    role.id = m_roleId;
    role.title = m_titleEdit->text().trimmed();
    role.type = m_typeEdit->text().trimmed();
    role.organization = m_orgEdit->text().trimmed();
    role.startDate = m_startEdit->text().trimmed();
    role.endDate = m_endEdit->text().trimmed();
    role.isActive = m_activeCheck->isChecked();
    role.achievements = m_achievementEdit->text().trimmed();
    role.contact = m_contactEdit->text().trimmed();
    role.supervisor = m_supervisorEdit->text().trimmed();
    role.description = m_descriptionEdit->toPlainText().trimmed();
    return role;
}

void RoleEditorDialog::validateAndAccept()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, QStringLiteral("请先填写角色名称。"));
        return;
    }
    accept();
}
