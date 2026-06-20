#ifndef IMPORTSERVICE_H
#define IMPORTSERVICE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include "service/CourseService.h"
#include "service/RoleService.h"
#include "service/AchievementService.h"
#include "service/ExperienceService.h"
#include "service/ActivityService.h"
#include "service/GoalService.h"
#include "service/AnalyticsService.h"

class ImportService {
public:
    static QJsonObject importData(const QString& entity, const QByteArray& fileData, const QString& filename) {
        QJsonObject err;
        if (entity != "courses" && entity != "roles" && entity != "achievements" &&
            entity != "experiences" && entity != "activities" && entity != "goals" && entity != "peers") {
            err["error"] = true;
            err["message"] = "暂不支持该类型的数据导入";
            return err;
        }

        QJsonArray rows = parseCsv(fileData, entity);
        int imported = 0;
        QJsonArray errors;
        for (int i = 0; i < rows.size(); ++i) {
            QJsonObject row = rows[i].toObject();
            if (row.isEmpty()) continue;
            bool ok = importRow(entity, row);
            if (ok) imported++;
            else {
                QJsonObject e;
                e["row"] = i + 2;
                e["error"] = "导入失败";
                errors.append(e);
                if (errors.size() >= 20) break;
            }
        }

        QJsonObject result;
        result["entity"] = entity;
        result["imported"] = imported;
        result["failed"] = errors.size();
        result["errors"] = errors;
        return result;
    }

private:
    static bool importRow(const QString& entity, const QJsonObject& row) {
        if (entity == "courses") {
            Course c;
            c.name = row["name"].toString();
            c.code = row["code"].toString();
            c.credits = row["credits"].toDouble();
            c.semester = row["semester"].toString();
            c.category = row["category"].toString("Required");
            c.score = row["score"].toDouble();
            c.status = row["status"].toString("Planned");
            c.teacher = row["teacher"].toString();
            CourseService::create(c);
            return true;
        } else if (entity == "roles") {
            Role r;
            r.title = row["title"].toString();
            r.type = row["type"].toString();
            r.organization = row["organization"].toString();
            r.description = row["description"].toString();
            r.startDate = row["startDate"].toString();
            r.endDate = row["endDate"].toString();
            RoleService::create(r);
            return true;
        } else if (entity == "achievements") {
            Achievement a;
            a.title = row["title"].toString();
            a.type = row["type"].toString();
            a.level = row["level"].toString();
            a.organization = row["organization"].toString();
            a.description = row["description"].toString();
            a.date = row["date"].toString();
            AchievementService::create(a);
            return true;
        } else if (entity == "experiences") {
            Experience e;
            e.title = row["title"].toString();
            e.type = row["type"].toString();
            e.organization = row["organization"].toString();
            e.role = row["role"].toString();
            e.description = row["description"].toString();
            e.startDate = row["startDate"].toString();
            e.endDate = row["endDate"].toString();
            ExperienceService::create(e);
            return true;
        } else if (entity == "activities") {
            Activity a;
            a.name = row["name"].toString();
            a.description = row["description"].toString();
            a.startDate = row["startDate"].toString();
            a.endDate = row["endDate"].toString();
            ActivityService::create(a);
            return true;
        } else if (entity == "goals") {
            Goal g;
            g.title = row["title"].toString();
            g.category = row["category"].toString();
            g.description = row["description"].toString();
            g.targetValue = row["targetValue"].toDouble();
            g.currentValue = row["currentValue"].toDouble();
            g.deadline = row["deadline"].toString();
            g.priority = row["priority"].toString("Medium");
            g.status = row["status"].toString("In Progress");
            GoalService::create(g);
            return true;
        } else if (entity == "peers") {
            PeerBenchmark p;
            p.name = row["name"].toString();
            p.major = row["major"].toString();
            p.semester = row["semester"].toString();
            p.gpa = row["gpa"].toDouble();
            p.credits = row["credits"].toDouble();
            p.note = row["note"].toString();
            PeerBenchmarkService::create(p);
            return true;
        }
        return false;
    }

