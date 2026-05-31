#pragma once

#include <QFrame>
#include <QTableWidget>
#include <QLabel>
#include <QListWidget>
#include <QJsonObject>

class SemesterAnalysisWidget : public QFrame {
    Q_OBJECT

public:
    explicit SemesterAnalysisWidget(QWidget* parent = nullptr);

    void loadReport(const QJsonObject& report);
    int semesterCount() const;
    int suggestionCount() const;

private:
    void setupUi();

    QTableWidget* m_semesterTable = nullptr;
    QListWidget* m_strengthList = nullptr;
    QListWidget* m_riskList = nullptr;
    QListWidget* m_suggestionList = nullptr;
    int m_semesterCount = 0;
    int m_suggestionCount = 0;
};
