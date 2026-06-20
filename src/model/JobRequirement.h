#ifndef JOBREQUIREMENT_H
#define JOBREQUIREMENT_H

#include <QString>
#include <QJsonObject>

class JobRequirement {
public:
    int id = 0;
    int jobId = 0;
    QString skillName;
    QString importance = "Required";
    QString proficiency = "Intermediate";

    JobRequirement() = default;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["jobId"] = jobId;
        obj["skillName"] = skillName;
        obj["importance"] = importance;
        obj["proficiency"] = proficiency;
        return obj;
    }

    static JobRequirement fromJson(const QJsonObject& obj) {
        JobRequirement req;
        req.id = obj["id"].toInt();
        req.jobId = obj["jobId"].toInt();
        req.skillName = obj["skillName"].toString();
        req.importance = obj["importance"].toString();
        req.proficiency = obj["proficiency"].toString();
        return req;
    }
};

#endif // JOBREQUIREMENT_H