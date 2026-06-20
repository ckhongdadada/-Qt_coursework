#ifndef GOAL_H
#define GOAL_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Goal {
public:
    int id = 0;
    QString title;
    QString category;
    QString description;
    double targetValue = 0;
    double currentValue = 0;
    QString unit;
    QString deadline;
    QString priority = "Medium";
    QString status = "In Progress";
    QString milestones;
    QDateTime createdAt;
    QDateTime updatedAt;

    Goal() = default;

    double progress() const {
        if (targetValue <= 0) return 0;
        return round(currentValue / targetValue * 10000.0) / 100.0;
    }

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["category"] = category;
        obj["description"] = description;
        obj["targetValue"] = targetValue;
        obj["currentValue"] = currentValue;
        obj["unit"] = unit;
        obj["deadline"] = deadline;
        obj["priority"] = priority;
        obj["status"] = status;
        obj["progress"] = progress();
        QJsonArray msArr;
        for (const QString& m : milestones.split(',', Qt::SkipEmptyParts)) {
            msArr.append(m.trimmed());
        }
        obj["milestones"] = msArr;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Goal fromDict(const QJsonObject& obj) {
        Goal g;
        g.id = obj["id"].toInt();
        g.title = obj["title"].toString();
        g.category = obj["category"].toString();
        g.description = obj["description"].toString();
        g.targetValue = obj["targetValue"].toDouble();
        g.currentValue = obj["currentValue"].toDouble();
        g.unit = obj["unit"].toString();
        g.deadline = obj["deadline"].toString();
        g.priority = obj["priority"].toString();
        g.status = obj["status"].toString();
        if (obj["milestones"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["milestones"].toArray()) parts.append(v.toString());
            g.milestones = parts.join(",");
        } else {
            g.milestones = obj["milestones"].toString();
        }
        g.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        g.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return g;
    }
};

#endif // GOAL_H