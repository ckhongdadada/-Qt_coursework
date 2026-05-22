#include "GoalsPage.h"
#include "dialogs/GoalEditorDialog.h"
#include "service/GoalService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;
#include "widgets/ToastNotification.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

GoalsPage::GoalsPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void GoalsPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("目标追踪"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(zh("正在读取目标数据..."), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    auto* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    auto* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText(zh("搜索目标标题 / 描述"));

    m_statusFilter = new QComboBox(filterCard);
    m_statusFilter->addItem(zh("全部状态"), "");
    m_statusFilter->addItem(zh("进行中"), QStringLiteral("In Progress"));
    m_statusFilter->addItem(zh("已完成"), QStringLiteral("Completed"));
    m_statusFilter->addItem(zh("已暂停"), QStringLiteral("On Hold"));
    m_statusFilter->addItem(zh("已取消"), QStringLiteral("Cancelled"));

    m_priorityFilter = new QComboBox(filterCard);
    m_priorityFilter->addItem(zh("全部优先级"), "");
    m_priorityFilter->addItem(zh("高优先级"), QStringLiteral("High"));
    m_priorityFilter->addItem(zh("中优先级"), QStringLiteral("Medium"));
    m_priorityFilter->addItem(zh("低优先级"), QStringLiteral("Low"));

    m_sortInput = new QLineEdit(filterCard);
    m_sortInput->setPlaceholderText(zh("排序：progress / deadline / title / priority"));

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_statusFilter, 0, 1);
    filterLayout->addWidget(m_priorityFilter, 1, 0);
    filterLayout->addWidget(m_sortInput, 1, 1);

    connect(m_searchInput, &QLineEdit::textChanged, this, &GoalsPage::refresh);
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GoalsPage::refresh);
    connect(m_priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GoalsPage::refresh);
    connect(m_sortInput, &QLineEdit::textChanged, this, &GoalsPage::refresh);
    layout->addWidget(filterCard);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(zh("目标总数"), &m_totalValue), 0, 0);
    metrics->addWidget(createMetricCard(zh("已完成目标"), &m_completedValue), 0, 1);
    metrics->addWidget(createMetricCard(zh("平均进度"), &m_progressValue, zh("基于 target/current 自动计算")), 0, 2);
    layout->addLayout(metrics);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    auto* addButton = new QPushButton(zh("新增目标"), this);
    auto* editButton = new QPushButton(zh("编辑选中目标"), this);
    auto* removeButton = new QPushButton(zh("删除选中目标"), this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &GoalsPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &GoalsPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &GoalsPage::onRemoveClicked);

    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    auto* listCard = new QFrame(this);
    listCard->setObjectName("contentCard");
    auto* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);

    auto* listTitle = new QLabel(zh("目标清单"), listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);

    auto* helper = new QLabel(zh("目标可以直接在原生窗口中维护，保存后会联动刷新总览、时间轴和简历分析。"), listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);

    m_list = new QListWidget(listCard);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &GoalsPage::onItemDoubleClicked);
    listLayout->addWidget(m_list);

    layout->addWidget(listCard, 1);
}

