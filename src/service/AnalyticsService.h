#ifndef ANALYTICSSERVICE_H
#define ANALYTICSSERVICE_H

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>

#include "dao/AchievementDao.h"
#include "dao/ActivityDao.h"
#include "dao/ExperienceDao.h"
#include "dao/GoalDao.h"
#include "dao/PeerBenchmarkDao.h"
#include "dao/RoleDao.h"
#include "model/PeerBenchmark.h"
#include "service/CourseService.h"
#include "service/GoalService.h"

class PeerBenchmarkService {
public:
    static QList<PeerBenchmark> getAll() {
        PeerBenchmarkDao dao;
        return dao.getAll();
    }

    static PeerBenchmark getById(int id) {
        PeerBenchmarkDao dao;
        return dao.getById(id);
    }

    static PeerBenchmark create(PeerBenchmark& peer) {
        PeerBenchmarkDao dao;
        dao.create(peer);
        const int lastId = dao.getLastInsertId();
        if (lastId > 0) {
            peer = dao.getById(lastId);
        }
        return peer;
    }

    static PeerBenchmark update(int id, PeerBenchmark& peer) {
        PeerBenchmarkDao dao;
        PeerBenchmark existing = dao.getById(id);
        if (existing.id == 0) {
            return existing;
        }
        peer.id = id;
        dao.update(peer);
        return dao.getById(id);
    }

    static bool remove(int id) {
        PeerBenchmarkDao dao;
        return dao.remove(id);
    }
};

class AnalyticsService {
public:
    static QJsonObject getSelfSummary() {
        QJsonObject courseStats = CourseService::getStatistics();
        QJsonObject goalStats = GoalService::getStatistics();
        AchievementDao achievementDao;
        ExperienceDao experienceDao;
        RoleDao roleDao;

        QJsonObject summary;
        summary["gpa"] = courseStats["gpa"];
        summary["credits"] = courseStats["totalCredits"];
        summary["achievementsCount"] = achievementDao.getCount();
        summary["experiencesCount"] = experienceDao.getCount();
        summary["rolesCount"] = roleDao.getCount();
        summary["goalsCompleted"] = goalStats["completed"];
        summary["goalsAverageProgress"] = goalStats["averageProgress"];
        return summary;
    }

    static QJsonArray getSemesterComparison() {
        QJsonArray semesterStats = CourseService::getSemesterStatistics();
        QJsonArray result;
        QJsonObject previous;

        for (int i = 0; i < semesterStats.size(); ++i) {
            QJsonObject current = semesterStats.at(i).toObject();
            if (!previous.isEmpty()) {
                current["gpaDelta"] =
                    qRound((current["gpa"].toDouble() - previous["gpa"].toDouble()) * 100) / 100.0;
                current["avgScoreDelta"] =
                    qRound((current["avgScore"].toDouble() - previous["avgScore"].toDouble()) * 100) / 100.0;
                current["creditsDelta"] =
                    qRound((current["credits"].toDouble() - previous["credits"].toDouble()) * 100) / 100.0;
            } else {
                current["gpaDelta"] = QJsonValue::Null;
                current["avgScoreDelta"] = QJsonValue::Null;
                current["creditsDelta"] = QJsonValue::Null;
            }
            result.append(current);
            previous = current;
        }

        return result;
    }

    static QJsonObject compareWithPeers() {
        QJsonObject selfSummary = getSelfSummary();
        QList<PeerBenchmark> peers = PeerBenchmarkService::getAll();
        QJsonArray peerArray;

        for (auto& peer : peers) {
            QJsonObject item = peer.toDict();
            item["gpaGap"] =
                qRound((selfSummary["gpa"].toDouble() - item["gpa"].toDouble()) * 100) / 100.0;
            item["creditsGap"] =
                qRound((selfSummary["credits"].toDouble() - item["credits"].toDouble()) * 100) / 100.0;
            item["achievementsGap"] =
                selfSummary["achievementsCount"].toInt() - item["achievementsCount"].toInt();
            item["experiencesGap"] =
                selfSummary["experiencesCount"].toInt() - item["experiencesCount"].toInt();
            peerArray.append(item);
        }

        QJsonObject result;
        result["self"] = selfSummary;
        result["peers"] = peerArray;
        return result;
    }

