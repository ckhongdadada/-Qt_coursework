#ifndef JOBAPI_H
#define JOBAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/JobService.h"
#include "util/JsonUtils.h"

class JobApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Job> list = JobService::getAll();
        QJsonArray arr;
        for (const auto& j : list) arr.append(j.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Job j = JobService::getById(id);
        if (j.id == 0) return JsonUtils::notFoundResponse("职位不存在");
        return JsonUtils::successResponse(j.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Job j = Job::fromDict(doc.object());
        j = JobService::create(j);
        if (j.id == 0) return JsonUtils::errorResponse("创建职位失败");
        return JsonUtils::createdResponse(j.toDict(), "职位创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Job j = Job::fromDict(doc.object());
        j = JobService::update(id, j);
        if (j.id == 0) return JsonUtils::notFoundResponse("职位不存在");
        return JsonUtils::successResponse(j.toDict(), "职位更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = JobService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除职位失败");
        return JsonUtils::successResponse(QJsonValue(), "职位删除成功");
    }

    static QHttpServerResponse importJobs(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isArray()) return JsonUtils::errorResponse("无效的JSON数据，需要数组");
        JobService::importJobs(doc.array());
        return JsonUtils::successResponse(QJsonValue(), "职位导入成功");
    }

    static QHttpServerResponse toggleRequirement(int jobId, int reqIndex) {
        Job j = JobService::toggleRequirement(jobId, reqIndex);
        if (j.id == 0) return JsonUtils::notFoundResponse("职位不存在");
        return JsonUtils::successResponse(j.toDict(), "需求状态已切换");
    }
};

#endif // JOBAPI_H