#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLabel>

class MetricGridWidget;
class SuggestionListWidget;

class OverviewPage : public BasePage {
    Q_OBJECT

public:
    explicit OverviewPage(QWidget* parent = nullptr);
    ~OverviewPage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);

private:
    void setupUi();

    MetricGridWidget* m_metricGrid = nullptr;
    SuggestionListWidget* m_recommendationWidget = nullptr;
    SuggestionListWidget* m_semesterWidget = nullptr;

    QLabel* m_totalCoursesValue = nullptr;
    QLabel* m_gpaValue = nullptr;
    QLabel* m_goalProgressValue = nullptr;
    QLabel* m_achievementValue = nullptr;
    QLabel* m_experienceValue = nullptr;
    QLabel* m_roleValue = nullptr;
    QLabel* m_activityValue = nullptr;
    QLabel* m_creditValue = nullptr;
};
