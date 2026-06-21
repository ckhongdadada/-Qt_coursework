#ifndef DAOBASE_H
#define DAOBASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "util/Logger.h"

class DaoBase {
public:
    DaoBase() : m_db(QSqlDatabase::database()) {
        if (!m_db.isOpen()) {
            Logger::warning("数据库未打开，尝试打开...");
            m_db.open();
        }
    }

    virtual ~DaoBase() = default;

protected:
    QSqlDatabase m_db;

    bool isOpen() const {
        return m_db.isOpen();
    }
};

#endif // DAOBASE_H