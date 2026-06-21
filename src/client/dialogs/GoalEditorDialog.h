#pragma once

#include <QDialog>

#include "model/Goal.h"

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QWidget;

class GoalEditorDialog : public QDialog {
public:
    explicit GoalEditorDialog(QWidget* parent = nullptr);

    void setGoal(const Goal& goal);
    Goal goal() const;

private:
    static void setComboValue(QComboBox* combo, const QString& value);
    void validateAndAccept();

    int m_goalId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
    QDoubleSpinBox* m_targetSpin = nullptr;
    QDoubleSpinBox* m_currentSpin = nullptr;
    QLineEdit* m_unitEdit = nullptr;
    QLineEdit* m_deadlineEdit = nullptr;
    QComboBox* m_priorityCombo = nullptr;
    QComboBox* m_statusCombo = nullptr;
    QLineEdit* m_milestonesEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
