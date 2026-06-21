#ifndef ROLESERVICE_H
#define ROLESERVICE_H

#include <QList>
#include "model/Role.h"
#include "dao/RoleDao.h"

class RoleService {
public:
    static QList<Role> getAll() {
        RoleDao dao;
        return dao.getAll();
    }

    static Role getById(int id) {
        RoleDao dao;
        return dao.getById(id);
    }

    static Role create(Role& role) {
        RoleDao dao;
        dao.create(role);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) role = dao.getById(lastId);
        return role;
    }

    static Role update(int id, Role& role) {
        RoleDao dao;
        Role existing = dao.getById(id);
        if (existing.id == 0) return existing;
        role.id = id;
        dao.update(role);
        return dao.getById(id);
    }

    static bool remove(int id) {
        RoleDao dao;
        return dao.remove(id);
    }

    static QJsonObject getStatistics() {
        RoleDao dao;
        QList<Role> roles = dao.getAll();
        int activeCount = 0;
        QMap<QString, int> typeCount;
        for (const auto& r : roles) {
            if (r.isActive) activeCount++;
            typeCount[r.type]++;
        }
        QJsonObject stats;
        stats["totalRoles"] = roles.size();
        stats["activeRoles"] = activeCount;
        QJsonObject typeBreakdown;
        for (auto it = typeCount.begin(); it != typeCount.end(); ++it) {
            typeBreakdown[it.key()] = it.value();
        }
        stats["typeBreakdown"] = typeBreakdown;
        return stats;
    }
};

#endif // ROLESERVICE_H