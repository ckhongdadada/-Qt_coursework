#ifndef GOALAPI_H
#define GOALAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/GoalService.h"
#include "util/JsonUtils.h"

class GoalApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Goal> list = GoalService::getAll();
        QJsonArray arr;
        for (const auto& g : list) arr.append(g.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Goal g = GoalService::getById(id);
        if (g.id == 0) return JsonUtils::notFoundResponse("目标不存在");
        return JsonUtils::successResponse(g.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Goal g = Goal::fromDict(doc.object());
        g = GoalService::create(g);
        if (g.id == 0) return JsonUtils::errorResponse("创建目标失败");
        return JsonUtils::createdResponse(g.toDict(), "目标创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Goal g = Goal::fromDict(doc.object());
        g = GoalService::update(id, g);
        if (g.id == 0) return JsonUtils::notFoundResponse("目标不存在");
        return JsonUtils::successResponse(g.toDict(), "目标更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = GoalService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除目标失败");
        return JsonUtils::successResponse(QJsonValue(), "目标删除成功");
    }

    static QHttpServerResponse getStatistics(const QHttpServerRequest&) {
        return JsonUtils::successResponse(GoalService::getStatistics());
    }
};

#endif // GOALAPI_H