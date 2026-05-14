#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>

class ExperiencesPage : public BasePage {
    Q_OBJECT

public:
    explicit ExperiencesPage(QWidget* parent = nullptr);
    ~ExperiencesPage() override = default;

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
    QComboBox* m_typeFilter = nullptr;

    QLabel* m_totalValue = nullptr;
    QLabel* m_ongoingValue = nullptr;
    QLabel* m_typeValue = nullptr;
};
