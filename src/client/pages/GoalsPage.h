#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>

class GoalsPage : public BasePage {
    Q_OBJECT

public:
    explicit GoalsPage(QWidget* parent = nullptr);
    ~GoalsPage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onRemoveClicked();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUi();

    QLabel* m_summaryLabel = nullptr;
    QListWidget* m_list = nullptr;
    QLineEdit* m_searchInput = nullptr;
    QComboBox* m_statusFilter = nullptr;
    QComboBox* m_priorityFilter = nullptr;
    QLineEdit* m_sortInput = nullptr;

    QLabel* m_totalValue = nullptr;
    QLabel* m_completedValue = nullptr;
    QLabel* m_progressValue = nullptr;
};
