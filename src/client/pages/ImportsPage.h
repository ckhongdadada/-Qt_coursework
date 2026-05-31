#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QFile>

class ImportsPage : public BasePage {
    Q_OBJECT

public:
    explicit ImportsPage(QWidget* parent = nullptr);
    ~ImportsPage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);
    void importCompleted();

private slots:
    void onChooseFileClicked();
    void onRunImportClicked();

private:
    void setupUi();

    QLabel* m_summaryLabel = nullptr;
    QComboBox* m_entityCombo = nullptr;
    QLabel* m_fileLabel = nullptr;
    QLabel* m_importedValue = nullptr;
    QLabel* m_failedValue = nullptr;
    QTableWidget* m_errorTable = nullptr;
    QString m_filePath;
};
