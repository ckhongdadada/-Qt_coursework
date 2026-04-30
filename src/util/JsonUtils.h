#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHttpServerResponse>
#include <QHttpHeaders>

class JsonUtils {
public:
    static void applyCommonHeaders(QHttpServerResponse& response) {
        QHttpHeaders headers = response.headers();
        headers.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin, "*");
        headers.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods,
                                "GET, POST, PUT, DELETE, OPTIONS");
        headers.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders,
                                "Content-Type, Authorization, X-Auth-Token");
        response.setHeaders(std::move(headers));
    }

    static QHttpServerResponse createResponse(bool success, const QJsonValue& data,
                                              const QString& message = "", int code = 200) {
        QJsonObject response;
        response["success"] = success;
        response["code"] = code;
        response["message"] = message;
        response["data"] = data;

        QJsonDocument doc(response);
        QByteArray body = doc.toJson(QJsonDocument::Compact);

        auto statusCode = static_cast<QHttpServerResponse::StatusCode>(code);
        QHttpServerResponse httpResponse("application/json; charset=UTF-8", body, statusCode);
        applyCommonHeaders(httpResponse);
        return httpResponse;
    }

    static QHttpServerResponse successResponse(const QJsonValue& data, const QString& message = "") {
        return createResponse(true, data, message, 200);
    }

    static QHttpServerResponse createdResponse(const QJsonValue& data, const QString& message = "") {
        return createResponse(true, data, message, 201);
    }

    static QHttpServerResponse errorResponse(const QString& message, int code = 400) {
        return createResponse(false, QJsonValue(), message, code);
    }

    static QHttpServerResponse notFoundResponse(const QString& message = "资源未找到") {
        return createResponse(false, QJsonValue(), message, 404);
    }
};

#endif // JSONUTILS_H
