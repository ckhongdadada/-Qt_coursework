#ifndef EXPERIENCEDAO_H
#define EXPERIENCEDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Experience.h"

class ExperienceDao : public DaoBase {
public:
    QList<Experience> getAll() const {
        QList<Experience> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM experiences ORDER BY updated_at DESC, id DESC");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    Experience getById(int id) const {
        Experience e;
        if (!isOpen()) return e;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM experiences WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) e = mapFromQuery(query);
        return e;
    }

    bool create(const Experience& e) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO experiences (title, type, organization, role, description, start_date, end_date, "
            "is_ongoing, technologies, achievements, supervisor, contact, location, url) "
            "VALUES (:title, :type, :organization, :role, :description, :start_date, :end_date, "
            ":is_ongoing, :technologies, :achievements, :supervisor, :contact, :location, :url)"
        );
        query.bindValue(":title", e.title);
        query.bindValue(":type", e.type);
        query.bindValue(":organization", e.organization);
        query.bindValue(":role", e.role);
        query.bindValue(":description", e.description);
        query.bindValue(":start_date", e.startDate);
        query.bindValue(":end_date", e.endDate);
        query.bindValue(":is_ongoing", e.isOngoing ? 1 : 0);
        query.bindValue(":technologies", e.technologies);
        query.bindValue(":achievements", e.achievements);
        query.bindValue(":supervisor", e.supervisor);
        query.bindValue(":contact", e.contact);
        query.bindValue(":location", e.location);
        query.bindValue(":url", e.url);
        return query.exec();
    }

    bool update(const Experience& e) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE experiences SET title = :title, type = :type, organization = :organization, "
            "role = :role, description = :description, start_date = :start_date, end_date = :end_date, "
            "is_ongoing = :is_ongoing, technologies = :technologies, achievements = :achievements, "
            "supervisor = :supervisor, contact = :contact, location = :location, url = :url, "
            "updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", e.id);
        query.bindValue(":title", e.title);
        query.bindValue(":type", e.type);
        query.bindValue(":organization", e.organization);
        query.bindValue(":role", e.role);
        query.bindValue(":description", e.description);
        query.bindValue(":start_date", e.startDate);
        query.bindValue(":end_date", e.endDate);
        query.bindValue(":is_ongoing", e.isOngoing ? 1 : 0);
        query.bindValue(":technologies", e.technologies);
        query.bindValue(":achievements", e.achievements);
        query.bindValue(":supervisor", e.supervisor);
        query.bindValue(":contact", e.contact);
        query.bindValue(":location", e.location);
        query.bindValue(":url", e.url);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM experiences WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM experiences");
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
    static Experience mapFromQuery(const QSqlQuery& query) {
        Experience e;
        e.id = query.value("id").toInt();
        e.title = query.value("title").toString();
        e.type = query.value("type").toString();
        e.organization = query.value("organization").toString();
        e.role = query.value("role").toString();
        e.description = query.value("description").toString();
        e.startDate = query.value("start_date").toString();
        e.endDate = query.value("end_date").toString();
        e.isOngoing = query.value("is_ongoing").toBool();
        e.technologies = query.value("technologies").toString();
        e.achievements = query.value("achievements").toString();
        e.supervisor = query.value("supervisor").toString();
        e.contact = query.value("contact").toString();
        e.location = query.value("location").toString();
        e.url = query.value("url").toString();
        e.createdAt = query.value("created_at").toDateTime();
        e.updatedAt = query.value("updated_at").toDateTime();
        return e;
    }
};

#endif // EXPERIENCEDAO_H