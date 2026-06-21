#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>

class JobsPage : public BasePage {
    Q_OBJECT

public:
    explicit JobsPage(QWidget* parent = nullptr);
    ~JobsPage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onRemoveClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onJobSelectionChanged(int row);
    void onRequirementClicked(QListWidgetItem* item);

private:
    void setupUi();

    QLabel* m_summaryLabel = nullptr;
    QListWidget* m_list = nullptr;
    QLineEdit* m_searchInput = nullptr;
    QLineEdit* m_statusInput = nullptr;

    QListWidget* m_requirementList = nullptr;
    QLabel* m_requirementSummaryLabel = nullptr;

    QLabel* m_totalValue = nullptr;
    QLabel* m_activeValue = nullptr;
    QLabel* m_requirementValue = nullptr;
};
