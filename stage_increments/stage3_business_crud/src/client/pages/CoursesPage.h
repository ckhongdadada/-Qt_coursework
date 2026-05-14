#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include "model/Course.h"
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>

class CoursesPage : public BasePage {
    Q_OBJECT

public:
    explicit CoursesPage(QWidget* parent = nullptr);
    ~CoursesPage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onRemoveClicked();
    void onCellDoubleClicked(int row, int col);

private:
    void setupUi();
    void updateSummary();

    QTableWidget* m_table = nullptr;
    QLineEdit* m_searchInput = nullptr;
    QLineEdit* m_statusInput = nullptr;
    QLineEdit* m_categoryInput = nullptr;
    QLineEdit* m_sortInput = nullptr;
    QLabel* m_summaryLabel = nullptr;
};
