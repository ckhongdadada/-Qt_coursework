#include "NavigationListWidget.h"

#include <QApplication>
#include <QListWidgetItem>
#include <QListView>
#include <QStyle>

namespace {

QStringList navBaseLabels()
{
    return {
        "总览",
        "课程库",
        "角色职责",
        "成果记录",
        "经历档案",
        "课外活动",
        "目标追踪",
        "目标岗位",
        "分析报告",
        "时间轴",
        "简历导出",
        "数据导入"
    };
}

QList<QStyle::StandardPixmap> navIconKinds()
{
    return {
        QStyle::SP_DirHomeIcon,
        QStyle::SP_FileIcon,
        QStyle::SP_FileDialogInfoView,
        QStyle::SP_DialogApplyButton,
        QStyle::SP_DirOpenIcon,
        QStyle::SP_FileDialogContentsView,
        QStyle::SP_ArrowRight,
        QStyle::SP_DesktopIcon,
        QStyle::SP_FileDialogDetailedView,
        QStyle::SP_BrowserReload,
        QStyle::SP_DialogSaveButton,
        QStyle::SP_DialogOpenButton
    };
}

QIcon navIconForIndex(int index)
{
    const QList<QStyle::StandardPixmap> icons = navIconKinds();
    const QStyle::StandardPixmap fallback = QStyle::SP_FileIcon;
    return QApplication::style()->standardIcon(icons.value(index, fallback));
}

}

NavigationListWidget::NavigationListWidget(QWidget* parent)
    : QListWidget(parent)
    , m_labels(navBaseLabels())
    , m_tooltips(navBaseLabels())
{
    setObjectName("navList");
    setProperty("collapsed", false);
    setSpacing(0);
    setIconSize(QSize(20, 20));
    setViewMode(QListView::ListMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setWrapping(false);
    setupItems();

    connect(this, &QListWidget::currentRowChanged, this, &NavigationListWidget::navigationRequested);
    connect(this, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item) return;
        const int row = this->row(item);
        if (row >= 0 && currentRow() != row) {
            setCurrentRow(row);
        } else {
            viewport()->update();
        }
    });
}

void NavigationListWidget::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed) return;
    m_collapsed = collapsed;
    setProperty("collapsed", collapsed);
    rebuildItems();
    style()->unpolish(this);
    style()->polish(this);
    if (currentItem()) {
        setCurrentItem(currentItem());
    }
    viewport()->update();
}

bool NavigationListWidget::isCollapsed() const { return m_collapsed; }

void NavigationListWidget::setCurrentNavIndex(int index)
{
    if (index >= 0 && index < count()) {
        setCurrentRow(index);
    }
}

int NavigationListWidget::currentNavIndex() const { return currentRow(); }

void NavigationListWidget::rebuildItems()
{
    for (int i = 0; i < count(); ++i) {
        if (auto* item = this->item(i)) {
            if (m_collapsed) {
                item->setText(QString());
                item->setTextAlignment(Qt::AlignCenter);
                item->setSizeHint(QSize(0, 36));
            } else {
                item->setText(m_labels.value(i));
                item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                item->setSizeHint(QSize(0, 42));
            }
            item->setIcon(navIconForIndex(i));
        }
    }

    if (m_collapsed) {
        setViewMode(QListView::IconMode);
        setFlow(QListView::TopToBottom);
        setGridSize(QSize(40, 40));
        setSpacing(4);
    } else {
        setViewMode(QListView::ListMode);
        setGridSize(QSize());
        setSpacing(0);
    }
}

void NavigationListWidget::syncSelectionStyle()
{
    style()->unpolish(this);
    style()->polish(this);
    viewport()->update();
}

void NavigationListWidget::setupItems()
{
    clear();
    for (int i = 0; i < m_labels.size(); ++i) {
        auto* item = new QListWidgetItem(m_labels.at(i), this);
        item->setIcon(navIconForIndex(i));
        item->setToolTip(m_tooltips.value(i));
        item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        item->setSizeHint(QSize(0, 42));
    }
}
