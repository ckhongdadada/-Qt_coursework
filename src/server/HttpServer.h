#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QHttpServer>
#include <QTcpServer>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include "util/Logger.h"
#include "util/JsonUtils.h"

#include "api/CourseApi.h"
#include "api/RoleApi.h"
#include "api/AchievementApi.h"
#include "api/ExperienceApi.h"
#include "api/ActivityApi.h"
#include "api/GoalApi.h"
#include "api/JobApi.h"
#include "api/DashboardApi.h"
#include "api/AnalyticsApi.h"
#include "api/TimelineApi.h"
#include "api/ImportApi.h"
#include "api/ResumeApi.h"
#include "api/AiApi.h"
#include "api/AuthApi.h"

class HttpServer {
public:
    bool start(quint16 port = 8080) {
        registerRoutes();

        m_tcpServer.listen(QHostAddress::Any, port);
        if (!m_server.bind(&m_tcpServer)) {
            Logger::error(QString("服务器启动失败，端口 %1 可能已被占用").arg(port));
            return false;
        }

        Logger::info(QString("服务器已启动，监听端口 %1").arg(port));
        return true;
    }

    void stop() {
        if (m_tcpServer.isListening()) {
            m_tcpServer.close();
            Logger::info("服务器已停止");
        }
    }

private:
    QString frontendRootPath() const {
        const QStringList candidates = {
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("frontend_dist"),
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../frontend_dist"),
            QDir::current().absoluteFilePath("frontend_dist")
        };

        for (const QString& candidate : candidates) {
            if (QFileInfo::exists(QDir(candidate).absoluteFilePath("index.html"))) {
                return candidate;
            }
        }
        return QString();
    }

    QHttpServerResponse staticFileResponse(const QString& relativePath) const {
        const QString root = frontendRootPath();
        if (root.isEmpty()) {
            return JsonUtils::errorResponse("前端静态资源不存在", 404);
        }

        const QString absolutePath = QDir(root).absoluteFilePath(relativePath);
        if (!QFileInfo::exists(absolutePath)) {
            return JsonUtils::errorResponse("静态资源不存在", 404);
        }

        QHttpServerResponse response = QHttpServerResponse::fromFile(absolutePath);
        JsonUtils::applyCommonHeaders(response);
        return response;
    }

    void registerRoutes() {
        m_server.addAfterRequestHandler(&m_server, [](const QHttpServerRequest&, QHttpServerResponse& resp) {
            JsonUtils::applyCommonHeaders(resp);
        });

        m_server.route("/", QHttpServerRequest::Method::Get,
            [this]() { return staticFileResponse("index.html"); });

        m_server.route("/assets/<arg>", QHttpServerRequest::Method::Get,
            [this](const QString& fileName) { return staticFileResponse("assets/" + fileName); });

        m_server.route("/api/courses/statistics", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return CourseApi::getStatistics(req); });

