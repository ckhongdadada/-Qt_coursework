#include "client/dialogs/AchievementEditorDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

AchievementEditorDialog::AchievementEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Achievement Editor");
    resize(540, 660);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel("Maintain awards, certificates, competitions, and honors here.", this);
    intro->setObjectName("pageSubtitle");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_titleEdit = new QLineEdit(this);
    m_typeEdit = new QLineEdit(this);
    m_levelEdit = new QLineEdit(this);
    m_orgEdit = new QLineEdit(this);
    m_dateEdit = new QLineEdit(this);
    m_certificateEdit = new QLineEdit(this);
    m_relatedCourseEdit = new QLineEdit(this);
    m_teamEdit = new QLineEdit(this);
    m_rankingEdit = new QLineEdit(this);
    m_prizeEdit = new QLineEdit(this);
    m_verifiedCheck = new QCheckBox("Verified / supporting material available", this);
    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setObjectName("richCardText");
    m_descriptionEdit->setMinimumHeight(120);

    form->addRow("Title", m_titleEdit);
    form->addRow("Type", m_typeEdit);
    form->addRow("Level", m_levelEdit);
    form->addRow("Organization", m_orgEdit);
    form->addRow("Date", m_dateEdit);
    form->addRow("Certificate", m_certificateEdit);
    form->addRow("Related course", m_relatedCourseEdit);
    form->addRow("Team members", m_teamEdit);
    form->addRow("Ranking", m_rankingEdit);
    form->addRow("Prize", m_prizeEdit);
    form->addRow("", m_verifiedCheck);
    form->addRow("Description", m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void AchievementEditorDialog::setAchievement(const Achievement& achievement)
{
    m_achievementId = achievement.id;
    m_titleEdit->setText(achievement.title);
    m_typeEdit->setText(achievement.type);
    m_levelEdit->setText(achievement.level);
    m_orgEdit->setText(achievement.organization);
    m_dateEdit->setText(achievement.date);
    m_certificateEdit->setText(achievement.certificate);
    m_relatedCourseEdit->setText(achievement.relatedCourse);
    m_teamEdit->setText(achievement.teamMembers);
    m_rankingEdit->setText(achievement.ranking);
    m_prizeEdit->setText(achievement.prize);
    m_verifiedCheck->setChecked(achievement.verified);
    m_descriptionEdit->setPlainText(achievement.description);
}

Achievement AchievementEditorDialog::achievement() const
{
    Achievement achievement;
    achievement.id = m_achievementId;
    achievement.title = m_titleEdit->text().trimmed();
    achievement.type = m_typeEdit->text().trimmed();
    achievement.level = m_levelEdit->text().trimmed();
    achievement.organization = m_orgEdit->text().trimmed();
    achievement.date = m_dateEdit->text().trimmed();
    achievement.certificate = m_certificateEdit->text().trimmed();
    achievement.relatedCourse = m_relatedCourseEdit->text().trimmed();
    achievement.teamMembers = m_teamEdit->text().trimmed();
    achievement.ranking = m_rankingEdit->text().trimmed();
    achievement.prize = m_prizeEdit->text().trimmed();
    achievement.verified = m_verifiedCheck->isChecked();
    achievement.description = m_descriptionEdit->toPlainText().trimmed();
    return achievement;
}

void AchievementEditorDialog::validateAndAccept()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, "Please enter an achievement title first.");
        return;
    }
    accept();
}