void GoalsPage::refresh()
{
    const QList<Goal> goals = GoalService::getAll();
    const QJsonObject stats = GoalService::getStatistics();
    const QString keyword = m_searchInput ? m_searchInput->text().trimmed().toLower() : QString();
    const QString statusKeyword = m_statusFilter ? m_statusFilter->currentData().toString().trimmed().toLower() : QString();
    const QString priorityKeyword = m_priorityFilter ? m_priorityFilter->currentData().toString().trimmed().toLower() : QString();
    const QString sortKey = m_sortInput ? m_sortInput->text().trimmed().toLower() : QStringLiteral("progress");

    QList<Goal> filteredGoals;
    for (const Goal& goal : goals) {
        const bool matchSearch = keyword.isEmpty()
            || goal.title.toLower().contains(keyword)
            || goal.description.toLower().contains(keyword);
        const bool matchStatus = statusKeyword.isEmpty() || goal.status.toLower().contains(statusKeyword);
        const bool matchPriority = priorityKeyword.isEmpty() || goal.priority.toLower().contains(priorityKeyword);
        if (matchSearch && matchStatus && matchPriority) {
            filteredGoals.append(goal);
        }
    }

    std::sort(filteredGoals.begin(), filteredGoals.end(), [sortKey](const Goal& left, const Goal& right) {
        if (sortKey == "deadline") return left.deadline < right.deadline;
        if (sortKey == "title") return left.title.toLower() < right.title.toLower();
        if (sortKey == "priority") return left.priority.toLower() < right.priority.toLower();
        return left.progress() > right.progress();
    });

    m_totalValue->setText(QString::number(stats.value("total").toInt()));
    m_completedValue->setText(QString::number(stats.value("completed").toInt()));
    m_progressValue->setText(QString("%1%").arg(stats.value("averageProgress").toDouble(), 0, 'f', 1));

    m_list->clear();
    for (const Goal& goal : filteredGoals) {
        const QString progress = zh("%1% · %2")
            .arg(goal.progress(), 0, 'f', 1)
            .arg(safeText(goal.status, zh("未设置状态")));
        const QString deadline = safeText(goal.deadline, zh("截止时间未填写"));
        const QString body = shortBody(
            goal.description,
            zh("目标值 %1 %2，当前值 %3 %4。")
                .arg(goal.targetValue, 0, 'f', 1)
                .arg(safeText(goal.unit, QString()))
                .arg(goal.currentValue, 0, 'f', 1)
                .arg(safeText(goal.unit, QString())));
        auto* item = new QListWidgetItem(
            zh("%1\n%2 · 截止 %3\n%4")
                .arg(safeText(goal.title, zh("未命名目标")))
                .arg(progress)
                .arg(deadline)
                .arg(body),
            m_list);
        item->setData(Qt::UserRole, goal.id);
    }

    if (m_list->count() == 0) {
        setupEmptyState(m_list, zh("暂无目标追踪数据"));
    } else {
        m_list->setCurrentRow(0);
    }

    m_summaryLabel->setText(
        zh("当前显示 %1 / %2 个目标，已完成 %3 个，平均进度 %4%。支持搜索、筛选和排序。")
            .arg(filteredGoals.size())
            .arg(goals.size())
            .arg(stats.value("completed").toInt())
            .arg(QString::number(stats.value("averageProgress").toDouble(), 'f', 1)));
}

void GoalsPage::onAddClicked()
{
    GoalEditorDialog dialog(this);
    dialog.setWindowTitle(zh("新增目标"));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal goal = dialog.goal();
    const Goal created = GoalService::create(goal);
    if (created.id == 0) {
        ToastNotification::display(this, zh("目标未能成功写入数据库。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Goals);
    ToastNotification::display(this, zh("目标已创建。"));
}

void GoalsPage::onEditClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一个目标。"));
        return;
    }

    const int goalId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (goalId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能编辑。"));
        return;
    }

    Goal goal = GoalService::getById(goalId);
    if (goal.id == 0) {
        ToastNotification::display(this, zh("未找到对应目标记录。"));
        return;
    }

    GoalEditorDialog dialog(this);
    dialog.setWindowTitle(zh("编辑目标"));
    dialog.setGoal(goal);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal updated = dialog.goal();
    const Goal saved = GoalService::update(goalId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, zh("目标更新失败。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Goals);
    ToastNotification::display(this, zh("目标已更新。"));
}

void GoalsPage::onRemoveClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一个目标。"));
        return;
    }

    const int goalId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (goalId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能删除。"));
        return;
    }

    const QString title = m_list->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(
        this,
        zh("删除目标"),
        zh("确定要删除目标“%1”吗？此操作会影响总览、时间轴和 AI 建议。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!GoalService::remove(goalId)) {
        ToastNotification::display(this, zh("目标删除失败，请稍后再试。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Goals);
    ToastNotification::display(this, zh("目标已删除。"));
}

void GoalsPage::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item && item->data(Qt::UserRole).toInt() > 0) {
        onEditClicked();
    }
}
