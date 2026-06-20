#ifndef ACTIVITYDAO_H
#define ACTIVITYDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Activity.h"

class ActivityDao : public DaoBase {
public:
    QList<Activity> getAll() const {
        QList<Activity> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM activities ORDER BY updated_at DESC, id DESC");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    Activity getById(int id) const {
        Activity a;
        if (!isOpen()) return a;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM activities WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) a = mapFromQuery(query);
        return a;
    }

    bool create(const Activity& a) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO activities (name, description, category, start_date, end_date, is_favorite, is_active, tags) "
            "VALUES (:name, :description, :category, :start_date, :end_date, :is_favorite, :is_active, :tags)"
        );
        query.bindValue(":name", a.name);
        query.bindValue(":description", a.description);
        query.bindValue(":category", a.category);
        query.bindValue(":start_date", a.startDate);
        query.bindValue(":end_date", a.endDate);
        query.bindValue(":is_favorite", a.isFavorite ? 1 : 0);
        query.bindValue(":is_active", a.isActive ? 1 : 0);
        query.bindValue(":tags", a.tags);
        return query.exec();
    }

    bool update(const Activity& a) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE activities SET name = :name, description = :description, category = :category, "
            "start_date = :start_date, end_date = :end_date, is_favorite = :is_favorite, "
            "is_active = :is_active, tags = :tags, updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", a.id);
        query.bindValue(":name", a.name);
        query.bindValue(":description", a.description);
        query.bindValue(":category", a.category);
        query.bindValue(":start_date", a.startDate);
        query.bindValue(":end_date", a.endDate);
        query.bindValue(":is_favorite", a.isFavorite ? 1 : 0);
        query.bindValue(":is_active", a.isActive ? 1 : 0);
        query.bindValue(":tags", a.tags);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM activities WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM activities");
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
    static Activity mapFromQuery(const QSqlQuery& query) {
        Activity a;
        a.id = query.value("id").toInt();
        a.name = query.value("name").toString();
        a.description = query.value("description").toString();
        a.category = query.value("category").toString();
        a.startDate = query.value("start_date").toString();
        a.endDate = query.value("end_date").toString();
        a.isFavorite = query.value("is_favorite").toBool();
        a.isActive = query.value("is_active").toBool();
        a.tags = query.value("tags").toString();
        a.createdAt = query.value("created_at").toDateTime();
        a.updatedAt = query.value("updated_at").toDateTime();
        return a;
    }
};

#endif // ACTIVITYDAO_H