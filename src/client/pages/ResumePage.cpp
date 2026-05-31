#include "ResumePage.h"

#include "widgets/ResumeEditorPanel.h"
#include "widgets/ResumeCandidatePanel.h"
#include "widgets/ResumePreviewDialog.h"
#include "service/ResumeService.h"
#include "utils/UiHelpers.h"
#include "widgets/ToastNotification.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

using namespace UiHelpers;

ResumePage::ResumePage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    resetOptions();
    m_candidatePanel->refreshCandidates();

    QSettings settings;
    m_avatarPath = settings.value("profile/avatar").toString();

    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(300);
    connect(m_updateTimer, &QTimer::timeout, this, &ResumePage::updatePreview);
}

void ResumePage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    auto* title = new QLabel(QString::fromUtf8("简历导出"), this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel(QString::fromUtf8("当前预览基于配置区实时生成，点击“预览简历”即可在独立窗口查看最新效果。"), this);
    m_summaryLabel->setObjectName("pageSubtitle");
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    auto* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard(QString::fromUtf8("简历分区数"), &m_sectionCountValue), 0, 0);
    metrics->addWidget(createMetricCard(QString::fromUtf8("身份标题"), &m_identityValue, QString::fromUtf8("来自简历生成配置")), 0, 1);
    layout->addLayout(metrics);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    m_previewButton = new QPushButton(QString::fromUtf8("预览简历"), this);
    auto* refreshButton = new QPushButton(QString::fromUtf8("刷新预览"), this);
    auto* resetButton = new QPushButton(QString::fromUtf8("恢复默认配置"), this);
    auto* exportJsonButton = new QPushButton(QString::fromUtf8("导出 JSON"), this);
    auto* exportHtmlButton = new QPushButton(QString::fromUtf8("导出 HTML"), this);
    auto* copyToClipboardButton = new QPushButton(QString::fromUtf8("复制到剪贴板"), this);

    connect(m_previewButton, &QPushButton::clicked, this, &ResumePage::onPreviewClicked);
    connect(refreshButton, &QPushButton::clicked, this, &ResumePage::onRefreshClicked);
    connect(resetButton, &QPushButton::clicked, this, &ResumePage::onResetClicked);
    connect(exportJsonButton, &QPushButton::clicked, this, &ResumePage::onExportJsonClicked);
    connect(exportHtmlButton, &QPushButton::clicked, this, &ResumePage::onExportHtmlClicked);
    connect(copyToClipboardButton, &QPushButton::clicked, this, &ResumePage::onCopyToClipboardClicked);

    actionLayout->addWidget(m_previewButton);
    actionLayout->addWidget(refreshButton);
    actionLayout->addWidget(resetButton);
    actionLayout->addWidget(exportJsonButton);
    actionLayout->addWidget(exportHtmlButton);
    actionLayout->addWidget(copyToClipboardButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    m_editorPanel = new ResumeEditorPanel(this);
    m_candidatePanel = new ResumeCandidatePanel(this);

    auto* bodySplitter = new QSplitter(Qt::Horizontal, this);
    bodySplitter->setChildrenCollapsible(false);
    bodySplitter->setHandleWidth(6);
    bodySplitter->setStyleSheet(
        "QSplitter::handle { background: #e5e7eb; border-radius: 3px; }"
        "QSplitter::handle:hover { background: #93c5fd; }"
        "QSplitter::handle:pressed { background: #3b82f6; }");
    bodySplitter->addWidget(m_editorPanel);
    bodySplitter->addWidget(m_candidatePanel);
    bodySplitter->setSizes({760, 280});
    layout->addWidget(bodySplitter, 1);

    connect(m_editorPanel, &ResumeEditorPanel::optionsChanged, this, &ResumePage::onOptionsChanged);
    connect(m_candidatePanel, &ResumeCandidatePanel::insertToSectionRequested, this, &ResumePage::onInsertToSectionRequested);
    connect(m_candidatePanel, &ResumeCandidatePanel::clearCustomContentRequested, this, &ResumePage::onClearCustomContentRequested);
    connect(m_candidatePanel, &ResumeCandidatePanel::candidateClicked, this, &ResumePage::onCandidateClicked);
}

void ResumePage::refresh()
{
    updatePreview();
}

