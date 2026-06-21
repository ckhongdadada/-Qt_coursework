#pragma once

#include <QDialog>

#include "model/Job.h"

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QWidget;

class JobEditorDialog : public QDialog {
public:
    explicit JobEditorDialog(QWidget* parent = nullptr);

    void setJob(const Job& job);
    Job job() const;

private:
    void validateAndAccept();

    int m_jobId = 0;
    QList<JobRequirement> m_existingRequirements;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_companyEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLineEdit* m_salaryEdit = nullptr;
    QLineEdit* m_sourceEdit = nullptr;
    QLineEdit* m_urlEdit = nullptr;
    QSpinBox* m_prioritySpin = nullptr;
    QCheckBox* m_activeCheck = nullptr;
    QPlainTextEdit* m_requirementsEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
