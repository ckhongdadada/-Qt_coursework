#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLabel>
#include <QListWidget>

class TimelinePage : public BasePage {
    Q_OBJECT

public:
    explicit TimelinePage(QWidget* parent = nullptr);
    ~TimelinePage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);

private:
    void setupUi();

    QLabel* m_summaryLabel = nullptr;
    QListWidget* m_eventList = nullptr;
    QListWidget* m_suggestionList = nullptr;

    QLabel* m_eventCountValue = nullptr;
    QLabel* m_strengthValue = nullptr;
    QLabel* m_riskValue = nullptr;
};
