#ifndef AISERVICE_H
#define AISERVICE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QNetworkAccessManager>

class AiService {
public:
    static QJsonObject analyze(const QJsonObject& data);
    static QJsonObject checkStatus();
    static QJsonObject chat(const QJsonObject& data);
    
    static void setAiServerUrl(const QString& url);
    static QString aiServerUrl();
    static bool isAiServerAvailable(bool forceRefresh = false);
    static void resetAiServerCheck();
    
    static void setBackendUrl(const QString& url);
    static QString backendUrl();

    static QJsonObject healthCheck();
    static QString currentMode();
    static bool isRuleMode();
    static bool isRemoteMode();

public:
    static QJsonObject analyzeLocal(const QJsonObject& data);
    static QJsonObject chatLocal(const QJsonObject& data);

private:
    static QJsonObject callAiServer(const QString& endpoint, const QJsonObject& data);
    static QJsonObject callBackendApi(const QString& endpoint, const QJsonObject& data = QJsonObject());
    static QJsonObject analyzeWithAi(const QJsonObject& data);
    static QJsonObject chatWithAi(const QJsonObject& data);
    static QJsonObject generateLocalReply(const QString& message);
    static QJsonObject generateLocalAnalysis(const QString& type);
    static QJsonObject buildCourseAdvice();
    static QJsonObject buildGoalAdvice();
    static QJsonObject buildExperienceAdvice();
    static QJsonObject buildResumeAdvice();
    static QJsonObject buildAchievementAdvice();
    static QJsonObject buildComprehensiveAdvice();
    static QJsonObject buildDefaultReply();

    static QString m_aiServerUrl;
    static QString m_backendUrl;
    static bool m_aiServerChecked;
    static bool m_aiServerAvailable;
    static qint64 m_lastAiServerCheckMs;
};

#endif // AISERVICE_H
