#pragma once

#include <QObject>
#include <QLabel>
#include <QProgressBar>
#include <QSystemTrayIcon>

class HttpServerThread;

class BackendRuntimeController : public QObject {
    Q_OBJECT

public:
    explicit BackendRuntimeController(QObject* parent = nullptr);

    void bindWidgets(QLabel* statusLabel, QProgressBar* progressBar, QSystemTrayIcon* trayIcon);

    void startBackendServer();
    void stopBackendServer();
    void checkFrontendExists();
    void openBrowser();
    void insertSampleDataIfNeeded();

    bool isServerReady() const;
    QString serverUrl() const;
    QString frontendPath() const;

signals:
    void serverStarted();
    void serverError(const QString& error);
    void backendReady();

private slots:
    void onBackendStarted();
    void onBackendError(const QString& error);

private:
    void updateBackendBadge(bool ready, const QString& detail = QString());

    HttpServerThread* m_serverThread = nullptr;
    QLabel* m_statusLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QSystemTrayIcon* m_trayIcon = nullptr;

    bool m_serverReady = false;
    QString m_frontendPath;
    QString m_serverUrl;
};
