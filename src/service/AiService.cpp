#include "service/AiService.h"
#include "service/CourseService.h"
#include "service/GoalService.h"
#include "dao/AchievementDao.h"
#include "dao/ExperienceDao.h"
#include "dao/RoleDao.h"
#include "util/Logger.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QDateTime>

QString AiService::m_aiServerUrl = "http://localhost:8001";
QString AiService::m_backendUrl = "http://127.0.0.1:8080";
bool AiService::m_aiServerChecked = false;
bool AiService::m_aiServerAvailable = false;
qint64 AiService::m_lastAiServerCheckMs = 0;

namespace {
constexpr int kAiHealthTimeoutMs = 8000;
constexpr int kAiGenerationTimeoutMs = 180000;
constexpr int kBackendApiTimeoutMs = 10000;
constexpr qint64 kAiAvailableCacheMs = 15000;
constexpr qint64 kAiUnavailableRetryMs = 3000;

bool hasAiError(const QJsonObject& result)
{
    if (!result.contains("error")) {
        return false;
    }
    const QJsonValue errorValue = result.value("error");
    if (errorValue.isBool()) {
        return errorValue.toBool();
    }
    return !errorValue.toString().trimmed().isEmpty();
}
}

void AiService::setAiServerUrl(const QString& url) {
    m_aiServerUrl = url;
    resetAiServerCheck();
}

QString AiService::aiServerUrl() {
    return m_aiServerUrl;
}

void AiService::setBackendUrl(const QString& url) {
    m_backendUrl = url;
}

QString AiService::backendUrl() {
    return m_backendUrl;
}

void AiService::resetAiServerCheck() {
    m_aiServerChecked = false;
    m_aiServerAvailable = false;
    m_lastAiServerCheckMs = 0;
}

