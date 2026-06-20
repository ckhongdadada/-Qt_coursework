#ifndef PEERBENCHMARK_H
#define PEERBENCHMARK_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

class PeerBenchmark {
public:
    int id = 0;
    QString name;
    QString major;
    QString semester;
    double gpa = 0;
    double credits = 0;
    int achievementsCount = 0;
    int experiencesCount = 0;
    QString note;
    QDateTime createdAt;
    QDateTime updatedAt;

    PeerBenchmark() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["major"] = major;
        obj["semester"] = semester;
        obj["gpa"] = gpa;
        obj["credits"] = credits;
        obj["achievementsCount"] = achievementsCount;
        obj["experiencesCount"] = experiencesCount;
        obj["note"] = note;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static PeerBenchmark fromDict(const QJsonObject& obj) {
        PeerBenchmark p;
        p.id = obj["id"].toInt();
        p.name = obj["name"].toString();
        p.major = obj["major"].toString();
        p.semester = obj["semester"].toString();
        p.gpa = obj["gpa"].toDouble();
        p.credits = obj["credits"].toDouble();
        p.achievementsCount = obj["achievementsCount"].toInt();
        p.experiencesCount = obj["experiencesCount"].toInt();
        p.note = obj["note"].toString();
        p.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        p.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return p;
    }
};

#endif // PEERBENCHMARK_H