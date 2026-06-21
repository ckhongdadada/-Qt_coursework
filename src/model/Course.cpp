#include "Course.h"

static const QMap<QString, QMap<QPair<int,int>,double>> SCALE_TABLES = {
    {"standard", {{{90,100},4.0},{{85,89},3.7},{{82,84},3.3},{{78,81},3.0},{{75,77},2.7},{{72,74},2.3},{{68,71},2.0},{{64,67},1.5},{{60,63},1.0},{{0,59},0.0}}},
    {"4.3", {{{95,100},4.3},{{90,94},4.0},{{85,89},3.7},{{82,84},3.3},{{78,81},3.0},{{75,77},2.7},{{72,74},2.3},{{68,71},2.0},{{64,67},1.5},{{60,63},1.0},{{0,59},0.0}}},
    {"5.0", {{{90,100},5.0},{{85,89},4.5},{{82,84},4.0},{{78,81},3.5},{{75,77},3.0},{{72,74},2.5},{{68,71},2.0},{{64,67},1.5},{{60,63},1.0},{{0,59},0.0}}},
};

double Course::calculateGradePoint(double score, const QString& scale) {
    QString s = scale.trimmed().toLower();
    if (s.isEmpty()) s = "standard";
    if (!SCALE_TABLES.contains(s)) s = "standard";
    const auto& table = SCALE_TABLES[s];
    for (auto it = table.begin(); it != table.end(); ++it) {
        if (score >= it.key().first && score <= it.key().second) {
            return it.value();
        }
    }
    return 0.0;
}