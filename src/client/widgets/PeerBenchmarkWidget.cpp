#include "PeerBenchmarkWidget.h"
#include "service/AnalyticsService.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPushButton>
#include <QHBoxLayout>

PeerBenchmarkWidget::PeerBenchmarkWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("contentCard");
    setupUi();
}

void PeerBenchmarkWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    QLabel* peerLabel = new QLabel("横向同学对比", this);
    peerLabel->setObjectName("sectionTitle");
    layout->addWidget(peerLabel);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(8);
    QPushButton* addBtn = new QPushButton("新增对照", this);
    QPushButton* editBtn = new QPushButton("编辑", this);
    QPushButton* delBtn = new QPushButton("删除", this);
    actionLayout->addWidget(addBtn);
    actionLayout->addWidget(editBtn);
    actionLayout->addWidget(delBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    m_peerTable = new QTableWidget(this);
    m_peerTable->setColumnCount(6);
    m_peerTable->setHorizontalHeaderLabels({"姓名", "专业", "学期", "GPA", "成果", "经历"});
    m_peerTable->horizontalHeader()->setStretchLastSection(true);
    m_peerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_peerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_peerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_peerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_peerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_peerTable, 1);

    connect(addBtn, &QPushButton::clicked, this, &PeerBenchmarkWidget::addPeerRequested);
    connect(editBtn, &QPushButton::clicked, this, [this]() {
        int id = currentPeerId();
        if (id > 0) emit editPeerRequested(id);
    });
    connect(delBtn, &QPushButton::clicked, this, [this]() {
        int id = currentPeerId();
        if (id > 0) emit removePeerRequested(id);
    });
    connect(m_peerTable, &QTableWidget::cellDoubleClicked, this, &PeerBenchmarkWidget::onCellDoubleClicked);
}

void PeerBenchmarkWidget::loadPeers(const QList<PeerBenchmark>& peers)
{
    m_peerCount = peers.size();
    m_peerTable->setRowCount(peers.size());
    for (int i = 0; i < peers.size(); ++i) {
        m_peerTable->setItem(i, 0, new QTableWidgetItem(peers[i].name));
        m_peerTable->setItem(i, 1, new QTableWidgetItem(peers[i].major));
        m_peerTable->setItem(i, 2, new QTableWidgetItem(peers[i].semester));
        m_peerTable->setItem(i, 3, new QTableWidgetItem(QString::number(peers[i].gpa, 'f', 2)));
        m_peerTable->setItem(i, 4, new QTableWidgetItem(QString::number(peers[i].achievementsCount)));
        m_peerTable->setItem(i, 5, new QTableWidgetItem(QString::number(peers[i].experiencesCount)));
        m_peerTable->item(i, 0)->setData(Qt::UserRole, peers[i].id);
    }
    if (peers.isEmpty()) {
        m_peerTable->setRowCount(1);
        m_peerTable->setSpan(0, 0, 1, 6);
        auto* emptyItem = new QTableWidgetItem("\n暂无同学对照数据\n点击[新增对照]添加同学信息\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(11); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_peerTable->setItem(0, 0, emptyItem);
    }
}

int PeerBenchmarkWidget::currentPeerId() const
{
    if (!m_peerTable || m_peerTable->currentRow() < 0) return 0;
    auto* item = m_peerTable->item(m_peerTable->currentRow(), 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

int PeerBenchmarkWidget::peerCount() const { return m_peerCount; }

void PeerBenchmarkWidget::onCellDoubleClicked(int row, int col)
{
    Q_UNUSED(col);
    if (row >= 0 && m_peerTable->item(row, 0) && m_peerTable->item(row, 0)->data(Qt::UserRole).toInt() > 0) {
        emit editPeerRequested(m_peerTable->item(row, 0)->data(Qt::UserRole).toInt());
    }
}
