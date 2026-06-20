#ifndef DASHBOARDSERVICE_H
#define DASHBOARDSERVICE_H

#include <QJsonObject>
#include <QJsonArray>
#include "service/CourseService.h"
#include "service/GoalService.h"
#include "dao/AchievementDao.h"
#include "dao/ExperienceDao.h"
#include "dao/RoleDao.h"
#include "dao/ActivityDao.h"

class DashboardService {
public:
    static QJsonObject getOverview(const QString& scale = "standard") {
        QJsonObject courseStats = CourseService::getStatistics(scale);
        QJsonObject goalStats = GoalService::getStatistics();
        AchievementDao achDao;
        ExperienceDao expDao;
        RoleDao roleDao;
        ActivityDao actDao;

        QJsonObject overview;
        overview["courses"] = courseStats;
        overview["goals"] = goalStats;
        overview["achievementsCount"] = achDao.getCount();
        overview["experiencesCount"] = expDao.getCount();
        overview["rolesCount"] = roleDao.getCount();
        overview["activitiesCount"] = actDao.getCount();
        return overview;
    }

    static QJsonArray getGpaTrend(const QString& scale = "standard") {
        return CourseService::getSemesterStatistics(scale);
    }

    static QJsonArray getRecommendations() {
        QJsonArray recommendations;
        QJsonObject courseStats = CourseService::getStatistics();
        double gpa = courseStats["gpa"].toDouble();
        double creditProgress = courseStats["creditProgress"].toDouble();

        if (gpa < 3.0) {
            recommendations.append("建议关注GPA提升，优化学习策略和课程选择。");
        }
        if (creditProgress < 50) {
            recommendations.append("学分进度偏低，建议合理规划课程安排。");
        }
        if (gpa >= 3.5) {
            recommendations.append("GPA表现优秀，可考虑挑战更高难度课程或参与科研项目。");
        }

        AchievementDao achDao;
        if (achDao.getCount() < 3) {
            recommendations.append("成就积累较少，建议积极参与竞赛或获取专业认证。");
        }

        if (recommendations.isEmpty()) {
            recommendations.append("整体发展良好，继续保持当前节奏。");
        }
        return recommendations;
    }
};

#endif // DASHBOARDSERVICE_H