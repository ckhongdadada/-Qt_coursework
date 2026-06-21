#include "SuggestionListWidget.h"
#include "utils/UiHelpers.h"
#include <QVBoxLayout>
#include <QLabel>

SuggestionListWidget::SuggestionListWidget(const QString& title, QWidget* parent)
    : QFrame(parent)
{
    setObjectName("contentCard");
    setupUi(title);
}

void SuggestionListWidget::setupUi(const QString& title)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("sectionTitle");
    layout->addWidget(titleLabel);

    m_list = new QListWidget(this);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_list);
}

void SuggestionListWidget::loadItems(const QJsonArray& items)
{
    m_list->clear();
    for (const auto& item : items) {
        m_list->addItem(UiHelpers::bullet(item.toString()));
    }
    if (m_list->count() == 0) {
        UiHelpers::setupEmptyState(m_list, "暂无数据");
    }
}

void SuggestionListWidget::loadStrings(const QStringList& items)
{
    m_list->clear();
    for (const auto& item : items) {
        m_list->addItem(UiHelpers::bullet(item));
    }
    if (m_list->count() == 0) {
        UiHelpers::setupEmptyState(m_list, "暂无数据");
    }
}

void SuggestionListWidget::clearItems()
{
    m_list->clear();
}

int SuggestionListWidget::count() const
{
    return m_list ? m_list->count() : 0;
}
