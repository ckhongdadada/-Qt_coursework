#pragma once

#include <QFrame>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QJsonObject>

class ResumeEditorPanel : public QFrame {
    Q_OBJECT

public:
    explicit ResumeEditorPanel(QWidget* parent = nullptr);

    void loadOptions(const QJsonObject& options);
    QJsonObject collectOptions() const;
    void setSelectedSection(const QString& key);

    QLineEdit* nameInput() const { return m_nameInput; }
    QTextEdit* summaryInput() const { return m_summaryInput; }
    QTextEdit* customContentInput() const { return m_customContentInput; }
    QTextEdit* educationBodyInput() const { return m_educationBodyInput; }
    QTextEdit* skillsBodyInput() const { return m_skillsBodyInput; }
    QTextEdit* projectBodyInput() const { return m_projectBodyInput; }
    QTextEdit* internshipInput() const { return m_internshipInput; }
    QTextEdit* awardsInput() const { return m_awardsInput; }

signals:
    void optionsChanged();
    void sectionChanged(const QString& key);

private:
    void setupUi();

    QLineEdit* m_nameInput = nullptr;
    QLineEdit* m_titleInput = nullptr;
    QLineEdit* m_emailInput = nullptr;
    QLineEdit* m_phoneInput = nullptr;
    QLineEdit* m_ageInput = nullptr;
    QLineEdit* m_cityInput = nullptr;
    QLineEdit* m_intentInput = nullptr;
    QLineEdit* m_schoolInput = nullptr;
    QLineEdit* m_majorInput = nullptr;
    QLineEdit* m_degreeInput = nullptr;
    QTextEdit* m_summaryInput = nullptr;
    QTextEdit* m_customContentInput = nullptr;
    QTextEdit* m_educationBodyInput = nullptr;
    QTextEdit* m_skillsBodyInput = nullptr;
    QLineEdit* m_projectNameInput = nullptr;
    QTextEdit* m_projectBodyInput = nullptr;
    QTextEdit* m_internshipInput = nullptr;
    QTextEdit* m_awardsInput = nullptr;
    QCheckBox* m_educationCheck = nullptr;
    QCheckBox* m_experienceCheck = nullptr;
    QCheckBox* m_achievementCheck = nullptr;
    QCheckBox* m_roleCheck = nullptr;
    QCheckBox* m_activityCheck = nullptr;
};
