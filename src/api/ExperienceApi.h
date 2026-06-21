#ifndef EXPERIENCEAPI_H
#define EXPERIENCEAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/ExperienceService.h"
#include "util/JsonUtils.h"

class ExperienceApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Experience> list = ExperienceService::getAll();
        QJsonArray arr;
        for (const auto& e : list) arr.append(e.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Experience e = ExperienceService::getById(id);
        if (e.id == 0) return JsonUtils::notFoundResponse("经历不存在");
        return JsonUtils::successResponse(e.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Experience e = Experience::fromDict(doc.object());
        e = ExperienceService::create(e);
        if (e.id == 0) return JsonUtils::errorResponse("创建经历失败");
        return JsonUtils::createdResponse(e.toDict(), "经历创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Experience e = Experience::fromDict(doc.object());
        e = ExperienceService::update(id, e);
        if (e.id == 0) return JsonUtils::notFoundResponse("经历不存在");
        return JsonUtils::successResponse(e.toDict(), "经历更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = ExperienceService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除经历失败");
        return JsonUtils::successResponse(QJsonValue(), "经历删除成功");
    }

    static QHttpServerResponse getStatistics(const QHttpServerRequest&) {
        return JsonUtils::successResponse(ExperienceService::getStatistics());
    }
};

#endif // EXPERIENCEAPI_H