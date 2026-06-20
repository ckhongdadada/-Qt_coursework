#include "client/dialogs/JobEditorDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

JobEditorDialog::JobEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Job Editor");
    resize(560, 700);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel("Maintain target jobs, requirement matching, and source information here.", this);
    intro->setObjectName("pageSubtitle");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_titleEdit = new QLineEdit(this);
    m_companyEdit = new QLineEdit(this);
    m_locationEdit = new QLineEdit(this);
    m_salaryEdit = new QLineEdit(this);
    m_sourceEdit = new QLineEdit(this);
    m_urlEdit = new QLineEdit(this);
    m_prioritySpin = new QSpinBox(this);
    m_prioritySpin->setRange(0, 5);
    m_activeCheck = new QCheckBox("Keep tracking this job", this);
    m_requirementsEdit = new QPlainTextEdit(this);
    m_requirementsEdit->setObjectName("richCardText");
    m_requirementsEdit->setMinimumHeight(140);
    m_requirementsEdit->setPlaceholderText("One requirement per line");
    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setObjectName("richCardText");
    m_descriptionEdit->setMinimumHeight(120);

    form->addRow("Title", m_titleEdit);
    form->addRow("Company", m_companyEdit);
    form->addRow("City / location", m_locationEdit);
    form->addRow("Salary range", m_salaryEdit);
    form->addRow("Source", m_sourceEdit);
    form->addRow("Priority", m_prioritySpin);
    form->addRow("URL", m_urlEdit);
    form->addRow("", m_activeCheck);
    form->addRow("Requirements", m_requirementsEdit);
    form->addRow("Description", m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void JobEditorDialog::setJob(const Job& job)
{
    m_jobId = job.id;
    m_existingRequirements = job.requirements;
    m_titleEdit->setText(job.title);
    m_companyEdit->setText(job.company);
    m_locationEdit->setText(job.location);
    m_salaryEdit->setText(job.salaryRange);
    m_sourceEdit->setText(job.source);
    m_urlEdit->setText(job.url);
    m_prioritySpin->setValue(job.priority);
    m_activeCheck->setChecked(job.isActive);
    QStringList lines;
    for (const auto& requirement : job.requirements) {
        lines << requirement.text;
    }
    m_requirementsEdit->setPlainText(lines.join('\n'));
    m_descriptionEdit->setPlainText(job.description);
}

Job JobEditorDialog::job() const
{
    Job job;
    job.id = m_jobId;
    job.title = m_titleEdit->text().trimmed();
    job.company = m_companyEdit->text().trimmed();
    job.location = m_locationEdit->text().trimmed();
    job.salaryRange = m_salaryEdit->text().trimmed();
    job.source = m_sourceEdit->text().trimmed();
    job.url = m_urlEdit->text().trimmed();
    job.priority = m_prioritySpin->value();
    job.isActive = m_activeCheck->isChecked();
    job.description = m_descriptionEdit->toPlainText().trimmed();

    const QStringList lines = m_requirementsEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const QString& rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        JobRequirement requirement;
        requirement.text = line;
        for (const auto& existing : m_existingRequirements) {
            if (existing.text.trimmed() == line) {
                requirement.met = existing.met;
                break;
            }
        }
        job.requirements.append(requirement);
    }
    return job;
}

void JobEditorDialog::validateAndAccept()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, "Please enter a job title first.");
        return;
    }
    accept();
}
