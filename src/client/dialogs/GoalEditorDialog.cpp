#include "client/dialogs/GoalEditorDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

GoalEditorDialog::GoalEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("目标编辑"));
    resize(520, 620);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel(QStringLiteral("这里维护目标标题、进度与里程碑，保存后会立即影响总览、时间轴和 AI 建议。"), this);
    intro->setWordWrap(true);
    intro->setObjectName(QStringLiteral("pageSubtitle"));
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_titleEdit = new QLineEdit(this);
    m_categoryEdit = new QLineEdit(this);
    m_categoryEdit->setPlaceholderText(QStringLiteral("例如：学习、竞赛、求职、成长"));

    m_targetSpin = new QDoubleSpinBox(this);
    m_targetSpin->setRange(0.0, 1000000.0);
    m_targetSpin->setDecimals(1);
    m_targetSpin->setSingleStep(1.0);

    m_currentSpin = new QDoubleSpinBox(this);
    m_currentSpin->setRange(0.0, 1000000.0);
    m_currentSpin->setDecimals(1);
    m_currentSpin->setSingleStep(1.0);

    m_unitEdit = new QLineEdit(this);
    m_deadlineEdit = new QLineEdit(this);
    m_deadlineEdit->setPlaceholderText(QStringLiteral("例如：2026-06-30"));

    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItems({QStringLiteral("Low"), QStringLiteral("Medium"), QStringLiteral("High"), QStringLiteral("Critical")});

    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItems({QStringLiteral("Not Started"), QStringLiteral("In Progress"), QStringLiteral("Completed"), QStringLiteral("Paused")});

    m_milestonesEdit = new QLineEdit(this);
    m_milestonesEdit->setPlaceholderText(QStringLiteral("用逗号分隔里程碑"));

    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setMinimumHeight(120);
    m_descriptionEdit->setPlaceholderText(QStringLiteral("记录目标背景、行动计划与评估方式"));
    m_descriptionEdit->setObjectName(QStringLiteral("richCardText"));

    form->addRow(QStringLiteral("目标标题"), m_titleEdit);
    form->addRow(QStringLiteral("分类"), m_categoryEdit);
    form->addRow(QStringLiteral("目标值"), m_targetSpin);
    form->addRow(QStringLiteral("当前值"), m_currentSpin);
    form->addRow(QStringLiteral("单位"), m_unitEdit);
    form->addRow(QStringLiteral("截止时间"), m_deadlineEdit);
    form->addRow(QStringLiteral("优先级"), m_priorityCombo);
    form->addRow(QStringLiteral("状态"), m_statusCombo);
    form->addRow(QStringLiteral("里程碑"), m_milestonesEdit);
    form->addRow(QStringLiteral("说明"), m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void GoalEditorDialog::setGoal(const Goal& goal)
{
    m_goalId = goal.id;
    m_titleEdit->setText(goal.title);
    m_categoryEdit->setText(goal.category);
    m_targetSpin->setValue(goal.targetValue);
    m_currentSpin->setValue(goal.currentValue);
    m_unitEdit->setText(goal.unit);
    m_deadlineEdit->setText(goal.deadline);
    setComboValue(m_priorityCombo, goal.priority);
    setComboValue(m_statusCombo, goal.status);
    m_milestonesEdit->setText(goal.milestones);
    m_descriptionEdit->setPlainText(goal.description);
}

Goal GoalEditorDialog::goal() const
{
    Goal goal;
    goal.id = m_goalId;
    goal.title = m_titleEdit->text().trimmed();
    goal.category = m_categoryEdit->text().trimmed();
    goal.targetValue = m_targetSpin->value();
    goal.currentValue = m_currentSpin->value();
    goal.unit = m_unitEdit->text().trimmed();
    goal.deadline = m_deadlineEdit->text().trimmed();
    goal.priority = m_priorityCombo->currentText();
    goal.status = m_statusCombo->currentText();
    goal.milestones = m_milestonesEdit->text().trimmed();
    goal.description = m_descriptionEdit->toPlainText().trimmed();
    return goal;
}

void GoalEditorDialog::setComboValue(QComboBox* combo, const QString& value)
{
    const int index = combo->findText(value);
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}

void GoalEditorDialog::validateAndAccept()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, QStringLiteral("请先填写目标标题。"));
        return;
    }
    if (m_targetSpin->value() <= 0.0) {
        ToastNotification::display(this, QStringLiteral("目标值需要大于 0。"));
        return;
    }
    if (m_currentSpin->value() > m_targetSpin->value()) {
        const auto result = QMessageBox::question(
            this,
            QStringLiteral("当前值超过目标值"),
            QStringLiteral("当前值已经大于目标值。是否仍然继续保存？"));
        if (result != QMessageBox::Yes) {
            return;
        }
    }
    accept();
}
