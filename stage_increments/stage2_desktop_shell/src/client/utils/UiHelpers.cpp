#include "client/utils/UiHelpers.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QListWidgetItem>

namespace UiHelpers {

QString zh(const char* text)
{
    return QString::fromUtf8(text);
}

QString safeText(const QString& value, const QString& fallback)
{
    const QString trimmed = value.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed;
}

QString shortBody(const QString& value, const QString& fallback)
{
    const QString text = value.simplified();
    return text.isEmpty() ? fallback : text;
}

QString bullet(const QString& value)
{
    return QStringLiteral("• %1").arg(value);
}

QString joinDateRange(const QString& startDate, const QString& endDate, bool active, const QString& activeLabel)
{
    if (!startDate.isEmpty() && !endDate.isEmpty()) {
        return QStringLiteral("%1 - %2").arg(startDate, endDate);
    }
    if (!startDate.isEmpty()) {
        return active ? QStringLiteral("%1 - %2").arg(startDate, activeLabel) : startDate;
    }
    if (!endDate.isEmpty()) {
        return endDate;
    }
    return QStringLiteral("时间未填写");
}

QString htmlToPlainSummary(const QString& html)
{
    QString text = html;
    text.replace(QStringLiteral("<h1>"), QString()).replace(QStringLiteral("</h1>"), QStringLiteral("\n"));
    text.replace(QStringLiteral("<h2>"), QStringLiteral("\n")).replace(QStringLiteral("</h2>"), QStringLiteral("\n"));
    text.replace(QStringLiteral("<p>"), QString()).replace(QStringLiteral("</p>"), QStringLiteral("\n"));
    text.replace(QStringLiteral("<strong>"), QString()).replace(QStringLiteral("</strong>"), QString());
    text.replace(QStringLiteral("<div class='item'>"), QStringLiteral("\n")).replace(QStringLiteral("</div>"), QStringLiteral("\n"));
    return text;
}

void setupEmptyState(QListWidget* list, const QString& hint)
{
    if (!list) {
        return;
    }

    auto* item = new QListWidgetItem(QStringLiteral("\n\n\n📭\n%1\n\n\n").arg(hint));
    item->setTextAlignment(Qt::AlignCenter);
    QFont font = item->font();
    font.setPointSize(12);
    item->setFont(font);
    item->setForeground(QBrush(QColor(QStringLiteral("#a8a096"))));
    item->setFlags(Qt::NoItemFlags);
    list->addItem(item);
}

} // namespace UiHelpers
