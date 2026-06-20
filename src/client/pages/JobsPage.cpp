#include "JobsPage.h"
#include "dialogs/JobEditorDialog.h"
#include "service/JobService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;
#include "widgets/ToastNotification.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>

JobsPage::JobsPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void JobsPage::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("目标岗位", this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel("正在读取岗位数据...", this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    QFrame* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText("搜索岗位名称 / 公司 / 城市");
    m_statusInput = new QLineEdit(filterCard);
    m_statusInput->setPlaceholderText("过滤激活状态");

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_statusInput, 0, 1);

    connect(m_searchInput, &QLineEdit::textChanged, this, &JobsPage::refresh);
    connect(m_statusInput, &QLineEdit::textChanged, this, &JobsPage::refresh);
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增岗位", this);
    QPushButton* editButton = new QPushButton("编辑选中岗位", this);
    QPushButton* removeButton = new QPushButton("删除选中岗位", this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &JobsPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &JobsPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &JobsPage::onRemoveClicked);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("岗位总数", &m_totalValue), 0, 0);
    metrics->addWidget(createMetricCard("关注中", &m_activeValue), 0, 1);
    metrics->addWidget(createMetricCard("平均要求匹配率", &m_requirementValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* listCard = new QFrame(this);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);

    QLabel* listTitle = new QLabel("岗位列表", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);

    m_list = new QListWidget(listCard);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &JobsPage::onItemDoubleClicked);
    connect(m_list, &QListWidget::currentRowChanged, this, &JobsPage::onJobSelectionChanged);
    listLayout->addWidget(m_list);
    bodyLayout->addWidget(listCard, 2);

    QFrame* reqCard = new QFrame(this);
    reqCard->setObjectName("contentCard");
    QVBoxLayout* reqLayout = new QVBoxLayout(reqCard);
    reqLayout->setContentsMargins(16, 14, 16, 14);
    reqLayout->setSpacing(10);

    QLabel* reqTitle = new QLabel("岗位要求匹配", reqCard);
    reqTitle->setObjectName("sectionTitle");
    reqLayout->addWidget(reqTitle);

    m_requirementSummaryLabel = new QLabel("请在左侧选择岗位", reqCard);
    m_requirementSummaryLabel->setObjectName("pageSubtitle");
    reqLayout->addWidget(m_requirementSummaryLabel);

    m_requirementList = new QListWidget(reqCard);
    m_requirementList->setObjectName("plainList");
    m_requirementList->setWordWrap(true);
    m_requirementList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_requirementList, &QListWidget::itemClicked, this, &JobsPage::onRequirementClicked);
    reqLayout->addWidget(m_requirementList);
    bodyLayout->addWidget(reqCard, 1);

    layout->addLayout(bodyLayout, 1);
}

void JobsPage::refresh()
{
    QList<Job> list = JobService::getAll();
    QString kw = m_searchInput ? m_searchInput->text().trimmed().toLower() : "";
    QString stat = m_statusInput ? m_statusInput->text().trimmed().toLower() : "";

    QList<Job> filtered;
    for (auto& j : list) {
        if (kw.isEmpty() || j.title.toLower().contains(kw) || j.company.toLower().contains(kw) || j.location.toLower().contains(kw)) {
            if (stat.isEmpty() || (j.isActive && stat == "active") || (!j.isActive && stat == "inactive")) {
                filtered.append(j);
            }
        }
    }

    int totalJob = list.size();
    int actJob = 0;
    double totalRatio = 0.0;
    for (const auto& j : list) {
        if (j.isActive) actJob++;
        if (!j.requirements.isEmpty()) {
            int metCount = 0;
            for (const auto& req : j.requirements) {
                if (req.met) metCount++;
            }
            totalRatio += (double)metCount / j.requirements.size();
        }
    }
    double avgRatio = totalJob > 0 ? (totalRatio / totalJob) : 0.0;

    m_totalValue->setText(QString::number(totalJob));
    m_activeValue->setText(QString::number(actJob));
    m_requirementValue->setText(QString("%1%").arg(avgRatio * 100.0, 0, 'f', 1));

    m_list->clear();
    for (auto& j : filtered) {
        QString txt = QString("%1\n%2 - %3\n优先级: %4")
            .arg(j.title)
            .arg(j.company, j.location)
            .arg(j.priority);
        QListWidgetItem* item = new QListWidgetItem(txt, m_list);
        item->setData(Qt::UserRole, j.id);
    }

    if (m_list->count() == 0) {
        setupEmptyState(m_list, "暂无目标岗位数据");
    }

    m_summaryLabel->setText(QString("显示 %1 / %2 项目标岗位").arg(filtered.size()).arg(list.size()));
    if (m_requirementList) m_requirementList->clear();
    if (m_requirementSummaryLabel) m_requirementSummaryLabel->setText("请在左侧选择岗位");
}

