#include "client/dialogs/PeerEditorDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

PeerEditorDialog::PeerEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Peer Benchmark Editor");
    resize(520, 620);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel("Add peer benchmark data to support horizontal comparison analysis.", this);
    intro->setObjectName("pageSubtitle");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_majorEdit = new QLineEdit(this);
    m_semesterEdit = new QLineEdit(this);
    m_semesterEdit->setPlaceholderText("For example: 2026-Spring");
    m_gpaSpin = new QDoubleSpinBox(this);
    m_gpaSpin->setRange(0.0, 5.0);
    m_gpaSpin->setDecimals(2);
    m_creditsSpin = new QDoubleSpinBox(this);
    m_creditsSpin->setRange(0.0, 300.0);
    m_creditsSpin->setDecimals(1);
    m_achievementSpin = new QSpinBox(this);
    m_achievementSpin->setRange(0, 999);
    m_experienceSpin = new QSpinBox(this);
    m_experienceSpin->setRange(0, 999);
    m_noteEdit = new QPlainTextEdit(this);
    m_noteEdit->setObjectName("richCardText");
    m_noteEdit->setMinimumHeight(120);

    form->addRow("Name", m_nameEdit);
    form->addRow("Major", m_majorEdit);
    form->addRow("Semester", m_semesterEdit);
    form->addRow("GPA", m_gpaSpin);
    form->addRow("Credits", m_creditsSpin);
    form->addRow("Achievement count", m_achievementSpin);
    form->addRow("Experience count", m_experienceSpin);
    form->addRow("Note", m_noteEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void PeerEditorDialog::setPeer(const PeerBenchmark& peer)
{
    m_peerId = peer.id;
    m_nameEdit->setText(peer.name);
    m_majorEdit->setText(peer.major);
    m_semesterEdit->setText(peer.semester);
    m_gpaSpin->setValue(peer.gpa);
    m_creditsSpin->setValue(peer.credits);
    m_achievementSpin->setValue(peer.achievementsCount);
    m_experienceSpin->setValue(peer.experiencesCount);
    m_noteEdit->setPlainText(peer.note);
}

PeerBenchmark PeerEditorDialog::peer() const
{
    PeerBenchmark peer;
    peer.id = m_peerId;
    peer.name = m_nameEdit->text().trimmed();
    peer.major = m_majorEdit->text().trimmed();
    peer.semester = m_semesterEdit->text().trimmed();
    peer.gpa = m_gpaSpin->value();
    peer.credits = m_creditsSpin->value();
    peer.achievementsCount = m_achievementSpin->value();
    peer.experiencesCount = m_experienceSpin->value();
    peer.note = m_noteEdit->toPlainText().trimmed();
    return peer;
}

void PeerEditorDialog::validateAndAccept()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, "Please enter a name first.");
        return;
    }
    accept();
}