    static QJsonArray parseCsv(const QByteArray& data, const QString& entity) {
        QJsonArray rows;
        QTextStream stream(data);
        stream.setEncoding(QStringConverter::Utf8);

        QString headerLine = stream.readLine();
        if (headerLine.isEmpty()) return rows;

        QStringList headers = parseCsvLine(headerLine);
        QStringList normalizedHeaders;
        for (const auto& h : headers) {
            normalizedHeaders.append(normalizeHeader(entity, h.trimmed()));
        }

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.trimmed().isEmpty()) continue;
            QStringList values = parseCsvLine(line);
            QJsonObject row;
            for (int i = 0; i < normalizedHeaders.size() && i < values.size(); ++i) {
                if (!normalizedHeaders[i].isEmpty()) {
                    row[normalizedHeaders[i]] = values[i].trimmed();
                }
            }
            rows.append(row);
        }
        return rows;
    }

    static QStringList parseCsvLine(const QString& line) {
        QStringList result;
        QString current;
        bool inQuotes = false;
        for (int i = 0; i < line.size(); ++i) {
            QChar c = line[i];
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                result.append(current);
                current.clear();
            } else {
                current += c;
            }
        }
        result.append(current);
        return result;
    }

    static QString normalizeHeader(const QString& entity, const QString& header) {
        static QMap<QString, QMap<QString, QString>> ALIASES = {
            {"courses", {
                {"课程名称", "name"}, {"name", "name"},
                {"课程代码", "code"}, {"code", "code"},
                {"学分", "credits"}, {"credits", "credits"},
                {"学期", "semester"}, {"semester", "semester"},
                {"类别", "category"}, {"category", "category"},
                {"分数", "score"}, {"score", "score"},
                {"状态", "status"}, {"status", "status"},
                {"教师", "teacher"}, {"teacher", "teacher"},
            }},
            {"roles", {
                {"标题", "title"}, {"title", "title"},
                {"类型", "type"}, {"type", "type"},
                {"组织", "organization"}, {"organization", "organization"},
                {"开始日期", "startDate"}, {"startDate", "startDate"},
                {"结束日期", "endDate"}, {"endDate", "endDate"},
                {"描述", "description"}, {"description", "description"},
            }},
            {"achievements", {
                {"标题", "title"}, {"title", "title"},
                {"类型", "type"}, {"type", "type"},
                {"级别", "level"}, {"level", "level"},
                {"组织", "organization"}, {"organization", "organization"},
                {"日期", "date"}, {"date", "date"},
                {"描述", "description"}, {"description", "description"},
            }},
            {"experiences", {
                {"标题", "title"}, {"title", "title"},
                {"类型", "type"}, {"type", "type"},
                {"组织", "organization"}, {"organization", "organization"},
                {"角色", "role"}, {"role", "role"},
                {"开始日期", "startDate"}, {"startDate", "startDate"},
                {"结束日期", "endDate"}, {"endDate", "endDate"},
                {"描述", "description"}, {"description", "description"},
            }},
            {"activities", {
                {"名称", "name"}, {"name", "name"},
                {"开始日期", "startDate"}, {"startDate", "startDate"},
                {"结束日期", "endDate"}, {"endDate", "endDate"},
                {"描述", "description"}, {"description", "description"},
            }},
            {"goals", {
                {"标题", "title"}, {"title", "title"},
                {"类别", "category"}, {"category", "category"},
                {"目标值", "targetValue"}, {"targetValue", "targetValue"},
                {"当前进度", "currentValue"}, {"currentValue", "currentValue"},
                {"截止日期", "deadline"}, {"deadline", "deadline"},
                {"优先级", "priority"}, {"priority", "priority"},
                {"状态", "status"}, {"status", "status"},
                {"描述", "description"}, {"description", "description"},
            }},
            {"peers", {
                {"姓名", "name"}, {"name", "name"},
                {"专业", "major"}, {"major", "major"},
                {"学期", "semester"}, {"semester", "semester"},
                {"GPA", "gpa"}, {"gpa", "gpa"},
                {"学分", "credits"}, {"credits", "credits"},
                {"备注", "note"}, {"note", "note"},
            }},
        };

        if (ALIASES.contains(entity) && ALIASES[entity].contains(header)) {
            return ALIASES[entity][header];
        }
        return QString();
    }
};

#endif // IMPORTSERVICE_H