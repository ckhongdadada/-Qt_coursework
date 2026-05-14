#include "ExperiencesPage.h"
#include "dialogs/ExperienceEditorDialog.h"
#include "service/ExperienceService.h"
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

ExperiencesPage::ExperiencesPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void ExperiencesPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("经历档案"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(zh("正在读取经历数据..."), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    auto* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    auto* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText(zh("搜索经历标题 / 组织 / 描述"));

    m_typeFilter = new QComboBox(filterCard);
    m_typeFilter->addItem(zh("全部类型"), "");
    m_typeFilter->addItem(zh("项目"), zh("项目"));
    m_typeFilter->addItem(zh("实习"), zh("实习"));
    m_typeFilter->addItem(zh("科研"), zh("科研"));
    m_typeFilter->addItem(zh("志愿"), zh("志愿"));
    m_typeFilter->addItem(zh("竞赛"), zh("竞赛"));
    m_typeFilter->addItem(zh("其他"), zh("其他"));

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_typeFilter, 0, 1);

    connect(m_searchInput, &QLineEdit::textChanged, this, &ExperiencesPage::refresh);
    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExperiencesPage::refresh);
    layout->addWidget(filterCard);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    auto* addButton = new QPushButton(zh("新增经历"), this);
    auto* editButton = new QPushButton(zh("编辑选中经历"), this);
    auto* removeButton = new QPushButton(zh("删除选中经历"), this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &ExperiencesPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &ExperiencesPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &ExperiencesPage::onRemoveClicked);

    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(zh("经历总数"), &m_totalValue), 0, 0);
    metrics->addWidget(createMetricCard(zh("进行中经历"), &m_ongoingValue), 0, 1);
    metrics->addWidget(createMetricCard(zh("经历类型数"), &m_typeValue, zh("按 type 字段统计")), 0, 2);
    layout->addLayout(metrics);

    auto* listCard = new QFrame(this);
    listCard->setObjectName("contentCard");
    auto* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);

    auto* listTitle = new QLabel(zh("经历时间线"), listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);

    auto* helper = new QLabel(zh("项目、实习、科研和志愿经历都可以在这里直接维护，双击条目即可编辑。"), listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);

    m_list = new QListWidget(listCard);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &ExperiencesPage::onItemDoubleClicked);
    listLayout->addWidget(m_list);

    layout->addWidget(listCard, 1);
}

void ExperiencesPage::refresh()
{
    const QList<Experience> experiences = ExperienceService::getAll();
    const QString keyword = m_searchInput ? m_searchInput->text().trimmed().toLower() : QString();
    const QString typeFilter = m_typeFilter ? m_typeFilter->currentData().toString().trimmed().toLower() : QString();

    QList<Experience> filteredExperiences;
    for (const Experience& experience : experiences) {
        const bool matchSearch = keyword.isEmpty()
            || experience.title.toLower().contains(keyword)
            || experience.organization.toLower().contains(keyword)
            || experience.description.toLower().contains(keyword);
        const bool matchType = typeFilter.isEmpty() || experience.type.toLower().contains(typeFilter);
        if (matchSearch && matchType) {
            filteredExperiences.append(experience);
        }
    }

    const QJsonObject stats = ExperienceService::getStatistics();
    const QJsonObject typeBreakdown = stats.value("typeBreakdown").toObject();

    m_totalValue->setText(QString::number(filteredExperiences.size()));
    m_ongoingValue->setText(QString::number(stats.value("ongoingExperiences").toInt()));
    m_typeValue->setText(QString::number(typeBreakdown.keys().size()));

    m_list->clear();
    for (const Experience& experience : filteredExperiences) {
        const QString organization = experience.organization.trimmed().isEmpty()
            ? safeText(experience.type, zh("未分类"))
            : experience.organization;
        const QString roleText = experience.role.trimmed().isEmpty() ? QString() : zh(" · %1").arg(experience.role);
        const QString body = shortBody(
            experience.description,
            experience.isOngoing ? zh("当前经历仍在进行中。") : zh("该经历阶段已完成。"));
        auto* item = new QListWidgetItem(
            zh("%1\n%2 · %3%4\n%5")
                .arg(safeText(experience.title, zh("未命名经历")))
                .arg(joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, zh("至今")))
                .arg(organization)
                .arg(roleText)
                .arg(body),
            m_list);
        item->setData(Qt::UserRole, experience.id);
    }

    if (m_list->count() == 0) {
        setupEmptyState(m_list, zh("暂无经历档案数据"));
    } else {
        m_list->setCurrentRow(0);
    }

    m_summaryLabel->setText(
        zh("显示 %1 / %2 段经历，其中进行中 %3 段。支持搜索和类型筛选。")
            .arg(filteredExperiences.size())
            .arg(experiences.size())
            .arg(stats.value("ongoingExperiences").toInt()));
}

void ExperiencesPage::onAddClicked()
{
    ExperienceEditorDialog dialog(this);
    dialog.setWindowTitle(zh("新增经历"));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience experience = dialog.experience();
    const Experience created = ExperienceService::create(experience);
    if (created.id == 0) {
        ToastNotification::display(this, zh("经历未能成功写入数据库。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Experiences);
    ToastNotification::display(this, zh("经历已创建。"));
}

void ExperiencesPage::onEditClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一段经历。"));
        return;
    }

    const int experienceId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (experienceId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能编辑。"));
        return;
    }

    Experience experience = ExperienceService::getById(experienceId);
    if (experience.id == 0) {
        ToastNotification::display(this, zh("未找到对应经历记录。"));
        return;
    }

    ExperienceEditorDialog dialog(this);
    dialog.setWindowTitle(zh("编辑经历"));
    dialog.setExperience(experience);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience updated = dialog.experience();
    const Experience saved = ExperienceService::update(experienceId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, zh("经历更新失败。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Experiences);
    ToastNotification::display(this, zh("经历已更新。"));
}

void ExperiencesPage::onRemoveClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一段经历。"));
        return;
    }

    const int experienceId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (experienceId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能删除。"));
        return;
    }

    const QString title = m_list->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(
        this,
        zh("删除经历"),
        zh("确定要删除经历“%1”吗？此操作会影响时间轴、简历和 AI 分析。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!ExperienceService::remove(experienceId)) {
        ToastNotification::display(this, zh("经历删除失败，请稍后再试。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Experiences);
    ToastNotification::display(this, zh("经历已删除。"));
}

void ExperiencesPage::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item && item->data(Qt::UserRole).toInt() > 0) {
        onEditClicked();
    }
}
