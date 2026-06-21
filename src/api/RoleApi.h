#ifndef ROLEAPI_H
#define ROLEAPI_H

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "service/RoleService.h"
#include "util/JsonUtils.h"

class RoleApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Role> roles = RoleService::getAll();
        QJsonArray arr;
        for (const auto& r : roles) arr.append(r.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Role role = RoleService::getById(id);
        if (role.id == 0) return JsonUtils::notFoundResponse("角色不存在");
        return JsonUtils::successResponse(role.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Role role = Role::fromDict(doc.object());
        role = RoleService::create(role);
        if (role.id == 0) return JsonUtils::errorResponse("创建角色失败");
        return JsonUtils::createdResponse(role.toDict(), "角色创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Role role = Role::fromDict(doc.object());
        role = RoleService::update(id, role);
        if (role.id == 0) return JsonUtils::notFoundResponse("角色不存在");
        return JsonUtils::successResponse(role.toDict(), "角色更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = RoleService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除角色失败");
        return JsonUtils::successResponse(QJsonValue(), "角色删除成功");
    }

    static QHttpServerResponse getStatistics(const QHttpServerRequest&) {
        return JsonUtils::successResponse(RoleService::getStatistics());
    }
};

#endif // ROLEAPI_H