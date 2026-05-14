#include "client/dialogs/ExperienceEditorDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

ExperienceEditorDialog::ExperienceEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Experience Editor");
    resize(540, 680);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel("Maintain project, internship, research, and practice experience here.", this);
    intro->setObjectName("pageSubtitle");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_titleEdit = new QLineEdit(this);
    m_typeEdit = new QLineEdit(this);
    m_orgEdit = new QLineEdit(this);
    m_roleEdit = new QLineEdit(this);
    m_startEdit = new QLineEdit(this);
    m_endEdit = new QLineEdit(this);
    m_ongoingCheck = new QCheckBox("Ongoing", this);
    m_techEdit = new QLineEdit(this);
    m_achievementEdit = new QLineEdit(this);
    m_supervisorEdit = new QLineEdit(this);
    m_contactEdit = new QLineEdit(this);
    m_locationEdit = new QLineEdit(this);
    m_urlEdit = new QLineEdit(this);
    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setObjectName("richCardText");
    m_descriptionEdit->setMinimumHeight(120);

    form->addRow("Title", m_titleEdit);
    form->addRow("Type", m_typeEdit);
    form->addRow("Organization / team", m_orgEdit);
    form->addRow("Role", m_roleEdit);
    form->addRow("Start date", m_startEdit);
    form->addRow("End date", m_endEdit);
    form->addRow("", m_ongoingCheck);
    form->addRow("Technologies", m_techEdit);
    form->addRow("Achievements", m_achievementEdit);
    form->addRow("Supervisor", m_supervisorEdit);
    form->addRow("Contact", m_contactEdit);
    form->addRow("Location", m_locationEdit);
    form->addRow("URL", m_urlEdit);
    form->addRow("Description", m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void ExperienceEditorDialog::setExperience(const Experience& experience)
{
    m_experienceId = experience.id;
    m_titleEdit->setText(experience.title);
    m_typeEdit->setText(experience.type);
    m_orgEdit->setText(experience.organization);
    m_roleEdit->setText(experience.role);
    m_startEdit->setText(experience.startDate);
    m_endEdit->setText(experience.endDate);
    m_ongoingCheck->setChecked(experience.isOngoing);
    m_techEdit->setText(experience.technologies);
    m_achievementEdit->setText(experience.achievements);
    m_supervisorEdit->setText(experience.supervisor);
    m_contactEdit->setText(experience.contact);
    m_locationEdit->setText(experience.location);
    m_urlEdit->setText(experience.url);
    m_descriptionEdit->setPlainText(experience.description);
}

Experience ExperienceEditorDialog::experience() const
{
    Experience experience;
    experience.id = m_experienceId;
    experience.title = m_titleEdit->text().trimmed();
    experience.type = m_typeEdit->text().trimmed();
    experience.organization = m_orgEdit->text().trimmed();
    experience.role = m_roleEdit->text().trimmed();
    experience.startDate = m_startEdit->text().trimmed();
    experience.endDate = m_endEdit->text().trimmed();
    experience.isOngoing = m_ongoingCheck->isChecked();
    experience.technologies = m_techEdit->text().trimmed();
    experience.achievements = m_achievementEdit->text().trimmed();
    experience.supervisor = m_supervisorEdit->text().trimmed();
    experience.contact = m_contactEdit->text().trimmed();
    experience.location = m_locationEdit->text().trimmed();
    experience.url = m_urlEdit->text().trimmed();
    experience.description = m_descriptionEdit->toPlainText().trimmed();
    return experience;
}

void ExperienceEditorDialog::validateAndAccept()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, "Please enter an experience title first.");
        return;
    }
    accept();
}
