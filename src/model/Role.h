#ifndef ROLE_H
#define ROLE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Role {
public:
    int id = 0;
    QString title;
    QString type;
    QString organization;
    QString description;
    QString startDate;
    QString endDate;
    bool isActive = true;
    QString achievements;
    QString contact;
    QString supervisor;
    QDateTime createdAt;
    QDateTime updatedAt;

    Role() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["type"] = type;
        obj["organization"] = organization;
        obj["description"] = description;
        obj["startDate"] = startDate;
        obj["endDate"] = endDate;
        obj["isActive"] = isActive;
        QJsonArray achArr;
        for (const QString& a : achievements.split(',', Qt::SkipEmptyParts)) {
            achArr.append(a.trimmed());
        }
        obj["achievements"] = achArr;
        obj["contact"] = contact;
        obj["supervisor"] = supervisor;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Role fromDict(const QJsonObject& obj) {
        Role r;
        r.id = obj["id"].toInt();
        r.title = obj["title"].toString();
        r.type = obj["type"].toString();
        r.organization = obj["organization"].toString();
        r.description = obj["description"].toString();
        r.startDate = obj["startDate"].toString();
        r.endDate = obj["endDate"].toString();
        r.isActive = obj["isActive"].toBool(true);
        if (obj["achievements"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["achievements"].toArray()) {
                parts.append(v.toString());
            }
            r.achievements = parts.join(",");
        } else {
            r.achievements = obj["achievements"].toString();
        }
        r.contact = obj["contact"].toString();
        r.supervisor = obj["supervisor"].toString();
        r.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        r.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return r;
    }
};

#endif // ROLE_H