#include "BasePage.h"

BasePage::BasePage(QWidget* parent)
    : QWidget(parent)
{
}

BasePage::~BasePage() = default;

QFrame* BasePage::createMetricCard(const QString& labelText, QLabel** valueLabel,
                                   const QString& helperText)
{
    QFrame* card = new QFrame(this);
    card->setObjectName("metricCard");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(4);

    QLabel* label = new QLabel(labelText, card);
    label->setObjectName("metricLabel");
    layout->addWidget(label);

    QLabel* value = new QLabel("--", card);
    value->setObjectName("metricValue");
    layout->addWidget(value);
    if (valueLabel) {
        *valueLabel = value;
    }

    if (!helperText.isEmpty()) {
        QLabel* helper = new QLabel(helperText, card);
        helper->setObjectName("metricHelper");
        helper->setWordWrap(true);
        layout->addWidget(helper);
    }

    return card;
}