bool AiService::isAiServerAvailable(bool forceRefresh) {
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const bool cacheActive = m_aiServerChecked
        && !forceRefresh
        && ((m_aiServerAvailable && (nowMs - m_lastAiServerCheckMs) < kAiAvailableCacheMs)
            || (!m_aiServerAvailable && (nowMs - m_lastAiServerCheckMs) < kAiUnavailableRetryMs));
    if (cacheActive) {
        return m_aiServerAvailable;
    }
    
    QNetworkAccessManager manager;
    QUrl url(m_aiServerUrl + "/health");
    QNetworkRequest request(url);
    request.setTransferTimeout(kAiHealthTimeoutMs);
    
    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    
    QTimer::singleShot(kAiHealthTimeoutMs, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    m_aiServerChecked = true;
    m_aiServerAvailable = false;
    if (reply->error() == QNetworkReply::NoError) {
        const QJsonObject health = QJsonDocument::fromJson(reply->readAll()).object();
        m_aiServerAvailable = health.value("model_loaded").toBool(false);
        if (!m_aiServerAvailable && health.value("model_loading").toBool(false)) {
            Logger::info("AI服务已启动，模型仍在加载: " + m_aiServerUrl);
        }
    }
    m_lastAiServerCheckMs = nowMs;
    
    if (m_aiServerAvailable) {
        Logger::info("AI服务可用: " + m_aiServerUrl);
    } else {
        Logger::warning("AI服务不可用，请启动本地大模型服务: " + reply->errorString());
    }
    
    reply->deleteLater();
    return m_aiServerAvailable;
}

QJsonObject AiService::callAiServer(const QString& endpoint, const QJsonObject& data) {
    QNetworkAccessManager manager;
    QUrl url(m_aiServerUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(kAiGenerationTimeoutMs);
    
    QJsonDocument doc(data);
    QEventLoop loop;
    QNetworkReply* reply = manager.post(request, doc.toJson());
    
    QTimer::singleShot(kAiGenerationTimeoutMs, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QJsonObject result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        result = QJsonDocument::fromJson(responseData).object();
        result["aiPowered"] = true;
    } else {
        result["error"] = reply->errorString();
        result["aiPowered"] = false;
    }
    
    reply->deleteLater();
    return result;
}

QJsonObject AiService::callBackendApi(const QString& endpoint, const QJsonObject& data) {
    QNetworkAccessManager manager;
    QUrl url(m_backendUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(kAiGenerationTimeoutMs);
    
    QEventLoop loop;
    QNetworkReply* reply;
    
    if (data.isEmpty()) {
        reply = manager.get(request);
    } else {
        QJsonDocument doc(data);
        reply = manager.post(request, doc.toJson());
    }
    
    QTimer::singleShot(kAiGenerationTimeoutMs, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QJsonObject result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonObject response = QJsonDocument::fromJson(responseData).object();
        if (response.contains("data")) {
            result = response["data"].toObject();
        } else {
            result = response;
        }
    } else {
        result["error"] = reply->errorString();
    }
    
    reply->deleteLater();
    return result;
}

QJsonObject AiService::analyze(const QJsonObject& data) {
    if (isAiServerAvailable()) {
        QJsonObject aiResult = analyzeWithAi(data);
        if (!hasAiError(aiResult)) {
            aiResult["aiPowered"] = true;
            return aiResult;
        }
        resetAiServerCheck();
        return aiResult;
    }

    QJsonObject backendResult = callBackendApi("/api/ai/analyze", data);
    if (!backendResult.contains("error")) {
        backendResult["aiPowered"] = true;
        return backendResult;
    }

    QJsonObject result;
    result["error"] = backendResult["error"].toString(QStringLiteral("大模型服务不可用"));
    result["reply"] = QStringLiteral("大模型服务当前不可用，请先启动本地模型服务后重试。");
    result["type"] = data["type"].toString("general");
    result["aiPowered"] = false;
    return result;
}

QJsonObject AiService::analyzeWithAi(const QJsonObject& data) {
    return callAiServer("/v1/analyze", data);
}

QJsonObject AiService::checkStatus() {
    QJsonObject status;
    status["available"] = false;
    
    QJsonObject backendStatus = callBackendApi("/api/ai/status");
    if (!backendStatus.contains("error")) {
        const bool available = backendStatus["available"].toBool(false);
        QString mode = backendStatus["mode"].toString(available ? "model" : "unavailable");
        if (mode.contains("rule", Qt::CaseInsensitive)) {
            mode = available ? "model" : "unavailable";
        }
        status["mode"] = mode;
        status["model"] = backendStatus["model"].toString("local-model");
        status["available"] = available;
        if (backendStatus.contains("aiServer")) {
            status["aiServer"] = backendStatus["aiServer"];
        }
    } else {
        const bool available = isAiServerAvailable();
        status["mode"] = available ? "remote-model" : "unavailable";
        status["model"] = "local-model";
        status["available"] = available;
    }
    
    status["version"] = "2.0.0";
    return status;
}

QJsonObject AiService::chat(const QJsonObject& data) {
    if (isAiServerAvailable()) {
        QJsonObject aiResult = chatWithAi(data);
        if (!hasAiError(aiResult)) {
            aiResult["aiPowered"] = true;
            return aiResult;
        }
        resetAiServerCheck();
        return aiResult;
    }

    QJsonObject backendResult = callBackendApi("/api/ai/chat", data);
    if (!backendResult.contains("error")) {
        backendResult["aiPowered"] = true;
        return backendResult;
    }

    QJsonObject result;
    result["error"] = backendResult["error"].toString(QStringLiteral("大模型服务不可用"));
    result["reply"] = QStringLiteral("大模型服务当前不可用，请先启动本地模型服务后重试。");
    result["type"] = "chat";
    result["aiPowered"] = false;
    return result;
}

QJsonObject AiService::chatWithAi(const QJsonObject& data) {
    QString message = data["message"].toString();
    
    QJsonObject requestData;
    QJsonArray messages;
    messages.append(QJsonObject{
        {"role", "user"},
        {"content", message}
    });
    requestData["messages"] = messages;
    requestData["max_tokens"] = 256;
    requestData["temperature"] = 0.3;
    
    QJsonObject response = callAiServer("/v1/chat/completions", requestData);
    
    QJsonObject result;
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            result["reply"] = choices[0].toObject()["message"].toObject()["content"].toString();
            result["aiPowered"] = true;
            return result;
        }
    }
    
    result["error"] = response.contains("error")
        ? response["error"].toString()
        : QStringLiteral("未知错误");
    result["aiPowered"] = false;
    return result;
}

QJsonObject AiService::healthCheck()
{
    QJsonObject result;
    result["aiServerAvailable"] = isAiServerAvailable();
    result["aiServerUrl"] = m_aiServerUrl;
    result["backendUrl"] = m_backendUrl;
    result["mode"] = currentMode();
    return result;
}

QString AiService::currentMode()
{
    return isAiServerAvailable() ? "remote-model" : "unavailable";
}

bool AiService::isRuleMode()
{
    return false;
}

bool AiService::isRemoteMode()
{
    return isAiServerAvailable();
}
