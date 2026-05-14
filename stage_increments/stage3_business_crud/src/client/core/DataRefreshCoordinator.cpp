#include "DataRefreshCoordinator.h"
#include "pages/OverviewPage.h"
#include "pages/CoursesPage.h"
#include "pages/RolesPage.h"
#include "pages/AchievementsPage.h"
#include "pages/ExperiencesPage.h"
#include "pages/ActivitiesPage.h"
#include "pages/GoalsPage.h"
#include "pages/JobsPage.h"
#include "pages/AnalysisPage.h"
#include "pages/TimelinePage.h"
#include "pages/ResumePage.h"
#include "pages/ImportsPage.h"

DataRefreshCoordinator::DataRefreshCoordinator(QObject* parent)
    : QObject(parent)
{}

void DataRefreshCoordinator::bindPages(
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
    ImportsPage* imports)
{
    m_overview = overview;
    m_courses = courses;
    m_roles = roles;
    m_achievements = achievements;
    m_experiences = experiences;
    m_activities = activities;
    m_goals = goals;
    m_jobs = jobs;
    m_analysis = analysis;
    m_timeline = timeline;
    m_resume = resume;
    m_imports = imports;
}

void DataRefreshCoordinator::connectSignals()
{
    if (m_courses) {
        connect(m_courses, &CoursesPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_roles) {
        connect(m_roles, &RolesPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_achievements) {
        connect(m_achievements, &AchievementsPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_experiences) {
        connect(m_experiences, &ExperiencesPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_activities) {
        connect(m_activities, &ActivitiesPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_goals) {
        connect(m_goals, &GoalsPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_jobs) {
        connect(m_jobs, &JobsPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_analysis) {
        connect(m_analysis, &AnalysisPage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_timeline) {
        connect(m_timeline, &TimelinePage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_resume) {
        connect(m_resume, &ResumePage::dataChanged, this, [this](DataDomain domain) {
            refreshByDomain(domain);
        });
    }
    if (m_imports) {
        connect(m_imports, &ImportsPage::importCompleted, this, [this]() {
            refreshAll();
        });
    }
}

void DataRefreshCoordinator::refreshByDomain(DataDomain domain)
{
    if (m_overview) m_overview->refresh();

    switch (domain) {
    case DataDomain::Courses:
        if (m_timeline) m_timeline->refresh();
        if (m_analysis) m_analysis->refresh();
        if (m_resume) m_resume->refresh();
        break;
    case DataDomain::Goals:
        if (m_timeline) m_timeline->refresh();
        if (m_analysis) m_analysis->refresh();
        if (m_resume) m_resume->refresh();
        break;
    case DataDomain::Roles:
        if (m_analysis) m_analysis->refresh();
        if (m_resume) m_resume->refresh();
        break;
    case DataDomain::Achievements:
        if (m_analysis) m_analysis->refresh();
        if (m_resume) m_resume->refresh();
        break;
    case DataDomain::Experiences:
        if (m_timeline) m_timeline->refresh();
        if (m_analysis) m_analysis->refresh();
        if (m_resume) m_resume->refresh();
        break;
    case DataDomain::Activities:
        if (m_timeline) m_timeline->refresh();
        if (m_analysis) m_analysis->refresh();
        if (m_resume) m_resume->refresh();
        break;
    case DataDomain::Jobs:
        if (m_timeline) m_timeline->refresh();
        if (m_analysis) m_analysis->refresh();
        break;
    case DataDomain::Analysis:
        if (m_timeline) m_timeline->refresh();
        break;
    case DataDomain::Resume:
        break;
    case DataDomain::Timeline:
        break;
    case DataDomain::All:
        refreshAll();
        break;
    }
}

void DataRefreshCoordinator::refreshAll()
{
    if (m_overview) m_overview->refresh();
    if (m_courses) m_courses->refresh();
    if (m_roles) m_roles->refresh();
    if (m_achievements) m_achievements->refresh();
    if (m_experiences) m_experiences->refresh();
    if (m_activities) m_activities->refresh();
    if (m_goals) m_goals->refresh();
    if (m_jobs) m_jobs->refresh();
    if (m_analysis) m_analysis->refresh();
    if (m_timeline) m_timeline->refresh();
    if (m_resume) m_resume->refresh();
}
