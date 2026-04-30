#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QDateTime>

class Logger {
public:
    static void info(const QString& message) {
        qInfo() << QString("[%1] [INFO] %2").arg(currentTime()).arg(message);
    }

    static void warning(const QString& message) {
        qWarning() << QString("[%1] [WARN] %2").arg(currentTime()).arg(message);
    }

    static void error(const QString& message) {
        qCritical() << QString("[%1] [ERROR] %2").arg(currentTime()).arg(message);
    }

    static void debug(const QString& message) {
        qDebug() << QString("[%1] [DEBUG] %2").arg(currentTime()).arg(message);
    }

private:
    static QString currentTime() {
        return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    }
};

#endif // LOGGER_H