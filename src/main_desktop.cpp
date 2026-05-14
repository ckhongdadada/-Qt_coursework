#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QStyleFactory>
#include "client/MainWindow.h"
#include "dao/DaoBase.h"
#include "util/Logger.h"

static bool initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    
    QString dbPath = QDir::currentPath() + "/pdp.db";
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        Logger::error("数据库连接失败: " + db.lastError().text());
        return false;
    }
    
    Logger::info("数据库连接成功: " + dbPath);
    
    QFile schemaFile(":/resources/schema.sql");
    if (!schemaFile.exists()) {
        schemaFile.setFileName(QDir::currentPath() + "/resources/schema.sql");
    }
    if (!schemaFile.exists()) {
        schemaFile.setFileName(QApplication::applicationDirPath() + "/resources/schema.sql");
    }
    
    if (schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString schema = schemaFile.readAll();
        schemaFile.close();
        
        QStringList statements = schema.split(";", Qt::SkipEmptyParts);
        QSqlQuery query(db);
        
        for (const QString& stmt : statements) {
            QString trimmed = stmt.trimmed();
            if (!trimmed.isEmpty()) {
                if (!query.exec(trimmed)) {
                    Logger::warning("SQL执行警告: " + query.lastError().text());
                }
            }
        }
        
        Logger::info("数据库Schema初始化完成");
    } else {
        Logger::warning("未找到schema.sql文件，跳过初始化");
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("学业发展规划系统");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("PDP Team");
    
#ifdef Q_OS_WIN
    app.setStyle(QStyleFactory::create("Fusion"));
#endif
    
    Logger::info("========================================");
    Logger::info("  学业发展规划系统 - 桌面版 v2.0.0");
    Logger::info("========================================");
    Logger::info("Qt版本: " + QString(qVersion()));
    Logger::info("应用路径: " + QApplication::applicationDirPath());
    Logger::info("工作目录: " + QDir::currentPath());
    
    if (!initializeDatabase()) {
        QMessageBox::critical(nullptr, "启动错误", 
            "数据库初始化失败，程序无法启动。\n\n"
            "请检查是否有足够的文件权限。");
        return 1;
    }
    
    MainWindow mainWindow;
    mainWindow.show();
    
    Logger::info("主窗口已显示，等待用户操作");
    
    int result = app.exec();
    
    Logger::info("应用程序退出，返回码: " + QString::number(result));
    
    return result;
}