QJsonObject ResumePage::defaultOptions() const
{
    QSettings settings;
    QJsonObject options;
    options["name"] = settings.value("profile/name", QString::fromUtf8("个人发展档案")).toString();
    options["title"] = settings.value("profile/title", QString::fromUtf8("个人成长规划简历")).toString();
    options["email"] = settings.value("profile/email", "").toString();
    options["phone"] = settings.value("profile/phone", "").toString();
    options["summary"] = settings.value("profile/summary", QString::fromUtf8("基于课程、经历、成果与目标自动生成的综合简历预览。")).toString();
    options["customContent"] = settings.value("profile/resumeCustomContent", "").toString();
    options["age"] = settings.value("profile/age", QString::fromUtf8("21岁")).toString();
    options["city"] = settings.value("profile/city", QString::fromUtf8("北京")).toString();
    options["intent"] = settings.value("profile/intent", QString::fromUtf8("大数据工程师")).toString();
    options["school"] = settings.value("profile/school", QString::fromUtf8("对外经济贸易大学")).toString();
    options["major"] = settings.value("profile/major", QString::fromUtf8("数据科学与大数据技术")).toString();
    options["degree"] = settings.value("profile/degree", QString::fromUtf8("本科")).toString();
    options["educationBody"] = settings.value("profile/educationBody", QString::fromUtf8("• GPA：85/100，专业排名前列\n• 核心课程：机器学习、数据分析、数据库系统\n• 荣誉：院级奖学金")).toString();
    options["skillsBody"] = settings.value("profile/skillsBody", QString::fromUtf8("• 熟悉 Python、C++、SQL 与数据分析流程\n• 能够使用 Qt、Flask 及前后端协作完成项目开发")).toString();
    options["projectName"] = settings.value("profile/projectName", QString::fromUtf8("个人发展规划系统")).toString();
    options["projectBody"] = settings.value("profile/projectBody", QString::fromUtf8("• 负责系统设计、模块拆分与界面实现\n• 完成课程、经历、目标、AI 助手等模块联动")).toString();
    options["sectionTitleIntent"] = settings.value("profile/sectionTitleIntent", QString::fromUtf8("求职意向")).toString();
    options["sectionTitleEducation"] = settings.value("profile/sectionTitleEducation", QString::fromUtf8("教育背景")).toString();
    options["sectionTitleSkills"] = settings.value("profile/sectionTitleSkills", QString::fromUtf8("技能特长")).toString();
    options["sectionTitleProjects"] = settings.value("profile/sectionTitleProjects", QString::fromUtf8("项目经验")).toString();
    options["sectionTitleInternship"] = settings.value("profile/sectionTitleInternship", QString::fromUtf8("实习经历")).toString();
    options["sectionTitleAwards"] = settings.value("profile/sectionTitleAwards", QString::fromUtf8("竞赛获奖")).toString();
    options["sectionTitleCustom"] = settings.value("profile/sectionTitleCustom", QString::fromUtf8("补充亮点")).toString();
    options["includeEducation"] = true;
    options["includeExperience"] = true;
    options["includeAchievements"] = true;
    options["includeRoles"] = true;
    options["includeActivities"] = false;
    return options;
}

QJsonObject ResumePage::currentOptions() const
{
    QJsonObject options = m_editorPanel->collectOptions();
    QSettings settings;
    options["sectionTitleIntent"] = settings.value("profile/sectionTitleIntent", QString::fromUtf8("求职意向")).toString();
    options["sectionTitleEducation"] = settings.value("profile/sectionTitleEducation", QString::fromUtf8("教育背景")).toString();
    options["sectionTitleSkills"] = settings.value("profile/sectionTitleSkills", QString::fromUtf8("技能特长")).toString();
    options["sectionTitleProjects"] = settings.value("profile/sectionTitleProjects", QString::fromUtf8("项目经验")).toString();
    options["sectionTitleInternship"] = settings.value("profile/sectionTitleInternship", QString::fromUtf8("实习经历")).toString();
    options["sectionTitleAwards"] = settings.value("profile/sectionTitleAwards", QString::fromUtf8("竞赛获奖")).toString();
    options["sectionTitleCustom"] = settings.value("profile/sectionTitleCustom", QString::fromUtf8("自我评价")).toString();
    return options;
}

void ResumePage::resetOptions()
{
    const QJsonObject options = defaultOptions();
    m_editorPanel->loadOptions(options);
    m_candidatePanel->refreshCandidates();
    updatePreview();
}

