#ifndef TIMELINEAPI_H
#define TIMELINEAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include "service/AnalyticsService.h"
#include "util/JsonUtils.h"

class TimelineApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        return JsonUtils::successResponse(AnalyticsService::getTimelineEvents());
    }
};

#endif // TIMELINEAPI_H