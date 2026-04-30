#ifndef ACHIEVEMENTAPI_H
#define ACHIEVEMENTAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/AchievementService.h"
#include "util/JsonUtils.h"

class AchievementApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Achievement> list = AchievementService::getAll();
        QJsonArray arr;
        for (const auto& a : list) arr.append(a.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Achievement a = AchievementService::getById(id);
        if (a.id == 0) return JsonUtils::notFoundResponse("成就不存在");
        return JsonUtils::successResponse(a.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Achievement a = Achievement::fromDict(doc.object());
        a = AchievementService::create(a);
        if (a.id == 0) return JsonUtils::errorResponse("创建成就失败");
        return JsonUtils::createdResponse(a.toDict(), "成就创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Achievement a = Achievement::fromDict(doc.object());
        a = AchievementService::update(id, a);
        if (a.id == 0) return JsonUtils::notFoundResponse("成就不存在");
        return JsonUtils::successResponse(a.toDict(), "成就更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = AchievementService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除成就失败");
        return JsonUtils::successResponse(QJsonValue(), "成就删除成功");
    }

    static QHttpServerResponse getStatistics(const QHttpServerRequest&) {
        return JsonUtils::successResponse(AchievementService::getStatistics());
    }
};

#endif // ACHIEVEMENTAPI_H