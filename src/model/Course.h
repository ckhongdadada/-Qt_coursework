#ifndef COURSE_H
#define COURSE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Course {
public:
    int id = 0;
    QString name;
    QString code;
    double credits = 0;
    QString semester;
    QString category = "Required";
    double score = 0;
    double gradePoint = 0;
    QString status = "Planned";
    QString teacher;
    QString location;
    QString description;
    QString tags;
    QDateTime createdAt;
    QDateTime updatedAt;

    Course() = default;

    static double calculateGradePoint(double score, const QString& scale = "standard");

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["code"] = code;
        obj["credits"] = credits;
        obj["semester"] = semester;
        obj["category"] = category;
        obj["score"] = score;
        obj["gradePoint"] = gradePoint;
        obj["status"] = status;
        obj["teacher"] = teacher;
        obj["location"] = location;
        obj["description"] = description;
        QJsonArray tagsArr;
        for (const QString& t : tags.split(',', Qt::SkipEmptyParts)) {
            tagsArr.append(t.trimmed());
        }
        obj["tags"] = tagsArr;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static Course fromDict(const QJsonObject& obj) {
        Course c;
        c.id = obj["id"].toInt();
        c.name = obj["name"].toString();
        c.code = obj["code"].toString();
        c.credits = obj["credits"].toDouble();
        c.semester = obj["semester"].toString();
        c.category = obj["category"].toString();
        c.score = obj["score"].toDouble();
        c.gradePoint = obj["gradePoint"].toDouble();
        c.status = obj["status"].toString();
        c.teacher = obj["teacher"].toString();
        c.location = obj["location"].toString();
        c.description = obj["description"].toString();
        if (obj["tags"].isArray()) {
            QStringList parts;
            for (const auto& v : obj["tags"].toArray()) {
                parts.append(v.toString());
            }
            c.tags = parts.join(",");
        } else {
            c.tags = obj["tags"].toString();
        }
        c.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        c.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return c;
    }
};

#endif // COURSE_H