#ifndef ANALYTICSAPI_H
#define ANALYTICSAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/AnalyticsService.h"
#include "util/JsonUtils.h"

class AnalyticsApi {
public:
    static QHttpServerResponse getSemesterComparison(const QHttpServerRequest&) {
        return JsonUtils::successResponse(AnalyticsService::getSemesterComparison());
    }

    static QHttpServerResponse getPeerComparison(const QHttpServerRequest&) {
        return JsonUtils::successResponse(AnalyticsService::compareWithPeers());
    }

    static QHttpServerResponse getReport(const QHttpServerRequest&) {
        return JsonUtils::successResponse(AnalyticsService::generateReport());
    }

    static QHttpServerResponse getPeers(const QHttpServerRequest&) {
        QList<PeerBenchmark> peers = PeerBenchmarkService::getAll();
        QJsonArray arr;
        for (const auto& p : peers) arr.append(p.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse createPeer(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        PeerBenchmark p = PeerBenchmark::fromDict(doc.object());
        p = PeerBenchmarkService::create(p);
        if (p.id == 0) return JsonUtils::errorResponse("创建失败");
        return JsonUtils::createdResponse(p.toDict(), "创建成功");
    }

    static QHttpServerResponse updatePeer(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        PeerBenchmark p = PeerBenchmark::fromDict(doc.object());
        p = PeerBenchmarkService::update(id, p);
        if (p.id == 0) return JsonUtils::notFoundResponse("同学数据不存在");
        return JsonUtils::successResponse(p.toDict(), "更新成功");
    }

    static QHttpServerResponse deletePeer(int id) {
        bool success = PeerBenchmarkService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除失败");
        return JsonUtils::successResponse(QJsonValue(), "删除成功");
    }
};

#endif // ANALYTICSAPI_H