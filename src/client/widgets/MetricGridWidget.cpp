#include "MetricGridWidget.h"
#include <QVBoxLayout>

MetricGridWidget::MetricGridWidget(QWidget* parent)
    : QFrame(parent)
{
}

void MetricGridWidget::setMetrics(const QVector<MetricEntry>& entries, int columns)
{
    qDeleteAll(m_valueLabels);
    m_valueLabels.clear();

    QGridLayout* grid = new QGridLayout(this);
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);

    for (int i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        int row = i / columns;
        int col = i % columns;

        QFrame* card = new QFrame(this);
        card->setObjectName("metricCard");
        card->setStyleSheet(
            "QFrame#metricCard { background: #ffffff; border: 1px solid #e7edf7; border-radius: 10px; padding: 14px; }"
            "QFrame#metricCard:hover { border-color: #3b82f6; }");
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 10);
        cardLayout->setSpacing(4);

        QLabel* label = new QLabel(entry.label, card);
        label->setObjectName("metricLabel");
        label->setStyleSheet("font-size: 12px; color: #64748b;");
        cardLayout->addWidget(label);

        QLabel* value = new QLabel("--", card);
        value->setObjectName("metricValue");
        value->setStyleSheet("font-size: 22px; font-weight: 700; color: #1f2937;");
        cardLayout->addWidget(value);

        if (!entry.hint.isEmpty()) {
            QLabel* hint = new QLabel(entry.hint, card);
            hint->setObjectName("metricHint");
            hint->setStyleSheet("font-size: 11px; color: #94a3b8;");
            hint->setWordWrap(true);
            cardLayout->addWidget(hint);
        }

        if (entry.valueTarget) {
            *entry.valueTarget = value;
        }
        m_valueLabels.append(value);
        grid->addWidget(card, row, col);
    }
}

void MetricGridWidget::setValue(int index, const QString& text)
{
    if (index >= 0 && index < m_valueLabels.size()) {
        m_valueLabels[index]->setText(text);
    }
}
