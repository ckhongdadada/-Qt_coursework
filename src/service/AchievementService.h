#ifndef ACHIEVEMENTSERVICE_H
#define ACHIEVEMENTSERVICE_H

#include <QList>
#include "model/Achievement.h"
#include "dao/AchievementDao.h"

class AchievementService {
public:
    static QList<Achievement> getAll() {
        AchievementDao dao;
        return dao.getAll();
    }

    static Achievement getById(int id) {
        AchievementDao dao;
        return dao.getById(id);
    }

    static Achievement create(Achievement& a) {
        AchievementDao dao;
        dao.create(a);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) a = dao.getById(lastId);
        return a;
    }

    static Achievement update(int id, Achievement& a) {
        AchievementDao dao;
        Achievement existing = dao.getById(id);
        if (existing.id == 0) return existing;
        a.id = id;
        dao.update(a);
        return dao.getById(id);
    }

    static bool remove(int id) {
        AchievementDao dao;
        return dao.remove(id);
    }

    static QJsonObject getStatistics() {
        AchievementDao dao;
        QList<Achievement> list = dao.getAll();
        QMap<QString, int> typeCount;
        QMap<QString, int> levelCount;
        int verifiedCount = 0;
        for (const auto& a : list) {
            typeCount[a.type]++;
            levelCount[a.level]++;
            if (a.verified) verifiedCount++;
        }
        QJsonObject stats;
        stats["totalAchievements"] = list.size();
        stats["verifiedAchievements"] = verifiedCount;
        QJsonObject tb, lb;
        for (auto it = typeCount.begin(); it != typeCount.end(); ++it) tb[it.key()] = it.value();
        for (auto it = levelCount.begin(); it != levelCount.end(); ++it) lb[it.key()] = it.value();
        stats["typeBreakdown"] = tb;
        stats["levelBreakdown"] = lb;
        return stats;
    }
};

#endif // ACHIEVEMENTSERVICE_H