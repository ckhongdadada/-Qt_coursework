#include "AchievementsPage.h"
#include "dialogs/AchievementEditorDialog.h"
#include "service/AchievementService.h"
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

AchievementsPage::AchievementsPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void AchievementsPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("成果记录"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(zh("正在读取成果数据..."), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    auto* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    auto* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText(zh("搜索成果标题 / 机构 / 描述"));

    m_typeFilter = new QComboBox(filterCard);
    m_typeFilter->addItem(zh("全部类型"), "");
    m_typeFilter->addItem(zh("证书"), zh("证书"));
    m_typeFilter->addItem(zh("竞赛"), zh("竞赛"));
    m_typeFilter->addItem(zh("奖项"), zh("奖项"));
    m_typeFilter->addItem(zh("课程成果"), zh("课程成果"));
    m_typeFilter->addItem(zh("开源贡献"), zh("开源贡献"));
    m_typeFilter->addItem(zh("论文报告"), zh("论文报告"));
    m_typeFilter->addItem(zh("其他"), zh("其他"));

    m_levelFilter = new QComboBox(filterCard);
    m_levelFilter->addItem(zh("全部级别"), "");
    m_levelFilter->addItem(zh("国家级"), zh("国家级"));
    m_levelFilter->addItem(zh("省级"), zh("省级"));
    m_levelFilter->addItem(zh("校级"), zh("校级"));
    m_levelFilter->addItem(zh("院级"), zh("院级"));
    m_levelFilter->addItem(zh("其他"), zh("其他"));

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_typeFilter, 0, 1);
    filterLayout->addWidget(m_levelFilter, 0, 2);

    connect(m_searchInput, &QLineEdit::textChanged, this, &AchievementsPage::refresh);
    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AchievementsPage::refresh);
    connect(m_levelFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AchievementsPage::refresh);
    layout->addWidget(filterCard);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    auto* addButton = new QPushButton(zh("新增成果"), this);
    auto* editButton = new QPushButton(zh("编辑选中成果"), this);
    auto* removeButton = new QPushButton(zh("删除选中成果"), this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &AchievementsPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &AchievementsPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &AchievementsPage::onRemoveClicked);

    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(zh("成果总数"), &m_totalValue), 0, 0);
    metrics->addWidget(createMetricCard(zh("已验证成果"), &m_verifiedValue), 0, 1);
    metrics->addWidget(createMetricCard(zh("成果级别数"), &m_levelValue, zh("按 level 字段统计")), 0, 2);
    metrics->addWidget(createMetricCard(zh("成果类型数"), &m_typeValue, zh("按 type 字段统计")), 0, 3);
    layout->addLayout(metrics);

    auto* listCard = new QFrame(this);
    listCard->setObjectName("contentCard");
    auto* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);

    auto* listTitle = new QLabel(zh("成果时间线"), listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);

    auto* helper = new QLabel(zh("竞赛、证书和奖项都可以直接在这里维护，双击条目即可进入编辑。"), listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);

    m_list = new QListWidget(listCard);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &AchievementsPage::onItemDoubleClicked);
    listLayout->addWidget(m_list);

    layout->addWidget(listCard, 1);
}

