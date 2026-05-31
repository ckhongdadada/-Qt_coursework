#include "AnalysisPage.h"
#include "widgets/SemesterAnalysisWidget.h"
#include "widgets/PeerBenchmarkWidget.h"
#include "dialogs/PeerEditorDialog.h"
#include "service/AnalyticsService.h"
#include "widgets/ToastNotification.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

AnalysisPage::AnalysisPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void AnalysisPage::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("分析报告", this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel("正在生成数据分析报告...", this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* refreshBtn = new QPushButton("重新生成报告", this);
    connect(refreshBtn, &QPushButton::clicked, this, &AnalysisPage::refresh);
    actionLayout->addWidget(refreshBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("分析学期数", &m_semesterValue), 0, 0);
    metrics->addWidget(createMetricCard("对比对象数", &m_peerValue), 0, 1);
    metrics->addWidget(createMetricCard("总结建议条数", &m_suggestionValue), 0, 2);
    layout->addLayout(metrics);

    QVBoxLayout* bodyLayout = new QVBoxLayout();
    bodyLayout->setSpacing(14);

    m_semesterWidget = new SemesterAnalysisWidget(this);
    m_peerWidget = new PeerBenchmarkWidget(this);
    m_semesterWidget->setMinimumHeight(420);
    m_peerWidget->setMinimumHeight(360);

    bodyLayout->addWidget(m_semesterWidget);
    bodyLayout->addWidget(m_peerWidget);
    layout->addLayout(bodyLayout, 1);

    connect(m_peerWidget, &PeerBenchmarkWidget::addPeerRequested, this, &AnalysisPage::onAddPeerRequested);
    connect(m_peerWidget, &PeerBenchmarkWidget::editPeerRequested, this, &AnalysisPage::onEditPeerRequested);
    connect(m_peerWidget, &PeerBenchmarkWidget::removePeerRequested, this, &AnalysisPage::onRemovePeerRequested);
}

void AnalysisPage::refresh()
{
    QJsonObject report = AnalyticsService::generateReport();
    m_semesterWidget->loadReport(report);

    QList<PeerBenchmark> peers = PeerBenchmarkService::getAll();
    m_peerWidget->loadPeers(peers);

    m_semesterValue->setText(QString::number(m_semesterWidget->semesterCount()));
    m_peerValue->setText(QString::number(m_peerWidget->peerCount()));
    m_suggestionValue->setText(QString::number(m_semesterWidget->suggestionCount()));

    m_summaryLabel->setText("报告生成成功，已评估各类维度的学习成果表现与差距。");
}

void AnalysisPage::onAddPeerRequested()
{
    PeerEditorDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        PeerBenchmark pb = dlg.peer();
        PeerBenchmarkService::create(pb);
        refresh();
        emit dataChanged(DataDomain::Analysis);
        ToastNotification::display(this, "对照同学已添加。");
    }
}

void AnalysisPage::onEditPeerRequested(int id)
{
    if (id <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }
    PeerBenchmark p = PeerBenchmarkService::getById(id);
    PeerEditorDialog dlg(this);
    dlg.setPeer(p);
    if (dlg.exec() == QDialog::Accepted) {
        PeerBenchmark pb = dlg.peer();
        PeerBenchmarkService::update(id, pb);
        refresh();
        emit dataChanged(DataDomain::Analysis);
        ToastNotification::display(this, "对照同学信息已更新。");
    }
}

void AnalysisPage::onRemovePeerRequested(int id)
{
    if (id <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }
    if (QMessageBox::question(this, "删除对照同学", "确定要删除这名对照同学记录吗？删除后将无法恢复。") == QMessageBox::Yes) {
        PeerBenchmarkService::remove(id);
        refresh();
        emit dataChanged(DataDomain::Analysis);
        ToastNotification::display(this, "对照同学已删除。");
    }
}
