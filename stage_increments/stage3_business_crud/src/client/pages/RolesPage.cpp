#include "RolesPage.h"
#include "dialogs/RoleEditorDialog.h"
#include "service/RoleService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;
#include "widgets/ToastNotification.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>

RolesPage::RolesPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void RolesPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("角色职责"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(zh("正在读取角色数据..."), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    auto* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    auto* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText(zh("搜索角色名称 / 组织 / 职责说明"));

    m_typeFilter = new QComboBox(filterCard);
    m_typeFilter->addItem(zh("全部类型"), "");
    m_typeFilter->addItem(zh("学生干部"), zh("学生干部"));
    m_typeFilter->addItem(zh("社团负责人"), zh("社团负责人"));
    m_typeFilter->addItem(zh("项目负责人"), zh("项目负责人"));
    m_typeFilter->addItem(zh("志愿者"), zh("志愿者"));
    m_typeFilter->addItem(zh("助教"), zh("助教"));
    m_typeFilter->addItem(zh("其他"), zh("其他"));

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_typeFilter, 0, 1);

    connect(m_searchInput, &QLineEdit::textChanged, this, &RolesPage::refresh);
    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RolesPage::refresh);
    layout->addWidget(filterCard);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    auto* addButton = new QPushButton(zh("新增角色"), this);
    auto* editButton = new QPushButton(zh("编辑选中角色"), this);
    auto* removeButton = new QPushButton(zh("删除选中角色"), this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &RolesPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &RolesPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &RolesPage::onRemoveClicked);

    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(zh("角色总数"), &m_totalValue), 0, 0);
    metrics->addWidget(createMetricCard(zh("进行中角色"), &m_activeValue), 0, 1);
    metrics->addWidget(createMetricCard(zh("角色类型数"), &m_typeValue, zh("按 type 字段统计")), 0, 2);
    layout->addLayout(metrics);

    auto* listCard = new QFrame(this);
    listCard->setObjectName("contentCard");
    auto* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);

    auto* listTitle = new QLabel(zh("职责记录"), listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);

    auto* helper = new QLabel(zh("班委、社团、项目和志愿服务中的角色职责都可以在这里维护，双击条目即可编辑。"), listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);

    m_list = new QListWidget(listCard);
    m_list->setObjectName("plainList");
    m_list->setWordWrap(true);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &RolesPage::onItemDoubleClicked);
    listLayout->addWidget(m_list);

    layout->addWidget(listCard, 1);
}

void RolesPage::refresh()
{
    const QList<Role> roles = RoleService::getAll();
    const QString keyword = m_searchInput ? m_searchInput->text().trimmed().toLower() : QString();
    const QString typeFilter = m_typeFilter ? m_typeFilter->currentData().toString().trimmed().toLower() : QString();

    QList<Role> filteredRoles;
    QSet<QString> filteredTypes;
    int filteredActiveCount = 0;
    for (const Role& role : roles) {
        const bool matchSearch = keyword.isEmpty()
            || role.title.toLower().contains(keyword)
            || role.organization.toLower().contains(keyword)
            || role.description.toLower().contains(keyword)
            || role.type.toLower().contains(keyword);
        const bool matchType = typeFilter.isEmpty() || role.type.toLower().contains(typeFilter);
        if (matchSearch && matchType) {
            filteredRoles.append(role);
            if (role.isActive) {
                ++filteredActiveCount;
            }
            if (!role.type.trimmed().isEmpty()) {
                filteredTypes.insert(role.type.trimmed());
            }
        }
    }

    if (m_totalValue) {
        m_totalValue->setText(QString::number(filteredRoles.size()));
    }
    if (m_activeValue) {
        m_activeValue->setText(QString::number(filteredActiveCount));
    }
    if (m_typeValue) {
        m_typeValue->setText(QString::number(filteredTypes.size()));
    }

    m_list->clear();
    for (const Role& role : filteredRoles) {
        const QString meta = zh("%1 · %2")
            .arg(joinDateRange(role.startDate, role.endDate, role.isActive, zh("至今")))
            .arg(safeText(role.organization, safeText(role.type, zh("未分类"))));
        const QString body = shortBody(
            role.description,
            role.isActive ? zh("当前角色仍在进行中。") : zh("该角色阶段已完成。"));
        const QString summary = zh("%1\n%2\n%3")
            .arg(safeText(role.title, zh("未命名角色")))
            .arg(meta)
            .arg(body);

        auto* item = new QListWidgetItem(summary, m_list);
        item->setData(Qt::UserRole, role.id);
    }

    if (m_list->count() == 0) {
        setupEmptyState(m_list, zh("暂无角色职责数据"));
    } else {
        m_list->setCurrentRow(0);
    }

    m_summaryLabel->setText(
        zh("显示 %1 / %2 个角色，其中进行中 %3 个。支持搜索和类型筛选。")
            .arg(filteredRoles.size())
            .arg(roles.size())
            .arg(filteredActiveCount));
}

void RolesPage::onAddClicked()
{
    RoleEditorDialog dialog(this);
    dialog.setWindowTitle(zh("新增角色"));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Role role = dialog.role();
    const Role created = RoleService::create(role);
    if (created.id == 0) {
        ToastNotification::display(this, zh("角色未能成功写入数据库。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Roles);
    ToastNotification::display(this, zh("角色已创建。"));
}

void RolesPage::onEditClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一个角色。"));
        return;
    }

    const int roleId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能编辑。"));
        return;
    }

    Role role = RoleService::getById(roleId);
    if (role.id == 0) {
        ToastNotification::display(this, zh("未找到对应角色记录。"));
        return;
    }

    RoleEditorDialog dialog(this);
    dialog.setWindowTitle(zh("编辑角色"));
    dialog.setRole(role);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Role updated = dialog.role();
    const Role saved = RoleService::update(roleId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, zh("角色更新失败。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Roles);
    ToastNotification::display(this, zh("角色已更新。"));
}

void RolesPage::onRemoveClicked()
{
    if (!m_list || !m_list->currentItem()) {
        ToastNotification::display(this, zh("请先选择一个角色。"));
        return;
    }

    const int roleId = m_list->currentItem()->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        ToastNotification::display(this, zh("当前项是占位信息，暂时不能删除。"));
        return;
    }

    const QString title = m_list->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(
        this,
        zh("删除角色"),
        zh("确定要删除角色“%1”吗？此操作会影响总览、时间轴和简历。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!RoleService::remove(roleId)) {
        ToastNotification::display(this, zh("角色删除失败，请稍后再试。"));
        return;
    }

    refresh();
    emit dataChanged(DataDomain::Roles);
    ToastNotification::display(this, zh("角色已删除。"));
}

void RolesPage::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item && item->data(Qt::UserRole).toInt() > 0) {
        onEditClicked();
    }
}

void RolesPage::updateSummary()
{
    refresh();
}
