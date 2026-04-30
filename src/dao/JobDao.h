#ifndef JOBDAO_H
#define JOBDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Job.h"
#include <QJsonDocument>

class JobDao : public DaoBase {
public:
    QList<Job> getAll() const {
        QList<Job> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM target_jobs ORDER BY updated_at DESC, id DESC");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    Job getById(int id) const {
        Job j;
        if (!isOpen()) return j;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM target_jobs WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) j = mapFromQuery(query);
        return j;
    }

    bool create(const Job& j) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO target_jobs (title, company, location, salary_range, description, requirements, "
            "is_active, priority, source, url) "
            "VALUES (:title, :company, :location, :salary_range, :description, :requirements, "
            ":is_active, :priority, :source, :url)"
        );
        query.bindValue(":title", j.title);
        query.bindValue(":company", j.company);
        query.bindValue(":location", j.location);
        query.bindValue(":salary_range", j.salaryRange);
        query.bindValue(":description", j.description);
        query.bindValue(":requirements", requirementsToJson(j.requirements));
        query.bindValue(":is_active", j.isActive ? 1 : 0);
        query.bindValue(":priority", j.priority);
        query.bindValue(":source", j.source);
        query.bindValue(":url", j.url);
        return query.exec();
    }

    bool update(const Job& j) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE target_jobs SET title = :title, company = :company, location = :location, "
            "salary_range = :salary_range, description = :description, requirements = :requirements, "
            "is_active = :is_active, priority = :priority, source = :source, url = :url, "
            "updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", j.id);
        query.bindValue(":title", j.title);
        query.bindValue(":company", j.company);
        query.bindValue(":location", j.location);
        query.bindValue(":salary_range", j.salaryRange);
        query.bindValue(":description", j.description);
        query.bindValue(":requirements", requirementsToJson(j.requirements));
        query.bindValue(":is_active", j.isActive ? 1 : 0);
        query.bindValue(":priority", j.priority);
        query.bindValue(":source", j.source);
        query.bindValue(":url", j.url);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM target_jobs WHERE id = :id");
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
    static QString requirementsToJson(const QList<JobRequirement>& reqs) {
        QJsonArray arr;
        for (const auto& r : reqs) arr.append(r.toDict());
        return QJsonDocument(arr).toJson(QJsonDocument::Compact);
    }

    static QList<JobRequirement> requirementsFromJson(const QString& json) {
        QList<JobRequirement> list;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        if (doc.isArray()) {
            for (const auto& v : doc.array()) {
                list.append(JobRequirement::fromDict(v.toObject()));
            }
        }
        return list;
    }

    static Job mapFromQuery(const QSqlQuery& query) {
        Job j;
        j.id = query.value("id").toInt();
        j.title = query.value("title").toString();
        j.company = query.value("company").toString();
        j.location = query.value("location").toString();
        j.salaryRange = query.value("salary_range").toString();
        j.description = query.value("description").toString();
        j.requirements = requirementsFromJson(query.value("requirements").toString());
        j.isActive = query.value("is_active").toBool();
        j.priority = query.value("priority").toInt();
        j.source = query.value("source").toString();
        j.url = query.value("url").toString();
        j.createdAt = query.value("created_at").toDateTime();
        j.updatedAt = query.value("updated_at").toDateTime();
        return j;
    }
};

#endif // JOBDAO_H