void AchievementsPage::refresh()
{
    const QList<Achievement> achievements = AchievementService::getAll();
    const QString keyword = m_searchInput ? m_searchInput->text().trimmed().toLower() : QString();
    const QString typeFilter = m_typeFilter ? m_typeFilter->currentData().toString().trimmed().toLower() : QString();
    const QString levelFilter = m_levelFilter ? m_levelFilter->currentData().toString().trimmed().toLower() : QString();

    QList<Achievement> filteredAchievements;
    for (const Achievement& achievement : achievements) {
        const bool matchSearch = keyword.isEmpty()
            || achievement.title.toLower().contains(keyword)
            || achievement.organization.toLower().contains(keyword)
            || achievement.description.toLower().contains(keyword);
        const bool matchType = typeFilter.isEmpty() || achievement.type.toLower().contains(typeFilter);
        const bool matchLevel = levelFilter.isEmpty() || achievement.level.toLower().contains(levelFilter);
        if (matchSearch && matchType && matchLevel) {
            filteredAchievements.append(achievement);
        }
    }

    const QJsonObject stats = AchievementService::getStatistics();
    const QJsonObject typeBreakdown = stats.value("typeBreakdown").toObject();
    const QJsonObject levelBreakdown = stats.value("levelBreakdown").toObject();

    m_totalValue->setText(QString::number(filteredAchievements.size()));
    m_verifiedValue->setText(QString::number(stats.value("verifiedAchievements").toInt()));
    m_levelValue->setText(QString::number(levelBreakdown.keys().size()));
    m_typeValue->setText(QString::number(typeBreakdown.keys().size()));

    m_list->clear();
    for (const Achievement& achievement : filteredAchievements) {
        const QString meta = achievement.level.trimmed().isEmpty()
            ? safeText(achievement.type, zh("未分类"))
            : zh("%1 · %2").arg(safeText(achievement.date, zh("日期未填写")), achievement.level);
        const QString detail = shortBody(
            achievement.description,
            achievement.organization.trimmed().isEmpty()
                ? zh("已记录一项新的成果。")
                : zh("归属机构：%1").arg(achievement.organization));
        auto* item = new QListWidgetItem(
            zh("%1\n%2\n%3")
                .arg(safeText(achievement.title, zh("未命名成果")))
                .arg(meta)
                .arg(detail),
            m_list);
        item->setData(Qt::UserRole, achievement.id);
    }

    if (m_list->count() == 0) {
        setupEmptyState(m_list, zh("暂无成果记录数据"));
    } else {
        m_list->setCurrentRow(0);
    }

    const QString mainLevel = levelBreakdown.isEmpty() ? zh("未分类") : levelBreakdown.keys().first();
    m_summaryLabel->setText(
        zh("显示 %1 / %2 项成果，已验证 %3 项。主要级别：%4。支持搜索、类型和级别筛选。")
            .arg(filteredAchievements.size())
            .arg(achievements.size())
            .arg(stats.value("verifiedAchievements").toInt())
            .arg(mainLevel));
}

void AchievementsPage::onAddClicked()
{
    AchievementEditorDialog dialog(this);
    dialog.setWindowTitle(zh("新增成果"));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Achievement achievement = dialog.achievement();
    const Achievement created = AchievementService::create(achievement);
    if (created.id == 0) {
        ToastNotification::display(this, zh("成果未能成功写入数据库。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Achievements);
    ToastNotification::display(this, zh("成果已创建。"));
}

void AchievementsPage::onEditClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一条成果。"));
        return;
    }

    const int achievementId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (achievementId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能编辑。"));
        return;
    }

    Achievement achievement = AchievementService::getById(achievementId);
    if (achievement.id == 0) {
        ToastNotification::display(this, zh("未找到对应成果记录。"));
        return;
    }

    AchievementEditorDialog dialog(this);
    dialog.setWindowTitle(zh("编辑成果"));
    dialog.setAchievement(achievement);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Achievement updated = dialog.achievement();
    const Achievement saved = AchievementService::update(achievementId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, zh("成果更新失败。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Achievements);
    ToastNotification::display(this, zh("成果已更新。"));
}

void AchievementsPage::onRemoveClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一条成果。"));
        return;
    }

    const int achievementId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (achievementId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能删除。"));
        return;
    }

    const QString title = m_list->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(
        this,
        zh("删除成果"),
        zh("确定要删除成果“%1”吗？此操作会影响总览、时间轴和简历。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!AchievementService::remove(achievementId)) {
        ToastNotification::display(this, zh("成果删除失败，请稍后再试。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Achievements);
    ToastNotification::display(this, zh("成果已删除。"));
}

void AchievementsPage::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item && item->data(Qt::UserRole).toInt() > 0) {
        onEditClicked();
    }
}
