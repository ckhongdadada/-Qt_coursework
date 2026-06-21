#ifndef JOB_H
#define JOB_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class JobRequirement {
public:
    QString text;
    bool met = false;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["text"] = text;
        obj["met"] = met;
        return obj;
    }

    static JobRequirement fromDict(const QJsonObject& obj) {
        JobRequirement r;
        r.text = obj["text"].toString();
        r.met = obj["met"].toBool(false);
        return r;
    }
};

class Job {
public:
    int id = 0;
    QString title;
    QString company;
    QString location;
    QString salaryRange;
    QString description;
    QList<JobRequirement> requirements;
    bool isActive = true;
    int priority = 0;
    QString source;
    QString url;
    QString status;
    QString appliedDate;
    QDateTime createdAt;
    QDateTime updatedAt;

    Job() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["company"] = company;
        obj["location"] = location;
        obj["salaryRange"] = salaryRange;
        obj["description"] = description;
        QJsonArray reqArr;
        for (const auto& r : requirements) {
            reqArr.append(r.toDict());
        }
        obj["requirements"] = reqArr;
        obj["isActive"] = isActive;
        obj["priority"] = priority;
        obj["source"] = source;
        obj["url"] = url;
        obj["status"] = status;
        obj["appliedDate"] = appliedDate;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Job fromDict(const QJsonObject& obj) {
        Job j;
        j.id = obj["id"].toInt();
        j.title = obj["title"].toString();
        j.company = obj["company"].toString();
        j.location = obj["location"].toString();
        j.salaryRange = obj["salaryRange"].toString();
        j.description = obj["description"].toString();
        if (obj["requirements"].isArray()) {
            for (const auto& v : obj["requirements"].toArray()) {
                j.requirements.append(JobRequirement::fromDict(v.toObject()));
            }
        }
        j.isActive = obj["isActive"].toBool(true);
        j.priority = obj["priority"].toInt();
        j.source = obj["source"].toString();
        j.url = obj["url"].toString();
        j.status = obj["status"].toString();
        j.appliedDate = obj["appliedDate"].toString();
        j.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        j.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return j;
    }
};

#endif // JOB_H