#ifndef USERDAO_H
#define USERDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/User.h"

class UserDao : public DaoBase {
public:
    QList<User> getAll() const {
        QList<User> list;
        if (!isOpen()) return list;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM users ORDER BY id");
        while (query.next()) list.append(mapFromQuery(query));
        return list;
    }

    User getById(int id) const {
        User u;
        if (!isOpen()) return u;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM users WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) u = mapFromQuery(query);
        return u;
    }

    User getByUsername(const QString& username) const {
        User u;
        if (!isOpen()) return u;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM users WHERE username = :username");
        query.bindValue(":username", username);
        query.exec();
        if (query.next()) u = mapFromQuery(query);
        return u;
    }

    User getByEmail(const QString& email) const {
        User u;
        if (!isOpen()) return u;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM users WHERE email = :email");
        query.bindValue(":email", email);
        query.exec();
        if (query.next()) u = mapFromQuery(query);
        return u;
    }

    bool create(const User& u) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO users (username, email, password_hash, display_name, role, is_active) "
            "VALUES (:username, :email, :password_hash, :display_name, :role, :is_active)"
        );
        query.bindValue(":username", u.username);
        query.bindValue(":email", u.email);
        query.bindValue(":password_hash", u.passwordHash);
        query.bindValue(":display_name", u.displayName);
        query.bindValue(":role", u.role);
        query.bindValue(":is_active", u.isActive ? 1 : 0);
        return query.exec();
    }

    bool update(const User& u) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE users SET username = :username, email = :email, password_hash = :password_hash, "
            "display_name = :display_name, role = :role, is_active = :is_active, "
            "last_login_at = :last_login_at, updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", u.id);
        query.bindValue(":username", u.username);
        query.bindValue(":email", u.email);
        query.bindValue(":password_hash", u.passwordHash);
        query.bindValue(":display_name", u.displayName);
        query.bindValue(":role", u.role);
        query.bindValue(":is_active", u.isActive ? 1 : 0);
        query.bindValue(":last_login_at", u.lastLoginAt);
        return query.exec();
    }

    bool updateLastLogin(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("UPDATE users SET last_login_at = CURRENT_TIMESTAMP, updated_at = CURRENT_TIMESTAMP WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM users WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getLastInsertId() const {
        QSqlQuery query(m_db);
        query.exec("SELECT last_insert_rowid()");
        if (query.next()) return query.value(0).toInt();
        return -1;
    }

private:
    static User mapFromQuery(const QSqlQuery& query) {
        User u;
        u.id = query.value("id").toInt();
        u.username = query.value("username").toString();
        u.email = query.value("email").toString();
        u.passwordHash = query.value("password_hash").toString();
        u.displayName = query.value("display_name").toString();
        u.role = query.value("role").toString();
        u.isActive = query.value("is_active").toBool();
        u.lastLoginAt = query.value("last_login_at").toString();
        u.createdAt = query.value("created_at").toDateTime();
        u.updatedAt = query.value("updated_at").toDateTime();
        return u;
    }
};

#endif // USERDAO_H