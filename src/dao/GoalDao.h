#ifndef GOALDAO_H
#define GOALDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Goal.h"

class GoalDao : public DaoBase {
public:
    QList<Goal> getAll() const {
        QList<Goal> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM goals ORDER BY updated_at DESC, id DESC");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    Goal getById(int id) const {
        Goal g;
        if (!isOpen()) return g;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM goals WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) g = mapFromQuery(query);
        return g;
    }

    bool create(const Goal& g) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO goals (title, category, description, target_value, current_value, unit, "
            "deadline, priority, status, milestones) "
            "VALUES (:title, :category, :description, :target_value, :current_value, :unit, "
            ":deadline, :priority, :status, :milestones)"
        );
        query.bindValue(":title", g.title);
        query.bindValue(":category", g.category);
        query.bindValue(":description", g.description);
        query.bindValue(":target_value", g.targetValue);
        query.bindValue(":current_value", g.currentValue);
        query.bindValue(":unit", g.unit);
        query.bindValue(":deadline", g.deadline);
        query.bindValue(":priority", g.priority);
        query.bindValue(":status", g.status);
        query.bindValue(":milestones", g.milestones);
        return query.exec();
    }

    bool update(const Goal& g) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE goals SET title = :title, category = :category, description = :description, "
            "target_value = :target_value, current_value = :current_value, unit = :unit, "
            "deadline = :deadline, priority = :priority, status = :status, milestones = :milestones, "
            "updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", g.id);
        query.bindValue(":title", g.title);
        query.bindValue(":category", g.category);
        query.bindValue(":description", g.description);
        query.bindValue(":target_value", g.targetValue);
        query.bindValue(":current_value", g.currentValue);
        query.bindValue(":unit", g.unit);
        query.bindValue(":deadline", g.deadline);
        query.bindValue(":priority", g.priority);
        query.bindValue(":status", g.status);
        query.bindValue(":milestones", g.milestones);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM goals WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM goals");
        if (query.next()) return query.value(0).toInt();
        return 0;
    }

    int getCompletedCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM goals WHERE status = 'Completed'");
        if (query.next()) return query.value(0).toInt();
        return 0;
    }

    double getAverageProgress() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT AVG(CASE WHEN target_value > 0 THEN current_value * 100.0 / target_value ELSE 0 END) FROM goals");
        if (query.next()) return query.value(0).toDouble();
        return 0;
    }

    int getLastInsertId() const {
        QSqlQuery query(m_db);
        query.exec("SELECT last_insert_rowid()");
        if (query.next()) return query.value(0).toInt();
        return -1;
    }

private:
    static Goal mapFromQuery(const QSqlQuery& query) {
        Goal g;
        g.id = query.value("id").toInt();
        g.title = query.value("title").toString();
        g.category = query.value("category").toString();
        g.description = query.value("description").toString();
        g.targetValue = query.value("target_value").toDouble();
        g.currentValue = query.value("current_value").toDouble();
        g.unit = query.value("unit").toString();
        g.deadline = query.value("deadline").toString();
        g.priority = query.value("priority").toString();
        g.status = query.value("status").toString();
        g.milestones = query.value("milestones").toString();
        g.createdAt = query.value("created_at").toDateTime();
        g.updatedAt = query.value("updated_at").toDateTime();
        return g;
    }
};

#endif // GOALDAO_H