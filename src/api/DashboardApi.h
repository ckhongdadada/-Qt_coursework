#ifndef DASHBOARDAPI_H
#define DASHBOARDAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QUrlQuery>
#include "service/DashboardService.h"
#include "util/JsonUtils.h"

class DashboardApi {
public:
    static QHttpServerResponse getOverview(const QHttpServerRequest& request) {
        QString scale = "standard";
        QUrlQuery query(request.url());
        if (query.hasQueryItem("scale")) scale = query.queryItemValue("scale");
        return JsonUtils::successResponse(DashboardService::getOverview(scale));
    }

    static QHttpServerResponse getGpaTrend(const QHttpServerRequest& request) {
        QString scale = "standard";
        QUrlQuery query(request.url());
        if (query.hasQueryItem("scale")) scale = query.queryItemValue("scale");
        return JsonUtils::successResponse(DashboardService::getGpaTrend(scale));
    }

    static QHttpServerResponse getRecommendations(const QHttpServerRequest&) {
        return JsonUtils::successResponse(DashboardService::getRecommendations());
    }
};

#endif // DASHBOARDAPI_H