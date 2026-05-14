#include "CrudPageShell.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QKeySequence>

CrudPageShell::CrudPageShell(QWidget* parent)
    : QFrame(parent)
{
    setupUi();
}

void CrudPageShell::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("pageTitle");
    layout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_subtitleLabel);

    m_filterWidget = new QWidget(this);
    m_filterWidget->setObjectName("contentCard");
    m_filterWidget->setStyleSheet("QWidget#contentCard { background: #ffffff; border: 1px solid #e7edf7; border-radius: 10px; }");
    layout->addWidget(m_filterWidget);

    m_actionWidget = new QWidget(this);
    QHBoxLayout* actionLayout = new QHBoxLayout(m_actionWidget);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(10);
    layout->addWidget(m_actionWidget);

    QFrame* tableCard = new QFrame(this);
    tableCard->setObjectName("contentCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(12, 12, 12, 12);
    cardLayout->setSpacing(10);

    m_helperLabel = new QLabel(tableCard);
    m_helperLabel->setObjectName("pageSubtitle");
    m_helperLabel->setWordWrap(true);
    m_helperLabel->hide();
    cardLayout->addWidget(m_helperLabel);

    m_table = new QTableWidget(tableCard);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    cardLayout->addWidget(m_table);

    layout->addWidget(tableCard, 1);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int row, int col) {
        Q_UNUSED(col);
        int id = currentRowId();
        if (id > 0) emit rowDoubleClicked(id);
    });
}

void CrudPageShell::setPageTitle(const QString& title)
{
    m_titleLabel->setText(title);
}

void CrudPageShell::setPageSubtitle(const QString& subtitle)
{
    m_subtitleLabel->setText(subtitle);
}

void CrudPageShell::setFilterFields(const QStringList& placeholders)
{
    qDeleteAll(m_filterInputs);
    m_filterInputs.clear();

    delete m_filterWidget->layout();
    QGridLayout* grid = new QGridLayout(m_filterWidget);
    grid->setContentsMargins(12, 12, 12, 12);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(10);

    for (int i = 0; i < placeholders.size(); ++i) {
        QLineEdit* input = new QLineEdit(m_filterWidget);
        input->setPlaceholderText(placeholders[i]);
        grid->addWidget(input, i / 2, i % 2);
        m_filterInputs.append(input);
        connect(input, &QLineEdit::textChanged, this, &CrudPageShell::filterChanged);
    }
}

void CrudPageShell::setActionButtons(const QStringList& labels, bool addDangerOnLast)
{
    delete m_actionWidget->layout();
    QHBoxLayout* actionLayout = new QHBoxLayout(m_actionWidget);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(10);

    for (int i = 0; i < labels.size(); ++i) {
        QPushButton* btn = new QPushButton(labels[i], m_actionWidget);
        if (addDangerOnLast && i == labels.size() - 1) {
            btn->setProperty("danger", true);
        }
        if (i == 0) {
            btn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
            connect(btn, &QPushButton::clicked, this, &CrudPageShell::addActionClicked);
        } else if (i == 1) {
            connect(btn, &QPushButton::clicked, this, &CrudPageShell::editActionClicked);
        } else if (i == 2) {
            connect(btn, &QPushButton::clicked, this, &CrudPageShell::removeActionClicked);
        }
        actionLayout->addWidget(btn);
    }
    actionLayout->addStretch();
}

void CrudPageShell::setTableHeaders(const QStringList& headers)
{
    m_emptyColumnSpan = headers.size();
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);
}

void CrudPageShell::setTableData(const QList<QStringList>& rows, const QList<int>& ids)
{
    m_table->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        const QStringList& values = rows[r];
        for (int c = 0; c < values.size() && c < m_table->columnCount(); ++c) {
            auto* item = new QTableWidgetItem(values.at(c));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, (r < ids.size()) ? ids[r] : 0);
            m_table->setItem(r, c, item);
        }
    }

    if (!rows.isEmpty()) {
        m_table->selectRow(0);
    } else {
        setEmptyMessage("暂无数据");
    }
}

void CrudPageShell::setEmptyMessage(const QString& msg)
{
    if (m_table->rowCount() == 0 || (m_table->rowCount() == 1 && m_table->item(0, 0) && m_table->item(0, 0)->data(Qt::UserRole).toInt() == 0)) {
        m_table->setRowCount(1);
        m_table->setSpan(0, 0, 1, m_emptyColumnSpan > 0 ? m_emptyColumnSpan : 1);
        auto* emptyItem = new QTableWidgetItem(QString("\n\n%1\n").arg(msg));
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(12); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_table->setItem(0, 0, emptyItem);
    }
}

void CrudPageShell::setHelperText(const QString& text)
{
    m_helperLabel->setText(text);
    m_helperLabel->setVisible(!text.isEmpty());
}

int CrudPageShell::currentRowId() const
{
    int row = m_table->currentRow();
    if (row < 0) return 0;
    auto* item = m_table->item(row, 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

QLineEdit* CrudPageShell::filterField(int index) const
{
    return (index >= 0 && index < m_filterInputs.size()) ? m_filterInputs[index] : nullptr;
}

QTableWidget* CrudPageShell::table() const
{
    return m_table;
}
