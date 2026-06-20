#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Activity {
public:
    int id = 0;
    QString name;
    QString description;
    QString category;
    QString startDate;
    QString endDate;
    bool isFavorite = false;
    bool isActive = true;
    QString tags;
    QDateTime createdAt;
    QDateTime updatedAt;

    Activity() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["description"] = description;
        obj["category"] = category;
        obj["startDate"] = startDate;
        obj["endDate"] = endDate;
        obj["isFavorite"] = isFavorite;
        obj["isActive"] = isActive;
        QJsonArray tagsArr;
        for (const QString& t : tags.split(',', Qt::SkipEmptyParts)) {
            tagsArr.append(t.trimmed());
        }
        obj["tags"] = tagsArr;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Activity fromDict(const QJsonObject& obj) {
        Activity a;
        a.id = obj["id"].toInt();
        a.name = obj["name"].toString();
        a.description = obj["description"].toString();
        a.category = obj["category"].toString();
        a.startDate = obj["startDate"].toString();
        a.endDate = obj["endDate"].toString();
        a.isFavorite = obj["isFavorite"].toBool(false);
        a.isActive = obj["isActive"].toBool(true);
        if (obj["tags"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["tags"].toArray()) parts.append(v.toString());
            a.tags = parts.join(",");
        } else {
            a.tags = obj["tags"].toString();
        }
        a.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return a;
    }
};

#endif // ACTIVITY_H