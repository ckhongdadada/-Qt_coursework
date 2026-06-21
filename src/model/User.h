#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

class User {
public:
    int id = 0;
    QString username;
    QString email;
    QString passwordHash;
    QString displayName;
    QString role = "user";
    bool isActive = true;
    QString lastLoginAt;
    QDateTime createdAt;
    QDateTime updatedAt;

    User() = default;

    QJsonObject toDict() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["username"] = username;
        obj["email"] = email;
        obj["displayName"] = displayName;
        obj["role"] = role;
        obj["isActive"] = isActive;
        obj["lastLoginAt"] = lastLoginAt;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
        return obj;
    }

    static User fromDict(const QJsonObject& obj) {
        User u;
        u.id = obj["id"].toInt();
        u.username = obj["username"].toString();
        u.email = obj["email"].toString();
        u.passwordHash = obj["passwordHash"].toString();
        u.displayName = obj["displayName"].toString();
        u.role = obj["role"].toString();
        u.isActive = obj["isActive"].toBool(true);
        u.lastLoginAt = obj["lastLoginAt"].toString();
        u.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        u.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
        return u;
    }
};

#endif // USER_H