#include "SemesterAnalysisWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QJsonArray>
#include <QJsonObject>

SemesterAnalysisWidget::SemesterAnalysisWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("contentCard");
    setupUi();
}

void SemesterAnalysisWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    QLabel* semLabel = new QLabel("学期趋势表现", this);
    semLabel->setObjectName("sectionTitle");
    layout->addWidget(semLabel);

    m_semesterTable = new QTableWidget(this);
    m_semesterTable->setColumnCount(4);
    m_semesterTable->setHorizontalHeaderLabels({"学期", "修读学分", "加权绩点", "排名"});
    m_semesterTable->horizontalHeader()->setStretchLastSection(true);
    m_semesterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_semesterTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_semesterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_semesterTable, 1);

    QLabel* strLabel = new QLabel("核心优势", this);
    strLabel->setObjectName("sectionTitle");
    layout->addWidget(strLabel);

    m_strengthList = new QListWidget(this);
    m_strengthList->setObjectName("plainList");
    m_strengthList->setWordWrap(true);
    m_strengthList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_strengthList, 1);

    QLabel* rskLabel = new QLabel("潜在风险", this);
    rskLabel->setObjectName("sectionTitle");
    layout->addWidget(rskLabel);

    m_riskList = new QListWidget(this);
    m_riskList->setObjectName("plainList");
    m_riskList->setWordWrap(true);
    m_riskList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_riskList, 1);

    QLabel* sugLabel = new QLabel("发展建议", this);
    sugLabel->setObjectName("sectionTitle");
    layout->addWidget(sugLabel);

    m_suggestionList = new QListWidget(this);
    m_suggestionList->setObjectName("plainList");
    m_suggestionList->setWordWrap(true);
    m_suggestionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_suggestionList, 1);
}

void SemesterAnalysisWidget::loadReport(const QJsonObject& report)
{
    QJsonArray semesters = report["semesters"].toArray();
    QJsonArray strengths = report["strengths"].toArray();
    QJsonArray risks = report["risks"].toArray();
    QJsonArray suggestions = report["suggestions"].toArray();

    m_semesterCount = semesters.size();
    m_suggestionCount = suggestions.size() + strengths.size() + risks.size();

    m_semesterTable->setRowCount(semesters.size());
    for (int i = 0; i < semesters.size(); ++i) {
        QJsonObject s = semesters[i].toObject();
        m_semesterTable->setItem(i, 0, new QTableWidgetItem(s["semester"].toString()));
        m_semesterTable->setItem(i, 1, new QTableWidgetItem(QString::number(s["credits"].toDouble(), 'f', 1)));
        m_semesterTable->setItem(i, 2, new QTableWidgetItem(QString::number(s["gpa"].toDouble(), 'f', 2)));
        m_semesterTable->setItem(i, 3, new QTableWidgetItem(s["rank"].toString()));
    }
    if (semesters.isEmpty()) {
        m_semesterTable->setRowCount(1);
        m_semesterTable->setSpan(0, 0, 1, 4);
        auto* emptyItem = new QTableWidgetItem("\n暂无学期对比数据\n添加课程后自动生成\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(11); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_semesterTable->setItem(0, 0, emptyItem);
    }

    m_strengthList->clear();
    for (auto v : strengths) m_strengthList->addItem(v.toString());
    m_riskList->clear();
    for (auto v : risks) m_riskList->addItem(v.toString());
    m_suggestionList->clear();
    for (auto v : suggestions) m_suggestionList->addItem(v.toString());
}

int SemesterAnalysisWidget::semesterCount() const { return m_semesterCount; }
int SemesterAnalysisWidget::suggestionCount() const { return m_suggestionCount; }
