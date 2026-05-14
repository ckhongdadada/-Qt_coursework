#pragma once

#include <QDialog>

#include "model/Experience.h"

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QWidget;

class ExperienceEditorDialog : public QDialog {
public:
    explicit ExperienceEditorDialog(QWidget* parent = nullptr);

    void setExperience(const Experience& experience);
    Experience experience() const;

private:
    void validateAndAccept();

    int m_experienceId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_typeEdit = nullptr;
    QLineEdit* m_orgEdit = nullptr;
    QLineEdit* m_roleEdit = nullptr;
    QLineEdit* m_startEdit = nullptr;
    QLineEdit* m_endEdit = nullptr;
    QCheckBox* m_ongoingCheck = nullptr;
    QLineEdit* m_techEdit = nullptr;
    QLineEdit* m_achievementEdit = nullptr;
    QLineEdit* m_supervisorEdit = nullptr;
    QLineEdit* m_contactEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLineEdit* m_urlEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