void JobsPage::onJobSelectionChanged(int row)
{
    if (row >= 0 && m_list->currentItem()) {
        int jobId = m_list->currentItem()->data(Qt::UserRole).toInt();
        if (jobId > 0) {
            Job job = JobService::getById(jobId);
            m_requirementList->clear();
            int metCount = 0;
            for (int i = 0; i < job.requirements.size(); ++i) {
                const auto& req = job.requirements[i];
                QListWidgetItem* item = new QListWidgetItem(QString("[%1] %2").arg(req.met ? "x" : " ").arg(req.text), m_requirementList);
                item->setData(Qt::UserRole, i);
                if (req.met) metCount++;
            }
            m_requirementSummaryLabel->setText(QString("此岗位共有 %1 项要求，已匹配 %2 项。").arg(job.requirements.size()).arg(metCount));
        }
    }
}

void JobsPage::onRequirementClicked(QListWidgetItem* item)
{
    if (!m_list->currentItem() || !item) return;
    int id = m_list->currentItem()->data(Qt::UserRole).toInt();
    int reqIdx = item->data(Qt::UserRole).toInt();
    if (id <= 0 || reqIdx < 0) return;

    Job j = JobService::getById(id);
    if (reqIdx < j.requirements.size()) {
        j.requirements[reqIdx].met = !j.requirements[reqIdx].met;
        JobService::update(id, j);

        item->setText(QString("[%1] %2").arg(j.requirements[reqIdx].met ? "x" : " ").arg(j.requirements[reqIdx].text));
        int metCount = 0;
        for (const auto& r : j.requirements) { if (r.met) metCount++; }
        m_requirementSummaryLabel->setText(QString("此岗位共有 %1 项要求，已匹配 %2 项。").arg(j.requirements.size()).arg(metCount));
    }
}

void JobsPage::onAddClicked()
{
    JobEditorDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Job jb = dlg.job();
        JobService::create(jb);
        refresh();
        emit dataChanged(DataDomain::Jobs);
        ToastNotification::display(this, "岗位已创建。");
    }
}

void JobsPage::onEditClicked()
{
    if (!m_list->currentItem()) {
        ToastNotification::display(this, "请先选择一个岗位。");
        return;
    }
    int id = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }
    Job j = JobService::getById(id);
    JobEditorDialog dlg(this);
    dlg.setJob(j);
    if (dlg.exec() == QDialog::Accepted) {
        Job jb = dlg.job();
        JobService::update(id, jb);
        refresh();
        emit dataChanged(DataDomain::Jobs);
        ToastNotification::display(this, "岗位已更新。");
    }
}

void JobsPage::onRemoveClicked()
{
    if (!m_list->currentItem()) {
        ToastNotification::display(this, "请先选择一个岗位。");
        return;
    }
    int id = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }
    if (QMessageBox::question(this, "删除岗位", "确定要删除该目标岗位吗？此操作会同步影响总览和时间轴。") == QMessageBox::Yes) {
        JobService::remove(id);
        refresh();
        emit dataChanged(DataDomain::Jobs);
        ToastNotification::display(this, "岗位已删除。");
    }
}

void JobsPage::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item && item->data(Qt::UserRole).toInt() > 0) {
        onEditClicked();
    }
}
