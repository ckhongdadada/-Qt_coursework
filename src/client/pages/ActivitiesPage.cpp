#include "ActivitiesPage.h"
#include "dialogs/ActivityEditorDialog.h"
#include "service/ActivityService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;
#include "widgets/ToastNotification.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>

ActivitiesPage::ActivitiesPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void ActivitiesPage::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("课外活动", this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel("正在读取活动数据...", this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    QFrame* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText("搜索活动名称 / 描述");
    m_categoryFilter = new QComboBox(filterCard);
    m_categoryFilter->addItem("全部分类", "");
    m_categoryFilter->addItem("学术", "学术");
    m_categoryFilter->addItem("文体", "文体");
    m_categoryFilter->addItem("志愿", "志愿");
    m_categoryFilter->addItem("社团", "社团");
    m_categoryFilter->addItem("竞赛", "竞赛");
    m_categoryFilter->addItem("其他", "其他");

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_categoryFilter, 0, 1);

    connect(m_searchInput, &QLineEdit::textChanged, this, &ActivitiesPage::refresh);
    connect(m_categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActivitiesPage::refresh);
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增活动", this);
    QPushButton* editButton = new QPushButton("编辑选中活动", this);
    QPushButton* removeButton = new QPushButton("删除选中活动", this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &ActivitiesPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &ActivitiesPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &ActivitiesPage::onRemoveClicked);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("活动总数", &m_totalValue), 0, 0);
    metrics->addWidget(createMetricCard("重点活动", &m_favoriteValue), 0, 1);
    metrics->addWidget(createMetricCard("进行中活动", &m_activeValue), 0, 2);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(this);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);

    QLabel* listTitle = new QLabel("活动记录", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);

    QLabel* helper = new QLabel("课外拓展、志愿服务与日常活动。支持双击编辑。", listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);

    m_list = new QListWidget(listCard);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &ActivitiesPage::onItemDoubleClicked);
    listLayout->addWidget(m_list);

    layout->addWidget(listCard, 1);
}

void ActivitiesPage::refresh()
{
    QList<Activity> list = ActivityService::getAll();
    QString kw = m_searchInput ? m_searchInput->text().trimmed().toLower() : "";
    QString cat = m_categoryFilter ? m_categoryFilter->currentData().toString().toLower() : "";

    QList<Activity> filtered;
    for (auto& a : list) {
        if (kw.isEmpty() || a.name.toLower().contains(kw) || a.description.toLower().contains(kw)) {
            if (cat.isEmpty() || a.category.toLower().contains(cat)) {
                filtered.append(a);
            }
        }
    }

    int totalAct = list.size();
    int favAct = 0;
    int actAct = 0;
    for (const auto& a : list) {
        if (a.isFavorite) favAct++;
        if (a.isActive) actAct++;
    }

    m_totalValue->setText(QString::number(totalAct));
    m_favoriteValue->setText(QString::number(favAct));
    m_activeValue->setText(QString::number(actAct));

    m_list->clear();
    for (auto& a : filtered) {
        QString timeRange = a.endDate.isEmpty()
            ? (a.startDate + (a.isActive ? "至今" : ""))
            : (a.startDate + " - " + a.endDate);
        QString txt = QString("%1 %2\n%3\n%4")
            .arg(a.isFavorite ? "★" : "").arg(a.name)
            .arg(a.category + " | " + timeRange)
            .arg(a.description);
        QListWidgetItem* item = new QListWidgetItem(txt, m_list);
        item->setData(Qt::UserRole, a.id);
    }

    if (m_list->count() == 0) {
        setupEmptyState(m_list, "暂无课外活动记录");
    }

    m_summaryLabel->setText(QString("显示 %1 / %2 项活动记录").arg(filtered.size()).arg(list.size()));
}

void ActivitiesPage::onAddClicked()
{
    ActivityEditorDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Activity act = dlg.activity();
        ActivityService::create(act);
        refresh();
        emit dataChanged(DataDomain::Activities);
        ToastNotification::display(this, "活动已创建。");
    }
}

void ActivitiesPage::onEditClicked()
{
    if (!m_list->currentItem()) {
        ToastNotification::display(this, "请先选择一项活动。");
        return;
    }
    int id = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }
    Activity a = ActivityService::getById(id);
    ActivityEditorDialog dlg(this);
    dlg.setActivity(a);
    if (dlg.exec() == QDialog::Accepted) {
        Activity act = dlg.activity();
        ActivityService::update(id, act);
        refresh();
        emit dataChanged(DataDomain::Activities);
        ToastNotification::display(this, "活动已更新。");
    }
}

void ActivitiesPage::onRemoveClicked()
{
    if (!m_list->currentItem()) {
        ToastNotification::display(this, "请先选择一项活动。");
        return;
    }
    int id = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }
    if (QMessageBox::question(this, "删除活动", "确定要删除该活动记录吗？此操作会同步影响总览和时间轴。") == QMessageBox::Yes) {
        ActivityService::remove(id);
        refresh();
        emit dataChanged(DataDomain::Activities);
        ToastNotification::display(this, "活动已删除。");
    }
}

void ActivitiesPage::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item && item->data(Qt::UserRole).toInt() > 0) {
        onEditClicked();
    }
}
