#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>

class RolesPage : public BasePage {
    Q_OBJECT

public:
    explicit RolesPage(QWidget* parent = nullptr);
    ~RolesPage() override = default;

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
    void updateSummary();

    QLabel* m_summaryLabel = nullptr;
    QListWidget* m_list = nullptr;
    QLineEdit* m_searchInput = nullptr;
    QComboBox* m_typeFilter = nullptr;

    QLabel* m_totalValue = nullptr;
    QLabel* m_activeValue = nullptr;
    QLabel* m_typeValue = nullptr;
};