    static QJsonObject generateReport() {
        QJsonObject summary = getSelfSummary();
        QJsonArray semesterComparison = getSemesterComparison();
        QJsonObject peerComparison = compareWithPeers();
        QJsonObject goalStats = GoalService::getStatistics();

        QJsonArray strengths;
        QJsonArray risks;
        QJsonArray suggestions;

        const double gpa = summary["gpa"].toDouble();
        const int achievementCount = summary["achievementsCount"].toInt();
        const double averageProgress = goalStats["averageProgress"].toDouble();

        if (gpa >= 3.5) {
            strengths.append("GPA 表现较强，学业竞争力较好。");
        } else {
            risks.append("GPA 仍有提升空间，建议优化课程复盘与备考节奏。");
        }

        if (achievementCount >= 3) {
            strengths.append("已有较好的成果积累，适合继续强化展示与总结。");
        } else {
            risks.append("成果积累偏少，可优先补充竞赛、证书或项目成果。");
        }

        if (averageProgress >= 70) {
            strengths.append("目标推进较稳定，执行力表现较好。");
        } else {
            suggestions.append("建议把长期目标拆成更清晰的阶段性里程碑。");
        }

        QJsonArray peers = peerComparison["peers"].toArray();
        if (!peers.isEmpty()) {
            QJsonObject bestPeer = peers.first().toObject();
            for (const auto& peerValue : peers) {
                const QJsonObject peer = peerValue.toObject();
                if (peer["gpa"].toDouble() > bestPeer["gpa"].toDouble()) {
                    bestPeer = peer;
                }
            }
            suggestions.append(
                QString("可参考同学 %1 的阶段表现，针对课程与成果进行对标提升。")
                    .arg(bestPeer["name"].toString()));
        }

        if (!semesterComparison.isEmpty()) {
            const QJsonObject latest = semesterComparison.last().toObject();
            if (!latest["gpaDelta"].isNull()) {
                const double delta = latest["gpaDelta"].toDouble();
                if (delta < 0) {
                    risks.append("最近一个学期的 GPA 较上一学期下降，需关注学习节奏。");
                } else if (delta > 0) {
                    strengths.append("最近一个学期的 GPA 相比上一阶段有所提升。");
                }
            }
        }

        if (suggestions.isEmpty()) {
            suggestions.append("继续保持课程、经历、成果和目标管理的同步更新。");
        }

        QJsonObject report;
        report["generatedAt"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        report["summary"] = summary;
        report["goalStats"] = goalStats;
        report["semesterComparison"] = semesterComparison;
        report["peerComparison"] = peerComparison;
        report["strengths"] = strengths;
        report["risks"] = risks;
        report["suggestions"] = suggestions;
        return report;
    }

    static QJsonArray getTimelineEvents() {
        QList<QJsonObject> events;

        RoleDao roleDao;
        AchievementDao achievementDao;
        ExperienceDao experienceDao;
        ActivityDao activityDao;
        GoalDao goalDao;

        for (const auto& role : roleDao.getAll()) {
            if (!role.startDate.isEmpty()) {
                QJsonObject event;
                event["type"] = "Role";
                event["title"] = role.title;
                event["subtitle"] = role.organization.isEmpty() ? role.type : role.organization;
                event["date"] = role.startDate;
                event["description"] = role.description.isEmpty() ? "开始新的角色任职" : role.description;
                events.append(event);
            }
            if (!role.endDate.isEmpty()) {
                QJsonObject event;
                event["type"] = "Role";
                event["title"] = role.title + " 结束";
                event["subtitle"] = role.organization.isEmpty() ? role.type : role.organization;
                event["date"] = role.endDate;
                event["description"] = "角色阶段完成";
                events.append(event);
            }
        }

        for (const auto& achievement : achievementDao.getAll()) {
            if (!achievement.date.isEmpty()) {
                QJsonObject event;
                event["type"] = "Achievement";
                event["title"] = achievement.title;
                event["subtitle"] = achievement.level.isEmpty() ? achievement.organization : achievement.level;
                event["date"] = achievement.date;
                event["description"] = achievement.description.isEmpty()
                    ? "获得新的成果或奖项"
                    : achievement.description;
                events.append(event);
            }
        }

        for (const auto& experience : experienceDao.getAll()) {
            if (!experience.startDate.isEmpty()) {
                QJsonObject event;
                event["type"] = "Experience";
                event["title"] = experience.title;
                event["subtitle"] = experience.organization.isEmpty()
                    ? experience.type
                    : experience.organization;
                event["date"] = experience.startDate;
                event["description"] = experience.description.isEmpty()
                    ? "开始新的经历"
                    : experience.description;
                events.append(event);
            }
            if (!experience.endDate.isEmpty()) {
                QJsonObject event;
                event["type"] = "Experience";
                event["title"] = experience.title + " 完成";
                event["subtitle"] = experience.organization.isEmpty()
                    ? experience.type
                    : experience.organization;
                event["date"] = experience.endDate;
                event["description"] = "阶段性经历完成";
                events.append(event);
            }
        }

        for (const auto& activity : activityDao.getAll()) {
            if (!activity.startDate.isEmpty()) {
                QJsonObject event;
                event["type"] = "Activity";
                event["title"] = activity.name;
                event["subtitle"] = "活动";
                event["date"] = activity.startDate;
                event["description"] = activity.description.isEmpty()
                    ? "开始新的活动"
                    : activity.description;
                events.append(event);
            }
        }

        for (const auto& goal : goalDao.getAll()) {
            if (!goal.deadline.isEmpty()) {
                QJsonObject event;
                event["type"] = "Goal";
                event["title"] = goal.title;
                event["subtitle"] =
                    QString("%1% / 截止 %2").arg(goal.progress()).arg(goal.deadline);
                event["date"] = goal.deadline;
                event["description"] = goal.description.isEmpty()
                    ? "目标里程碑或截止时间"
                    : goal.description;
                events.append(event);
            }
        }

        std::sort(events.begin(), events.end(), [](const QJsonObject& left, const QJsonObject& right) {
            return left["date"].toString() > right["date"].toString();
        });

        QJsonArray result;
        for (const auto& event : events) {
            result.append(event);
        }
        return result;
    }
};

#endif // ANALYTICSSERVICE_H
