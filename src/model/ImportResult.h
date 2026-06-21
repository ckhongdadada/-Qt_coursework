#ifndef IMPORTRESULT_H
#define IMPORTRESULT_H

#include "ImportErrorItem.h"
#include <QList>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class ImportResult {
public:
    int totalRows = 0;
    int successCount = 0;
    int failCount = 0;
    int skipCount = 0;
    QString entityType;
    QList<ImportErrorItem> errors;

    ImportResult() = default;

    bool isSuccess() const { return failCount == 0 && errors.isEmpty(); }
    bool hasWarnings() const;
    bool hasErrors() const;

    void addError(int row, const QString& field, const QString& message);
    void addWarning(int row, const QString& field, const QString& message);

    QJsonObject toJson() const;
    static ImportResult fromJson(const QJsonObject& obj);

    QString summary() const;
};

inline bool ImportResult::hasWarnings() const {
    for (const auto& e : errors) { if (e.isWarning()) return true; }
    return false;
}

inline bool ImportResult::hasErrors() const {
    for (const auto& e : errors) { if (e.isError()) return true; }
    return false;
}

inline void ImportResult::addError(int row, const QString& field, const QString& message) {
    failCount++;
    errors.append(ImportErrorItem(row, field, message, "error"));
}

inline void ImportResult::addWarning(int row, const QString& field, const QString& message) {
    skipCount++;
    errors.append(ImportErrorItem(row, field, message, "warning"));
}

inline QJsonObject ImportResult::toJson() const {
    QJsonObject obj;
    obj["totalRows"] = totalRows;
    obj["successCount"] = successCount;
    obj["failCount"] = failCount;
    obj["skipCount"] = skipCount;
    obj["entityType"] = entityType;
    QJsonArray errArr;
    for (const auto& e : errors) {
        QJsonObject errObj;
        errObj["row"] = e.row;
        errObj["field"] = e.field;
        errObj["message"] = e.message;
        errObj["severity"] = e.severity;
        errArr.append(errObj);
    }
    obj["errors"] = errArr;
    return obj;
}

inline ImportResult ImportResult::fromJson(const QJsonObject& obj) {
    ImportResult r;
    r.totalRows = obj["totalRows"].toInt();
    r.successCount = obj["successCount"].toInt();
    r.failCount = obj["failCount"].toInt();
    r.skipCount = obj["skipCount"].toInt();
    r.entityType = obj["entityType"].toString();
    QJsonArray errArr = obj["errors"].toArray();
    for (const auto& v : errArr) {
        QJsonObject e = v.toObject();
        r.errors.append(ImportErrorItem(e["row"].toInt(), e["field"].toString(), e["message"].toString(), e["severity"].toString()));
    }
    return r;
}

inline QString ImportResult::summary() const {
    return QString("导入完成：总计 %1 行，成功 %2，失败 %3，跳过 %4。")
        .arg(totalRows).arg(successCount).arg(failCount).arg(skipCount);
}

#endif