bool ResumePage::isVisibleSection(const QString& section) const
{
    const QJsonObject options = m_editorPanel->collectOptions();
    if (section == "education") return options["includeEducation"].toBool(true);
    if (section == "project") return options["includeExperience"].toBool(true);
    if (section == "skills") return options["includeAchievements"].toBool(true);
    if (section == "intent") return options["includeRoles"].toBool(true);
    if (section == "custom") return options["includeActivities"].toBool(false);
    return true;
}

QString ResumePage::selectedClass(const QString& key) const
{
    return m_selectedSection == key ? QString(" section-selected") : QString();
}

QString ResumePage::actionHtml(const QString& key) const
{
    return QString("<div class='resume-actions'><a href='copy:%1'>复制</a><a href='delete:%1'>删除</a></div>").arg(key);
}

QString ResumePage::buildResumePreviewHtml() const
{
    const QJsonObject options = currentOptions();
    const QString intentTitle = options["sectionTitleIntent"].toString();
    const QString eduTitle = options["sectionTitleEducation"].toString();
    const QString skillsTitle = options["sectionTitleSkills"].toString();
    const QString projectTitle = options["sectionTitleProjects"].toString();
    const QString internshipTitle = options["sectionTitleInternship"].toString();
    const QString awardsTitle = options["sectionTitleAwards"].toString();
    const QString customTitle = options["sectionTitleCustom"].toString();

    auto br = [](QString text) {
        return text.replace("\n", "<br>");
    };

    auto sectionRow = [this](const QString& key, const QString& title, const QString& body) {
        const QString border = (m_selectedSection == key)
            ? "border='1' bordercolor='#2f80ed'"
            : "border='0'";
        const QString background = (m_selectedSection == key) ? "#f8fbff" : "#ffffff";
        return QString(
            "<table width='100%' cellspacing='0' cellpadding='0' %1 style='background:%2; margin-top:14px;'>"
            "<tr>"
            "<td width='118' valign='top' style='padding:10px 18px 10px 0; border-top:1px solid #d7dde5;'>"
            "<a href='section:%3' style='text-decoration:none; color:#1f4e79;'>"
            "<font face='Microsoft YaHei' size='4' color='#1f4e79'><b>%4</b></font>"
            "</a>"
            "</td>"
            "<td valign='top' style='padding:10px 0 10px 0; border-top:1px solid #d7dde5;'>"
            "%5"
            "</td>"
            "</tr>"
            "</table>")
            .arg(border, background, key, title, body);
    };

    QString html;
    html += "<html><body bgcolor='#eef1f5'>";
    html += "<center>";
    html += "<table width='794' cellspacing='0' cellpadding='0' bgcolor='#ffffff' style='background:#ffffff;'>";
    html += "<tr><td style='padding:48px 58px 52px 58px;'>";

    html += "<table width='100%' cellspacing='0' cellpadding='0' style='border-bottom:2px solid #1f4e79;'>";
    html += "<tr>";
    html += "<td valign='top' style='padding-bottom:22px;'>";
    html += QString("<font face='Microsoft YaHei' size='6' color='#111111'><b>%1</b></font><br>")
                .arg(safeText(options["name"].toString(), QString::fromUtf8("个人成长规划简历")));
    html += QString("<p style='line-height:1.7; margin:10px 0 10px 0;'><font face='Microsoft YaHei' size='3' color='#4b5563'>%1</font></p>")
                .arg(br(safeText(options["summary"].toString(), QString::fromUtf8("用最简练的语言，精准突出与你目标岗位最相关的核心能力。"))));
    html += QString("<font face='Microsoft YaHei' size='3' color='#4b5563'>%1 &nbsp; | &nbsp; %2 &nbsp; | &nbsp; %3 &nbsp; | &nbsp; %4</font>")
                .arg(safeText(options["age"].toString(), QString::fromUtf8("年龄待补充")),
                     safeText(options["city"].toString(), QString::fromUtf8("城市待补充")),
                     safeText(options["phone"].toString(), QString::fromUtf8("电话待补充")),
                     safeText(options["email"].toString(), QString::fromUtf8("邮箱待补充")));
    html += "</td>";
    html += "<td width='116' align='right' valign='top' style='padding-bottom:22px;'>";
    if (!m_avatarPath.isEmpty() && QFile::exists(m_avatarPath)) {
        html += QString("<a href='action:avatar'><img width='96' height='118' src='%1'></a>")
                    .arg(QUrl::fromLocalFile(m_avatarPath).toString());
    } else {
        html += QString::fromUtf8("<table width='96' height='118' cellspacing='0' cellpadding='0' border='1' bordercolor='#d7dde5'><tr><td align='center' valign='middle'><a href='action:avatar' style='text-decoration:none; color:#7b8794;'><font face='Microsoft YaHei' size='2' color='#7b8794'>点击<br>选择<br>照片</font></a></td></tr></table>");
    }
    html += "</td></tr></table>";

    if (isVisibleSection("intent")) {
        const QString body = QString("<font face='Microsoft YaHei' size='3' color='#333333'>职位：%1 &nbsp; | &nbsp; 方向：%2 &nbsp; | &nbsp; 城市：%3</font>")
            .arg(safeText(options["title"].toString(), QString::fromUtf8("职位头衔")),
                 safeText(options["intent"].toString(), QString::fromUtf8("求职方向")),
                 safeText(options["city"].toString(), QString::fromUtf8("城市")));
        html += sectionRow("intent", intentTitle, body);
    }

    if (isVisibleSection("education")) {
        const QString body = QString(
            "<table width='100%' cellspacing='0' cellpadding='0'>"
            "<tr><td><font face='Microsoft YaHei' size='3' color='#111111'><b>%1</b></font></td>"
            "<td align='right'><font face='Microsoft YaHei' size='3' color='#555555'>%2</font></td></tr>"
            "<tr><td colspan='2'><font face='Microsoft YaHei' size='2' color='#6b7280'>%3</font></td></tr>"
            "<tr><td colspan='2' style='padding-top:8px; line-height:1.8;'><font face='Microsoft YaHei' size='3' color='#333333'>%4</font></td></tr>"
            "</table>")
            .arg(safeText(options["school"].toString(), QString::fromUtf8("学校待补充")),
                 safeText(options["degree"].toString(), QString::fromUtf8("学历待补充")),
                 safeText(options["major"].toString(), QString::fromUtf8("专业待补充")),
                 br(safeText(options["educationBody"].toString(), QString::fromUtf8("请补充教育背景描述。"))));
        html += sectionRow("education", eduTitle, body);
    }

    if (isVisibleSection("skills")) {
        const QString body = QString("<font face='Microsoft YaHei' size='3' color='#333333'>%1</font>")
            .arg(br(safeText(options["skillsBody"].toString(), QString::fromUtf8("请补充技能特长。"))));
        html += sectionRow("skills", skillsTitle, body);
    }

    if (isVisibleSection("project")) {
        const QString body = QString(
            "<font face='Microsoft YaHei' size='3' color='#111111'><b>%1</b></font><br>"
            "<font face='Microsoft YaHei' size='3' color='#333333'>%2</font>")
            .arg(safeText(options["projectName"].toString(), QString::fromUtf8("项目名称待补充")),
                 br(safeText(options["projectBody"].toString(), QString::fromUtf8("请补充项目经验。"))));
        html += sectionRow("project", projectTitle, body);
    }

    const QString internship = options["internship"].toString().trimmed();
    if (!internship.isEmpty()) {
        const QString internshipHtml = QString(internship).replace("\n", "<br/>");
        html += sectionRow("internship", internshipTitle, QString("<font face='Microsoft YaHei' size='3' color='#333333'>%1</font>").arg(internshipHtml));
    }

    const QString awards = options["awards"].toString().trimmed();
    if (!awards.isEmpty()) {
        const QString awardsHtml = QString(awards).replace("\n", "<br/>");
        html += sectionRow("awards", awardsTitle, QString("<font face='Microsoft YaHei' size='3' color='#333333'>%1</font>").arg(awardsHtml));
    }

    if (isVisibleSection("custom") && !options["customContent"].toString().trimmed().isEmpty()) {
        html += sectionRow("custom", customTitle, QString("<font face='Microsoft YaHei' size='3' color='#333333'>%1</font>").arg(options["customContent"].toString().replace("\n", "<br/>")));
    }

    html += "</td></tr></table></center></body></html>";
    return html;
}

