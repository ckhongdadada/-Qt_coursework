#pragma once

#include <QDialog>

#include "model/Activity.h"

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QWidget;

class ActivityEditorDialog : public QDialog {
public:
    explicit ActivityEditorDialog(QWidget* parent = nullptr);

    void setActivity(const Activity& activity);
    Activity activity() const;

private:
    void validateAndAccept();

    int m_activityId = 0;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
    QLineEdit* m_startEdit = nullptr;
    QLineEdit* m_endEdit = nullptr;
    QLineEdit* m_tagsEdit = nullptr;
    QCheckBox* m_favoriteCheck = nullptr;
    QCheckBox* m_activeCheck = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
