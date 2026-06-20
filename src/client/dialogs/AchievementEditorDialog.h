#pragma once

#include <QDialog>

#include "model/Achievement.h"

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QWidget;

class AchievementEditorDialog : public QDialog {
public:
    explicit AchievementEditorDialog(QWidget* parent = nullptr);

    void setAchievement(const Achievement& achievement);
    Achievement achievement() const;

private:
    void validateAndAccept();

    int m_achievementId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_typeEdit = nullptr;
    QLineEdit* m_levelEdit = nullptr;
    QLineEdit* m_orgEdit = nullptr;
    QLineEdit* m_dateEdit = nullptr;
    QLineEdit* m_certificateEdit = nullptr;
    QLineEdit* m_relatedCourseEdit = nullptr;
    QLineEdit* m_teamEdit = nullptr;
    QLineEdit* m_rankingEdit = nullptr;
    QLineEdit* m_prizeEdit = nullptr;
    QCheckBox* m_verifiedCheck = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
