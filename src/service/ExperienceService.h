#ifndef EXPERIENCESERVICE_H
#define EXPERIENCESERVICE_H

#include <QList>
#include "model/Experience.h"
#include "dao/ExperienceDao.h"

class ExperienceService {
public:
    static QList<Experience> getAll() {
        ExperienceDao dao;
        return dao.getAll();
    }

    static Experience getById(int id) {
        ExperienceDao dao;
        return dao.getById(id);
    }

    static Experience create(Experience& e) {
        ExperienceDao dao;
        dao.create(e);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) e = dao.getById(lastId);
        return e;
    }

    static Experience update(int id, Experience& e) {
        ExperienceDao dao;
        Experience existing = dao.getById(id);
        if (existing.id == 0) return existing;
        e.id = id;
        dao.update(e);
        return dao.getById(id);
    }

    static bool remove(int id) {
        ExperienceDao dao;
        return dao.remove(id);
    }

    static QJsonObject getStatistics() {
        ExperienceDao dao;
        QList<Experience> list = dao.getAll();
        int ongoingCount = 0;
        QMap<QString, int> typeCount;
        for (const auto& e : list) {
            if (e.isOngoing) ongoingCount++;
            typeCount[e.type]++;
        }
        QJsonObject stats;
        stats["totalExperiences"] = list.size();
        stats["ongoingExperiences"] = ongoingCount;
        QJsonObject tb;
        for (auto it = typeCount.begin(); it != typeCount.end(); ++it) tb[it.key()] = it.value();
        stats["typeBreakdown"] = tb;
        return stats;
    }
};

#endif // EXPERIENCESERVICE_H