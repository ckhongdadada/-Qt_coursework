#ifndef AUTHAPI_H
#define AUTHAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include "service/AuthService.h"
#include "util/JsonUtils.h"

class AuthApi {
public:
    static QHttpServerResponse registerUser(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        QJsonObject obj = doc.object();
        QString username = obj["username"].toString();
        QString email = obj["email"].toString();
        QString password = obj["password"].toString();
        QString displayName = obj["displayName"].toString();

        if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
            return JsonUtils::errorResponse("用户名、邮箱和密码不能为空");
        }

        QJsonObject result = AuthService::registerUser(username, email, password, displayName);
        if (result["error"].toBool()) {
            return JsonUtils::errorResponse(result["message"].toString());
        }
        return JsonUtils::createdResponse(result, "注册成功");
    }

    static QHttpServerResponse login(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        QJsonObject obj = doc.object();
        QString username = obj["username"].toString();
        QString password = obj["password"].toString();

        if (username.isEmpty() || password.isEmpty()) {
            return JsonUtils::errorResponse("用户名和密码不能为空");
        }

        QJsonObject result = AuthService::login(username, password);
        if (result["error"].toBool()) {
            return JsonUtils::errorResponse(result["message"].toString(), 401);
        }
        return JsonUtils::successResponse(result, "登录成功");
    }

    static QHttpServerResponse getMe(const QHttpServerRequest& request) {
        QString authHeader = headerValue(request, "authorization");
        QString token;
        if (authHeader.startsWith("Bearer ")) {
            token = authHeader.mid(7);
        }
        if (token.isEmpty()) {
            token = headerValue(request, "x-auth-token");
        }
        if (token.isEmpty()) {
            return JsonUtils::errorResponse("请先登录", 401);
        }

        int userId = extractUserIdFromToken(token);
        if (userId <= 0) {
            return JsonUtils::errorResponse("无效的认证信息", 401);
        }

        QJsonObject result = AuthService::getMe(userId);
        if (result["error"].toBool()) {
            return JsonUtils::notFoundResponse(result["message"].toString());
        }
        return JsonUtils::successResponse(result);
    }

    static QHttpServerResponse changePassword(const QHttpServerRequest& request) {
        QString authHeader = headerValue(request, "authorization");
        QString token;
        if (authHeader.startsWith("Bearer ")) token = authHeader.mid(7);
        if (token.isEmpty()) token = headerValue(request, "x-auth-token");
        if (token.isEmpty()) return JsonUtils::errorResponse("请先登录", 401);

        int userId = extractUserIdFromToken(token);
        if (userId <= 0) return JsonUtils::errorResponse("无效的认证信息", 401);

        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        QJsonObject obj = doc.object();
        QString oldPassword = obj["oldPassword"].toString();
        QString newPassword = obj["newPassword"].toString();

        QJsonObject result = AuthService::changePassword(userId, oldPassword, newPassword);
        if (result["error"].toBool()) {
            return JsonUtils::errorResponse(result["message"].toString());
        }
        return JsonUtils::successResponse(QJsonValue(), "密码修改成功");
    }

    static QHttpServerResponse updateMe(const QHttpServerRequest& request) {
        QString authHeader = headerValue(request, "authorization");
        QString token;
        if (authHeader.startsWith("Bearer ")) token = authHeader.mid(7);
        if (token.isEmpty()) token = headerValue(request, "x-auth-token");
        if (token.isEmpty()) return JsonUtils::errorResponse("请先登录", 401);

        int userId = extractUserIdFromToken(token);
        if (userId <= 0) return JsonUtils::errorResponse("无效的认证信息", 401);

        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        QJsonObject obj = doc.object();

        QJsonObject result = AuthService::updateProfile(userId, obj);
        if (result["error"].toBool()) {
            return JsonUtils::errorResponse(result["message"].toString());
        }
        return JsonUtils::successResponse(result, "资料更新成功");
    }

private:
    static QString headerValue(const QHttpServerRequest& request, const char* name) {
        return QString::fromUtf8(request.headers().value(name).toByteArray());
    }

    static int extractUserIdFromToken(const QString& token) {
        QStringList parts = token.split('.');
        if (parts.size() >= 1) {
            QByteArray decoded = QByteArray::fromBase64(parts[0].toUtf8());
            bool ok;
            int userId = decoded.toInt(&ok);
            return ok ? userId : 0;
        }
        return 0;
    }
};

#endif // AUTHAPI_H
