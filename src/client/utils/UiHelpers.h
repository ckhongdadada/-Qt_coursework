#pragma once

#include <QListWidget>
#include <QString>

namespace UiHelpers {

QString zh(const char* text);
QString safeText(const QString& value, const QString& fallback = QStringLiteral("未填写"));
QString shortBody(const QString& value, const QString& fallback);
QString bullet(const QString& value);
QString joinDateRange(const QString& startDate, const QString& endDate, bool active, const QString& activeLabel);
QString htmlToPlainSummary(const QString& html);
void setupEmptyState(QListWidget* list, const QString& hint = QStringLiteral("这里空空如也，快去添加第一笔记录吧~"));

} // namespace UiHelpers
