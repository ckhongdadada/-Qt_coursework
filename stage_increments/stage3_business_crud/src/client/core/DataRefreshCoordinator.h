#pragma once

#include "core/DataDomain.h"
#include <QObject>

class OverviewPage;
class CoursesPage;
class RolesPage;
class AchievementsPage;
class ExperiencesPage;
class ActivitiesPage;
class GoalsPage;
class JobsPage;
class AnalysisPage;
class TimelinePage;
class ResumePage;
class ImportsPage;

class DataRefreshCoordinator : public QObject {
    Q_OBJECT

public:
    explicit DataRefreshCoordinator(QObject* parent = nullptr);

    void bindPages(
        OverviewPage* overview,
        CoursesPage* courses,
        RolesPage* roles,
        AchievementsPage* achievements,
        ExperiencesPage* experiences,
        ActivitiesPage* activities,
        GoalsPage* goals,
        JobsPage* jobs,
        AnalysisPage* analysis,
        TimelinePage* timeline,
        ResumePage* resume,
        ImportsPage* imports
    );

    void connectSignals();
    void refreshByDomain(DataDomain domain);
    void refreshAll();

private:
    OverviewPage* m_overview = nullptr;
    CoursesPage* m_courses = nullptr;
    RolesPage* m_roles = nullptr;
    AchievementsPage* m_achievements = nullptr;
    ExperiencesPage* m_experiences = nullptr;
    ActivitiesPage* m_activities = nullptr;
    GoalsPage* m_goals = nullptr;
    JobsPage* m_jobs = nullptr;
    AnalysisPage* m_analysis = nullptr;
    TimelinePage* m_timeline = nullptr;
    ResumePage* m_resume = nullptr;
    ImportsPage* m_imports = nullptr;
};
