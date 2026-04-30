#ifndef GOALSERVICE_H
#define GOALSERVICE_H

#include <QList>
#include "model/Goal.h"
#include "dao/GoalDao.h"

class GoalService {
public:
    static QList<Goal> getAll() {
        GoalDao dao;
        return dao.getAll();
    }

    static Goal getById(int id) {
        GoalDao dao;
        return dao.getById(id);
    }

    static Goal create(Goal& g) {
        GoalDao dao;
        dao.create(g);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) g = dao.getById(lastId);
        return g;
    }

    static Goal update(int id, Goal& g) {
        GoalDao dao;
        Goal existing = dao.getById(id);
        if (existing.id == 0) return existing;
        g.id = id;
        dao.update(g);
        return dao.getById(id);
    }

    static bool remove(int id) {
        GoalDao dao;
        return dao.remove(id);
    }

    static QJsonObject getStatistics() {
        GoalDao dao;
        QJsonObject stats;
        stats["total"] = dao.getCount();
        stats["completed"] = dao.getCompletedCount();
        stats["averageProgress"] = qRound(dao.getAverageProgress() * 100) / 100.0;
        return stats;
    }
};

#endif // GOALSERVICE_H