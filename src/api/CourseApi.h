#ifndef COURSEAPI_H
#define COURSEAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/CourseService.h"
#include "util/JsonUtils.h"
#include "util/Logger.h"

class CourseApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Course> courses = CourseService::getAll();
        QJsonArray arr;
        for (const auto& c : courses) arr.append(c.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Course course = CourseService::getById(id);
        if (course.id == 0) return JsonUtils::notFoundResponse("课程不存在");
        return JsonUtils::successResponse(course.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Course course = Course::fromDict(doc.object());
        course = CourseService::create(course);
        if (course.id == 0) return JsonUtils::errorResponse("创建课程失败");
        return JsonUtils::createdResponse(course.toDict(), "课程创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Course course = Course::fromDict(doc.object());
        course = CourseService::update(id, course);
        if (course.id == 0) return JsonUtils::notFoundResponse("课程不存在");
        return JsonUtils::successResponse(course.toDict(), "课程更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = CourseService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除课程失败");
        return JsonUtils::successResponse(QJsonValue(), "课程删除成功");
    }

    static QHttpServerResponse getStatistics(const QHttpServerRequest& request) {
        QString scale = "standard";
        double creditTarget = 120;
        QUrlQuery query(request.url());
        if (query.hasQueryItem("scale")) scale = query.queryItemValue("scale");
        if (query.hasQueryItem("creditTarget")) creditTarget = query.queryItemValue("creditTarget").toDouble();
        QJsonObject stats = CourseService::getStatistics(scale, creditTarget);
        return JsonUtils::successResponse(stats);
    }
};

#endif // COURSEAPI_H