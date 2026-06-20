#ifndef ACTIVITYSERVICE_H
#define ACTIVITYSERVICE_H

#include <QList>
#include "model/Activity.h"
#include "dao/ActivityDao.h"

class ActivityService {
public:
    static QList<Activity> getAll() {
        ActivityDao dao;
        return dao.getAll();
    }

    static Activity getById(int id) {
        ActivityDao dao;
        return dao.getById(id);
    }

    static Activity create(Activity& a) {
        ActivityDao dao;
        dao.create(a);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) a = dao.getById(lastId);
        return a;
    }

    static Activity update(int id, Activity& a) {
        ActivityDao dao;
        Activity existing = dao.getById(id);
        if (existing.id == 0) return existing;
        a.id = id;
        dao.update(a);
        return dao.getById(id);
    }

    static bool remove(int id) {
        ActivityDao dao;
        return dao.remove(id);
    }
};

#endif // ACTIVITYSERVICE_H