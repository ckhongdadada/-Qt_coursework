#pragma once

#include <QFrame>
#include <QLabel>
#include <QGridLayout>
#include <QVector>

struct MetricEntry {
    QString label;
    QLabel** valueTarget = nullptr;
    QString hint;
};

class MetricGridWidget : public QFrame {
    Q_OBJECT

public:
    explicit MetricGridWidget(QWidget* parent = nullptr);

    void setMetrics(const QVector<MetricEntry>& entries, int columns = 4);
    void setValue(int index, const QString& text);

private:
    QVector<QLabel*> m_valueLabels;
};
