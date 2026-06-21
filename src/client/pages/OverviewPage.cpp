#include "OverviewPage.h"
#include "service/CourseService.h"
#include "service/DashboardService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;
#include "widgets/MetricGridWidget.h"
#include "widgets/SuggestionListWidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QVBoxLayout>

OverviewPage::OverviewPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void OverviewPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("总览"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    auto* subtitle = new QLabel(zh("快速查看个人发展状态和近期建议。"), this);
    subtitle->setObjectName("pageSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    m_metricGrid = new MetricGridWidget(this);
    m_metricGrid->setMetrics({
        {zh("课程总数"), &m_totalCoursesValue, {}},
        {zh("当前 GPA"), &m_gpaValue, {}},
        {zh("目标平均进度"), &m_goalProgressValue, {}},
        {zh("成果数量"), &m_achievementValue, {}},
        {zh("经历数量"), &m_experienceValue, {}},
        {zh("角色数量"), &m_roleValue, {}},
        {zh("活动数量"), &m_activityValue, {}},
        {zh("已修学分"), &m_creditValue, zh("基于课程完成状态自动统计")}
    }, 4);
    layout->addWidget(m_metricGrid);

    auto* lowerLayout = new QHBoxLayout();
    lowerLayout->setSpacing(14);
    m_recommendationWidget = new SuggestionListWidget(zh("近期建议"), this);
    m_semesterWidget = new SuggestionListWidget(zh("学期走势"), this);
    lowerLayout->addWidget(m_recommendationWidget, 2);
    lowerLayout->addWidget(m_semesterWidget, 1);
    layout->addLayout(lowerLayout, 1);
}

void OverviewPage::refresh()
{
    const QJsonObject overview = DashboardService::getOverview();
    const QJsonObject courses = overview.value("courses").toObject();
    const QJsonObject goals = overview.value("goals").toObject();

    if (m_totalCoursesValue) {
        m_totalCoursesValue->setText(QString::number(courses.value("totalCourses").toInt()));
    }
    if (m_gpaValue) {
        m_gpaValue->setText(QString::number(courses.value("gpa").toDouble(), 'f', 2));
    }
    if (m_goalProgressValue) {
        m_goalProgressValue->setText(zh("%1%").arg(goals.value("averageProgress").toDouble(), 0, 'f', 1));
    }
    if (m_achievementValue) {
        m_achievementValue->setText(QString::number(overview.value("achievementsCount").toInt()));
    }
    if (m_experienceValue) {
        m_experienceValue->setText(QString::number(overview.value("experiencesCount").toInt()));
    }
    if (m_roleValue) {
        m_roleValue->setText(QString::number(overview.value("rolesCount").toInt()));
    }
    if (m_activityValue) {
        m_activityValue->setText(QString::number(overview.value("activitiesCount").toInt()));
    }
    if (m_creditValue) {
        m_creditValue->setText(QString::number(courses.value("completedCredits").toDouble(), 'f', 1));
    }

    if (m_recommendationWidget) {
        m_recommendationWidget->loadItems(DashboardService::getRecommendations());
    }

    QStringList semesterStrings;
    const QJsonArray semesters = CourseService::getSemesterStatistics();
    for (const QJsonValue& semesterValue : semesters) {
        const QJsonObject semester = semesterValue.toObject();
        semesterStrings.append(
            zh("%1  |  GPA %2  |  平均分 %3")
                .arg(semester.value("semester").toString())
                .arg(QString::number(semester.value("gpa").toDouble(), 'f', 2))
                .arg(QString::number(semester.value("avgScore").toDouble(), 'f', 1)));
    }
    if (m_semesterWidget) {
        m_semesterWidget->loadStrings(semesterStrings);
    }
}
