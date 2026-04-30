#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "server/HttpServer.h"
#include "util/Logger.h"

bool initializeDatabase() {
    Logger::info("初始化数据库...");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("pdp.db");

    if (!db.open()) {
        Logger::error(QString("数据库连接失败: %1").arg(db.lastError().text()));
        return false;
    }

    QStringList searchPaths = {
        "resources/schema.sql",
        QCoreApplication::applicationDirPath() + "/resources/schema.sql",
        QDir::currentPath() + "/resources/schema.sql"
    };

    QString schemaPath;
    for (const QString& path : searchPaths) {
        if (QFile::exists(path)) {
            schemaPath = path;
            break;
        }
    }

    if (!schemaPath.isEmpty()) {
        QFile scriptFile(schemaPath);
        if (scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&scriptFile);
            QString sql = stream.readAll();

            QStringList statements = sql.split(';', Qt::SkipEmptyParts);
            QSqlQuery query(db);

            for (const QString& statement : statements) {
                QString trimmed = statement.trimmed();
                if (!trimmed.isEmpty()) {
                    if (!query.exec(trimmed)) {
                        if (!query.lastError().text().contains("UNIQUE constraint failed")) {
                            Logger::warning(QString("SQL执行警告: %1").arg(query.lastError().text()));
                        }
                    }
                }
            }

            scriptFile.close();
            Logger::info("数据库初始化完成");
        }
    } else {
        Logger::warning("未找到schema.sql，使用现有数据库");
    }

    return true;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    Logger::info("========== 学业发展规划系统启动 ==========");

    if (!initializeDatabase()) {
        Logger::error("数据库初始化失败，退出程序");
        return 1;
    }

    HttpServer server;
    if (!server.start(8080)) {
        Logger::error("服务器启动失败，退出程序");
        return 1;
    }

    Logger::info("系统启动完成，按Ctrl+C退出");

    return app.exec();
}