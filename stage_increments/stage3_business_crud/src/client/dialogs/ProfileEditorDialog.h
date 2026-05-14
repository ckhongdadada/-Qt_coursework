#pragma once

#include <QDialog>

class QLineEdit;
class QWidget;

class ProfileEditorDialog : public QDialog {
public:
    explicit ProfileEditorDialog(QWidget* parent = nullptr);

    void showNear(QWidget* anchor);
    void save();

private:
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_sidEdit = nullptr;
    QLineEdit* m_deptEdit = nullptr;
    QLineEdit* m_emailEdit = nullptr;
    QLineEdit* m_phoneEdit = nullptr;
};
