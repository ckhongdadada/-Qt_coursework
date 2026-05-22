#pragma once

#include <QDialog>

#include "model/Course.h"

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QWidget;

class CourseEditorDialog : public QDialog {
public:
    explicit CourseEditorDialog(QWidget* parent = nullptr);

    void setCourse(const Course& course);
    Course course() const;

private:
    static void setComboValue(QComboBox* combo, const QString& value);
    void validateAndAccept();

    int m_courseId = 0;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_codeEdit = nullptr;
    QLineEdit* m_semesterEdit = nullptr;
    QDoubleSpinBox* m_creditsSpin = nullptr;
    QDoubleSpinBox* m_scoreSpin = nullptr;
    QComboBox* m_categoryCombo = nullptr;
    QComboBox* m_statusCombo = nullptr;
    QLineEdit* m_teacherEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLineEdit* m_tagsEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};
