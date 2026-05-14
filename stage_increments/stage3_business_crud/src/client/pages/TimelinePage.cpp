#include "TimelinePage.h"
#include "service/AnalyticsService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;

#include <QFrame>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QVBoxLayout>

TimelinePage::TimelinePage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void TimelinePage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("时间轴"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(zh("正在整理发展历程..."), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(zh("成长节点"), &m_eventCountValue), 0, 0);
    metrics->addWidget(createMetricCard(zh("核心优势"), &m_strengthValue), 0, 1);
    metrics->addWidget(createMetricCard(zh("潜在风险"), &m_riskValue), 0, 2);
    layout->addLayout(metrics);

    auto* contentGrid = new QGridLayout();
    contentGrid->setHorizontalSpacing(14);
    contentGrid->setVerticalSpacing(14);

    auto* eventCard = new QFrame(this);
    eventCard->setObjectName("contentCard");
    auto* eventLayout = new QVBoxLayout(eventCard);
    eventLayout->setContentsMargins(16, 14, 16, 14);
    eventLayout->setSpacing(10);

    auto* eventTitle = new QLabel(zh("发展历程"), eventCard);
    eventTitle->setObjectName("sectionTitle");
    eventLayout->addWidget(eventTitle);

    auto* eventHelper = new QLabel(zh("系统会从角色、成果、经历、活动和目标中自动汇总关键时间节点。"), eventCard);
    eventHelper->setObjectName("pageSubtitle");
    eventHelper->setWordWrap(true);
    eventLayout->addWidget(eventHelper);

    m_eventList = new QListWidget(eventCard);
    m_eventList->setObjectName("plainList");
    m_eventList->setWordWrap(true);
    m_eventList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    eventLayout->addWidget(m_eventList, 1);

    auto* suggestionCard = new QFrame(this);
    suggestionCard->setObjectName("contentCard");
    auto* suggestionLayout = new QVBoxLayout(suggestionCard);
    suggestionLayout->setContentsMargins(16, 14, 16, 14);
    suggestionLayout->setSpacing(10);

    auto* suggestionTitle = new QLabel(zh("阶段建议"), suggestionCard);
    suggestionTitle->setObjectName("sectionTitle");
    suggestionLayout->addWidget(suggestionTitle);

    m_suggestionList = new QListWidget(suggestionCard);
    m_suggestionList->setObjectName("plainList");
    m_suggestionList->setWordWrap(true);
    m_suggestionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    suggestionLayout->addWidget(m_suggestionList, 1);

    contentGrid->addWidget(eventCard, 0, 0, 1, 2);
    contentGrid->addWidget(suggestionCard, 0, 2);
    contentGrid->setColumnStretch(0, 2);
    contentGrid->setColumnStretch(1, 2);
    contentGrid->setColumnStretch(2, 1);
    layout->addLayout(contentGrid, 1);
}

void TimelinePage::refresh()
{
    const QJsonArray events = AnalyticsService::getTimelineEvents();
    const QJsonObject report = AnalyticsService::generateReport();
    const QJsonArray strengths = report.value("strengths").toArray();
    const QJsonArray risks = report.value("risks").toArray();
    const QJsonArray suggestions = report.value("suggestions").toArray();

    if (m_eventCountValue) {
        m_eventCountValue->setText(QString::number(events.size()));
    }
    if (m_strengthValue) {
        m_strengthValue->setText(QString::number(strengths.size()));
    }
    if (m_riskValue) {
        m_riskValue->setText(QString::number(risks.size()));
    }

    m_eventList->clear();
    for (const QJsonValue& value : events) {
        const QJsonObject event = value.toObject();
        auto* item = new QListWidgetItem(
            zh("%1\n%2 · %3\n%4")
                .arg(safeText(event.value("title").toString(), zh("未命名节点")))
                .arg(safeText(event.value("date").toString(), zh("日期未填写")))
                .arg(safeText(event.value("subtitle").toString(), event.value("type").toString()))
                .arg(shortBody(event.value("description").toString(), zh("已记录新的成长节点。"))),
            m_eventList);
        item->setData(Qt::UserRole, event.value("type").toString());
    }

    if (m_eventList->count() == 0) {
        setupEmptyState(m_eventList, zh("暂无时间轴节点"));
    }

    m_suggestionList->clear();
    for (const QJsonValue& value : strengths) {
        m_suggestionList->addItem(zh("优势：%1").arg(value.toString()));
    }
    for (const QJsonValue& value : risks) {
        m_suggestionList->addItem(zh("风险：%1").arg(value.toString()));
    }
    for (const QJsonValue& value : suggestions) {
        m_suggestionList->addItem(zh("建议：%1").arg(value.toString()));
    }

    if (m_suggestionList->count() == 0) {
        setupEmptyState(m_suggestionList, zh("暂无阶段建议"));
    }

    m_summaryLabel->setText(
        zh("当前共整理 %1 个成长节点，识别 %2 条优势和 %3 条风险。")
            .arg(events.size())
            .arg(strengths.size())
            .arg(risks.size()));
}