void ResumePage::refreshResumePreviewDialog()
{
    if (!m_previewDialog) {
        return;
    }
    m_previewDialog->setSelectedSection(m_selectedSection);
    m_previewDialog->setResumeHtml(buildResumePreviewHtml());
}

void ResumePage::updatePreview()
{
    const QJsonObject options = currentOptions();
    int sectionCount = 0;
    if (isVisibleSection("intent")) sectionCount++;
    if (isVisibleSection("education")) sectionCount++;
    if (isVisibleSection("skills")) sectionCount++;
    if (isVisibleSection("project")) sectionCount++;
    if (!options["internship"].toString().trimmed().isEmpty()) sectionCount++;
    if (!options["awards"].toString().trimmed().isEmpty()) sectionCount++;
    if (isVisibleSection("custom") && !options["customContent"].toString().trimmed().isEmpty()) sectionCount++;

    if (m_sectionCountValue) {
        m_sectionCountValue->setText(QString::number(sectionCount));
    }
    if (m_identityValue) {
        m_identityValue->setText(safeText(options["title"].toString(), QString::fromUtf8("个人成长规划简历")));
    }
    m_summaryLabel->setText(QString::fromUtf8("当前配置已更新，点击“预览简历”查看最新内容。"));

    if (m_previewDialog && m_previewDialog->isVisible()) {
        refreshResumePreviewDialog();
    }
}

