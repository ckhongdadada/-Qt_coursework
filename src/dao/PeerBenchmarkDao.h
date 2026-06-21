#ifndef PEERBENCHMARKDAO_H
#define PEERBENCHMARKDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/PeerBenchmark.h"

class PeerBenchmarkDao : public DaoBase {
public:
    QList<PeerBenchmark> getAll() const {
        QList<PeerBenchmark> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM peer_benchmarks ORDER BY gpa DESC, credits DESC");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    PeerBenchmark getById(int id) const {
        PeerBenchmark p;
        if (!isOpen()) return p;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM peer_benchmarks WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) p = mapFromQuery(query);
        return p;
    }

    bool create(const PeerBenchmark& p) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO peer_benchmarks (name, major, semester, gpa, credits, "
            "achievements_count, experiences_count, note) "
            "VALUES (:name, :major, :semester, :gpa, :credits, "
            ":achievements_count, :experiences_count, :note)"
        );
        query.bindValue(":name", p.name);
        query.bindValue(":major", p.major);
        query.bindValue(":semester", p.semester);
        query.bindValue(":gpa", p.gpa);
        query.bindValue(":credits", p.credits);
        query.bindValue(":achievements_count", p.achievementsCount);
        query.bindValue(":experiences_count", p.experiencesCount);
        query.bindValue(":note", p.note);
        return query.exec();
    }

    bool update(const PeerBenchmark& p) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE peer_benchmarks SET name = :name, major = :major, semester = :semester, "
            "gpa = :gpa, credits = :credits, achievements_count = :achievements_count, "
            "experiences_count = :experiences_count, note = :note, "
            "updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", p.id);
        query.bindValue(":name", p.name);
        query.bindValue(":major", p.major);
        query.bindValue(":semester", p.semester);
        query.bindValue(":gpa", p.gpa);
        query.bindValue(":credits", p.credits);
        query.bindValue(":achievements_count", p.achievementsCount);
        query.bindValue(":experiences_count", p.experiencesCount);
        query.bindValue(":note", p.note);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM peer_benchmarks WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getLastInsertId() const {
        QSqlQuery query(m_db);
        query.exec("SELECT last_insert_rowid()");
        if (query.next()) return query.value(0).toInt();
        return -1;
    }

private:
    static PeerBenchmark mapFromQuery(const QSqlQuery& query) {
        PeerBenchmark p;
        p.id = query.value("id").toInt();
        p.name = query.value("name").toString();
        p.major = query.value("major").toString();
        p.semester = query.value("semester").toString();
        p.gpa = query.value("gpa").toDouble();
        p.credits = query.value("credits").toDouble();
        p.achievementsCount = query.value("achievements_count").toInt();
        p.experiencesCount = query.value("experiences_count").toInt();
        p.note = query.value("note").toString();
        p.createdAt = query.value("created_at").toDateTime();
        p.updatedAt = query.value("updated_at").toDateTime();
        return p;
    }
};

#endif // PEERBENCHMARKDAO_H