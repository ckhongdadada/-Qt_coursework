#include "client/dialogs/ActivityEditorDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

ActivityEditorDialog::ActivityEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Activity Editor");
    resize(520, 600);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel("Maintain extracurricular activity, competition, project, and volunteer records here.", this);
    intro->setObjectName("pageSubtitle");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_categoryEdit = new QLineEdit(this);
    m_startEdit = new QLineEdit(this);
    m_endEdit = new QLineEdit(this);
    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText("Comma-separated tags");
    m_favoriteCheck = new QCheckBox("Mark as key activity", this);
    m_activeCheck = new QCheckBox("Ongoing", this);
    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setObjectName("richCardText");
    m_descriptionEdit->setMinimumHeight(120);

    form->addRow("Name", m_nameEdit);
    form->addRow("Category", m_categoryEdit);
    form->addRow("Start date", m_startEdit);
    form->addRow("End date", m_endEdit);
    form->addRow("Tags", m_tagsEdit);
    form->addRow("", m_favoriteCheck);
    form->addRow("", m_activeCheck);
    form->addRow("Description", m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void ActivityEditorDialog::setActivity(const Activity& activity)
{
    m_activityId = activity.id;
    m_nameEdit->setText(activity.name);
    m_categoryEdit->setText(activity.category);
    m_startEdit->setText(activity.startDate);
    m_endEdit->setText(activity.endDate);
    m_tagsEdit->setText(activity.tags);
    m_favoriteCheck->setChecked(activity.isFavorite);
    m_activeCheck->setChecked(activity.isActive);
    m_descriptionEdit->setPlainText(activity.description);
}

Activity ActivityEditorDialog::activity() const
{
    Activity activity;
    activity.id = m_activityId;
    activity.name = m_nameEdit->text().trimmed();
    activity.category = m_categoryEdit->text().trimmed();
    activity.startDate = m_startEdit->text().trimmed();
    activity.endDate = m_endEdit->text().trimmed();
    activity.tags = m_tagsEdit->text().trimmed();
    activity.isFavorite = m_favoriteCheck->isChecked();
    activity.isActive = m_activeCheck->isChecked();
    activity.description = m_descriptionEdit->toPlainText().trimmed();
    return activity;
}

void ActivityEditorDialog::validateAndAccept()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, "Please enter an activity name first.");
        return;
    }
    accept();
}
