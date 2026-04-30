#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QString>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDateTime>
#include "dao/UserDao.h"
#include "model/User.h"

class AuthService {
public:
    static QString hashPassword(const QString& password) {
        QByteArray hash = QCryptographicHash::hash(
            (password + "pdp_salt_2024").toUtf8(),
            QCryptographicHash::Sha256
        );
        return QString(hash.toHex());
    }

    static bool verifyPassword(const QString& password, const QString& hash) {
        return hashPassword(password) == hash;
    }

    static QJsonObject registerUser(const QString& username, const QString& email, const QString& password, const QString& displayName = "") {
        UserDao userDao;

        User existing = userDao.getByUsername(username);
        if (existing.id > 0) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "用户名已存在";
            err["field"] = "username";
            return err;
        }

        existing = userDao.getByEmail(email);
        if (existing.id > 0) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "邮箱已被注册";
            err["field"] = "email";
            return err;
        }

        if (password.length() < 8) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "密码至少8位";
            err["field"] = "password";
            return err;
        }

        User user;
        user.username = username;
        user.email = email;
        user.passwordHash = hashPassword(password);
        user.displayName = displayName.isEmpty() ? username : displayName;
        user.role = "user";
        user.isActive = true;

        userDao.create(user);
        int lastId = userDao.getLastInsertId();
        if (lastId > 0) user = userDao.getById(lastId);

        QString token = generateToken(user.id, user.username, user.role);
        QJsonObject result;
        result["error"] = false;
        result["user"] = user.toDict();
        result["token"] = token;
        return result;
    }

    static QJsonObject login(const QString& username, const QString& password) {
        UserDao userDao;
        User user = userDao.getByUsername(username);
        if (user.id == 0) {
            user = userDao.getByEmail(username);
        }
        if (user.id == 0 || !verifyPassword(password, user.passwordHash)) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "用户名或密码错误";
            return err;
        }
        if (!user.isActive) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "账户已被禁用";
            return err;
        }

        userDao.updateLastLogin(user.id);
        user = userDao.getById(user.id);

        QString token = generateToken(user.id, user.username, user.role);
        QJsonObject result;
        result["error"] = false;
        result["user"] = user.toDict();
        result["token"] = token;
        return result;
    }

    static QJsonObject getMe(int userId) {
        UserDao userDao;
        User user = userDao.getById(userId);
        if (user.id == 0) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "用户不存在";
            return err;
        }
        QJsonObject result;
        result["error"] = false;
        result["user"] = user.toDict();
        return result;
    }

    static QJsonObject changePassword(int userId, const QString& oldPassword, const QString& newPassword) {
        UserDao userDao;
        User user = userDao.getById(userId);
        if (user.id == 0) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "用户不存在";
            return err;
        }
        if (!verifyPassword(oldPassword, user.passwordHash)) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "原密码错误";
            return err;
        }
        if (newPassword.length() < 8) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "新密码至少8位";
            return err;
        }
        user.passwordHash = hashPassword(newPassword);
        userDao.update(user);
        QJsonObject result;
        result["error"] = false;
        result["message"] = "密码修改成功";
        return result;
    }

    static QJsonObject updateProfile(int userId, const QJsonObject& data) {
        UserDao userDao;
        User user = userDao.getById(userId);
        if (user.id == 0) {
            QJsonObject err;
            err["error"] = true;
            err["message"] = "用户不存在";
            return err;
        }
        if (data.contains("displayName")) user.displayName = data["displayName"].toString();
        if (data.contains("email")) {
            QString newEmail = data["email"].toString();
            User existing = userDao.getByEmail(newEmail);
            if (existing.id > 0 && existing.id != userId) {
                QJsonObject err;
                err["error"] = true;
                err["message"] = "邮箱已被其他用户使用";
                err["field"] = "email";
                return err;
            }
            user.email = newEmail;
        }
        userDao.update(user);
        user = userDao.getById(userId);
        QJsonObject result;
        result["error"] = false;
        result["user"] = user.toDict();
        return result;
    }

private:
    static QString generateToken(int userId, const QString& username, const QString& role) {
        QString data = QString("%1:%2:%3:%4")
            .arg(userId)
            .arg(username)
            .arg(role)
            .arg(QDateTime::currentSecsSinceEpoch() + 72 * 3600);
        QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
        return QString("%1.%2.%3")
            .arg(QString(QByteArray::number(userId).toBase64()))
            .arg(QString(QCryptographicHash::hash(username.toUtf8(), QCryptographicHash::Md5).toHex()).left(12))
            .arg(QString(hash.toHex()).left(24));
    }
};

#endif // AUTHSERVICE_H