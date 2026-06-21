#ifndef IMPORTERRORITEM_H
#define IMPORTERRORITEM_H

#include <QString>

class ImportErrorItem {
public:
    int row = 0;
    QString field;
    QString message;
    QString severity = "error";

    ImportErrorItem() = default;
    ImportErrorItem(int r, const QString& f, const QString& msg, const QString& sev = "error")
        : row(r), field(f), message(msg), severity(sev) {}

    bool isError() const { return severity == "error"; }
    bool isWarning() const { return severity == "warning"; }
};

#endif
