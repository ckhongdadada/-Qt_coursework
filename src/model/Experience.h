#ifndef EXPERIENCE_H
#define EXPERIENCE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Experience {
public:
    int id = 0;
    QString title;
    QString type;
    QString organization;
    QString role;
    QString description;
    QString startDate;
    QString endDate;
    bool isOngoing = false;
    QString technologies;
    QString achievements;
    QString supervisor;
    QString contact;
    QString location;
    QString url;
    QDateTime createdAt;
    QDateTime updatedAt;

    Experience() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["type"] = type;
        obj["organization"] = organization;
        obj["role"] = role;
        obj["description"] = description;
        obj["startDate"] = startDate;
        obj["endDate"] = endDate;
        obj["isOngoing"] = isOngoing;
        QJsonArray techArr;
        for (const QString& t : technologies.split(',', Qt::SkipEmptyParts)) {
            techArr.append(t.trimmed());
        }
        obj["technologies"] = techArr;
        QJsonArray achArr;
        for (const QString& a : achievements.split(',', Qt::SkipEmptyParts)) {
            achArr.append(a.trimmed());
        }
        obj["achievements"] = achArr;
        obj["supervisor"] = supervisor;
        obj["contact"] = contact;
        obj["location"] = location;
        obj["url"] = url;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Experience fromDict(const QJsonObject& obj) {
        Experience e;
        e.id = obj["id"].toInt();
        e.title = obj["title"].toString();
        e.type = obj["type"].toString();
        e.organization = obj["organization"].toString();
        e.role = obj["role"].toString();
        e.description = obj["description"].toString();
        e.startDate = obj["startDate"].toString();
        e.endDate = obj["endDate"].toString();
        e.isOngoing = obj["isOngoing"].toBool(false);
        if (obj["technologies"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["technologies"].toArray()) parts.append(v.toString());
            e.technologies = parts.join(",");
        } else {
            e.technologies = obj["technologies"].toString();
        }
        if (obj["achievements"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["achievements"].toArray()) parts.append(v.toString());
            e.achievements = parts.join(",");
        } else {
            e.achievements = obj["achievements"].toString();
        }
        e.supervisor = obj["supervisor"].toString();
        e.contact = obj["contact"].toString();
        e.location = obj["location"].toString();
        e.url = obj["url"].toString();
        e.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        e.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return e;
    }
};

#endif // EXPERIENCE_H