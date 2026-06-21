#ifndef ACTIVITYAPI_H
#define ACTIVITYAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/ActivityService.h"
#include "util/JsonUtils.h"

class ActivityApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Activity> list = ActivityService::getAll();
        QJsonArray arr;
        for (const auto& a : list) arr.append(a.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Activity a = ActivityService::getById(id);
        if (a.id == 0) return JsonUtils::notFoundResponse("活动不存在");
        return JsonUtils::successResponse(a.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Activity a = Activity::fromDict(doc.object());
        a = ActivityService::create(a);
        if (a.id == 0) return JsonUtils::errorResponse("创建活动失败");
        return JsonUtils::createdResponse(a.toDict(), "活动创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Activity a = Activity::fromDict(doc.object());
        a = ActivityService::update(id, a);
        if (a.id == 0) return JsonUtils::notFoundResponse("活动不存在");
        return JsonUtils::successResponse(a.toDict(), "活动更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = ActivityService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除活动失败");
        return JsonUtils::successResponse(QJsonValue(), "活动删除成功");
    }
};

#endif // ACTIVITYAPI_H