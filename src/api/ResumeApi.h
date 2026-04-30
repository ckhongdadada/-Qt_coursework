#ifndef RESUMEAPI_H
#define RESUMEAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include "service/ResumeService.h"
#include "util/JsonUtils.h"

class ResumeApi {
public:
    static QHttpServerResponse generate(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject options = doc.isObject() ? doc.object() : QJsonObject();
        QJsonObject resume = ResumeService::generate(options);
        return JsonUtils::successResponse(resume, "简历生成成功");
    }

    static QHttpServerResponse exportJson(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject options = doc.isObject() ? doc.object() : QJsonObject();
        QByteArray data = ResumeService::exportJson(options);
        QHttpServerResponse response("application/json; charset=UTF-8", data,
                                     QHttpServerResponse::StatusCode::Ok);
        QHttpHeaders headers = response.headers();
        headers.replaceOrAppend(QHttpHeaders::WellKnownHeader::ContentDisposition,
                                "attachment; filename=resume.json");
        response.setHeaders(std::move(headers));
        JsonUtils::applyCommonHeaders(response);
        return response;
    }

    static QHttpServerResponse exportHtml(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject options = doc.isObject() ? doc.object() : QJsonObject();
        QByteArray data = ResumeService::exportHtml(options);
        QHttpServerResponse response("text/html; charset=UTF-8", data,
                                     QHttpServerResponse::StatusCode::Ok);
        QHttpHeaders headers = response.headers();
        headers.replaceOrAppend(QHttpHeaders::WellKnownHeader::ContentDisposition,
                                "attachment; filename=resume.html");
        response.setHeaders(std::move(headers));
        JsonUtils::applyCommonHeaders(response);
        return response;
    }
};

#endif // RESUMEAPI_H
