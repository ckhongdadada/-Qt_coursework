#ifndef AIAPI_H
#define AIAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include "service/AiService.h"
#include "util/JsonUtils.h"

class AiApi {
public:
    static QHttpServerResponse analyze(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        QJsonObject result = AiService::analyze(doc.object());
        return JsonUtils::successResponse(result, "分析完成");
    }

    static QHttpServerResponse checkStatus(const QHttpServerRequest&) {
        return JsonUtils::successResponse(AiService::checkStatus());
    }

    static QHttpServerResponse chat(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        QJsonObject result = AiService::chat(doc.object());
        return JsonUtils::successResponse(result);
    }
};

#endif // AIAPI_H