#ifndef ROLEDAO_H
#define ROLEDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Role.h"

class RoleDao : public DaoBase {
public:
    QList<Role> getAll() const {
        QList<Role> roles;
        if (!isOpen()) return roles;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM roles ORDER BY updated_at DESC, id DESC");
        while (query.next()) roles.append(mapFromQuery(query));
        return roles;
    }

    Role getById(int id) const {
        Role role;
        if (!isOpen()) return role;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM roles WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) role = mapFromQuery(query);
        return role;
    }

    bool create(const Role& role) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO roles (title, type, organization, description, start_date, end_date, "
            "is_active, achievements, contact, supervisor) "
            "VALUES (:title, :type, :organization, :description, :start_date, :end_date, "
            ":is_active, :achievements, :contact, :supervisor)"
        );
        query.bindValue(":title", role.title);
        query.bindValue(":type", role.type);
        query.bindValue(":organization", role.organization);
        query.bindValue(":description", role.description);
        query.bindValue(":start_date", role.startDate);
        query.bindValue(":end_date", role.endDate);
        query.bindValue(":is_active", role.isActive ? 1 : 0);
        query.bindValue(":achievements", role.achievements);
        query.bindValue(":contact", role.contact);
        query.bindValue(":supervisor", role.supervisor);
        return query.exec();
    }

    bool update(const Role& role) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE roles SET title = :title, type = :type, organization = :organization, "
            "description = :description, start_date = :start_date, end_date = :end_date, "
            "is_active = :is_active, achievements = :achievements, contact = :contact, "
            "supervisor = :supervisor, updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", role.id);
        query.bindValue(":title", role.title);
        query.bindValue(":type", role.type);
        query.bindValue(":organization", role.organization);
        query.bindValue(":description", role.description);
        query.bindValue(":start_date", role.startDate);
        query.bindValue(":end_date", role.endDate);
        query.bindValue(":is_active", role.isActive ? 1 : 0);
        query.bindValue(":achievements", role.achievements);
        query.bindValue(":contact", role.contact);
        query.bindValue(":supervisor", role.supervisor);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM roles WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM roles");
        if (query.next()) return query.value(0).toInt();
        return 0;
    }

    int getLastInsertId() const {
        QSqlQuery query(m_db);
        query.exec("SELECT last_insert_rowid()");
        if (query.next()) return query.value(0).toInt();
        return -1;
    }

private:
    static Role mapFromQuery(const QSqlQuery& query) {
        Role r;
        r.id = query.value("id").toInt();
        r.title = query.value("title").toString();
        r.type = query.value("type").toString();
        r.organization = query.value("organization").toString();
        r.description = query.value("description").toString();
        r.startDate = query.value("start_date").toString();
        r.endDate = query.value("end_date").toString();
        r.isActive = query.value("is_active").toBool();
        r.achievements = query.value("achievements").toString();
        r.contact = query.value("contact").toString();
        r.supervisor = query.value("supervisor").toString();
        r.createdAt = query.value("created_at").toDateTime();
        r.updatedAt = query.value("updated_at").toDateTime();
        return r;
    }
};

#endif // ROLEDAO_H