#ifndef ACHIEVEMENTDAO_H
#define ACHIEVEMENTDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Achievement.h"

class AchievementDao : public DaoBase {
public:
    QList<Achievement> getAll() const {
        QList<Achievement> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM achievements ORDER BY updated_at DESC, id DESC");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    Achievement getById(int id) const {
        Achievement a;
        if (!isOpen()) return a;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM achievements WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) a = mapFromQuery(query);
        return a;
    }

    bool create(const Achievement& a) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO achievements (title, type, level, organization, description, date, "
            "certificate, related_course, team_members, ranking, prize, verified) "
            "VALUES (:title, :type, :level, :organization, :description, :date, "
            ":certificate, :related_course, :team_members, :ranking, :prize, :verified)"
        );
        query.bindValue(":title", a.title);
        query.bindValue(":type", a.type);
        query.bindValue(":level", a.level);
        query.bindValue(":organization", a.organization);
        query.bindValue(":description", a.description);
        query.bindValue(":date", a.date);
        query.bindValue(":certificate", a.certificate);
        query.bindValue(":related_course", a.relatedCourse);
        query.bindValue(":team_members", a.teamMembers);
        query.bindValue(":ranking", a.ranking);
        query.bindValue(":prize", a.prize);
        query.bindValue(":verified", a.verified ? 1 : 0);
        return query.exec();
    }

    bool update(const Achievement& a) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE achievements SET title = :title, type = :type, level = :level, "
            "organization = :organization, description = :description, date = :date, "
            "certificate = :certificate, related_course = :related_course, "
            "team_members = :team_members, ranking = :ranking, prize = :prize, "
            "verified = :verified, updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", a.id);
        query.bindValue(":title", a.title);
        query.bindValue(":type", a.type);
        query.bindValue(":level", a.level);
        query.bindValue(":organization", a.organization);
        query.bindValue(":description", a.description);
        query.bindValue(":date", a.date);
        query.bindValue(":certificate", a.certificate);
        query.bindValue(":related_course", a.relatedCourse);
        query.bindValue(":team_members", a.teamMembers);
        query.bindValue(":ranking", a.ranking);
        query.bindValue(":prize", a.prize);
        query.bindValue(":verified", a.verified ? 1 : 0);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM achievements WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM achievements");
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
    static Achievement mapFromQuery(const QSqlQuery& query) {
        Achievement a;
        a.id = query.value("id").toInt();
        a.title = query.value("title").toString();
        a.type = query.value("type").toString();
        a.level = query.value("level").toString();
        a.organization = query.value("organization").toString();
        a.description = query.value("description").toString();
        a.date = query.value("date").toString();
        a.certificate = query.value("certificate").toString();
        a.relatedCourse = query.value("related_course").toString();
        a.teamMembers = query.value("team_members").toString();
        a.ranking = query.value("ranking").toString();
        a.prize = query.value("prize").toString();
        a.verified = query.value("verified").toBool();
        a.createdAt = query.value("created_at").toDateTime();
        a.updatedAt = query.value("updated_at").toDateTime();
        return a;
    }
};

#endif // ACHIEVEMENTDAO_H