void ResumePage::ensureResumePreviewDialog()
{
    if (m_previewDialog) {
        return;
    }

    m_previewDialog = new ResumePreviewDialog(this);
    connect(m_previewDialog, &ResumePreviewDialog::sectionSelected, this, &ResumePage::onSectionSelected);
    connect(m_previewDialog, &ResumePreviewDialog::avatarClicked, this, &ResumePage::onAvatarClicked);
    connect(m_previewDialog, &ResumePreviewDialog::refreshRequested, this, &ResumePage::onRefreshClicked);
    connect(m_previewDialog, &ResumePreviewDialog::copyRequested, this, &ResumePage::onCopyToClipboardClicked);
    connect(m_previewDialog, &ResumePreviewDialog::exportJsonRequested, this, &ResumePage::onExportJsonClicked);
    connect(m_previewDialog, &ResumePreviewDialog::exportHtmlRequested, this, &ResumePage::onExportHtmlClicked);
}

void ResumePage::showResumePreviewDialog()
{
    ensureResumePreviewDialog();
    refreshResumePreviewDialog();
    m_previewDialog->show();
    m_previewDialog->raise();
    m_previewDialog->activateWindow();
}

void ResumePage::onPreviewClicked()
{
    showResumePreviewDialog();
}

void ResumePage::onRefreshClicked()
{
    refresh();
    ToastNotification::display(this, QString::fromUtf8("简历预览已刷新。"));
}

void ResumePage::onResetClicked()
{
    resetOptions();
    ToastNotification::display(this, QString::fromUtf8("已恢复默认配置。"));
}

