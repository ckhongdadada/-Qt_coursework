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

private:
    static QJsonObject callAiServer(const QString& endpoint, const QJsonObject& data);
    static QJsonObject callBackendApi(const QString& endpoint, const QJsonObject& data = QJsonObject());
    static QJsonObject analyzeWithAi(const QJsonObject& data);
    static QJsonObject chatWithAi(const QJsonObject& data);

    static QString m_aiServerUrl;
    static QString m_backendUrl;
    static bool m_aiServerChecked;
    static bool m_aiServerAvailable;
    static qint64 m_lastAiServerCheckMs;
};

#endif // AISERVICE_H
