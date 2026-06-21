#ifndef IMPORTAPI_H
#define IMPORTAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include "service/ImportService.h"
#include "util/JsonUtils.h"

class ImportApi {
public:
    static QHttpServerResponse importEntity(const QString& entity, const QHttpServerRequest& request) {
        QString contentType = QString::fromUtf8(request.headers().value("content-type").toByteArray());
        if (contentType.contains("multipart/form-data") || contentType.contains("text/csv")) {
            QJsonObject result = ImportService::importData(entity, request.body(), "data.csv");
            if (result["error"].toBool()) {
                return JsonUtils::errorResponse(result["message"].toString());
            }
            return JsonUtils::successResponse(result, "导入完成");
        }

        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (doc.isObject() || doc.isArray()) {
            QJsonObject result;
            result["entity"] = entity;
            result["imported"] = 0;
            result["failed"] = 0;
            result["message"] = "请使用CSV文件上传导入";
            return JsonUtils::successResponse(result);
        }

        return JsonUtils::errorResponse("请上传CSV文件");
    }
};

#endif // IMPORTAPI_H
