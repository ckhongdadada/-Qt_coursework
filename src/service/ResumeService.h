#ifndef RESUMESERVICE_H
#define RESUMESERVICE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include "dao/CourseDao.h"
#include "dao/AchievementDao.h"
#include "dao/ExperienceDao.h"
#include "dao/RoleDao.h"
#include "dao/ActivityDao.h"
#include "dao/GoalDao.h"

class ResumeService {
public:
    static QJsonObject generate(const QJsonObject& options) {
        QJsonObject resume;

        QString name = options["name"].toString("个人简历");
        resume["name"] = name;
        resume["title"] = options["title"].toString();
        resume["email"] = options["email"].toString();
        resume["phone"] = options["phone"].toString();
        resume["summary"] = options["summary"].toString();
        resume["customContent"] = options["customContent"].toString();

        QJsonArray sections;

        if (options["includeEducation"].toBool(true)) {
            CourseDao courseDao;
            QList<Course> courses = courseDao.getAll();
            QJsonArray eduArr;
            for (const auto& c : courses) {
                if (c.status == "Completed") eduArr.append(c.toDict());
            }
            QJsonObject sec;
            sec["title"] = "教育经历";
            sec["items"] = eduArr;
            sections.append(sec);
        }

        if (options["includeExperience"].toBool(true)) {
            ExperienceDao expDao;
            QList<Experience> exps = expDao.getAll();
            QJsonArray expArr;
            for (const auto& e : exps) expArr.append(e.toDict());
            QJsonObject sec;
            sec["title"] = "实践经历";
            sec["items"] = expArr;
            sections.append(sec);
        }

        if (options["includeAchievements"].toBool(true)) {
            AchievementDao achDao;
            QList<Achievement> achs = achDao.getAll();
            QJsonArray achArr;
            for (const auto& a : achs) achArr.append(a.toDict());
            QJsonObject sec;
            sec["title"] = "成就荣誉";
            sec["items"] = achArr;
            sections.append(sec);
        }

        if (options["includeRoles"].toBool(true)) {
            RoleDao roleDao;
            QList<Role> roles = roleDao.getAll();
            QJsonArray roleArr;
            for (const auto& r : roles) roleArr.append(r.toDict());
            QJsonObject sec;
            sec["title"] = "角色任职";
            sec["items"] = roleArr;
            sections.append(sec);
        }

        if (options["includeActivities"].toBool(false)) {
            ActivityDao actDao;
            QList<Activity> acts = actDao.getAll();
            QJsonArray actArr;
            for (const auto& a : acts) actArr.append(a.toDict());
            QJsonObject sec;
            sec["title"] = "活动参与";
            sec["items"] = actArr;
            sections.append(sec);
        }

        if (!options["customContent"].toString().trimmed().isEmpty()) {
            QJsonObject item;
            item["title"] = "补充亮点";
            item["description"] = options["customContent"].toString().trimmed();
            QJsonArray customItems;
            customItems.append(item);
            QJsonObject sec;
            sec["title"] = "自定义补充";
            sec["items"] = customItems;
            sections.append(sec);
        }

        resume["sections"] = sections;
        return resume;
    }

    static QByteArray exportJson(const QJsonObject& options) {
        QJsonObject resume = generate(options);
        return QJsonDocument(resume).toJson(QJsonDocument::Indented);
    }

    static QByteArray exportHtml(const QJsonObject& options) {
        QJsonObject resume = generate(options);
        QString html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
        html += "<title>" + resume["name"].toString() + "</title>";
        html += "<style>";
        html += "html,body{overflow-x:hidden;}";
        html += "body{font-family:'Microsoft YaHei',sans-serif;max-width:1120px;margin:0 auto;padding:36px;background:#f7f5ef;color:#2d241c;line-height:1.75;}";
        html += ".shell{background:#fffdf9;border:1px solid #ddcfbe;border-radius:20px;padding:34px 40px;box-sizing:border-box;}";
        html += ".hero{margin-bottom:30px;padding-bottom:22px;border-bottom:1px solid #eadfce;}";
        html += ".meta-wrap{display:flex;flex-wrap:wrap;gap:8px 18px;margin-top:10px;}";
        html += "h1{margin:0 0 6px 0;font-size:34px;color:#241c15;line-height:1.2;}";
        html += "h2{margin:30px 0 14px 0;font-size:20px;color:#2f261e;padding-bottom:8px;border-bottom:1px solid #f0e6d8;}";
        html += ".meta{color:#75685d;font-size:14px;margin:0;}";
        html += ".summary{color:#5b4e43;font-size:15px;margin-top:16px;background:#faf6ef;border:1px solid #eadfce;border-radius:12px;padding:14px 16px;}";
        html += ".item{margin:12px 0;padding:14px 16px;border:1px solid #e1d5c8;border-radius:12px;background:#fcf8f2;box-sizing:border-box;overflow-wrap:anywhere;word-break:break-word;}";
        html += ".item strong{display:block;color:#2d241c;font-size:16px;line-height:1.5;margin-bottom:4px;}";
        html += ".item .sub{color:#7b6f62;font-size:13px;line-height:1.6;margin-bottom:6px;}";
        html += ".item p{margin:0;color:#4b3e32;font-size:14px;line-height:1.75;overflow-wrap:anywhere;word-break:break-word;}";
        html += "</style></head><body>";
        html += "<div class='shell'><div class='hero'>";
        html += "<h1>" + resume["name"].toString() + "</h1>";
        if (!resume["title"].toString().isEmpty())
            html += "<p class='meta'>" + resume["title"].toString() + "</p>";
        if (!resume["email"].toString().isEmpty())
            html += "<p class='meta'>邮箱： " + resume["email"].toString() + "</p>";
        if (!resume["phone"].toString().isEmpty())
            html += "<p class='meta'>电话： " + resume["phone"].toString() + "</p>";
        if (!resume["summary"].toString().isEmpty())
            html += "<p class='summary'>" + resume["summary"].toString() + "</p>";
        html += "</div>";

        QJsonArray sections = resume["sections"].toArray();
        for (const auto& sec : sections) {
            QJsonObject s = sec.toObject();
            html += "<h2>" + s["title"].toString() + "</h2>";
            QJsonArray items = s["items"].toArray();
            for (const auto& item : items) {
                QJsonObject it = item.toObject();
                html += "<div class='item'>";
                html += "<strong>" + it["title"].toString() + it["name"].toString() + "</strong>";
                QString subtitle;
                if (!it["organization"].toString().isEmpty()) {
                    subtitle += it["organization"].toString();
                }
                if (!it["type"].toString().isEmpty()) {
                    subtitle += subtitle.isEmpty() ? it["type"].toString() : " · " + it["type"].toString();
                }
                if (!it["date"].toString().isEmpty()) {
                    subtitle += subtitle.isEmpty() ? it["date"].toString() : " · " + it["date"].toString();
                }
                if (!subtitle.isEmpty()) {
                    html += "<div class='sub'>" + subtitle + "</div>";
                }
                if (!it["description"].toString().isEmpty())
                    html += "<p>" + it["description"].toString() + "</p>";
                html += "</div>";
            }
        }
        html += "</div></body></html>";
        return html.toUtf8();
    }
};

#endif // RESUMESERVICE_H
