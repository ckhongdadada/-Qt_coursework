#include "ImportsPage.h"
#include "service/ImportService.h"
#include "utils/UiHelpers.h"
using namespace UiHelpers;
#include "widgets/ToastNotification.h"

#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ImportsPage::ImportsPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void ImportsPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(zh("数据导入"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(zh("支持从外部 CSV 文件批量导入课程、角色、成果、经历、活动、目标和对标同学数据。"), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    auto* controlCard = new QFrame(this);
    controlCard->setObjectName("contentCard");
    auto* controlLayout = new QVBoxLayout(controlCard);
    controlLayout->setContentsMargins(16, 14, 16, 14);
    controlLayout->setSpacing(12);

    auto* form = new QFormLayout();
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(12);

    m_entityCombo = new QComboBox(controlCard);
    m_entityCombo->addItem(zh("课程数据"), QStringLiteral("courses"));
    m_entityCombo->addItem(zh("角色职责"), QStringLiteral("roles"));
    m_entityCombo->addItem(zh("成果记录"), QStringLiteral("achievements"));
    m_entityCombo->addItem(zh("实践经历"), QStringLiteral("experiences"));
    m_entityCombo->addItem(zh("课外活动"), QStringLiteral("activities"));
    m_entityCombo->addItem(zh("目标数据"), QStringLiteral("goals"));
    m_entityCombo->addItem(zh("对标同学"), QStringLiteral("peers"));
    form->addRow(zh("选择要导入的数据类别："), m_entityCombo);

    m_fileLabel = new QLabel(zh("尚未选择文件"), controlCard);
    m_fileLabel->setObjectName("richCardText");
    m_fileLabel->setWordWrap(true);

    auto* chooseButton = new QPushButton(zh("选择 CSV 文件"), controlCard);
    connect(chooseButton, &QPushButton::clicked, this, &ImportsPage::onChooseFileClicked);

    auto* fileRow = new QHBoxLayout();
    fileRow->setSpacing(10);
    fileRow->addWidget(m_fileLabel, 1);
    fileRow->addWidget(chooseButton);
    form->addRow(zh("选择数据源文件："), fileRow);

    controlLayout->addLayout(form);

    auto* runButton = new QPushButton(zh("开始导入"), controlCard);
    connect(runButton, &QPushButton::clicked, this, &ImportsPage::onRunImportClicked);
    controlLayout->addWidget(runButton, 0, Qt::AlignRight);
    layout->addWidget(controlCard);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(zh("成功导入条数"), &m_importedValue), 0, 0);
    metrics->addWidget(createMetricCard(zh("导入失败条数"), &m_failedValue), 0, 1);
    layout->addLayout(metrics);

    auto* errorCard = new QFrame(this);
    errorCard->setObjectName("contentCard");
    auto* errorLayout = new QVBoxLayout(errorCard);
    errorLayout->setContentsMargins(16, 14, 16, 14);
    errorLayout->setSpacing(10);

    auto* errorTitle = new QLabel(zh("导入失败明细"), errorCard);
    errorTitle->setObjectName("sectionTitle");
    errorLayout->addWidget(errorTitle);

    m_errorTable = new QTableWidget(errorCard);
    m_errorTable->setColumnCount(2);
    m_errorTable->setHorizontalHeaderLabels({zh("出错行号"), zh("错误原因")});
    m_errorTable->horizontalHeader()->setStretchLastSection(true);
    m_errorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_errorTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_errorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    errorLayout->addWidget(m_errorTable, 1);

    layout->addWidget(errorCard, 1);
}

void ImportsPage::refresh()
{
    if (m_importedValue) {
        m_importedValue->setText(QStringLiteral("0"));
    }
    if (m_failedValue) {
        m_failedValue->setText(QStringLiteral("0"));
    }
    if (m_errorTable) {
        m_errorTable->setRowCount(0);
    }
}

void ImportsPage::onChooseFileClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        zh("选择数据文件"),
        QDir::homePath(),
        zh("CSV 文件 (*.csv);;所有文件 (*.*)"));

    if (!path.isEmpty()) {
        m_filePath = path;
        m_fileLabel->setText(path);
    }
}

void ImportsPage::onRunImportClicked()
{
    if (m_filePath.isEmpty()) {
        ToastNotification::display(this, zh("请先选择数据源文件。"));
        return;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        ToastNotification::display(this, zh("无法读取文件，请检查文件权限。"));
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QString entity = m_entityCombo ? m_entityCombo->currentData().toString() : QString();
    const QJsonObject result = ImportService::importData(entity, data, m_filePath);

    if (result.contains("error") && result.value("error").toBool()) {
        ToastNotification::display(this, zh("导入失败：") + result.value("message").toString());
        return;
    }

    const int imported = result.value("imported").toInt();
    const int failed = result.value("failed").toInt();
    m_importedValue->setText(QString::number(imported));
    m_failedValue->setText(QString::number(failed));

    const QJsonArray errors = result.value("errors").toArray();
    m_errorTable->setRowCount(errors.size());
    for (int index = 0; index < errors.size(); ++index) {
        const QJsonObject error = errors.at(index).toObject();
        m_errorTable->setItem(index, 0, new QTableWidgetItem(zh("第 %1 行").arg(error.value("row").toInt())));
        m_errorTable->setItem(index, 1, new QTableWidgetItem(error.value("error").toString()));
    }

    m_summaryLabel->setText(
        zh("文件处理完成：成功导入 %1 条，并同步刷新了各模块数据。").arg(imported));

    emit importCompleted();
    emit dataChanged(DataDomain::All);
    ToastNotification::display(this, zh("导入完成，共写入 %1 条数据。").arg(imported));
}