        m_server.route("/api/courses", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return CourseApi::getAll(req); });

        m_server.route("/api/courses", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return CourseApi::create(req); });

        m_server.route("/api/courses/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return CourseApi::getById(id); });

        m_server.route("/api/courses/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return CourseApi::update(id, req); });

        m_server.route("/api/courses/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return CourseApi::remove(id); });

        m_server.route("/api/roles/statistics", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return RoleApi::getStatistics(req); });

        m_server.route("/api/roles", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return RoleApi::getAll(req); });

        m_server.route("/api/roles", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return RoleApi::create(req); });

        m_server.route("/api/roles/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return RoleApi::getById(id); });

        m_server.route("/api/roles/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return RoleApi::update(id, req); });

        m_server.route("/api/roles/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return RoleApi::remove(id); });

        m_server.route("/api/achievements/statistics", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AchievementApi::getStatistics(req); });

        m_server.route("/api/achievements", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AchievementApi::getAll(req); });

        m_server.route("/api/achievements", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AchievementApi::create(req); });

        m_server.route("/api/achievements/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return AchievementApi::getById(id); });

        m_server.route("/api/achievements/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return AchievementApi::update(id, req); });

        m_server.route("/api/achievements/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return AchievementApi::remove(id); });

        m_server.route("/api/experiences/statistics", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return ExperienceApi::getStatistics(req); });

        m_server.route("/api/experiences", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return ExperienceApi::getAll(req); });

        m_server.route("/api/experiences", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return ExperienceApi::create(req); });

        m_server.route("/api/experiences/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return ExperienceApi::getById(id); });

        m_server.route("/api/experiences/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return ExperienceApi::update(id, req); });

        m_server.route("/api/experiences/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return ExperienceApi::remove(id); });

        m_server.route("/api/activities", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return ActivityApi::getAll(req); });

        m_server.route("/api/activities", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return ActivityApi::create(req); });

        m_server.route("/api/activities/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return ActivityApi::getById(id); });

        m_server.route("/api/activities/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return ActivityApi::update(id, req); });

        m_server.route("/api/activities/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return ActivityApi::remove(id); });

        m_server.route("/api/goals/statistics", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return GoalApi::getStatistics(req); });

        m_server.route("/api/goals", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return GoalApi::getAll(req); });

        m_server.route("/api/goals", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return GoalApi::create(req); });

        m_server.route("/api/goals/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return GoalApi::getById(id); });

        m_server.route("/api/goals/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return GoalApi::update(id, req); });

        m_server.route("/api/goals/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return GoalApi::remove(id); });

        m_server.route("/api/jobs", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return JobApi::getAll(req); });

        m_server.route("/api/jobs", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return JobApi::create(req); });

        m_server.route("/api/jobs/import", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return JobApi::importJobs(req); });

        m_server.route("/api/jobs/<arg>", QHttpServerRequest::Method::Get,
            [](int id, const QHttpServerRequest&) { return JobApi::getById(id); });

        m_server.route("/api/jobs/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return JobApi::update(id, req); });

        m_server.route("/api/jobs/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return JobApi::remove(id); });

        m_server.route("/api/jobs/<arg>/requirements/<arg>/toggle", QHttpServerRequest::Method::Post,
            [](int jobId, int reqIndex, const QHttpServerRequest&) { return JobApi::toggleRequirement(jobId, reqIndex); });

        m_server.route("/api/dashboard/overview", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return DashboardApi::getOverview(req); });

        m_server.route("/api/dashboard/gpa-trend", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return DashboardApi::getGpaTrend(req); });

        m_server.route("/api/dashboard/recommendations", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return DashboardApi::getRecommendations(req); });

        m_server.route("/api/analytics/semester-comparison", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AnalyticsApi::getSemesterComparison(req); });

        m_server.route("/api/analytics/peer-comparison", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AnalyticsApi::getPeerComparison(req); });

        m_server.route("/api/analytics/report", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AnalyticsApi::getReport(req); });

        m_server.route("/api/analytics/peers", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AnalyticsApi::getPeers(req); });

        m_server.route("/api/analytics/peers", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AnalyticsApi::createPeer(req); });

        m_server.route("/api/analytics/peers/<arg>", QHttpServerRequest::Method::Put,
            [](int id, const QHttpServerRequest& req) { return AnalyticsApi::updatePeer(id, req); });

        m_server.route("/api/analytics/peers/<arg>", QHttpServerRequest::Method::Delete,
            [](int id, const QHttpServerRequest&) { return AnalyticsApi::deletePeer(id); });

        m_server.route("/api/timeline", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return TimelineApi::getAll(req); });

        m_server.route("/api/imports/<arg>", QHttpServerRequest::Method::Post,
            [](const QString& entity, const QHttpServerRequest& req) { return ImportApi::importEntity(entity, req); });

        m_server.route("/api/resume/generate", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return ResumeApi::generate(req); });

        m_server.route("/api/resume/export/json", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return ResumeApi::exportJson(req); });

        m_server.route("/api/resume/export/html", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return ResumeApi::exportHtml(req); });

        m_server.route("/api/ai/analyze", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AiApi::analyze(req); });

        m_server.route("/api/ai/status", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AiApi::checkStatus(req); });

        m_server.route("/api/ai/chat", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AiApi::chat(req); });

        m_server.route("/api/auth/register", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AuthApi::registerUser(req); });

        m_server.route("/api/auth/login", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AuthApi::login(req); });

        m_server.route("/api/auth/me", QHttpServerRequest::Method::Get,
            [](const QHttpServerRequest& req) { return AuthApi::getMe(req); });

        m_server.route("/api/auth/me", QHttpServerRequest::Method::Put,
            [](const QHttpServerRequest& req) { return AuthApi::updateMe(req); });

        m_server.route("/api/auth/change-password", QHttpServerRequest::Method::Post,
            [](const QHttpServerRequest& req) { return AuthApi::changePassword(req); });

        Logger::info("所有API路由已注册");
    }

    QHttpServer m_server;
    QTcpServer m_tcpServer;
};

#endif // HTTPSERVER_H
