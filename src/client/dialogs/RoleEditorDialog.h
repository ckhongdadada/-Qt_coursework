#pragma once

#include <QDialog>

#include "model/Role.h"

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QWidget;

class RoleEditorDialog : public QDialog {
public:
    explicit RoleEditorDialog(QWidget* parent = nullptr);

    void setRole(const Role& role);
    Role role() const;

private:
    void validateAndAccept();

    int m_roleId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_typeEdit = nullptr;
    QLineEdit* m_orgEdit = nullptr;
    QLineEdit* m_startEdit = nullptr;
    QLineEdit* m_endEdit = nullptr;
    QCheckBox* m_activeCheck = nullptr;
    QLineEdit* m_achievementEdit = nullptr;
    QLineEdit* m_contactEdit = nullptr;
    QLineEdit* m_supervisorEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
