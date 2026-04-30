#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Achievement {
public:
    int id = 0;
    QString title;
    QString type;
    QString level;
    QString organization;
    QString description;
    QString date;
    QString certificate;
    QString relatedCourse;
    QString teamMembers;
    QString ranking;
    QString prize;
    bool verified = false;
    QDateTime createdAt;
    QDateTime updatedAt;

    Achievement() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["type"] = type;
        obj["level"] = level;
        obj["organization"] = organization;
        obj["description"] = description;
        obj["date"] = date;
        obj["certificate"] = certificate;
        obj["relatedCourse"] = relatedCourse;
        QJsonArray tmArr;
        for (const QString& m : teamMembers.split(',', Qt::SkipEmptyParts)) {
            tmArr.append(m.trimmed());
        }
        obj["teamMembers"] = tmArr;
        obj["ranking"] = ranking;
        obj["prize"] = prize;
        obj["verified"] = verified;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Achievement fromDict(const QJsonObject& obj) {
        Achievement a;
        a.id = obj["id"].toInt();
        a.title = obj["title"].toString();
        a.type = obj["type"].toString();
        a.level = obj["level"].toString();
        a.organization = obj["organization"].toString();
        a.description = obj["description"].toString();
        a.date = obj["date"].toString();
        a.certificate = obj["certificate"].toString();
        a.relatedCourse = obj["relatedCourse"].toString();
        if (obj["teamMembers"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["teamMembers"].toArray()) {
                parts.append(v.toString());
            }
            a.teamMembers = parts.join(",");
        } else {
            a.teamMembers = obj["teamMembers"].toString();
        }
        a.ranking = obj["ranking"].toString();
        a.prize = obj["prize"].toString();
        a.verified = obj["verified"].toBool(false);
        a.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return a;
    }
};

#endif // ACHIEVEMENT_H