void ResumePage::onExportJsonClicked()
{
    const QString path = QFileDialog::getSaveFileName(this, QString::fromUtf8("导出 JSON 简历"), QDir::homePath() + "/resume.json", "JSON Files (*.json)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ToastNotification::display(this, QString::fromUtf8("无法写入 JSON 文件。"));
        return;
    }
    file.write(ResumeService::exportJson(currentOptions()));
    file.close();
    ToastNotification::display(this, QString::fromUtf8("JSON 简历已导出。"));
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void ResumePage::onExportHtmlClicked()
{
    const QString path = QFileDialog::getSaveFileName(this, QString::fromUtf8("导出 HTML 简历"), QDir::homePath() + "/resume.html", "HTML Files (*.html)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ToastNotification::display(this, QString::fromUtf8("无法写入 HTML 文件。"));
        return;
    }
    file.write(ResumeService::exportHtml(currentOptions()));
    file.close();
    ToastNotification::display(this, QString::fromUtf8("HTML 简历已导出。"));
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void ResumePage::onCopyToClipboardClicked()
{
    const QString html = QString::fromUtf8(ResumeService::exportHtml(currentOptions()));
    QGuiApplication::clipboard()->setText(html);
    ToastNotification::display(this, QString::fromUtf8("简历 HTML 已复制到剪贴板。"));
}

void ResumePage::onOptionsChanged()
{
    saveOptions();
    m_updateTimer->start();
}

void ResumePage::saveOptions()
{
    const QJsonObject options = m_editorPanel->collectOptions();
    QSettings settings;
    settings.setValue("profile/name", options["name"].toString());
    settings.setValue("profile/title", options["title"].toString());
    settings.setValue("profile/email", options["email"].toString());
    settings.setValue("profile/phone", options["phone"].toString());
    settings.setValue("profile/age", options["age"].toString());
    settings.setValue("profile/city", options["city"].toString());
    settings.setValue("profile/intent", options["intent"].toString());
    settings.setValue("profile/school", options["school"].toString());
    settings.setValue("profile/major", options["major"].toString());
    settings.setValue("profile/degree", options["degree"].toString());
    settings.setValue("profile/summary", options["summary"].toString());
    settings.setValue("profile/customContent", options["customContent"].toString());
    settings.setValue("profile/educationBody", options["educationBody"].toString());
    settings.setValue("profile/skillsBody", options["skillsBody"].toString());
    settings.setValue("profile/projectName", options["projectName"].toString());
    settings.setValue("profile/projectBody", options["projectBody"].toString());
    settings.setValue("profile/internship", options["internship"].toString());
    settings.setValue("profile/awards", options["awards"].toString());
}

void ResumePage::onSectionSelected(const QString& key)
{
    m_selectedSection = key;
    if (m_editorPanel) {
        m_editorPanel->setSelectedSection(key);
    }
    updatePreview();
}

void ResumePage::onAvatarClicked()
{
    QWidget* dialogParent = (m_previewDialog && m_previewDialog->isVisible()) ? static_cast<QWidget*>(m_previewDialog) : static_cast<QWidget*>(this);
    const QString filePath = QFileDialog::getOpenFileName(
        dialogParent,
        QString::fromUtf8("选择证件照"),
        QString(),
        "Images (*.png *.jpg *.jpeg)",
        nullptr,
        QFileDialog::DontUseNativeDialog);
    if (filePath.isEmpty()) {
        return;
    }

    m_avatarPath = filePath;
    QSettings settings;
    settings.setValue("profile/avatar", m_avatarPath);
    updatePreview();
    if (m_previewDialog && m_previewDialog->isVisible()) {
        m_previewDialog->raise();
        m_previewDialog->activateWindow();
    }
}

void ResumePage::onCandidateClicked(const QString& snippet)
{
    if (snippet.isEmpty()) {
        return;
    }
    QTextEdit* customContent = m_editorPanel->customContentInput();
    QString current = customContent->toPlainText().trimmed();
    if (!current.contains(snippet)) {
        if (!current.isEmpty()) {
            current += "\n";
        }
        current += snippet;
        customContent->setPlainText(current);
        ToastNotification::display(this, QString::fromUtf8("已插入到补充内容区，你可以继续修改。"));
    } else {
        ToastNotification::display(this, QString::fromUtf8("这条素材已经在补充内容区中了。"));
    }
}

void ResumePage::onInsertToSectionRequested(const QString& section)
{
    const QString snippet = m_candidatePanel->currentSnippet();
    if (snippet.isEmpty()) {
        ToastNotification::display(this, QString::fromUtf8("请先选择一条素材。"));
        return;
    }
    
    QTextEdit* targetEdit = nullptr;
    QString sectionName;
    
    if (section == "education") {
        targetEdit = m_editorPanel->educationBodyInput();
        sectionName = "教育背景";
    } else if (section == "project") {
        targetEdit = m_editorPanel->projectBodyInput();
        sectionName = "项目经验";
    } else if (section == "awards") {
        targetEdit = m_editorPanel->awardsInput();
        sectionName = "竞赛获奖";
    } else if (section == "custom") {
        targetEdit = m_editorPanel->customContentInput();
        sectionName = "补充内容";
    } else {
        targetEdit = m_editorPanel->customContentInput();
        sectionName = "补充内容";
    }
    
    if (!targetEdit) {
        ToastNotification::display(this, QString::fromUtf8("找不到目标编辑区域。"));
        return;
    }
    
    QString current = targetEdit->toPlainText().trimmed();
    if (!current.isEmpty()) {
        current += "\n";
    }
    current += snippet;
    targetEdit->setPlainText(current);
    ToastNotification::display(this, QString::fromUtf8("已插入到%1。").arg(sectionName));
}

void ResumePage::onClearCustomContentRequested()
{
    m_editorPanel->customContentInput()->clear();
}
