#include "MainWindow.h"
#include "Version.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QListWidgetItem>
#include <QListView>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QPageSize>
#include <QTextDocument>
#include <QSettings>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QScreen>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QSplitter>
#include <QTextOption>

#include "server/HttpServer.h"
#include "service/AchievementService.h"
#include "service/ActivityService.h"
#include "service/AiService.h"
#include "service/AnalyticsService.h"
#include "service/CourseService.h"
#include "service/DashboardService.h"
#include "service/ExperienceService.h"
#include "service/GoalService.h"
#include "service/ImportService.h"
#include "service/JobService.h"
#include "service/ResumeService.h"
#include "service/RoleService.h"
#include "client/dialogs/ProfileEditorDialog.h"
#include "client/dialogs/CourseEditorDialog.h"
#include "client/dialogs/GoalEditorDialog.h"
#include "client/dialogs/RoleEditorDialog.h"
#include "client/dialogs/AchievementEditorDialog.h"
#include "client/dialogs/ActivityEditorDialog.h"
#include "client/dialogs/ExperienceEditorDialog.h"
#include "client/dialogs/JobEditorDialog.h"
#include "client/dialogs/PeerEditorDialog.h"
#include "client/widgets/SidebarWidget.h"
#include "client/widgets/AiPanelWidget.h"
#include "client/widgets/ToastNotification.h"
#include "client/pages/CoursesPage.h"
#include "client/pages/RolesPage.h"
#include "client/pages/AchievementsPage.h"
#include "client/pages/ExperiencesPage.h"
#include "client/pages/ActivitiesPage.h"
#include "client/pages/GoalsPage.h"
#include "client/pages/JobsPage.h"
#include "client/pages/AnalysisPage.h"
#include "client/pages/TimelinePage.h"
#include "client/pages/ResumePage.h"
#include "client/pages/OverviewPage.h"
#include "client/pages/ImportsPage.h"
#include "client/core/DataDomain.h"
#include "util/Logger.h"

namespace {

constexpr int kSidebarExpandedWidth = 260;
constexpr int kSidebarCollapsedWidth = 56;
constexpr int kAiSidebarWidth = 380;
constexpr int kAiCollapsedWidth = 50;
constexpr int kMaxContentWidth = 1080;

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

QStringList navCollapsedIcons()
{
    return {
        QString(QChar(0x2302)),
        QString(QChar(0x25A4)),
        QString(QChar(0x25C9)),
        QString(QChar(0x2605)),
        QString(QChar(0x25A3)),
        QString(QChar(0x273F)),
        QString(QChar(0x25CE)),
        QString(QChar(0x25C8)),
        QString(QChar(0x25A6)),
        QString(QChar(0x25F4)),
        QString(QChar(0x2263)),
        QString(QChar(0x21A5))
    };
}

QStringList navExpandedLabels()
{
    return navBaseLabels();
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

QString safeText(const QString& value, const QString& fallback = "未填写")
{
    const QString trimmed = value.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed;
}

QString shortBody(const QString& value, const QString& fallback)
{
    const QString text = value.simplified();
    return text.isEmpty() ? fallback : text;
}

QString bullet(const QString& value)
{
    return QString("• %1").arg(value);
}

QString joinDateRange(const QString& startDate, const QString& endDate, bool active, const QString& activeLabel)
{
    if (!startDate.isEmpty() && !endDate.isEmpty()) {
        return QString("%1 - %2").arg(startDate, endDate);
    }
    if (!startDate.isEmpty()) {
        return active ? QString("%1 - %2").arg(startDate, activeLabel) : startDate;
    }
    if (!endDate.isEmpty()) {
        return endDate;
    }
    return "时间未填写";
}

QJsonObject defaultResumeOptions()
{
    QSettings settings;
    QJsonObject options;
    options["name"] = settings.value("profile/name", "个人发展档案").toString();
    options["title"] = settings.value("profile/title", "个人成长规划简历").toString();
    options["email"] = settings.value("profile/email", "").toString();
    options["phone"] = settings.value("profile/phone", "").toString();
    options["summary"] = settings.value("profile/summary", "基于课程、经历、成果与目标自动生成的综合简历预览。").toString();
    options["customContent"] = settings.value("profile/resumeCustomContent", "").toString();
    options["age"] = settings.value("profile/age", "21岁").toString();
    options["city"] = settings.value("profile/city", "北京").toString();
    options["intent"] = settings.value("profile/intent", "大数据工程师").toString();
    options["school"] = settings.value("profile/school", "对外经济贸易大学").toString();
    options["major"] = settings.value("profile/major", "数据科学与大数据技术").toString();
    options["degree"] = settings.value("profile/degree", "本科").toString();
    options["educationBody"] = settings.value("profile/educationBody", "• GPA：85/100，专业排名前列\n• 核心课程：机器学习、数据分析、数据库系统\n• 荣誉：院级奖学金").toString();
    options["skillsBody"] = settings.value("profile/skillsBody", "• 熟悉 Python、C++、SQL 与数据分析流程\n• 能够使用 Qt、Flask 及前后端协作完成项目开发").toString();
    options["projectName"] = settings.value("profile/projectName", "个人发展规划系统").toString();
    options["projectBody"] = settings.value("profile/projectBody", "• 负责系统设计、模块拆分与界面实现\n• 完成课程、经历、目标、AI 助手等模块联动").toString();
    options["sectionTitleIntent"] = settings.value("profile/sectionTitleIntent", "求职意向").toString();
    options["sectionTitleEducation"] = settings.value("profile/sectionTitleEducation", "教育背景").toString();
    options["sectionTitleSkills"] = settings.value("profile/sectionTitleSkills", "技能特长").toString();
    options["sectionTitleProjects"] = settings.value("profile/sectionTitleProjects", "项目经验").toString();
    options["sectionTitleCustom"] = settings.value("profile/sectionTitleCustom", "补充亮点").toString();
    options["includeEducation"] = true;
    options["includeExperience"] = true;
    options["includeAchievements"] = true;
    options["includeRoles"] = true;
    options["includeActivities"] = false;
    return options;
}

QString htmlToPlainSummary(const QString& html)
{
    QString text = html;
    text.replace("<h1>", "").replace("</h1>", "\n");
    text.replace("<h2>", "\n").replace("</h2>", "\n");
    text.replace("<p>", "").replace("</p>", "\n");
    text.replace("<strong>", "").replace("</strong>", "");
    text.replace("<div class='item'>", "\n").replace("</div>", "\n");
    return text;
}

static void setupEmptyState(QListWidget* list, const QString& hint = "这里空空如也，快去添加第一笔记录吧~") {
    auto* item = new QListWidgetItem("\n\n\n馃摥\n" + hint + "\n\n\n");
    item->setTextAlignment(Qt::AlignCenter);
    QFont f = item->font(); f.setPointSize(12); item->setFont(f);
    item->setForeground(QBrush(QColor("#a8a096")));
    item->setFlags(Qt::NoItemFlags);
    list->addItem(item);
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_serverUrl("http://127.0.0.1:8080/")
{
    setWindowTitle(QString("学业发展规划系统 - Qt 桌面版 v%1").arg(PDP_VERSION));
    resize(1360, 860);
    setMinimumSize(1080, 720);

    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());

    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupSystemTray();
    applyWindowStyle();
    checkFrontendExists();
    startBackendServer();
    m_sidebar->refreshData();

    m_sidebar->navigationList()->setCurrentRow(0);
    refreshCurrentPage();
    insertSampleDataIfNeeded();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());

    if (m_serverThread) {
        m_serverThread->stop();
        m_serverThread->wait(3000);
        delete m_serverThread;
    }

    if (m_trayIcon) {
        m_trayIcon->hide();
        delete m_trayIcon;
    }
}

void MainWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Create and add sidebar widget
    m_sidebar = new SidebarWidget(this);
    rootLayout->addWidget(m_sidebar);

    QWidget* contentShell = new QWidget(this);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentShell);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    QFrame* topbar = new QFrame(contentShell);
    topbar->setObjectName("topbar");
    QHBoxLayout* topbarLayout = new QHBoxLayout(topbar);
    topbarLayout->setContentsMargins(32, 10, 32, 10);
    topbarLayout->setSpacing(16);

    QWidget* topbarLeft = new QWidget(topbar);
    QVBoxLayout* topbarTextLayout = new QVBoxLayout(topbarLeft);
    topbarTextLayout->setContentsMargins(0, 0, 0, 0);
    topbarTextLayout->setSpacing(2);

    m_topbarKicker = new QLabel("Personal development planning website", topbarLeft);
    m_topbarKicker->setObjectName("topbarKicker");
    topbarTextLayout->addWidget(m_topbarKicker);

    m_topbarTitle = new QLabel("个人发展规划工作台", topbarLeft);
    m_topbarTitle->setObjectName("topbarTitle");
    topbarTextLayout->addWidget(m_topbarTitle);
    topbarLayout->addWidget(topbarLeft);
    topbarLayout->addStretch();

    m_topbarPill = new QLabel("Knowledge base", topbar);
    m_topbarPill->setObjectName("topbarPill");
    topbarLayout->addWidget(m_topbarPill, 0, Qt::AlignVCenter);
    
    contentLayout->addWidget(topbar);

    QWidget* mainShell = new QWidget(contentShell);
    QVBoxLayout* mainShellLayout = new QVBoxLayout(mainShell);
    mainShellLayout->setContentsMargins(0, 0, 0, 0);
    mainShellLayout->setSpacing(0);
    mainShellLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_mainInner = new QWidget(mainShell);
    m_mainInner->setMaximumWidth(kMaxContentWidth);
    m_mainInner->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QVBoxLayout* mainInnerLayout = new QVBoxLayout(m_mainInner);
    mainInnerLayout->setContentsMargins(32, 28, 32, 36);
    mainInnerLayout->setSpacing(16);

    m_stack = new QStackedWidget(this);
    m_overviewPage = new OverviewPage(this);
    m_stack->addWidget(m_overviewPage);
    m_coursesPage = new CoursesPage(this);
    m_stack->addWidget(m_coursesPage);
    m_rolesPage = new RolesPage(this);
    m_stack->addWidget(m_rolesPage);
    m_achievementsPage = new AchievementsPage(this);
    m_stack->addWidget(m_achievementsPage);
    m_experiencesPage = new ExperiencesPage(this);
    m_stack->addWidget(m_experiencesPage);
    m_activitiesPage = new ActivitiesPage(this);
    m_stack->addWidget(m_activitiesPage);
    m_goalsPage = new GoalsPage(this);
    m_stack->addWidget(m_goalsPage);
    m_jobsPage = new JobsPage(this);
    m_stack->addWidget(m_jobsPage);
    m_analysisPage = new AnalysisPage(this);
    m_stack->addWidget(m_analysisPage);
    m_timelinePage = new TimelinePage(this);
    m_stack->addWidget(m_timelinePage);
    m_resumePage = new ResumePage(this);
    m_stack->addWidget(m_resumePage);
    m_importsPage = new ImportsPage(this);
    m_stack->addWidget(m_importsPage);
    mainInnerLayout->addWidget(m_stack, 1);

    QScrollArea* scrollArea = new QScrollArea(mainShell);
    scrollArea->setObjectName("mainScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    QWidget* scrollViewport = new QWidget(scrollArea);
    QHBoxLayout* scrollViewportLayout = new QHBoxLayout(scrollViewport);
    scrollViewportLayout->setContentsMargins(20, 0, 20, 0);
    scrollViewportLayout->setSpacing(0);
    m_leftStretchSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_rightStretchSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    scrollViewportLayout->addSpacerItem(m_leftStretchSpacer);
    scrollViewportLayout->addWidget(m_mainInner, 0, Qt::AlignTop);
    scrollViewportLayout->addSpacerItem(m_rightStretchSpacer);
    scrollArea->setWidget(scrollViewport);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    mainShellLayout->addWidget(scrollArea, 1);
    contentLayout->addWidget(mainShell, 1);

    rootLayout->addWidget(contentShell, 1);

    // Create and add AI panel widget
    m_aiPanel = new AiPanelWidget(this);
    rootLayout->addWidget(m_aiPanel);

    setCentralWidget(centralWidget);
    centralWidget->installEventFilter(this);

    // Connect sidebar navigation
    connect(m_sidebar->navigationList(), &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);

    // Connect page dataChanged signals for cross-page updates
    connect(m_coursesPage, &CoursesPage::dataChanged, this, [this](DataDomain domain) {
        if (m_overviewPage) m_overviewPage->refresh();
        if (domain == DataDomain::Courses && m_timelinePage) {
            m_timelinePage->refresh();
        }
    });
    connect(m_rolesPage, &RolesPage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });
    connect(m_achievementsPage, &AchievementsPage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });
    connect(m_experiencesPage, &ExperiencesPage::dataChanged, this, [this](DataDomain domain) {
        if (m_overviewPage) m_overviewPage->refresh();
        if (domain == DataDomain::Experiences && m_timelinePage) {
            m_timelinePage->refresh();
        }
    });
    connect(m_activitiesPage, &ActivitiesPage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });
    connect(m_goalsPage, &GoalsPage::dataChanged, this, [this](DataDomain domain) {
        if (m_overviewPage) m_overviewPage->refresh();
        if (domain == DataDomain::Goals && m_timelinePage) {
            m_timelinePage->refresh();
        }
    });
    connect(m_jobsPage, &JobsPage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });
    connect(m_analysisPage, &AnalysisPage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });
    connect(m_timelinePage, &TimelinePage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });
    connect(m_resumePage, &ResumePage::dataChanged, this, [this](DataDomain) { if (m_overviewPage) m_overviewPage->refresh(); });

    connect(m_importsPage, &ImportsPage::importCompleted, this, [this]() {
        if (m_overviewPage) m_overviewPage->refresh();
        if (m_coursesPage) m_coursesPage->refresh();
        if (m_rolesPage) m_rolesPage->refresh();
        if (m_achievementsPage) m_achievementsPage->refresh();
        if (m_experiencesPage) m_experiencesPage->refresh();
        if (m_activitiesPage) m_activitiesPage->refresh();
        if (m_goalsPage) m_goalsPage->refresh();
        if (m_jobsPage) m_jobsPage->refresh();
        if (m_timelinePage) m_timelinePage->refresh();
    });

    // Connect AI panel signals
    connect(m_aiPanel, &AiPanelWidget::applyToResumeRequested, this, [this](const QString& summary) {
        if (!m_resumePage) {
            ToastNotification::display(this, "简历配置区尚未准备好。");
            return;
        }
        if (summary.isEmpty()) {
            ToastNotification::display(this, "请先生成一段 AI 建议。");
            return;
        }
        if (m_sidebar && m_sidebar->navigationList()) {
            m_sidebar->navigationList()->setCurrentRow(10);
        }
        m_resumePage->refresh();
        ToastNotification::display(this, "已将 AI 建议写入简历摘要，你可以继续在简历页微调。");
    });

    connect(m_aiPanel, &AiPanelWidget::createGoalRequested, this, [this](const QString& title, const QString& description) {
        if (description.isEmpty()) {
            ToastNotification::display(this, "请先生成一段 AI 建议。");
            return;
        }

        GoalEditorDialog dialog(this);
        dialog.setWindowTitle("从 AI 建议生成目标");
        Goal draft;
        draft.title = title.isEmpty() ? "AI 建议跟进目标" : title;
        draft.category = "General";
        draft.description = description;
        draft.targetValue = 1;
        draft.currentValue = 0;
        draft.unit = "项";
        draft.priority = "High";
        draft.status = "In Progress";
        dialog.setGoal(draft);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        Goal goal = dialog.goal();
        const Goal created = GoalService::create(goal);
        if (created.id == 0) {
            ToastNotification::display(this, "未能根据 AI 建议创建目标。");
            return;
        }

        if (m_sidebar && m_sidebar->navigationList()) {
            m_sidebar->navigationList()->setCurrentRow(6);
        }
        if (m_goalsPage) m_goalsPage->refresh();
        if (m_overviewPage) m_overviewPage->refresh();
        if (m_timelinePage) m_timelinePage->refresh();
        ToastNotification::display(this, "已根据 AI 建议生成目标草稿。");
    });

    connect(m_aiPanel, &AiPanelWidget::analysisRequested, this, [this](const QString& type) {
        m_aiPanel->setOutput("正在分析中，请稍候...");
        QApplication::processEvents();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QJsonObject payload;
        payload["type"] = type;
        const QJsonObject result = AiService::analyze(payload);
        QApplication::restoreOverrideCursor();

        QStringList lines;
        lines << QString("分析类型：%1").arg(type);
        lines << QString("AI 模式：%1").arg(result["aiPowered"].toBool() ? "模型分析" : "规则引擎");

        const QJsonArray suggestions = result["suggestions"].toArray();
        if (!suggestions.isEmpty()) {
            lines << "";
            lines << "建议：";
            for (const auto& item : suggestions) {
                lines << QString("- %1").arg(item.toString());
            }
        } else if (result.contains("reply")) {
            lines << "";
            lines << result["reply"].toString();
        } else {
            lines << "";
            lines << "当前没有返回建议内容。";
        }

        m_aiPanel->setOutput(lines.join('\n'));
        m_aiPanel->refreshStatus();
    });

    connect(m_aiPanel, &AiPanelWidget::chatMessageSent, this, [this](const QString& message) {
        m_aiPanel->setOutput("AI 正在思考中...");
        QApplication::processEvents();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QJsonObject payload;
        payload["message"] = message;
        const QJsonObject result = AiService::chat(payload);
        QApplication::restoreOverrideCursor();

        QString output = QString("问题：%1\n\n答复：\n%2")
            .arg(message)
            .arg(result["reply"].toString());
        m_aiPanel->setOutput(output);
        m_aiPanel->refreshStatus();
    });
}

QFrame* MainWindow::createMetricCard(const QString& labelText, QLabel** valueLabel,
                                     const QString& helperText)
{
    QFrame* card = new QFrame(this);
    card->setObjectName("metricCard");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(6);

    QLabel* label = new QLabel(labelText, card);
    label->setObjectName("metricLabel");
    layout->addWidget(label);

    QLabel* value = new QLabel("--", card);
    value->setObjectName("metricValue");
    layout->addWidget(value);
    *valueLabel = value;

    if (!helperText.isEmpty()) {
        QLabel* helper = new QLabel(helperText, card);
        helper->setObjectName("metricHelper");
        helper->setWordWrap(true);
        layout->addWidget(helper);
    }

    return card;
}

QWidget* MainWindow::createOverviewPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("总览", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(
        "快速查看个人发展状态和近期建议。",
        page);
    subtitle->setObjectName("pageSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("课程总数", &m_totalCoursesValue), 0, 0);
    metrics->addWidget(createMetricCard("当前 GPA", &m_gpaValue), 0, 1);
    metrics->addWidget(createMetricCard("目标平均进度", &m_goalProgressValue), 0, 2);
    metrics->addWidget(createMetricCard("成果数量", &m_achievementValue), 0, 3);
    metrics->addWidget(createMetricCard("经历数量", &m_experienceValue), 1, 0);
    metrics->addWidget(createMetricCard("角色数量", &m_roleValue), 1, 1);
    metrics->addWidget(createMetricCard("活动数量", &m_activityValue), 1, 2);
    metrics->addWidget(
        createMetricCard("已修学分", &m_creditValue, "基于课程完成状态自动统计"), 1, 3);
    layout->addLayout(metrics);

    QHBoxLayout* lowerLayout = new QHBoxLayout();
    lowerLayout->setSpacing(14);

    QFrame* recommendationCard = new QFrame(page);
    recommendationCard->setObjectName("contentCard");
    QVBoxLayout* recommendationLayout = new QVBoxLayout(recommendationCard);
    recommendationLayout->setContentsMargins(16, 14, 16, 14);
    recommendationLayout->setSpacing(10);
    QLabel* recommendationTitle = new QLabel("近期建议", recommendationCard);
    recommendationTitle->setObjectName("sectionTitle");
    recommendationLayout->addWidget(recommendationTitle);
    m_recommendationList = new QListWidget(recommendationCard);
    m_recommendationList->setObjectName("plainList");
    m_recommendationList->setWordWrap(true);
    m_recommendationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recommendationLayout->addWidget(m_recommendationList);
    lowerLayout->addWidget(recommendationCard, 2);

    QFrame* semesterCard = new QFrame(page);
    semesterCard->setObjectName("contentCard");
    QVBoxLayout* semesterLayout = new QVBoxLayout(semesterCard);
    semesterLayout->setContentsMargins(16, 14, 16, 14);
    semesterLayout->setSpacing(10);
    QLabel* semesterTitle = new QLabel("学期走势", semesterCard);
    semesterTitle->setObjectName("sectionTitle");
    semesterLayout->addWidget(semesterTitle);
    m_semesterList = new QListWidget(semesterCard);
    m_semesterList->setObjectName("plainList");
    m_semesterList->setWordWrap(true);
    m_semesterList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    semesterLayout->addWidget(m_semesterList);
    lowerLayout->addWidget(semesterCard, 1);

    layout->addLayout(lowerLayout, 1);
    return page;
}

QWidget* MainWindow::createCoursesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("课程库", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_courseSummaryLabel = new QLabel("正在读取课程数据...", page);
    m_courseSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_courseSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_courseSearchInput = new QLineEdit(filterCard);
    m_courseSearchInput->setPlaceholderText("搜索课程名称 / 代码 / 教师");
    m_courseStatusInput = new QLineEdit(filterCard);
    m_courseStatusInput->setPlaceholderText("状态过滤，例如 Completed");
    m_courseCategoryInput = new QLineEdit(filterCard);
    m_courseCategoryInput->setPlaceholderText("类别过滤，例如 Required");
    m_courseSortInput = new QLineEdit(filterCard);
    m_courseSortInput->setPlaceholderText("排序：updated / semester / credits / score / gpa / name");
    filterLayout->addWidget(m_courseSearchInput, 0, 0);
    filterLayout->addWidget(m_courseStatusInput, 0, 1);
    filterLayout->addWidget(m_courseCategoryInput, 1, 0);
    filterLayout->addWidget(m_courseSortInput, 1, 1);
    connect(m_courseSearchInput, &QLineEdit::textChanged, this, [this]() { refreshCourses(); });
    connect(m_courseStatusInput, &QLineEdit::textChanged, this, [this]() { refreshCourses(); });
    connect(m_courseCategoryInput, &QLineEdit::textChanged, this, [this]() { refreshCourses(); });
    connect(m_courseSortInput, &QLineEdit::textChanged, this, [this]() { refreshCourses(); });
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增课程", page);
    QPushButton* editButton = new QPushButton("编辑选中课程", page);
    QPushButton* removeButton = new QPushButton("删除选中课程", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addCourse);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedCourse);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedCourse);
    addButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QFrame* tableCard = new QFrame(page);
    tableCard->setObjectName("contentCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(12, 12, 12, 12);
    cardLayout->setSpacing(10);

    QLabel* helper = new QLabel("课程库现在支持原生 Qt 录入与编辑。双击任意一行也可以直接打开编辑弹窗。", tableCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    cardLayout->addWidget(helper);

    m_courseTable = new QTableWidget(tableCard);
    m_courseTable->setColumnCount(7);
    m_courseTable->setHorizontalHeaderLabels(
        {"课程名称", "代码", "学期", "学分", "分数", "绩点", "状态"});
    m_courseTable->horizontalHeader()->setStretchLastSection(true);
    m_courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_courseTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_courseTable->verticalHeader()->setVisible(false);
    m_courseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_courseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_courseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_courseTable->setAlternatingRowColors(true);
    connect(m_courseTable, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedCourse();
    });
    cardLayout->addWidget(m_courseTable);

    layout->addWidget(tableCard, 1);
    return page;
}

QWidget* MainWindow::createRolesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("角色职责", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_roleSummaryLabel = new QLabel("正在读取角色数据...", page);
    m_roleSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_roleSummaryLabel);

    QFrame* roleFilterCard = new QFrame(page);
    roleFilterCard->setObjectName("contentCard");
    QGridLayout* roleFilterLayout = new QGridLayout(roleFilterCard);
    roleFilterLayout->setContentsMargins(12, 12, 12, 12);
    roleFilterLayout->setHorizontalSpacing(10);
    roleFilterLayout->setVerticalSpacing(10);
    m_roleSearchInput = new QLineEdit(roleFilterCard);
    m_roleSearchInput->setPlaceholderText("搜索角色名称 / 组织 / 描述");
    m_roleTypeFilter = new QComboBox(roleFilterCard);
    m_roleTypeFilter->addItem("全部类型", "");
    m_roleTypeFilter->addItem("任职", "任职");
    m_roleTypeFilter->addItem("班委", "班委");
    m_roleTypeFilter->addItem("社团", "社团");
    m_roleTypeFilter->addItem("团队", "团队");
    m_roleTypeFilter->addItem("实习", "实习");
    m_roleTypeFilter->addItem("其他", "其他");
    roleFilterLayout->addWidget(m_roleSearchInput, 0, 0);
    roleFilterLayout->addWidget(m_roleTypeFilter, 0, 1);
    connect(m_roleSearchInput, &QLineEdit::textChanged, this, [this]() { refreshRoles(); });
    connect(m_roleTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshRoles(); });
    layout->addWidget(roleFilterCard);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("角色总数", &m_rolesTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("进行中角色", &m_rolesActiveValue), 0, 1);
    metrics->addWidget(createMetricCard("角色类型数", &m_rolesTypeValue, "按 type 字段统计"), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增角色", page);
    QPushButton* editButton = new QPushButton("编辑选中角色", page);
    QPushButton* removeButton = new QPushButton("删除选中角色", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addRole);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedRole);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedRole);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("角色时间线", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("支持原生 Qt 维护任职和职责记录。双击条目即可直接编辑。", listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_roleList = new QListWidget(listCard);
    m_roleList->setObjectName("plainList");
    m_roleList->setWordWrap(true);
    m_roleList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_roleList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { editSelectedRole(); });
    listLayout->addWidget(m_roleList);
    layout->addWidget(listCard, 1);

    return page;
}

QWidget* MainWindow::createAchievementsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("�ɹ���¼", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_achievementSummaryLabel = new QLabel("���ڶ�ȡ�ɹ�����...", page);
    m_achievementSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_achievementSummaryLabel);

    QFrame* achFilterCard = new QFrame(page);
    achFilterCard->setObjectName("contentCard");
    QGridLayout* achFilterLayout = new QGridLayout(achFilterCard);
    achFilterLayout->setContentsMargins(12, 12, 12, 12);
    achFilterLayout->setHorizontalSpacing(10);
    achFilterLayout->setVerticalSpacing(10);
    m_achievementSearchInput = new QLineEdit(achFilterCard);
    m_achievementSearchInput->setPlaceholderText("搜索成果标题 / 机构 / 描述");
    m_achievementTypeFilter = new QComboBox(achFilterCard);
    m_achievementTypeFilter->addItem("全部类型", "");
    m_achievementTypeFilter->addItem("证书", "证书");
    m_achievementTypeFilter->addItem("竞赛", "竞赛");
    m_achievementTypeFilter->addItem("奖项", "奖项");
    m_achievementTypeFilter->addItem("课程成果", "课程成果");
    m_achievementTypeFilter->addItem("开源贡献", "开源贡献");
    m_achievementTypeFilter->addItem("论文报告", "论文报告");
    m_achievementTypeFilter->addItem("其他", "其他");
    m_achievementLevelFilter = new QComboBox(achFilterCard);
    m_achievementLevelFilter->addItem("全部级别", "");
    m_achievementLevelFilter->addItem("国家级", "国家级");
    m_achievementLevelFilter->addItem("省级", "省级");
    m_achievementLevelFilter->addItem("校级", "校级");
    m_achievementLevelFilter->addItem("院级", "院级");
    m_achievementLevelFilter->addItem("其他", "其他");
    achFilterLayout->addWidget(m_achievementSearchInput, 0, 0);
    achFilterLayout->addWidget(m_achievementTypeFilter, 0, 1);
    achFilterLayout->addWidget(m_achievementLevelFilter, 0, 2);
    connect(m_achievementSearchInput, &QLineEdit::textChanged, this, [this]() { refreshAchievements(); });
    connect(m_achievementTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshAchievements(); });
    connect(m_achievementLevelFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshAchievements(); });
    layout->addWidget(achFilterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增成果", page);
    QPushButton* editButton = new QPushButton("编辑选中成果", page);
    QPushButton* removeButton = new QPushButton("删除选中成果", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addAchievement);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedAchievement);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedAchievement);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("成果总数", &m_achievementTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("已验证成果数", &m_achievementVerifiedValue), 0, 1);
    metrics->addWidget(createMetricCard("成果级别数", &m_achievementLevelValue, "按 level 字段统计"), 0, 2);
    metrics->addWidget(createMetricCard("成果类型数", &m_achievementTypeValue, "按 type 字段统计"), 0, 3);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("成果时间线", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("竞赛、证书和奖项都可以直接在这里维护。双击条目进入原生编辑弹窗。", listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_achievementList = new QListWidget(listCard);
    m_achievementList->setObjectName("plainList");
    m_achievementList->setWordWrap(true);
    m_achievementList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_achievementList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { editSelectedAchievement(); });
    listLayout->addWidget(m_achievementList);
    layout->addWidget(listCard, 1);

    return page;
}

QWidget* MainWindow::createExperiencesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("经历档案", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_experienceSummaryLabel = new QLabel("正在读取经历数据...", page);
    m_experienceSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_experienceSummaryLabel);

    QFrame* expFilterCard = new QFrame(page);
    expFilterCard->setObjectName("contentCard");
    QGridLayout* expFilterLayout = new QGridLayout(expFilterCard);
    expFilterLayout->setContentsMargins(12, 12, 12, 12);
    expFilterLayout->setHorizontalSpacing(10);
    expFilterLayout->setVerticalSpacing(10);
    m_experienceSearchInput = new QLineEdit(expFilterCard);
    m_experienceSearchInput->setPlaceholderText("搜索经历标题 / 组织 / 描述");
    m_experienceTypeFilter = new QComboBox(expFilterCard);
    m_experienceTypeFilter->addItem("全部类型", "");
    m_experienceTypeFilter->addItem("项目", "项目");
    m_experienceTypeFilter->addItem("实习", "实习");
    m_experienceTypeFilter->addItem("科研", "科研");
    m_experienceTypeFilter->addItem("志愿", "志愿");
    m_experienceTypeFilter->addItem("竞赛", "竞赛");
    m_experienceTypeFilter->addItem("其他", "其他");
    expFilterLayout->addWidget(m_experienceSearchInput, 0, 0);
    expFilterLayout->addWidget(m_experienceTypeFilter, 0, 1);
    connect(m_experienceSearchInput, &QLineEdit::textChanged, this, [this]() { refreshExperiences(); });
    connect(m_experienceTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshExperiences(); });
    layout->addWidget(expFilterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增经历", page);
    QPushButton* editButton = new QPushButton("编辑选中经历", page);
    QPushButton* removeButton = new QPushButton("删除选中经历", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addExperience);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedExperience);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedExperience);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("经历总数", &m_experienceTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("进行中经历", &m_experienceOngoingValue), 0, 1);
    metrics->addWidget(createMetricCard("经历类型数", &m_experienceTypeValue, "按 type 字段统计"), 0, 2);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("经历时间线", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("项目、实习、科研和志愿经历都可以在原生界面中维护。双击条目即可编辑。", listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_experienceList = new QListWidget(listCard);
    m_experienceList->setObjectName("plainList");
    m_experienceList->setWordWrap(true);
    m_experienceList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_experienceList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { editSelectedExperience(); });
    listLayout->addWidget(m_experienceList);
    layout->addWidget(listCard, 1);

    return page;
}

QWidget* MainWindow::createActivitiesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("课外活动", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_activitySummaryLabel = new QLabel("正在读取活动数据...", page);
    m_activitySummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_activitySummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_activitySearchInput = new QLineEdit(filterCard);
    m_activitySearchInput->setPlaceholderText("搜索活动名称 / 描述");
    m_activityCategoryFilter = new QComboBox(filterCard);
    m_activityCategoryFilter->addItem("全部分类", "");
    m_activityCategoryFilter->addItem("学术", "学术");
    m_activityCategoryFilter->addItem("文体", "文体");
    m_activityCategoryFilter->addItem("志愿", "志愿");
    m_activityCategoryFilter->addItem("社团", "社团");
    m_activityCategoryFilter->addItem("竞赛", "竞赛");
    m_activityCategoryFilter->addItem("其他", "其他");
    filterLayout->addWidget(m_activitySearchInput, 0, 0);
    filterLayout->addWidget(m_activityCategoryFilter, 0, 1);
    connect(m_activitySearchInput, &QLineEdit::textChanged, this, [this]() { refreshActivities(); });
    connect(m_activityCategoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshActivities(); });
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增活动", page);
    QPushButton* editButton = new QPushButton("编辑选中活动", page);
    QPushButton* removeButton = new QPushButton("删除选中活动", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addActivity);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedActivity);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedActivity);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("活动总数", &m_activityTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("重点活动", &m_activityFavoriteValue), 0, 1);
    metrics->addWidget(createMetricCard("进行中活动", &m_activityActiveValue), 0, 2);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
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
    m_activityList = new QListWidget(listCard);
    m_activityList->setObjectName("plainList");
    m_activityList->setWordWrap(true);
    m_activityList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_activityList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { editSelectedActivity(); });
    listLayout->addWidget(m_activityList);
    layout->addWidget(listCard, 1);

    return page;
}

QWidget* MainWindow::createGoalsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("目标追踪", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_goalSummaryLabel = new QLabel("正在读取目标数据...", page);
    m_goalSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_goalSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_goalSearchInput = new QLineEdit(filterCard);
    m_goalSearchInput->setPlaceholderText("搜索目标标题 / 描述");
    m_goalStatusFilter = new QComboBox(filterCard);
    m_goalStatusFilter->addItem("全部状态", "");
    m_goalStatusFilter->addItem("进行中", "In Progress");
    m_goalStatusFilter->addItem("已完成", "Completed");
    m_goalStatusFilter->addItem("已暂停", "On Hold");
    m_goalStatusFilter->addItem("已取消", "Cancelled");
    m_goalPriorityFilter = new QComboBox(filterCard);
    m_goalPriorityFilter->addItem("全部优先级", "");
    m_goalPriorityFilter->addItem("高优先级", "High");
    m_goalPriorityFilter->addItem("中优先级", "Medium");
    m_goalPriorityFilter->addItem("低优先级", "Low");
    m_goalSortInput = new QLineEdit(filterCard);
    m_goalSortInput->setPlaceholderText("排序：progress / deadline / title / priority");
    filterLayout->addWidget(m_goalSearchInput, 0, 0);
    filterLayout->addWidget(m_goalStatusFilter, 0, 1);
    filterLayout->addWidget(m_goalPriorityFilter, 1, 0);
    filterLayout->addWidget(m_goalSortInput, 1, 1);
    connect(m_goalSearchInput, &QLineEdit::textChanged, this, [this]() { refreshGoals(); });
    connect(m_goalStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshGoals(); });
    connect(m_goalPriorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshGoals(); });
    connect(m_goalSortInput, &QLineEdit::textChanged, this, [this]() { refreshGoals(); });
    layout->addWidget(filterCard);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("目标总数", &m_goalTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("已完成目标", &m_goalCompletedValue), 0, 1);
    metrics->addWidget(createMetricCard("平均进度", &m_goalProgressMetricValue, "基于 target/current 自动计算"), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增目标", page);
    QPushButton* editButton = new QPushButton("编辑选中目标", page);
    QPushButton* removeButton = new QPushButton("删除选中目标", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addGoal);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedGoal);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedGoal);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("目标清单", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("目标可以直接在原生窗口中维护，保存后会联动刷新总览、时间轴和简历分析。", listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_goalList = new QListWidget(listCard);
    m_goalList->setObjectName("plainList");
    m_goalList->setWordWrap(true);
    m_goalList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_goalList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        editSelectedGoal();
    });
    listLayout->addWidget(m_goalList);
    layout->addWidget(listCard, 1);

    return page;
}

QWidget* MainWindow::createJobsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("目标岗位", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_jobSummaryLabel = new QLabel("正在读取岗位数据...", page);
    m_jobSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_jobSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_jobSearchInput = new QLineEdit(filterCard);
    m_jobSearchInput->setPlaceholderText("搜索岗位名称 / 公司 / 城市");
    m_jobStatusInput = new QLineEdit(filterCard);
    m_jobStatusInput->setPlaceholderText("过滤激活状态");
    filterLayout->addWidget(m_jobSearchInput, 0, 0);
    filterLayout->addWidget(m_jobStatusInput, 0, 1);
    connect(m_jobSearchInput, &QLineEdit::textChanged, this, [this]() { refreshJobs(); });
    connect(m_jobStatusInput, &QLineEdit::textChanged, this, [this]() { refreshJobs(); });
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增岗位", page);
    QPushButton* editButton = new QPushButton("编辑选中岗位", page);
    QPushButton* removeButton = new QPushButton("删除选中岗位", page);
    removeButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addJob);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedJob);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeSelectedJob);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("岗位总数", &m_jobTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("关注中", &m_jobActiveValue), 0, 1);
    metrics->addWidget(createMetricCard("平均要求匹配率", &m_jobRequirementValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("岗位列表", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    m_jobList = new QListWidget(listCard);
    m_jobList->setObjectName("plainList");
    m_jobList->setWordWrap(true);
    m_jobList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_jobList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { editSelectedJob(); });
    connect(m_jobList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && m_jobList->currentItem()) {
            int jobId = m_jobList->currentItem()->data(Qt::UserRole).toInt();
            if (jobId > 0) {
                Job job = JobService::getById(jobId);
                m_jobRequirementList->clear();
                int metCount = 0;
                for (int i = 0; i < job.requirements.size(); ++i) {
                    const auto& req = job.requirements[i];
                    QListWidgetItem* item = new QListWidgetItem(QString("[%1] %2").arg(req.met ? "x" : " ").arg(req.text), m_jobRequirementList);
                    item->setData(Qt::UserRole, i);
                    if (req.met) metCount++;
                }
                m_jobRequirementSummaryLabel->setText(QString("此岗位共有 %1 项要求，已匹配 %2 项。").arg(job.requirements.size()).arg(metCount));
            }
        }
    });
    listLayout->addWidget(m_jobList);
    bodyLayout->addWidget(listCard, 2);

    QFrame* reqCard = new QFrame(page);
    reqCard->setObjectName("contentCard");
    QVBoxLayout* reqLayout = new QVBoxLayout(reqCard);
    reqLayout->setContentsMargins(16, 14, 16, 14);
    reqLayout->setSpacing(10);
    QLabel* reqTitle = new QLabel("岗位要求匹配", reqCard);
    reqTitle->setObjectName("sectionTitle");
    reqLayout->addWidget(reqTitle);
    m_jobRequirementSummaryLabel = new QLabel("请在左侧选择岗位", reqCard);
    m_jobRequirementSummaryLabel->setObjectName("pageSubtitle");
    reqLayout->addWidget(m_jobRequirementSummaryLabel);
    m_jobRequirementList = new QListWidget(reqCard);
    m_jobRequirementList->setObjectName("plainList");
    m_jobRequirementList->setWordWrap(true);
    m_jobRequirementList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_jobRequirementList, &QListWidget::itemClicked, this, [this](QListWidgetItem*) { toggleSelectedJobRequirement(); });
    reqLayout->addWidget(m_jobRequirementList);
    bodyLayout->addWidget(reqCard, 1);

    layout->addLayout(bodyLayout, 1);
    return page;
}

QWidget* MainWindow::createAnalysisPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("分析报告", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_analysisSummaryLabel = new QLabel("正在生成数据分析报告...", page);
    m_analysisSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_analysisSummaryLabel);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addPeerBtn = new QPushButton("录入对照同学数据", page);
    QPushButton* editPeerBtn = new QPushButton("编辑同学数据", page);
    QPushButton* delPeerBtn = new QPushButton("删除同学数据", page);
    QPushButton* refreshBtn = new QPushButton("重新生成报告", page);
    connect(addPeerBtn, &QPushButton::clicked, this, &MainWindow::addPeer);
    connect(editPeerBtn, &QPushButton::clicked, this, &MainWindow::editSelectedPeer);
    connect(delPeerBtn, &QPushButton::clicked, this, &MainWindow::removeSelectedPeer);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshAnalysis);
    actionLayout->addWidget(addPeerBtn);
    actionLayout->addWidget(editPeerBtn);
    actionLayout->addWidget(delPeerBtn);
    actionLayout->addWidget(refreshBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("分析学期数", &m_analysisSemesterValue), 0, 0);
    metrics->addWidget(createMetricCard("对比对象数", &m_analysisPeerValue), 0, 1);
    metrics->addWidget(createMetricCard("总结建议条数", &m_analysisSuggestionValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* tableCard = new QFrame(page);
    tableCard->setObjectName("contentCard");
    QVBoxLayout* tLayout = new QVBoxLayout(tableCard);
    tLayout->setContentsMargins(16, 14, 16, 14);
    tLayout->setSpacing(10);

    QLabel* semLabel = new QLabel("学期趋势表现", tableCard);
    semLabel->setObjectName("sectionTitle");
    tLayout->addWidget(semLabel);
    m_analysisSemesterTable = new QTableWidget(tableCard);
    m_analysisSemesterTable->setColumnCount(4);
    m_analysisSemesterTable->setHorizontalHeaderLabels({"学期", "修读学分", "加权绩点", "排名"});
    m_analysisSemesterTable->horizontalHeader()->setStretchLastSection(true);
    m_analysisSemesterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_analysisSemesterTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_analysisSemesterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tLayout->addWidget(m_analysisSemesterTable, 1);

    QLabel* peerLabel = new QLabel("横向同学对比", tableCard);
    peerLabel->setObjectName("sectionTitle");
    tLayout->addWidget(peerLabel);
    m_analysisPeerTable = new QTableWidget(tableCard);
    m_analysisPeerTable->setColumnCount(6);
    m_analysisPeerTable->setHorizontalHeaderLabels({"姓名", "专业", "学期", "GPA", "成果", "经历"});
    m_analysisPeerTable->horizontalHeader()->setStretchLastSection(true);
    m_analysisPeerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_analysisPeerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_analysisPeerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_analysisPeerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_analysisPeerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_analysisPeerTable, &QTableWidget::cellDoubleClicked, this, [this]() { editSelectedPeer(); });
    tLayout->addWidget(m_analysisPeerTable, 1);

    bodyLayout->addWidget(tableCard, 2);

    QFrame* suggestionCard = new QFrame(page);
    suggestionCard->setObjectName("contentCard");
    QVBoxLayout* sLayout = new QVBoxLayout(suggestionCard);
    sLayout->setContentsMargins(16, 14, 16, 14);
    sLayout->setSpacing(10);
    QLabel* strLabel = new QLabel("核心优势", suggestionCard);
    strLabel->setObjectName("sectionTitle");
    sLayout->addWidget(strLabel);
    m_analysisStrengthList = new QListWidget(suggestionCard);
    m_analysisStrengthList->setObjectName("plainList");
    m_analysisStrengthList->setWordWrap(true);
    m_analysisStrengthList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sLayout->addWidget(m_analysisStrengthList, 1);

    QLabel* rskLabel = new QLabel("潜在风险", suggestionCard);
    rskLabel->setObjectName("sectionTitle");
    sLayout->addWidget(rskLabel);
    m_analysisRiskList = new QListWidget(suggestionCard);
    m_analysisRiskList->setObjectName("plainList");
    m_analysisRiskList->setWordWrap(true);
    m_analysisRiskList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sLayout->addWidget(m_analysisRiskList, 1);

    QLabel* sugLabel = new QLabel("发展建议", suggestionCard);
    sugLabel->setObjectName("sectionTitle");
    sLayout->addWidget(sugLabel);
    m_analysisSuggestionList = new QListWidget(suggestionCard);
    m_analysisSuggestionList->setObjectName("plainList");
    m_analysisSuggestionList->setWordWrap(true);
    m_analysisSuggestionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sLayout->addWidget(m_analysisSuggestionList, 1);

    bodyLayout->addWidget(suggestionCard, 1);
    layout->addLayout(bodyLayout, 1);

    return page;
}

QWidget* MainWindow::createTimelinePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("时间轴", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_timelineSummaryLabel = new QLabel("正在生成成长时间轴...", page);
    m_timelineSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_timelineSummaryLabel);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("事件总数", &m_timelineEventCountValue), 0, 0);
    metrics->addWidget(createMetricCard("优势条数", &m_timelineStrengthValue), 0, 1);
    metrics->addWidget(createMetricCard("风险条数", &m_timelineRiskValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->setSpacing(14);

    QFrame* timelineCard = new QFrame(page);
    timelineCard->setObjectName("contentCard");
    QVBoxLayout* timelineLayout = new QVBoxLayout(timelineCard);
    timelineLayout->setContentsMargins(16, 14, 16, 14);
    timelineLayout->setSpacing(10);
    QLabel* eventTitle = new QLabel("成长事件", timelineCard);
    eventTitle->setObjectName("sectionTitle");
    timelineLayout->addWidget(eventTitle);
    m_timelineList = new QListWidget(timelineCard);
    m_timelineList->setObjectName("plainList");
    m_timelineList->setWordWrap(true);
    m_timelineList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    timelineLayout->addWidget(m_timelineList);
    bottom->addWidget(timelineCard, 2);

    QFrame* suggestionCard = new QFrame(page);
    suggestionCard->setObjectName("contentCard");
    QVBoxLayout* suggestionLayout = new QVBoxLayout(suggestionCard);
    suggestionLayout->setContentsMargins(16, 14, 16, 14);
    suggestionLayout->setSpacing(10);
    QLabel* suggestionTitle = new QLabel("阶段建议", suggestionCard);
    suggestionTitle->setObjectName("sectionTitle");
    suggestionLayout->addWidget(suggestionTitle);
    m_timelineSuggestionList = new QListWidget(suggestionCard);
    m_timelineSuggestionList->setObjectName("plainList");
    m_timelineSuggestionList->setWordWrap(true);
    m_timelineSuggestionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    suggestionLayout->addWidget(m_timelineSuggestionList);
    bottom->addWidget(suggestionCard, 1);

    layout->addLayout(bottom, 1);
    return page;
}

QWidget* MainWindow::createResumePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("简历导出", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_resumeSummaryLabel = new QLabel("正在生成原生简历预览...", page);
    m_resumeSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_resumeSummaryLabel);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("简历分区数", &m_resumeSectionCountValue), 0, 0);
    metrics->addWidget(createMetricCard("身份标题", &m_resumeIdentityValue, "来自简历生成配置"), 0, 1);
    layout->addLayout(metrics);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* refreshButton = new QPushButton("刷新预览", page);
    QPushButton* resetButton = new QPushButton("恢复默认配置", page);
    QPushButton* exportJsonButton = new QPushButton("导出 JSON", page);
    QPushButton* exportHtmlButton = new QPushButton("导出 HTML", page);
    QPushButton* copyToClipboardButton = new QPushButton("复制到剪贴板", page);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshResume);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetResumeOptions);
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::exportResumeJson);
    connect(exportHtmlButton, &QPushButton::clicked, this, &MainWindow::exportResumeHtml);
    connect(copyToClipboardButton, &QPushButton::clicked, this, [this]() {
        QJsonObject options = currentResumeOptions();
        QString html = QString::fromUtf8(ResumeService::exportHtml(options));
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(html);
        ToastNotification::display(this, "简历 HTML 已复制到剪贴板。");
    });
    actionLayout->addWidget(refreshButton);
    actionLayout->addWidget(resetButton);
    actionLayout->addWidget(exportJsonButton);
    actionLayout->addWidget(exportHtmlButton);
    actionLayout->addWidget(copyToClipboardButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* configCard = new QFrame(page);
    configCard->setObjectName("contentCard");
    configCard->setMinimumWidth(320);
    configCard->setMaximumWidth(360);
    QVBoxLayout* configLayout = new QVBoxLayout(configCard);
    configLayout->setContentsMargins(16, 14, 16, 14);
    configLayout->setSpacing(12);

    QLabel* configTitle = new QLabel("简历配置", configCard);
    configTitle->setObjectName("sectionTitle");
    configLayout->addWidget(configTitle);

    QLabel* configHint = new QLabel("这组字段就是简历页面的单一数据源。预览、导出与后续 AI 优化都会基于这里的配置。", configCard);
    configHint->setObjectName("pageSubtitle");
    configHint->setWordWrap(true);
    configLayout->addWidget(configHint);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(10);

    m_resumeNameInput = new QLineEdit(configCard);
    m_resumeTitleInput = new QLineEdit(configCard);
    m_resumeEmailInput = new QLineEdit(configCard);
    m_resumePhoneInput = new QLineEdit(configCard);
    m_resumeSummaryInput = new QTextEdit(configCard);
    m_resumeCustomContentInput = new QTextEdit(configCard);
    m_resumeSummaryInput->setObjectName("richCardText");
    m_resumeCustomContentInput->setObjectName("richCardText");
    m_resumeSummaryInput->setMinimumHeight(120);
    m_resumeCustomContentInput->setMinimumHeight(150);
    m_resumeSummaryInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resumeCustomContentInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resumeSummaryInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_resumeCustomContentInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_resumeNameInput->setPlaceholderText("例如：张三");
    m_resumeTitleInput->setPlaceholderText("例如：个人成长规划简历");
    m_resumeEmailInput->setPlaceholderText("例如：name@example.com");
    m_resumePhoneInput->setPlaceholderText("例如：13800000000");
    m_resumeSummaryInput->setPlaceholderText("用 2-4 句话概括你的学习方向、实践重点和成长亮点。");
    m_resumeCustomContentInput->setPlaceholderText("这里用于手动补充简历亮点。右侧候选素材支持点击插入，你也可以直接编辑。");

    formLayout->addRow("姓名", m_resumeNameInput);
    formLayout->addRow("身份标题", m_resumeTitleInput);
    formLayout->addRow("邮箱", m_resumeEmailInput);
    formLayout->addRow("电话", m_resumePhoneInput);
    formLayout->addRow("个人摘要", m_resumeSummaryInput);
    formLayout->addRow("补充内容", m_resumeCustomContentInput);
    configLayout->addLayout(formLayout);

    QLabel* sectionHint = new QLabel("分区开关", configCard);
    sectionHint->setObjectName("sectionTitle");
    configLayout->addWidget(sectionHint);

    m_resumeEducationCheck = new QCheckBox("教育经历", configCard);
    m_resumeExperienceCheck = new QCheckBox("实践经历", configCard);
    m_resumeAchievementCheck = new QCheckBox("成果记录", configCard);
    m_resumeRoleCheck = new QCheckBox("角色任职", configCard);
    m_resumeActivityCheck = new QCheckBox("活动参与", configCard);
    configLayout->addWidget(m_resumeEducationCheck);
    configLayout->addWidget(m_resumeExperienceCheck);
    configLayout->addWidget(m_resumeAchievementCheck);
    configLayout->addWidget(m_resumeRoleCheck);
    configLayout->addWidget(m_resumeActivityCheck);
    configLayout->addStretch();

    QFrame* candidateCard = new QFrame(page);
    candidateCard->setObjectName("contentCard");
    candidateCard->setMinimumWidth(220);
    candidateCard->setMaximumWidth(260);
    QVBoxLayout* candidateLayout = new QVBoxLayout(candidateCard);
    candidateLayout->setContentsMargins(16, 14, 16, 14);
    candidateLayout->setSpacing(10);

    QLabel* candidateTitle = new QLabel("备选素材", candidateCard);
    candidateTitle->setObjectName("sectionTitle");
    candidateLayout->addWidget(candidateTitle);

    QLabel* candidateHint = new QLabel("从已有课程、经历、成果和活动中挑选内容，点击即可插入到补充内容区，再按你的需要微调。", candidateCard);
    candidateHint->setObjectName("pageSubtitle");
    candidateHint->setWordWrap(true);
    candidateLayout->addWidget(candidateHint);

    m_resumeCandidateTypeCombo = new QComboBox(candidateCard);
    m_resumeCandidateTypeCombo->addItem("课程亮点", "course");
    m_resumeCandidateTypeCombo->addItem("实践经历", "experience");
    m_resumeCandidateTypeCombo->addItem("成果记录", "achievement");
    m_resumeCandidateTypeCombo->addItem("角色职责", "role");
    m_resumeCandidateTypeCombo->addItem("课外活动", "activity");
    candidateLayout->addWidget(m_resumeCandidateTypeCombo);

    m_resumeCandidateList = new QListWidget(candidateCard);
    m_resumeCandidateList->setObjectName("plainList");
    m_resumeCandidateList->setWordWrap(true);
    m_resumeCandidateList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    candidateLayout->addWidget(m_resumeCandidateList, 1);

    QHBoxLayout* candidateActionLayout = new QHBoxLayout();
    candidateActionLayout->setSpacing(8);
    QPushButton* insertToSummaryButton = new QPushButton("追加到摘要", candidateCard);
    QPushButton* replaceSummaryButton = new QPushButton("替换摘要", candidateCard);
    insertToSummaryButton->setStyleSheet("QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 8px; padding: 6px 12px; color: #42526b; } QPushButton:hover { background: #f4f8ff; }");
    replaceSummaryButton->setStyleSheet("QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 8px; padding: 6px 12px; color: #42526b; } QPushButton:hover { background: #f4f8ff; }");
    candidateActionLayout->addWidget(insertToSummaryButton);
    candidateActionLayout->addWidget(replaceSummaryButton);
    candidateLayout->addLayout(candidateActionLayout);

    QPushButton* clearCustomContentButton = new QPushButton("清空补充内容", candidateCard);
    candidateLayout->addWidget(clearCustomContentButton);
    candidateLayout->addStretch(0);

    connect(insertToSummaryButton, &QPushButton::clicked, this, [this]() {
        if (!m_resumeCandidateList || !m_resumeSummaryInput) return;
        QListWidgetItem* item = m_resumeCandidateList->currentItem();
        if (!item) {
            ToastNotification::display(this, "请先选择一条素材。");
            return;
        }
        const QString snippet = item->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) return;
        QString current = m_resumeSummaryInput->toPlainText().trimmed();
        if (!current.isEmpty()) current += "\n";
        current += snippet;
        m_resumeSummaryInput->setPlainText(current);
        ToastNotification::display(this, "已追加到个人摘要。");
    });
    connect(replaceSummaryButton, &QPushButton::clicked, this, [this]() {
        if (!m_resumeCandidateList || !m_resumeSummaryInput) return;
        QListWidgetItem* item = m_resumeCandidateList->currentItem();
        if (!item) {
            ToastNotification::display(this, "请先选择一条素材。");
            return;
        }
        const QString snippet = item->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) return;
        m_resumeSummaryInput->setPlainText(snippet);
        ToastNotification::display(this, "已替换个人摘要。");
    });

    QFrame* previewCard = new QFrame(page);
    previewCard->setObjectName("contentCard");
    previewCard->setMinimumWidth(460);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(16, 14, 16, 14);
    previewLayout->setSpacing(10);
    QLabel* previewTitle = new QLabel("简历预览", previewCard);
    previewTitle->setObjectName("sectionTitle");
    previewLayout->addWidget(previewTitle);
    m_resumePreview = new QTextBrowser(previewCard);
    m_resumePreview->setObjectName("richCardText");
    m_resumePreview->setOpenExternalLinks(true);
    m_resumePreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resumePreview->setLineWrapMode(QTextEdit::WidgetWidth);
    m_resumePreview->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_resumePreview->document()->setDocumentMargin(10);
    previewLayout->addWidget(m_resumePreview, 1);
    bodyLayout->addWidget(configCard, 5);
    bodyLayout->addWidget(candidateCard, 4);
    bodyLayout->addWidget(previewCard, 7);
    layout->addLayout(bodyLayout, 1);

    connect(m_resumeNameInput, &QLineEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumeTitleInput, &QLineEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumeEmailInput, &QLineEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumePhoneInput, &QLineEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumeSummaryInput, &QTextEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumeCustomContentInput, &QTextEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumeEducationCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); });
    connect(m_resumeExperienceCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); });
    connect(m_resumeAchievementCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); });
    connect(m_resumeRoleCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); });
    connect(m_resumeActivityCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); });
    connect(m_resumeCandidateTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        refreshResumeCandidates();
    });
    connect(m_resumeCandidateList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item || !m_resumeCustomContentInput) {
            return;
        }
        const QString snippet = item->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) {
            return;
        }
        QString current = m_resumeCustomContentInput->toPlainText().trimmed();
        if (!current.contains(snippet)) {
            if (!current.isEmpty()) {
                current += "\n";
            }
            current += snippet;
            m_resumeCustomContentInput->setPlainText(current);
            ToastNotification::display(this, "已插入到补充内容区，你可以继续修改。");
        } else {
            ToastNotification::display(this, "这条素材已经在补充内容区中了。");
        }
    });
    connect(clearCustomContentButton, &QPushButton::clicked, this, [this]() {
        if (m_resumeCustomContentInput) {
            m_resumeCustomContentInput->clear();
        }
    });

    resetResumeOptions();
    refreshResumeCandidates();

    return page;
}

QWidget* MainWindow::createResumeEditorPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("简历导出", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_resumeSummaryLabel = new QLabel("左侧填写简历信息，右侧选择备选素材。点击「预览简历」查看效果。", page);
    m_resumeSummaryLabel->setObjectName("pageSubtitle");
    m_resumeSummaryLabel->setWordWrap(true);
    layout->addWidget(m_resumeSummaryLabel);

    QFrame* toolbarCard = new QFrame(page);
    toolbarCard->setObjectName("contentCard");
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbarCard);
    toolbarLayout->setContentsMargins(16, 12, 16, 12);
    toolbarLayout->setSpacing(10);

    QLabel* logo = new QLabel("简历工坊", toolbarCard);
    logo->setStyleSheet("font-size: 16px; font-weight: 700; color: #2f67ff;");
    toolbarLayout->addWidget(logo);

    QPushButton* addModuleButton = new QPushButton("添加模块", toolbarCard);
    toolbarLayout->addWidget(addModuleButton);
    toolbarLayout->addStretch();

    const QString toolButtonStyle =
        "QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 10px; padding: 7px 14px; color: #42526b; }"
        "QPushButton:hover { background: #f4f8ff; border-color: #9db8ff; }";
    const QString activeButtonStyle =
        "QPushButton { background: #2f67ff; border: 1px solid #2f67ff; border-radius: 10px; padding: 7px 16px; color: white; font-weight: 600; }";

    QPushButton* previewButton = new QPushButton("预览简历", toolbarCard);
    QPushButton* resetTopButton = new QPushButton("重置", toolbarCard);
    QPushButton* exportJsonButton = new QPushButton("导出 JSON", toolbarCard);
    QPushButton* exportHtmlButton = new QPushButton("导出 HTML", toolbarCard);
    QPushButton* exportPdfButton = new QPushButton("导出 PDF", toolbarCard);
    previewButton->setStyleSheet(toolButtonStyle);
    resetTopButton->setStyleSheet(toolButtonStyle);
    exportJsonButton->setStyleSheet(toolButtonStyle);
    exportHtmlButton->setStyleSheet(toolButtonStyle);
    exportPdfButton->setStyleSheet(activeButtonStyle);
    toolbarLayout->addWidget(previewButton);
    toolbarLayout->addWidget(resetTopButton);
    toolbarLayout->addWidget(exportJsonButton);
    toolbarLayout->addWidget(exportHtmlButton);
    toolbarLayout->addWidget(exportPdfButton);
    layout->addWidget(toolbarCard);

    QSplitter* bodySplitter = new QSplitter(Qt::Horizontal, page);
    bodySplitter->setObjectName("resumeBodySplitter");
    bodySplitter->setChildrenCollapsible(false);
    bodySplitter->setHandleWidth(1);
    bodySplitter->setStyleSheet(
        "#resumeBodySplitter { background: transparent; }"
        "#resumeBodySplitter::handle { background: #d8deea; width: 1px; }"
        "#resumeBodySplitter::handle:hover { background: #3b82f6; }"
    );

    QFrame* editorCard = new QFrame(bodySplitter);
    editorCard->setObjectName("contentCard");
    editorCard->setMinimumWidth(400);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorCard);
    editorLayout->setContentsMargins(16, 14, 16, 14);
    editorLayout->setSpacing(12);

    QLabel* editorTitle = new QLabel("简历信息", editorCard);
    editorTitle->setObjectName("sectionTitle");
    editorLayout->addWidget(editorTitle);

    m_resumeEditorTabs = new QTabWidget(editorCard);
    QWidget* contentTab = new QWidget(m_resumeEditorTabs);
    QWidget* styleTab = new QWidget(m_resumeEditorTabs);
    m_resumeEditorTabs->addTab(contentTab, "内容编辑");
    m_resumeEditorTabs->addTab(styleTab, "样式设置");
    editorLayout->addWidget(m_resumeEditorTabs, 1);

    QVBoxLayout* contentTabLayout = new QVBoxLayout(contentTab);
    contentTabLayout->setContentsMargins(16, 12, 16, 16);
    contentTabLayout->setSpacing(12);
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(10);

    m_resumeNameInput = new QLineEdit(contentTab);
    m_resumeTitleInput = new QLineEdit(contentTab);
    m_resumeAgeInput = new QLineEdit(contentTab);
    m_resumeCityInput = new QLineEdit(contentTab);
    m_resumeEmailInput = new QLineEdit(contentTab);
    m_resumePhoneInput = new QLineEdit(contentTab);
    m_resumeIntentInput = new QLineEdit(contentTab);
    m_resumeSchoolInput = new QLineEdit(contentTab);
    m_resumeMajorInput = new QLineEdit(contentTab);
    m_resumeDegreeInput = new QLineEdit(contentTab);
    m_resumeProjectNameInput = new QLineEdit(contentTab);
    m_resumeSummaryInput = new QTextEdit(contentTab);
    m_resumeEducationBodyInput = new QTextEdit(contentTab);
    m_resumeSkillsBodyInput = new QTextEdit(contentTab);
    m_resumeProjectBodyInput = new QTextEdit(contentTab);
    m_resumeInternshipInput = new QTextEdit(contentTab);
    m_resumeAwardsInput = new QTextEdit(contentTab);
    m_resumeCustomContentInput = new QTextEdit(contentTab);
    QList<QTextEdit*> textEdits = {m_resumeSummaryInput, m_resumeEducationBodyInput, m_resumeSkillsBodyInput, m_resumeProjectBodyInput, m_resumeInternshipInput, m_resumeAwardsInput, m_resumeCustomContentInput};
    for (QTextEdit* edit : textEdits) {
        edit->setObjectName("richCardText");
        edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        edit->setMinimumHeight(90);
    }
    m_resumeEducationBodyInput->setMinimumHeight(120);
    m_resumeProjectBodyInput->setMinimumHeight(120);
    m_resumeInternshipInput->setMinimumHeight(100);
    m_resumeAwardsInput->setMinimumHeight(100);

    formLayout->addRow("姓名", m_resumeNameInput);
    formLayout->addRow("身份标题", m_resumeTitleInput);
    formLayout->addRow("年龄", m_resumeAgeInput);
    formLayout->addRow("城市", m_resumeCityInput);
    formLayout->addRow("邮箱", m_resumeEmailInput);
    formLayout->addRow("电话", m_resumePhoneInput);
    formLayout->addRow("求职意向", m_resumeIntentInput);
    formLayout->addRow("学校名称", m_resumeSchoolInput);
    formLayout->addRow("专业名称", m_resumeMajorInput);
    formLayout->addRow("学历学位", m_resumeDegreeInput);
    formLayout->addRow("个人简介", m_resumeSummaryInput);
    formLayout->addRow("教育描述", m_resumeEducationBodyInput);
    formLayout->addRow("技能特长", m_resumeSkillsBodyInput);
    formLayout->addRow("项目名称", m_resumeProjectNameInput);
    formLayout->addRow("项目经验", m_resumeProjectBodyInput);
    formLayout->addRow("实习经历", m_resumeInternshipInput);
    formLayout->addRow("竞赛获奖", m_resumeAwardsInput);
    formLayout->addRow("自我评价", m_resumeCustomContentInput);
    contentTabLayout->addLayout(formLayout);

    QVBoxLayout* styleTabLayout = new QVBoxLayout(styleTab);
    styleTabLayout->setContentsMargins(16, 12, 16, 16);
    styleTabLayout->setSpacing(12);
    m_resumeSectionVisibleCheck = new QCheckBox("显示当前模块", styleTab);
    styleTabLayout->addWidget(m_resumeSectionVisibleCheck);
    m_resumeEducationCheck = new QCheckBox("显示教育背景", styleTab);
    m_resumeExperienceCheck = new QCheckBox("显示项目经验", styleTab);
    m_resumeAchievementCheck = new QCheckBox("显示技能特长", styleTab);
    m_resumeRoleCheck = new QCheckBox("显示求职意向", styleTab);
    m_resumeActivityCheck = new QCheckBox("显示自我评价", styleTab);
    styleTabLayout->addWidget(m_resumeEducationCheck);
    styleTabLayout->addWidget(m_resumeExperienceCheck);
    styleTabLayout->addWidget(m_resumeAchievementCheck);
    styleTabLayout->addWidget(m_resumeRoleCheck);
    styleTabLayout->addWidget(m_resumeActivityCheck);
    QLabel* styleHint = new QLabel("勾选需要显示的简历模块。", styleTab);
    styleHint->setObjectName("pageSubtitle");
    styleHint->setWordWrap(true);
    styleTabLayout->addWidget(styleHint);
    styleTabLayout->addStretch();

    bodySplitter->addWidget(editorCard);

    QFrame* candidateCard = new QFrame(bodySplitter);
    candidateCard->setObjectName("contentCard");
    candidateCard->setMinimumWidth(200);
    candidateCard->setMaximumWidth(400);
    QVBoxLayout* candidateLayout = new QVBoxLayout(candidateCard);
    candidateLayout->setContentsMargins(12, 12, 12, 12);
    candidateLayout->setSpacing(8);

    QLabel* candidateTitle = new QLabel("备选素材", candidateCard);
    candidateTitle->setObjectName("sectionTitle");
    candidateLayout->addWidget(candidateTitle);

    QLabel* candidateHint = new QLabel("从已有数据中选择素材，点击即可插入到对应编辑区。", candidateCard);
    candidateHint->setObjectName("pageSubtitle");
    candidateHint->setWordWrap(true);
    candidateLayout->addWidget(candidateHint);

    m_resumeCandidateTypeCombo = new QComboBox(candidateCard);
    m_resumeCandidateTypeCombo->addItem("课程亮点", "course");
    m_resumeCandidateTypeCombo->addItem("实践经历", "experience");
    m_resumeCandidateTypeCombo->addItem("成果记录", "achievement");
    m_resumeCandidateTypeCombo->addItem("角色职责", "role");
    m_resumeCandidateTypeCombo->addItem("课外活动", "activity");
    candidateLayout->addWidget(m_resumeCandidateTypeCombo);

    m_resumeCandidateList = new QListWidget(candidateCard);
    m_resumeCandidateList->setObjectName("plainList");
    m_resumeCandidateList->setWordWrap(true);
    m_resumeCandidateList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    candidateLayout->addWidget(m_resumeCandidateList, 1);

    QHBoxLayout* candidateActionLayout = new QHBoxLayout();
    QPushButton* clearCustomContentButton = new QPushButton("清空补充", candidateCard);
    QPushButton* insertSummaryButton = new QPushButton("插入到简介", candidateCard);
    candidateActionLayout->addWidget(clearCustomContentButton);
    candidateActionLayout->addWidget(insertSummaryButton);
    candidateLayout->addLayout(candidateActionLayout);

    bodySplitter->addWidget(candidateCard);
    bodySplitter->setStretchFactor(0, 7);
    bodySplitter->setStretchFactor(1, 3);
    layout->addWidget(bodySplitter, 1);

    m_resumePreview = new QTextBrowser(this);
    m_resumePreview->setObjectName("resumePaper");
    m_resumePreview->setOpenLinks(false);
    m_resumePreview->setOpenExternalLinks(false);
    m_resumePreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resumePreview->setFrameShape(QFrame::NoFrame);
    m_resumePreview->setStyleSheet("background: white; border: 1px solid #e0e0e0; border-radius: 4px; font-size: 13px; line-height: 1.6;");
    m_resumePreview->document()->setDocumentMargin(20);

    QList<QLineEdit*> lineInputs = {m_resumeNameInput, m_resumeTitleInput, m_resumeAgeInput, m_resumeCityInput, m_resumeEmailInput, m_resumePhoneInput, m_resumeIntentInput, m_resumeSchoolInput, m_resumeMajorInput, m_resumeDegreeInput, m_resumeProjectNameInput};
    for (QLineEdit* input : lineInputs) connect(input, &QLineEdit::textChanged, this, [this]() { refreshResume(); });
    for (QTextEdit* edit : textEdits) connect(edit, &QTextEdit::textChanged, this, [this]() { refreshResume(); });
    connect(m_resumeEducationCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); refreshResumeEditorPanel(); });
    connect(m_resumeExperienceCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); refreshResumeEditorPanel(); });
    connect(m_resumeAchievementCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); refreshResumeEditorPanel(); });
    connect(m_resumeRoleCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); refreshResumeEditorPanel(); });
    connect(m_resumeActivityCheck, &QCheckBox::checkStateChanged, this, [this]() { refreshResume(); refreshResumeEditorPanel(); });
    connect(m_resumeCandidateTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { refreshResumeCandidates(); });
    connect(m_resumeCandidateList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item) return;
        const QString snippet = item->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) return;
        QTextEdit* targetEdit = m_resumeCustomContentInput;
        if (m_resumeSelectedSection == "education") targetEdit = m_resumeEducationBodyInput;
        else if (m_resumeSelectedSection == "skills") targetEdit = m_resumeSkillsBodyInput;
        else if (m_resumeSelectedSection == "project") targetEdit = m_resumeProjectBodyInput;
        QString current = targetEdit->toPlainText().trimmed();
        if (!current.contains(snippet)) {
            if (!current.isEmpty()) current += "\n";
            current += snippet;
            targetEdit->setPlainText(current);
        }
    });
    connect(clearCustomContentButton, &QPushButton::clicked, this, [this]() {
        if (m_resumeCustomContentInput) m_resumeCustomContentInput->clear();
    });
    connect(insertSummaryButton, &QPushButton::clicked, this, [this]() {
        if (!m_resumeCandidateList || !m_resumeCandidateList->currentItem() || !m_resumeSummaryInput) return;
        const QString snippet = m_resumeCandidateList->currentItem()->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) return;
        QString current = m_resumeSummaryInput->toPlainText().trimmed();
        if (!current.contains(snippet)) {
            if (!current.isEmpty()) current += "\n";
            current += snippet;
            m_resumeSummaryInput->setPlainText(current);
        }
    });
    connect(m_resumeSectionVisibleCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (m_resumeSelectedSection == "education") m_resumeEducationCheck->setChecked(checked);
        else if (m_resumeSelectedSection == "project") m_resumeExperienceCheck->setChecked(checked);
        else if (m_resumeSelectedSection == "skills") m_resumeAchievementCheck->setChecked(checked);
        else if (m_resumeSelectedSection == "intent") m_resumeRoleCheck->setChecked(checked);
        else if (m_resumeSelectedSection == "custom") m_resumeActivityCheck->setChecked(checked);
    });
    connect(m_resumePreview, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        const QString value = url.toString();
        if (value.startsWith("section:")) {
            setSelectedResumeSection(value.mid(QString("section:").size()));
        } else if (value.startsWith("delete:")) {
            const QString section = value.mid(QString("delete:").size());
            if (section == "education") m_resumeEducationCheck->setChecked(false);
            else if (section == "skills") m_resumeAchievementCheck->setChecked(false);
            else if (section == "project") m_resumeExperienceCheck->setChecked(false);
            else if (section == "intent") m_resumeRoleCheck->setChecked(false);
            else if (section == "custom") m_resumeActivityCheck->setChecked(false);
        } else if (value.startsWith("copy:")) {
            const QString section = value.mid(QString("copy:").size());
            QString snippet;
            if (section == "education") snippet = m_resumeEducationBodyInput->toPlainText().trimmed();
            else if (section == "skills") snippet = m_resumeSkillsBodyInput->toPlainText().trimmed();
            else if (section == "project") snippet = m_resumeProjectBodyInput->toPlainText().trimmed();
            if (!snippet.isEmpty()) {
                QString current = m_resumeCustomContentInput->toPlainText().trimmed();
                if (!current.isEmpty()) current += "\n";
                current += snippet;
                m_resumeCustomContentInput->setPlainText(current);
            }
        }
    });

    connect(addModuleButton, &QPushButton::clicked, this, [this]() {
        if (m_resumeCustomContentInput) {
            m_resumeCustomContentInput->append("新增模块：请在这里补充新的模块内容。");
        }
    });
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::exportResumeJson);
    connect(exportHtmlButton, &QPushButton::clicked, this, &MainWindow::exportResumeHtml);
    connect(exportPdfButton, &QPushButton::clicked, this, &MainWindow::exportResumePdf);
    connect(resetTopButton, &QPushButton::clicked, this, &MainWindow::resetResumeOptions);
    connect(previewButton, &QPushButton::clicked, this, [this]() {
        refreshResume();
        QDialog* previewDialog = new QDialog(this);
        previewDialog->setWindowTitle("简历预览");
        previewDialog->setMinimumSize(720, 880);
        previewDialog->resize(800, 950);
        previewDialog->setStyleSheet(
            "QDialog { background: #e8ecf1; }"
            "QLabel { color: #333; }"
            "QPushButton { background: white; border: 1px solid #d0d5dd; border-radius: 6px; padding: 8px 20px; font-size: 13px; }"
            "QPushButton:hover { background: #f0f4ff; border-color: #3b82f6; }"
            "QPushButton:disabled { color: #999; background: #f5f5f5; }"
        );
        QVBoxLayout* dialogLayout = new QVBoxLayout(previewDialog);
        dialogLayout->setContentsMargins(24, 20, 24, 16);
        dialogLayout->setSpacing(12);

        QHBoxLayout* headerLayout = new QHBoxLayout();
        QLabel* titleLabel = new QLabel("📄 简历预览", previewDialog);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #1f2937;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();
        
        m_resumePageLabel = new QLabel("第 1 / 1 页", previewDialog);
        m_resumePageLabel->setStyleSheet("font-size: 13px; color: #6b7280; padding: 6px 12px; background: white; border-radius: 12px;");
        headerLayout->addWidget(m_resumePageLabel);
        dialogLayout->addLayout(headerLayout);

        QFrame* paperFrame = new QFrame(previewDialog);
        paperFrame->setObjectName("previewPaper");
        paperFrame->setStyleSheet(
            "#previewPaper {"
            "  background: white;"
            "  border: 1px solid #d1d5db;"
            "  border-radius: 2px;"
            "  box-shadow: 0 2px 8px rgba(0,0,0,0.08);"
            "}"
        );
        QVBoxLayout* paperLayout = new QVBoxLayout(paperFrame);
        paperLayout->setContentsMargins(0, 0, 0, 0);
        paperLayout->setSpacing(0);

        QTextBrowser* previewBrowser = new QTextBrowser(paperFrame);
        previewBrowser->setObjectName("resumePreviewBrowser");
        previewBrowser->setOpenExternalLinks(true);
        previewBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        previewBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        previewBrowser->setFrameShape(QFrame::NoFrame);
        previewBrowser->setStyleSheet(
            "#resumePreviewBrowser {"
            "  background: white;"
            "  border: none;"
            "  font-family: 'Microsoft YaHei', 'PingFang SC', sans-serif;"
            "}"
        );
        previewBrowser->document()->setDocumentMargin(0);
        previewBrowser->setHtml(m_resumePreview->toHtml());
        paperLayout->addWidget(previewBrowser);

        dialogLayout->addWidget(paperFrame, 1);

        QHBoxLayout* footerLayout = new QHBoxLayout();
        footerLayout->setSpacing(10);
        
        QPushButton* prevBtn = new QPushButton("◀ 上一页", previewDialog);
        QPushButton* nextBtn = new QPushButton("下一页 ▶", previewDialog);
        QPushButton* closeBtn = new QPushButton("关闭", previewDialog);
        closeBtn->setStyleSheet(
            "QPushButton { background: linear-gradient(135deg, #3b82f6, #2563eb); color: white; border: none; border-radius: 6px; padding: 10px 28px; font-weight: 600; font-size: 13px; }"
            "QPushButton:hover { background: linear-gradient(135deg, #2563eb, #1d4ed8); }"
        );
        
        footerLayout->addWidget(prevBtn);
        footerLayout->addStretch();
        footerLayout->addWidget(closeBtn);
        footerLayout->addWidget(nextBtn);
        dialogLayout->addLayout(footerLayout);

        const auto updatePageInfo = [this, previewBrowser]() {
            const int docHeight = previewBrowser->document()->size().height();
            const int viewHeight = previewBrowser->viewport()->height();
            const int totalPages = qMax(1, (int)ceil(docHeight / (double)viewHeight));
            const int scrollPos = previewBrowser->verticalScrollBar()->value();
            const int currentPage = qMin(totalPages, qMax(1, (int)floor(scrollPos / (double)viewHeight) + 1));
            if (m_resumePageLabel) {
                m_resumePageLabel->setText(QString("第 %1 / %2 页").arg(currentPage).arg(totalPages));
            }
        };

        connect(nextBtn, &QPushButton::clicked, this, [previewBrowser, nextBtn, prevBtn, updatePageInfo]() {
            const int step = previewBrowser->viewport()->height() - 40;
            const int maxVal = previewBrowser->verticalScrollBar()->maximum();
            const int newVal = qMin(maxVal, previewBrowser->verticalScrollBar()->value() + step);
            previewBrowser->verticalScrollBar()->setValue(newVal);
            prevBtn->setEnabled(newVal > 0);
            nextBtn->setEnabled(newVal < maxVal);
            updatePageInfo();
        });

        connect(prevBtn, &QPushButton::clicked, this, [previewBrowser, nextBtn, prevBtn, updatePageInfo]() {
            const int step = previewBrowser->viewport()->height() - 40;
            const int newVal = qMax(0, previewBrowser->verticalScrollBar()->value() - step);
            previewBrowser->verticalScrollBar()->setValue(newVal);
            prevBtn->setEnabled(newVal > 0);
            nextBtn->setEnabled(newVal < previewBrowser->verticalScrollBar()->maximum());
            updatePageInfo();
        });

        connect(closeBtn, &QPushButton::clicked, previewDialog, &QDialog::accept);
        connect(previewBrowser->verticalScrollBar(), &QScrollBar::valueChanged, this, [prevBtn, nextBtn, previewBrowser, updatePageInfo](int value) {
            prevBtn->setEnabled(value > 0);
            nextBtn->setEnabled(value < previewBrowser->verticalScrollBar()->maximum());
            updatePageInfo();
        });
        
        prevBtn->setEnabled(false);
        QTimer::singleShot(100, previewBrowser, [nextBtn, previewBrowser, updatePageInfo]() {
            nextBtn->setEnabled(previewBrowser->verticalScrollBar()->maximum() > 0);
            updatePageInfo();
        });
        
        previewDialog->exec();
        previewDialog->deleteLater();
    });

    resetResumeOptions();
    refreshResumeCandidates();
    setSelectedResumeSection("education");
    return page;
}

QWidget* MainWindow::createImportsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("数据导入", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_importSummaryLabel = new QLabel("支持从外部系统批量导入通用课程、角色、成果等记录。", page);
    m_importSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_importSummaryLabel);

    QFrame* controlCard = new QFrame(page);
    controlCard->setObjectName("contentCard");
    QVBoxLayout* cl = new QVBoxLayout(controlCard);
    cl->setContentsMargins(16, 14, 16, 14);
    cl->setSpacing(12);

    QFormLayout* form = new QFormLayout();
    m_importEntityCombo = new QComboBox(controlCard);
    m_importEntityCombo->addItem("课程数据 (courses)", "courses");
    m_importEntityCombo->addItem("角色职责 (roles)", "roles");
    m_importEntityCombo->addItem("成果记录 (achievements)", "achievements");
    m_importEntityCombo->addItem("实践经历 (experiences)", "experiences");
    m_importEntityCombo->addItem("课外活动 (activities)", "activities");
    m_importEntityCombo->addItem("目标数据 (goals)", "goals");
    m_importEntityCombo->addItem("对标同学 (peers)", "peers");
    form->addRow("选择要导入的数据类别：", m_importEntityCombo);
    
    m_importFileLabel = new QLabel("尚未选择文件", controlCard);
    m_importFileLabel->setObjectName("richCardText");
    QPushButton* pickBtn = new QPushButton("选择 CSV 文件", controlCard);
    connect(pickBtn, &QPushButton::clicked, this, &MainWindow::chooseImportFile);
    
    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(m_importFileLabel, 1);
    fileRow->addWidget(pickBtn);
    form->addRow("选择数据源文件：", fileRow);
    cl->addLayout(form);

    QPushButton* runBtn = new QPushButton("开始导入", controlCard);
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::runImport);
    cl->addWidget(runBtn, 0, Qt::AlignRight);
    layout->addWidget(controlCard);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("成功导入条数", &m_importResultImportedValue), 0, 0);
    metrics->addWidget(createMetricCard("导入失败条数", &m_importResultFailedValue), 0, 1);
    layout->addLayout(metrics);

    QFrame* errCard = new QFrame(page);
    errCard->setObjectName("contentCard");
    QVBoxLayout* el = new QVBoxLayout(errCard);
    el->setContentsMargins(16, 14, 16, 14);
    QLabel* elTitle = new QLabel("导入失败明细", errCard);
    elTitle->setObjectName("sectionTitle");
    el->addWidget(elTitle);
    m_importErrorTable = new QTableWidget(errCard);
    m_importErrorTable->setColumnCount(2);
    m_importErrorTable->setHorizontalHeaderLabels({"出错行号", "错误原因"});
    m_importErrorTable->horizontalHeader()->setStretchLastSection(true);
    m_importErrorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_importErrorTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    el->addWidget(m_importErrorTable, 1);
    layout->addWidget(errCard, 1);

    return page;
}

QWidget* MainWindow::createPlaceholderPage(const QString& titleText, const QString& description)
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    QLabel* title = new QLabel(titleText, page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    QFrame* card = new QFrame(page);
    card->setObjectName("contentCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 22, 22, 22);
    QLabel* text = new QLabel(description, card);
    text->setObjectName("placeholderText");
    text->setWordWrap(true);
    cardLayout->addWidget(text);
    layout->addWidget(card);
    layout->addStretch();

    return page;
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");
    QAction* openBrowserAction = fileMenu->addAction("打开网页预览(&O)");
    openBrowserAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openBrowserAction, &QAction::triggered, this, &MainWindow::onOpenBrowser);

    QAction* refreshAction = fileMenu->addAction("刷新当前页面(&R)");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshCurrentPage);

    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onQuitTriggered);

    QMenu* helpMenu = menuBar()->addMenu("帮助(&H)");
    QAction* aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutTriggered);
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar("工具栏");
    m_toolBar->setMovable(false);

    QAction* openAction = m_toolBar->addAction("网页预览");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenBrowser);

    m_refreshAction = m_toolBar->addAction("刷新");
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshCurrentPage);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("正在启动后端服务...", this);
    statusBar()->addWidget(m_statusLabel, 1);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(140);
    m_progressBar->setRange(0, 0);
    m_progressBar->setTextVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::setupSystemTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        Logger::warning("系统托盘不可用");
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("学业发展规划系统");

    m_trayMenu = new QMenu(this);
    m_openBrowserAction = m_trayMenu->addAction("打开网页预览");
    connect(m_openBrowserAction, &QAction::triggered, this, &MainWindow::onOpenBrowser);
    m_trayMenu->addSeparator();
    m_quitAction = m_trayMenu->addAction("退出");
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::onQuitTriggered);
    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
    m_trayIcon->show();
}

void MainWindow::applyWindowStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background: #f7f5ef;
            color: #2d241c;
        }
        #sidebar {
            background: #efebe2;
            border-right: 1px solid #ddd3c6;
        }
        #topbar {
            background: #f7f5ef;
            border-bottom: 1px solid #ddd3c6;
        }
        #topbarKicker {
            color: #8a7d70;
            font-size: 13px;
            font-weight: 500;
        }
        #topbarTitle {
            color: #241c15;
            font-size: 18px;
            font-weight: 500;
        }
        #topbarPill {
            padding: 7px 14px;
            border: 1px solid #ddd3c6;
            border-radius: 999px;
            color: #8a7d70;
            background: transparent;
            font-size: 13px;
        }
        #brandMark, #sidebarInfoAvatar {
            min-width: 28px;
            min-height: 28px;
            max-width: 28px;
            max-height: 28px;
            border: 1px solid #ddd3c6;
            border-radius: 8px;
            background: #fffdf9;
            color: #2d241c;
            font-size: 12px;
            font-weight: 500;
            qproperty-alignment: 'AlignCenter';
        }
        #sidebarTitle {
            font-size: 14px;
            font-weight: 500;
            color: #2f261e;
        }
        #sidebarSubtitle, #sidebarFooter, #windowSubHeading, #pageSubtitle, #placeholderText,
        #sidebarInfoKicker, #sidebarInfoDetail {
            color: #75685d;
            font-size: 12px;
            line-height: 1.5;
        }
        #sidebarInfoTitle {
            color: #241c15;
            font-size: 13px;
            font-weight: 500;
        }
        #sidebarInfoCard {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 14px;
        }
        #windowHeading {
            font-size: 28px;
            font-weight: 700;
            color: #241c15;
        }
        #pageTitle {
            font-size: 24px;
            font-weight: 700;
            color: #241c15;
        }
        #connectionLabel {
            background: #fffdf9;
            border: 1px solid #dacdbd;
            border-radius: 10px;
            padding: 10px 12px;
            color: #5b4e43;
        }
        #navList {
            border: none;
            background: transparent;
            outline: none;
            font-size: 14px;
        }
        #navList[collapsed="false"]::item {
            padding: 10px 12px;
            margin: 2px 0;
            border-radius: 10px;
            color: #5d5146;
        }
        #navList[collapsed="false"]::item:hover {
            background: #f4ede2;
        }
        #navList[collapsed="false"]::item:selected {
            background: #fffdf9;
            border: 1px solid #d7cab8;
            color: #2f261e;
        }
        #navList[collapsed="true"]::item {
            padding: 6px 0;
            margin: 3px 6px;
            border-radius: 8px;
            color: #5d5146;
        }
        #navList[collapsed="true"]::item:hover {
            background: #f4ede2;
        }
        #navList[collapsed="true"]::item:selected {
            background: #fffdf9;
            border: 1px solid #d7cab8;
            color: #2f261e;
            margin: 3px 6px;
            border-radius: 8px;
        }
        #metricCard, #contentCard {
            background: #fffdf9;
            border: 1px solid #ddcfbe;
            border-radius: 14px;
        }
        #metricLabel {
            color: #7b6f62;
            font-size: 13px;
        }
        #metricValue {
            color: #2d241c;
            font-size: 27px;
            font-weight: 700;
        }
        #metricHelper {
            color: #8b7d70;
            font-size: 12px;
        }
        #sectionTitle {
            color: #2d241c;
            font-size: 16px;
            font-weight: 700;
        }
        #plainList {
            border: none;
            background: transparent;
            outline: none;
        }
        #plainList::item {
            background: #f8f3ea;
            border: 1px solid #e1d5c8;
            border-radius: 10px;
            margin: 4px 0;
            padding: 12px 14px;
            color: #45392e;
        }
        QTableWidget {
            background: transparent;
            border: none;
            gridline-color: #e6dacb;
            color: #342b23;
            alternate-background-color: #fcf8f2;
        }
        QHeaderView::section {
            background: #f2eadf;
            color: #5c4f43;
            border: none;
            border-bottom: 1px solid #e2d5c6;
            padding: 8px;
            font-weight: 600;
        }
        QTextBrowser#richCardText, QTextEdit#richCardText, QPlainTextEdit#richCardText,
        QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox {
            background: #fcf8f2;
            border: 1px solid #e1d5c8;
            border-radius: 10px;
            padding: 10px 12px;
            color: #3a3028;
        }
        QTextBrowser#richCardText {
            padding: 0;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QScrollBar:horizontal {
            max-height: 0px;
            min-height: 0px;
            height: 0px;
            background: transparent;
            border: none;
        }
        QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {
            background: transparent;
            border: none;
            width: 18px;
        }
        QCheckBox {
            color: #4a3d32;
            spacing: 8px;
        }
        QPushButton {
            background: #fffdf9;
            border: 1px solid #dacdbd;
            border-radius: 10px;
            padding: 8px 14px;
            color: #45392e;
        }
        #sidebarToggle {
            border: none;
            background: transparent;
            color: #75685d;
            font-size: 16px;
            padding: 4px 8px;
        }
        QPushButton:hover {
            background: #f4ede2;
        }
        QPushButton[danger="true"] {
            background: #fff0f0;
            color: #d94040;
            border: 1px solid #f0c0c0;
        }
        QPushButton[danger="true"]:hover {
            background: #ffe0e0;
        }
        QToolBar, QMenuBar, QStatusBar {
            background: #f7f5ef;
        }
        
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: rgba(160, 150, 140, 150);
            min-height: 30px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(140, 130, 120, 200);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
        QScrollBar:horizontal {
            border: none;
            background: transparent;
            height: 8px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: rgba(160, 150, 140, 150);
            min-width: 30px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: rgba(140, 130, 120, 200);
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            border: none;
            background: none;
            width: 0px;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
        }
    )");
}

void MainWindow::checkFrontendExists()
{
    const QStringList searchPaths = {
        QDir(QApplication::applicationDirPath()).absoluteFilePath("frontend_dist/index.html"),
        QDir(QApplication::applicationDirPath()).absoluteFilePath("../frontend_dist/index.html"),
        QDir::current().absoluteFilePath("frontend_dist/index.html")
    };

    for (const QString& path : searchPaths) {
        if (QFile::exists(path)) {
            m_frontendPath = QFileInfo(path).absolutePath();
            Logger::info("找到前端静态文件: " + m_frontendPath);
            return;
        }
    }

    m_frontendPath.clear();
    Logger::warning("未找到 frontend_dist，网页预览将依赖后端托管或本地开发服务。");
}

void MainWindow::startBackendServer()
{
    m_statusLabel->setText("正在启动后端服务...");
    updateBackendBadge(false, "启动中");

    m_serverThread = new HttpServerThread(this);
    connect(m_serverThread, &HttpServerThread::serverStarted, this, &MainWindow::onBackendStarted);
    connect(m_serverThread, &HttpServerThread::serverError, this, &MainWindow::onBackendError);
    m_serverThread->start();
}

void MainWindow::openBrowser()
{
    const QString url = m_serverReady ? m_serverUrl : "file:///" + m_frontendPath + "/index.html";
        Logger::info("打开网页预览: " + url);
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::refreshOverview()
{
    const QJsonObject overview = DashboardService::getOverview();
    const QJsonObject courses = overview["courses"].toObject();
    const QJsonObject goals = overview["goals"].toObject();

    m_totalCoursesValue->setText(QString::number(courses["totalCourses"].toInt()));
    m_gpaValue->setText(QString::number(courses["gpa"].toDouble(), 'f', 2));
    m_goalProgressValue->setText(QString("%1%").arg(goals["averageProgress"].toDouble(), 0, 'f', 1));
    m_achievementValue->setText(QString::number(overview["achievementsCount"].toInt()));
    m_experienceValue->setText(QString::number(overview["experiencesCount"].toInt()));
    m_roleValue->setText(QString::number(overview["rolesCount"].toInt()));
    m_activityValue->setText(QString::number(overview["activitiesCount"].toInt()));
    m_creditValue->setText(QString::number(courses["completedCredits"].toDouble(), 'f', 1));

    m_recommendationList->clear();
    const QJsonArray recommendations = DashboardService::getRecommendations();
    for (const auto& item : recommendations) {
        m_recommendationList->addItem(bullet(item.toString()));
    }
    if (m_recommendationList->count() == 0) {
        setupEmptyState(m_recommendationList, "暂无建议");
    }

    m_semesterList->clear();
    const QJsonArray semesters = CourseService::getSemesterStatistics();
    for (const auto& semesterValue : semesters) {
        const QJsonObject semester = semesterValue.toObject();
        m_semesterList->addItem(
            QString("%1  路 GPA %2 路 平均分 %3")
                .arg(semester["semester"].toString())
                .arg(QString::number(semester["gpa"].toDouble(), 'f', 2))
                .arg(QString::number(semester["avgScore"].toDouble(), 'f', 1)));
    }
    if (m_semesterList->count() == 0) {
        setupEmptyState(m_semesterList, "无学期数据");
    }
}

void MainWindow::refreshCourses()
{
    QList<Course> courses = CourseService::getAll();
    const QString keyword = m_courseSearchInput ? m_courseSearchInput->text().trimmed().toLower() : QString();
    const QString statusKeyword = m_courseStatusInput ? m_courseStatusInput->text().trimmed().toLower() : QString();
    const QString categoryKeyword = m_courseCategoryInput ? m_courseCategoryInput->text().trimmed().toLower() : QString();
    const QString sortKey = m_courseSortInput ? m_courseSortInput->text().trimmed().toLower() : QString("updated");

    QList<Course> filteredCourses;
    for (const Course& course : courses) {
        const bool matchSearch = keyword.isEmpty()
            || course.name.toLower().contains(keyword)
            || course.code.toLower().contains(keyword)
            || course.teacher.toLower().contains(keyword);
        const bool matchStatus = statusKeyword.isEmpty() || course.status.toLower().contains(statusKeyword);
        const bool matchCategory = categoryKeyword.isEmpty() || course.category.toLower().contains(categoryKeyword);
        if (matchSearch && matchStatus && matchCategory) {
            filteredCourses.append(course);
        }
    }

    std::sort(filteredCourses.begin(), filteredCourses.end(), [sortKey](const Course& left, const Course& right) {
        if (sortKey == "name") return left.name.toLower() < right.name.toLower();
        if (sortKey == "semester") return left.semester > right.semester;
        if (sortKey == "credits") return left.credits > right.credits;
        if (sortKey == "score") return left.score > right.score;
        if (sortKey == "gpa") return left.gradePoint > right.gradePoint;
        return left.updatedAt > right.updatedAt;
    });

    m_courseTable->setRowCount(filteredCourses.size());

    int completedCount = 0;
    for (int row = 0; row < filteredCourses.size(); ++row) {
        const Course& course = filteredCourses.at(row);
        if (course.status == "Completed") {
            ++completedCount;
        }

        const QStringList values = {
            course.name,
            course.code,
            course.semester,
            QString::number(course.credits, 'f', 1),
            course.score > 0 ? QString::number(course.score, 'f', 1) : "--",
            course.gradePoint > 0 ? QString::number(course.gradePoint, 'f', 2) : "--",
            course.status
        };

        for (int column = 0; column < values.size(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, course.id);
            m_courseTable->setItem(row, column, item);
        }
    }

    if (!filteredCourses.isEmpty()) {
        m_courseTable->selectRow(0);
    } else {
        m_courseTable->setRowCount(1);
        m_courseTable->setSpan(0, 0, 1, 7);
        auto* emptyItem = new QTableWidgetItem("\n\n暂无课程数据\n点击[新增课程]添加第一门课程\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(12); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_courseTable->setItem(0, 0, emptyItem);
    }

    m_courseSummaryLabel->setText(
        QString("当前显示 %1 / %2 门课程，其中已完成 %3 门。支持搜索、状态筛选和排序，并会同步影响总览、时间轴与简历导出。")
            .arg(filteredCourses.size())
            .arg(courses.size())
            .arg(completedCount));
}

void MainWindow::refreshRoles()
{
    QList<Role> roles = RoleService::getAll();
    const QString keyword = m_roleSearchInput ? m_roleSearchInput->text().trimmed().toLower() : QString();
    const QString typeFilter = m_roleTypeFilter ? m_roleTypeFilter->currentData().toString().toLower() : QString();

    QList<Role> filteredRoles;
    for (const Role& role : roles) {
        const bool matchSearch = keyword.isEmpty()
            || role.title.toLower().contains(keyword)
            || role.organization.toLower().contains(keyword)
            || role.description.toLower().contains(keyword);
        const bool matchType = typeFilter.isEmpty() || role.type.toLower().contains(typeFilter);
        if (matchSearch && matchType) {
            filteredRoles.append(role);
        }
    }

    const QJsonObject stats = RoleService::getStatistics();
    const QJsonObject typeBreakdown = stats["typeBreakdown"].toObject();

    m_rolesTotalValue->setText(QString::number(filteredRoles.size()));
    m_rolesActiveValue->setText(QString::number(stats["activeRoles"].toInt()));
    m_rolesTypeValue->setText(QString::number(typeBreakdown.keys().size()));

    m_roleList->clear();
    for (const Role& role : filteredRoles) {
        const QString summary = QString("%1\n%2 路 %3\n%4")
            .arg(safeText(role.title))
            .arg(joinDateRange(role.startDate, role.endDate, role.isActive, "至今"))
            .arg(safeText(role.organization, safeText(role.type)))
            .arg(shortBody(role.description, role.isActive ? "当前角色仍在进行中。" : "该角色阶段已完成。"));
        m_roleList->addItem(summary);
        m_roleList->item(m_roleList->count() - 1)->setData(Qt::UserRole, role.id);
    }
    if (m_roleList->count() == 0) {
        setupEmptyState(m_roleList, "暂无角色职责数据");
    } else {
        m_roleList->setCurrentRow(0);
    }

    const QString dominantType = typeBreakdown.isEmpty() ? "未分类" : typeBreakdown.keys().first();
    m_roleSummaryLabel->setText(
        QString("显示 %1 / %2 个角色，其中进行中 %3 个。主要类型：%4。支持搜索和类型筛选。")
            .arg(filteredRoles.size())
            .arg(roles.size())
            .arg(stats["activeRoles"].toInt())
            .arg(dominantType));
}

void MainWindow::refreshAchievements()
{
    QList<Achievement> achievements = AchievementService::getAll();
    const QString keyword = m_achievementSearchInput ? m_achievementSearchInput->text().trimmed().toLower() : QString();
    const QString typeFilter = m_achievementTypeFilter ? m_achievementTypeFilter->currentData().toString().toLower() : QString();
    const QString levelFilter = m_achievementLevelFilter ? m_achievementLevelFilter->currentData().toString().toLower() : QString();

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
    const QJsonObject typeBreakdown = stats["typeBreakdown"].toObject();
    const QJsonObject levelBreakdown = stats["levelBreakdown"].toObject();

    m_achievementTotalValue->setText(QString::number(filteredAchievements.size()));
    m_achievementVerifiedValue->setText(QString::number(stats["verifiedAchievements"].toInt()));
    m_achievementLevelValue->setText(QString::number(levelBreakdown.keys().size()));
    m_achievementTypeValue->setText(QString::number(typeBreakdown.keys().size()));

    m_achievementList->clear();
    for (const Achievement& achievement : filteredAchievements) {
        const QString meta = achievement.level.trimmed().isEmpty()
            ? safeText(achievement.type)
            : QString("%1 路 %2").arg(safeText(achievement.date, "日期未填写"), achievement.level);
        const QString detail = shortBody(
            achievement.description,
            achievement.organization.trimmed().isEmpty()
                ? "已记录一项新的成果。"
                : QString("归属机构：%1").arg(achievement.organization));
        m_achievementList->addItem(QString("%1\n%2\n%3")
                                       .arg(safeText(achievement.title))
                                       .arg(meta)
                                       .arg(detail));
        m_achievementList->item(m_achievementList->count() - 1)->setData(Qt::UserRole, achievement.id);
    }
    if (m_achievementList->count() == 0) {
        setupEmptyState(m_achievementList, "暂无成果记录数据");
    } else {
        m_achievementList->setCurrentRow(0);
    }

    const QString mainLevel = levelBreakdown.isEmpty() ? "未分级" : levelBreakdown.keys().first();
    m_achievementSummaryLabel->setText(
        QString("显示 %1 / %2 项成果，已验证 %3 项。主要级别：%4。支持搜索、类型和级别筛选。")
            .arg(filteredAchievements.size())
            .arg(achievements.size())
            .arg(stats["verifiedAchievements"].toInt())
            .arg(mainLevel));
}

void MainWindow::refreshExperiences()
{
    QList<Experience> experiences = ExperienceService::getAll();
    const QString keyword = m_experienceSearchInput ? m_experienceSearchInput->text().trimmed().toLower() : QString();
    const QString typeFilter = m_experienceTypeFilter ? m_experienceTypeFilter->currentData().toString().toLower() : QString();

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
    const QJsonObject typeBreakdown = stats["typeBreakdown"].toObject();

    m_experienceTotalValue->setText(QString::number(filteredExperiences.size()));
    m_experienceOngoingValue->setText(QString::number(stats["ongoingExperiences"].toInt()));
    m_experienceTypeValue->setText(QString::number(typeBreakdown.keys().size()));

    m_experienceList->clear();
    for (const Experience& experience : filteredExperiences) {
        const QString org = experience.organization.trimmed().isEmpty()
            ? safeText(experience.type)
            : experience.organization;
        const QString roleText = experience.role.trimmed().isEmpty() ? QString() : QString(" 路 %1").arg(experience.role);
        const QString body = shortBody(
            experience.description,
            experience.isOngoing ? "当前经历仍在进行中。" : "该经历阶段已完成。");
        m_experienceList->addItem(
            QString("%1\n%2 路 %3%4\n%5")
                .arg(safeText(experience.title))
                .arg(joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, "至今"))
                .arg(org)
                .arg(roleText)
                .arg(body));
        m_experienceList->item(m_experienceList->count() - 1)->setData(Qt::UserRole, experience.id);
    }
    if (m_experienceList->count() == 0) {
        setupEmptyState(m_experienceList, "暂无经历档案数据");
    } else {
        m_experienceList->setCurrentRow(0);
    }

    m_experienceSummaryLabel->setText(
        QString("显示 %1 / %2 段经历，其中进行中 %3 段。支持搜索和类型筛选。")
            .arg(filteredExperiences.size())
            .arg(experiences.size())
            .arg(stats["ongoingExperiences"].toInt()));
}

void MainWindow::refreshGoals()
{
    QList<Goal> goals = GoalService::getAll();
    const QJsonObject stats = GoalService::getStatistics();
    const QString keyword = m_goalSearchInput ? m_goalSearchInput->text().trimmed().toLower() : QString();
    const QString statusKeyword = m_goalStatusFilter ? m_goalStatusFilter->currentData().toString().toLower() : QString();
    const QString priorityKeyword = m_goalPriorityFilter ? m_goalPriorityFilter->currentData().toString().toLower() : QString();
    const QString sortKey = m_goalSortInput ? m_goalSortInput->text().trimmed().toLower() : QString("progress");

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

    m_goalTotalValue->setText(QString::number(stats["total"].toInt()));
    m_goalCompletedValue->setText(QString::number(stats["completed"].toInt()));
    m_goalProgressMetricValue->setText(QString("%1%").arg(stats["averageProgress"].toDouble(), 0, 'f', 1));

    m_goalList->clear();
    for (const Goal& goal : filteredGoals) {
        const QString progress = QString("%1% 路 %2")
            .arg(goal.progress(), 0, 'f', 1)
            .arg(safeText(goal.status));
        const QString deadline = safeText(goal.deadline, "截止时间未填写");
        const QString body = shortBody(
            goal.description,
            QString("目标值 %1 %2，当前值 %3 %4。")
                .arg(goal.targetValue, 0, 'f', 1)
                .arg(safeText(goal.unit, ""))
                .arg(goal.currentValue, 0, 'f', 1)
                .arg(safeText(goal.unit, "")));
        m_goalList->addItem(
            QString("%1\n%2 路 截止 %3\n%4")
                .arg(safeText(goal.title))
                .arg(progress)
                .arg(deadline)
                .arg(body));
        m_goalList->item(m_goalList->count() - 1)->setData(Qt::UserRole, goal.id);
    }
    if (m_goalList->count() == 0) {
        setupEmptyState(m_goalList, "暂无目标追踪数据");
    } else {
        m_goalList->setCurrentRow(0);
    }

    m_goalSummaryLabel->setText(
        QString("当前显示 %1 / %2 个目标，已完成 %3 个，平均进度 %4%。支持搜索、筛选和排序。")
            .arg(filteredGoals.size())
            .arg(goals.size())
            .arg(stats["completed"].toInt())
            .arg(QString::number(stats["averageProgress"].toDouble(), 'f', 1)));
}

void MainWindow::refreshTimeline()
{
    const QJsonArray events = AnalyticsService::getTimelineEvents();
    const QJsonObject report = AnalyticsService::generateReport();
    const QJsonArray strengths = report["strengths"].toArray();
    const QJsonArray risks = report["risks"].toArray();
    const QJsonArray suggestions = report["suggestions"].toArray();

    m_timelineEventCountValue->setText(QString::number(events.size()));
    m_timelineStrengthValue->setText(QString::number(strengths.size()));
    m_timelineRiskValue->setText(QString::number(risks.size()));

    m_timelineList->clear();
    for (const auto& eventValue : events) {
        const QJsonObject event = eventValue.toObject();
        m_timelineList->addItem(
            QString("%1\n%2 路 %3\n%4")
                .arg(safeText(event["title"].toString()))
                .arg(safeText(event["date"].toString(), "日期未填写"))
                .arg(safeText(event["subtitle"].toString(), event["type"].toString()))
                .arg(shortBody(event["description"].toString(), "已记录新的成长节点。")));
    }
    if (m_timelineList->count() == 0) {
        setupEmptyState(m_timelineList, "暂无时间轴事件");
    }

    m_timelineSuggestionList->clear();
    for (const auto& item : strengths) {
        m_timelineSuggestionList->addItem(QString("优势：%1").arg(item.toString()));
    }
    for (const auto& item : risks) {
        m_timelineSuggestionList->addItem(QString("风险：%1").arg(item.toString()));
    }
    for (const auto& item : suggestions) {
        m_timelineSuggestionList->addItem(QString("建议：%1").arg(item.toString()));
    }
    if (m_timelineSuggestionList->count() == 0) {
        setupEmptyState(m_timelineSuggestionList, "暂无阶段建议");
    }

    m_timelineSummaryLabel->setText(
        QString("时间轴从课程、角色、成果、经历和目标中提取事件，并配套生成阶段性分析。"));
}

void MainWindow::refreshResumeCandidates()
{
    if (!m_resumeCandidateList || !m_resumeCandidateTypeCombo) {
        return;
    }

    const QString type = m_resumeCandidateTypeCombo->currentData().toString();
    m_resumeCandidateList->clear();

    auto appendCandidate = [this](const QString& title, const QString& detail, const QString& snippet) {
        auto* item = new QListWidgetItem(QString("%1\n%2").arg(title, detail), m_resumeCandidateList);
        item->setData(Qt::UserRole, snippet);
        item->setToolTip(snippet);
        item->setSizeHint(QSize(0, 54));
    };

    if (type == "course") {
        const QList<Course> courses = CourseService::getAll();
        for (const Course& course : courses) {
            const QString title = safeText(course.name);
            const QString detail = QString("%1 · %2 学分 · %3")
                .arg(safeText(course.semester, "学期待补充"))
                .arg(QString::number(course.credits, 'f', 1))
                .arg(safeText(course.status, "状态待补充"));
            const QString snippet = QString("课程亮点：%1（%2），学分 %3，当前状态为 %4。%5")
                .arg(title)
                .arg(safeText(course.semester, "学期待补充"))
                .arg(QString::number(course.credits, 'f', 1))
                .arg(safeText(course.status, "状态待补充"))
                .arg(shortBody(course.description, "可突出课程学习成果、方法与能力提升。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "experience") {
        const QList<Experience> experiences = ExperienceService::getAll();
        for (const Experience& experience : experiences) {
            const QString title = safeText(experience.title);
            const QString detail = QString("%1 · %2")
                .arg(safeText(experience.organization, "组织待补充"))
                .arg(joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, "至今"));
            const QString snippet = QString("实践经历：在 %1 参与 %2，担任 %3。%4")
                .arg(safeText(experience.organization, "相关组织"))
                .arg(title)
                .arg(safeText(experience.role, "核心成员"))
                .arg(shortBody(experience.description, "可突出项目职责、方法与结果。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "achievement") {
        const QList<Achievement> achievements = AchievementService::getAll();
        for (const Achievement& achievement : achievements) {
            const QString title = safeText(achievement.title);
            const QString detail = QString("%1 · %2")
                .arg(safeText(achievement.level, "级别待补充"))
                .arg(safeText(achievement.date, "日期待补充"));
            const QString snippet = QString("成果记录：获得 %1（%2，%3）。%4")
                .arg(title)
                .arg(safeText(achievement.level, "级别待补充"))
                .arg(safeText(achievement.organization, "组织待补充"))
                .arg(shortBody(achievement.description, "可突出成果价值、贡献和影响。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "role") {
        const QList<Role> roles = RoleService::getAll();
        for (const Role& role : roles) {
            const QString title = safeText(role.title);
            const QString detail = QString("%1 · %2")
                .arg(safeText(role.organization, "组织待补充"))
                .arg(joinDateRange(role.startDate, role.endDate, role.isActive, "至今"));
            const QString snippet = QString("角色职责：在 %1 担任 %2。%3")
                .arg(safeText(role.organization, "相关组织"))
                .arg(title)
                .arg(shortBody(role.description, "可强调组织协调、沟通推进与执行成果。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "activity") {
        const QList<Activity> activities = ActivityService::getAll();
        for (const Activity& activity : activities) {
            const QString title = safeText(activity.name);
            const QString detail = QString("%1 · %2")
                .arg(safeText(activity.category, "类别待补充"))
                .arg(joinDateRange(activity.startDate, activity.endDate, activity.isActive, "至今"));
            const QString snippet = QString("课外活动：参与 %1（%2）。%3")
                .arg(title)
                .arg(safeText(activity.category, "类别待补充"))
                .arg(shortBody(activity.description, "可体现长期投入、协作方式与具体贡献。"));
            appendCandidate(title, detail, snippet);
        }
    }

    if (m_resumeCandidateList->count() == 0) {
        setupEmptyState(m_resumeCandidateList, "当前分类下还没有可插入的素材。");
    } else {
        m_resumeCandidateList->setCurrentRow(0);
    }
}

void MainWindow::refreshResume()
{
    const QJsonObject options = currentResumeOptions();
    const auto isVisibleSection = [this](const QString& section) {
        if (section == "education") return m_resumeEducationCheck && m_resumeEducationCheck->isChecked();
        if (section == "project") return m_resumeExperienceCheck && m_resumeExperienceCheck->isChecked();
        if (section == "skills") return m_resumeAchievementCheck && m_resumeAchievementCheck->isChecked();
        if (section == "intent") return m_resumeRoleCheck && m_resumeRoleCheck->isChecked();
        if (section == "custom") return m_resumeActivityCheck && m_resumeActivityCheck->isChecked();
        return true;
    };
    const auto selectedClass = [this](const QString& key) {
        return m_resumeSelectedSection == key ? QString(" section-selected") : QString();
    };
    const auto actionHtml = [](const QString& key) {
        return QString("<div class='resume-actions'><a href='copy:%1'>复制</a><a href='delete:%1'>删除</a></div>").arg(key);
    };

    const QString intentTitle = options["sectionTitleIntent"].toString();
    const QString eduTitle = options["sectionTitleEducation"].toString();
    const QString skillsTitle = options["sectionTitleSkills"].toString();
    const QString projectTitle = options["sectionTitleProjects"].toString();
    const QString internshipTitle = options["sectionTitleInternship"].toString();
    const QString awardsTitle = options["sectionTitleAwards"].toString();
    const QString customTitle = options["sectionTitleCustom"].toString();

    int sectionCount = 0;
    if (isVisibleSection("intent")) sectionCount++;
    if (isVisibleSection("education")) sectionCount++;
    if (isVisibleSection("skills")) sectionCount++;
    if (isVisibleSection("project")) sectionCount++;
    if (!options["internship"].toString().trimmed().isEmpty()) sectionCount++;
    if (!options["awards"].toString().trimmed().isEmpty()) sectionCount++;
    if (isVisibleSection("custom") && !options["customContent"].toString().trimmed().isEmpty()) sectionCount++;

    QString html;
    html += "<html><head><style>";
    html += "body{background:#f8fafc;color:#1f2937;font-family:'Microsoft YaHei',sans-serif;margin:0;}";
    html += ".resume-page{max-width:100%;margin:0 auto;padding:20px 24px;background:#ffffff;border-radius:12px;box-shadow:0 1px 3px rgba(0,0,0,0.1);}";
    html += ".resume-header{display:flex;gap:28px;align-items:flex-start;padding-bottom:28px;border-bottom:2px solid #e7edf7;margin-bottom:8px;}";
    html += ".avatar{width:100px;height:100px;border:2px solid #3b82f6;border-radius:12px;display:flex;align-items:center;justify-content:center;background:linear-gradient(135deg,#eff6ff,#dbeafe);color:#3b82f6;font-size:24px;font-weight:700;}";
    html += ".header-main{flex:1;}";
    html += ".name{font-size:32px;font-weight:700;color:#111827;margin:0 0 12px 0;letter-spacing:1px;}";
    html += ".meta{font-size:14px;color:#64748b;line-height:2.0;}";
    html += ".meta-item{display:inline-flex;align-items:center;gap:4px;margin-right:16px;}";
    html += ".intent-row{display:flex;gap:20px;flex-wrap:wrap;margin-top:14px;padding-top:14px;border-top:1px dashed #e2e8f0;}";
    html += ".intent-tag{background:#eff6ff;color:#3b82f6;padding:6px 14px;border-radius:20px;font-size:13px;font-weight:500;}";
    html += ".resume-section{position:relative;display:grid;grid-template-columns:140px 1fr;gap:24px;padding:20px 0;border-bottom:1px solid #f1f5f9;}";
    html += ".resume-section.section-selected{border:2px dashed #3b82f6;border-radius:12px;padding:20px 16px;background:#fafbff;}";
    html += ".section-label{font-size:15px;font-weight:700;color:#3b82f6;padding-top:2px;}";
    html += ".section-content{font-size:14px;color:#475467;line-height:1.85;white-space:pre-wrap;overflow-wrap:anywhere;}";
    html += ".section-title-main{font-size:16px;font-weight:700;color:#1f2937;margin-bottom:6px;}";
    html += ".section-sub{font-size:13px;color:#64748b;margin-bottom:10px;}";
    html += ".resume-actions{position:absolute;right:10px;top:10px;display:flex;gap:8px;}";
    html += ".resume-actions a{font-size:12px;color:#3b82f6;text-decoration:none;background:#eff6ff;border:1px solid #bfdbfe;border-radius:6px;padding:4px 10px;}";
    html += ".resume-actions a:hover{background:#dbeafe;}";
    html += ".section-anchor{text-decoration:none;color:inherit;display:block;}";
    html += ".bullet-list{margin:0;padding-left:20px;}";
    html += ".bullet-list li{margin:6px 0;line-height:1.7;}";
    html += ".highlight{color:#3b82f6;font-weight:500;}";
    html += ".divider{height:1px;background:linear-gradient(90deg,transparent,#e2e8f0,transparent);margin:16px 0;}";
    html += "</style></head><body><div class='resume-page'>";
    html += "<div class='resume-header'>";
    html += "<div class='avatar'>照<br/>片</div>";
    html += "<div class='header-main'>";
    html += QString("<div class='name'>%1</div>").arg(safeText(options["name"].toString(), "个人成长规划简历"));
    html += QString("<div class='meta'><span class='meta-item'>📍 %1</span><span class='meta-item'>🎂 %2</span><span class='meta-item'>📞 %3</span><span class='meta-item'>✉️ %4</span></div>")
        .arg(safeText(options["city"].toString(), "城市待补充"))
        .arg(safeText(options["age"].toString(), "年龄待补充"))
        .arg(safeText(options["phone"].toString(), "电话待补充"))
        .arg(safeText(options["email"].toString(), "邮箱待补充"));
    html += QString("<div class='intent-row'><span class='intent-tag'>💼 %1</span><span class='intent-tag'>🎯 %2</span></div>")
        .arg(safeText(options["title"].toString(), "职位头衔"))
        .arg(safeText(options["intent"].toString(), "求职方向"));
    html += "</div></div>";

    if (isVisibleSection("intent")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("intent"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:intent'>%1</a></div>").arg(intentTitle);
        html += "<div class='section-content'>";
        html += actionHtml("intent");
        html += QString("<div>%1</div>").arg(safeText(options["summary"].toString(), "请补充你的求职意向和个人简介。"));
        html += "</div></div>";
    }

    if (isVisibleSection("education")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("education"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:education'>%1</a></div>").arg(eduTitle);
        html += "<div class='section-content'>";
        html += actionHtml("education");
        html += QString("<div class='section-title-main'>%1</div>").arg(safeText(options["school"].toString(), "学校待补充"));
        html += QString("<div class='section-sub'>%1 · %2</div>").arg(safeText(options["major"].toString(), "专业待补充")).arg(safeText(options["degree"].toString(), "学历待补充"));
        html += QString("<div>%1</div>").arg(safeText(options["educationBody"].toString(), "请补充教育背景描述。").replace("\n", "<br/>"));
        html += "</div></div>";
    }

    if (isVisibleSection("skills")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("skills"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:skills'>%1</a></div>").arg(skillsTitle);
        html += "<div class='section-content'>";
        html += actionHtml("skills");
        html += QString("<div>%1</div>").arg(safeText(options["skillsBody"].toString(), "请补充技能特长。").replace("\n", "<br/>"));
        html += "</div></div>";
    }

    if (isVisibleSection("project")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("project"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:project'>%1</a></div>").arg(projectTitle);
        html += "<div class='section-content'>";
        html += actionHtml("project");
        html += QString("<div class='section-title-main'>%1</div>").arg(safeText(options["projectName"].toString(), "项目名称待补充"));
        html += QString("<div>%1</div>").arg(safeText(options["projectBody"].toString(), "请补充项目经验。").replace("\n", "<br/>"));
        html += "</div></div>";
    }

    QString internship = options["internship"].toString().trimmed();
    if (!internship.isEmpty()) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("internship"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:internship'>%1</a></div>").arg(internshipTitle);
        html += "<div class='section-content'>";
        html += actionHtml("internship");
        html += QString("<div>%1</div>").arg(internship.replace("\n", "<br/>"));
        html += "</div></div>";
    }

    QString awards = options["awards"].toString().trimmed();
    if (!awards.isEmpty()) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("awards"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:awards'>%1</a></div>").arg(awardsTitle);
        html += "<div class='section-content'>";
        html += actionHtml("awards");
        html += QString("<div>%1</div>").arg(awards.replace("\n", "<br/>"));
        html += "</div></div>";
    }

    if (isVisibleSection("custom") && !options["customContent"].toString().trimmed().isEmpty()) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("custom"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:custom'>%1</a></div>").arg(customTitle);
        html += "<div class='section-content'>";
        html += actionHtml("custom");
        html += QString("<div>%1</div>").arg(options["customContent"].toString().replace("\n", "<br/>"));
        html += "</div></div>";
    }

    html += "</div></body></html>";

    if (m_resumePreview) {
        m_resumePreview->setHtml(html);
    }
    if (m_resumeSectionCountValue) {
        m_resumeSectionCountValue->setText(QString::number(sectionCount));
    }
    if (m_resumeIdentityValue) {
        m_resumeIdentityValue->setText(safeText(options["title"].toString(), "个人成长规划简历"));
    }
    refreshResumeEditorPanel();
}

void MainWindow::refreshResumeEditorPanel()
{
    if (!m_resumeEditorTitleLabel || !m_resumeSectionVisibleCheck) {
        return;
    }

    bool visible = true;
    QString title = "教育背景";
    if (m_resumeSelectedSection == "intent") {
        title = "求职意向";
        visible = m_resumeRoleCheck ? m_resumeRoleCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "education") {
        title = "教育背景";
        visible = m_resumeEducationCheck ? m_resumeEducationCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "skills") {
        title = "技能特长";
        visible = m_resumeAchievementCheck ? m_resumeAchievementCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "project") {
        title = "项目经验";
        visible = m_resumeExperienceCheck ? m_resumeExperienceCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "custom") {
        title = "自我评价";
        visible = m_resumeActivityCheck ? m_resumeActivityCheck->isChecked() : true;
    }

    m_resumeEditorTitleLabel->setText(title);
    QSignalBlocker blocker(m_resumeSectionVisibleCheck);
    m_resumeSectionVisibleCheck->setChecked(visible);
}

void MainWindow::setSelectedResumeSection(const QString& sectionKey)
{
    m_resumeSelectedSection = sectionKey;
    refreshResumeEditorPanel();
    refreshResume();
}

void MainWindow::updateBackendBadge(bool ready, const QString& detail)
{
    const QString state = ready ? "运行中" : "未就绪";
    const QString extra = detail.isEmpty() ? QString() : QString(" 路 %1").arg(detail);
    if (m_statusLabel) {
        m_statusLabel->setText(QString("后端状态：%1%2").arg(state, extra));
    }
}

void MainWindow::addRole()
{
    RoleEditorDialog dialog(this);
    dialog.setWindowTitle("新增角色");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Role role = dialog.role();
    const Role created = RoleService::create(role);
    if (created.id == 0) {
        ToastNotification::display(this, "角色未能成功写入数据库。");
        return;
    }

    refreshRoles();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "角色已创建。");
}

void MainWindow::editSelectedRole()
{
    if (!m_roleList || !m_roleList->currentItem()) {
        ToastNotification::display(this, "请先选择一个角色。");
        return;
    }

    const int roleId = m_roleList->currentItem()->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }

    Role role = RoleService::getById(roleId);
    if (role.id == 0) {
        ToastNotification::display(this, "未找到对应角色记录。");
        return;
    }

    RoleEditorDialog dialog(this);
    dialog.setWindowTitle("编辑角色");
    dialog.setRole(role);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Role updated = dialog.role();
    const Role saved = RoleService::update(roleId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "角色更新失败。");
        return;
    }

    refreshRoles();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "角色已更新。");
}

void MainWindow::removeSelectedRole()
{
    if (!m_roleList || !m_roleList->currentItem()) {
        ToastNotification::display(this, "请先选择一个角色。");
        return;
    }

    const int roleId = m_roleList->currentItem()->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }

    const QString title = m_roleList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(this, "删除角色",
        QString("确定要删除角色“%1”吗？此操作会同步影响时间轴和简历。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!RoleService::remove(roleId)) {
        ToastNotification::display(this, "角色删除失败，请稍后再试。");
        return;
    }

    refreshRoles();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "角色已删除。");
}

void MainWindow::addAchievement()
{
    AchievementEditorDialog dialog(this);
    dialog.setWindowTitle("新增成果");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Achievement achievement = dialog.achievement();
    const Achievement created = AchievementService::create(achievement);
    if (created.id == 0) {
        ToastNotification::display(this, "成果未能成功写入数据库。");
        return;
    }

    refreshAchievements();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "成果已创建。");
}

void MainWindow::editSelectedAchievement()
{
    if (!m_achievementList || !m_achievementList->currentItem()) {
        ToastNotification::display(this, "请先选择一条成果。");
        return;
    }

    const int achievementId = m_achievementList->currentItem()->data(Qt::UserRole).toInt();
    if (achievementId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }

    Achievement achievement = AchievementService::getById(achievementId);
    if (achievement.id == 0) {
        ToastNotification::display(this, "未找到对应成果记录。");
        return;
    }

    AchievementEditorDialog dialog(this);
    dialog.setWindowTitle("编辑成果");
    dialog.setAchievement(achievement);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Achievement updated = dialog.achievement();
    const Achievement saved = AchievementService::update(achievementId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "成果更新失败。");
        return;
    }

    refreshAchievements();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "成果已更新。");
}

void MainWindow::removeSelectedAchievement()
{
    if (!m_achievementList || !m_achievementList->currentItem()) {
        ToastNotification::display(this, "请先选择一条成果。");
        return;
    }

    const int achievementId = m_achievementList->currentItem()->data(Qt::UserRole).toInt();
    if (achievementId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }

    const QString title = m_achievementList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(this, "删除成果",
        QString("确定要删除成果“%1”吗？此操作会影响总览、时间轴和简历。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!AchievementService::remove(achievementId)) {
        ToastNotification::display(this, "成果删除失败，请稍后再试。");
        return;
    }

    refreshAchievements();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "成果已删除。");
}

void MainWindow::addExperience()
{
    ExperienceEditorDialog dialog(this);
    dialog.setWindowTitle("新增经历");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience experience = dialog.experience();
    const Experience created = ExperienceService::create(experience);
    if (created.id == 0) {
        ToastNotification::display(this, "经历未能成功写入数据库。");
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "经历已创建。");
}

void MainWindow::editSelectedExperience()
{
    if (!m_experienceList || !m_experienceList->currentItem()) {
        ToastNotification::display(this, "请先选择一段经历。");
        return;
    }

    const int experienceId = m_experienceList->currentItem()->data(Qt::UserRole).toInt();
    if (experienceId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }

    Experience experience = ExperienceService::getById(experienceId);
    if (experience.id == 0) {
        ToastNotification::display(this, "未找到对应经历记录。");
        return;
    }

    ExperienceEditorDialog dialog(this);
    dialog.setWindowTitle("编辑经历");
    dialog.setExperience(experience);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience updated = dialog.experience();
    const Experience saved = ExperienceService::update(experienceId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "经历更新失败。");
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "经历已更新。");
}

void MainWindow::removeSelectedExperience()
{
    if (!m_experienceList || !m_experienceList->currentItem()) {
        ToastNotification::display(this, "请先选择一段经历。");
        return;
    }

    const int experienceId = m_experienceList->currentItem()->data(Qt::UserRole).toInt();
    if (experienceId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }

    const QString title = m_experienceList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(this, "删除经历",
        QString("确定要删除经历“%1”吗？此操作会影响时间轴、简历和 AI 分析。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!ExperienceService::remove(experienceId)) {
        ToastNotification::display(this, "经历删除失败，请稍后再试。");
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "经历已删除。");
}

void MainWindow::addCourse()
{
    CourseEditorDialog dialog(this);
    dialog.setWindowTitle("新增课程");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Course course = dialog.course();
    const Course created = CourseService::create(course);
    if (created.id == 0) {
        ToastNotification::display(this, "课程未能成功写入数据库。");
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "课程已创建。");
}

void MainWindow::editSelectedCourse()
{
    if (!m_courseTable || m_courseTable->currentRow() < 0) {
        ToastNotification::display(this, "请先在课程表中选择一门课程。");
        return;
    }

    const QTableWidgetItem* idItem = m_courseTable->item(m_courseTable->currentRow(), 0);
    if (!idItem) {
        ToastNotification::display(this, "当前选中行没有有效课程数据。");
        return;
    }

    const int courseId = idItem->data(Qt::UserRole).toInt();
    Course course = CourseService::getById(courseId);
    if (course.id == 0) {
        ToastNotification::display(this, "未找到对应课程记录。");
        return;
    }

    CourseEditorDialog dialog(this);
    dialog.setWindowTitle("编辑课程");
    dialog.setCourse(course);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Course updated = dialog.course();
    const Course saved = CourseService::update(courseId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "课程更新失败。");
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "课程已更新。");
}

void MainWindow::removeSelectedCourse()
{
    if (!m_courseTable || m_courseTable->currentRow() < 0) {
        ToastNotification::display(this, "请先在课程表中选择一门课程。");
        return;
    }

    const QTableWidgetItem* idItem = m_courseTable->item(m_courseTable->currentRow(), 0);
    if (!idItem) {
        return;
    }

    const int courseId = idItem->data(Qt::UserRole).toInt();
    const QString courseName = idItem->text();
    const auto result = QMessageBox::question(
        this,
        "删除课程",
        QString("确定要删除课程“%1”吗？此操作会影响总览、时间轴和简历导出。").arg(courseName));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!CourseService::remove(courseId)) {
        ToastNotification::display(this, "课程删除失败，请稍后再试。");
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "课程已删除。");
}

void MainWindow::addGoal()
{
    GoalEditorDialog dialog(this);
    dialog.setWindowTitle("新增目标");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal goal = dialog.goal();
    const Goal created = GoalService::create(goal);
    if (created.id == 0) {
        ToastNotification::display(this, "目标未能成功写入数据库。");
        return;
    }

    refreshGoals();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "目标已创建。");
}

void MainWindow::editSelectedGoal()
{
    if (!m_goalList || !m_goalList->currentItem()) {
        ToastNotification::display(this, "请先选择一个目标。");
        return;
    }

    const int goalId = m_goalList->currentItem()->data(Qt::UserRole).toInt();
    if (goalId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能编辑。");
        return;
    }

    Goal goal = GoalService::getById(goalId);
    if (goal.id == 0) {
        ToastNotification::display(this, "未找到对应目标记录。");
        return;
    }

    GoalEditorDialog dialog(this);
    dialog.setWindowTitle("编辑目标");
    dialog.setGoal(goal);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal updated = dialog.goal();
    const Goal saved = GoalService::update(goalId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "目标更新失败。");
        return;
    }

    refreshGoals();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "目标已更新。");
}

void MainWindow::removeSelectedGoal()
{
    if (!m_goalList || !m_goalList->currentItem()) {
        ToastNotification::display(this, "请先选择一个目标。");
        return;
    }

    const int goalId = m_goalList->currentItem()->data(Qt::UserRole).toInt();
    if (goalId <= 0) {
        ToastNotification::display(this, "当前项是占位信息，暂时不能删除。");
        return;
    }

    const QString title = m_goalList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(
        this,
        "删除目标",
        QString("确定要删除目标“%1”吗？此操作会影响总览、时间轴和 AI 建议。").arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!GoalService::remove(goalId)) {
        ToastNotification::display(this, "目标删除失败，请稍后再试。");
        return;
    }

    refreshGoals();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "目标已删除。");
}

QJsonObject MainWindow::currentResumeOptions() const
{
    if (!m_resumeNameInput) {
        return defaultResumeOptions();
    }

    QJsonObject options;
    options["name"] = safeText(m_resumeNameInput->text(), "个人发展档案");
    options["title"] = safeText(m_resumeTitleInput->text(), "个人成长规划简历");
    options["age"] = safeText(m_resumeAgeInput ? m_resumeAgeInput->text() : QString(), "21岁");
    options["city"] = safeText(m_resumeCityInput ? m_resumeCityInput->text() : QString(), "北京");
    options["email"] = m_resumeEmailInput->text().trimmed();
    options["phone"] = m_resumePhoneInput->text().trimmed();
    options["intent"] = safeText(m_resumeIntentInput ? m_resumeIntentInput->text() : QString(), "大数据工程师");
    options["school"] = safeText(m_resumeSchoolInput ? m_resumeSchoolInput->text() : QString(), "学校待补充");
    options["major"] = safeText(m_resumeMajorInput ? m_resumeMajorInput->text() : QString(), "专业待补充");
    options["degree"] = safeText(m_resumeDegreeInput ? m_resumeDegreeInput->text() : QString(), "学历待补充");
    options["summary"] = shortBody(m_resumeSummaryInput->toPlainText(), "基于课程、经历、成果与目标自动生成的综合简历预览。");
    options["educationBody"] = m_resumeEducationBodyInput ? m_resumeEducationBodyInput->toPlainText().trimmed() : QString();
    options["skillsBody"] = m_resumeSkillsBodyInput ? m_resumeSkillsBodyInput->toPlainText().trimmed() : QString();
    options["projectName"] = safeText(m_resumeProjectNameInput ? m_resumeProjectNameInput->text() : QString(), "项目名称待补充");
    options["projectBody"] = m_resumeProjectBodyInput ? m_resumeProjectBodyInput->toPlainText().trimmed() : QString();
    options["internship"] = m_resumeInternshipInput ? m_resumeInternshipInput->toPlainText().trimmed() : QString();
    options["awards"] = m_resumeAwardsInput ? m_resumeAwardsInput->toPlainText().trimmed() : QString();
    options["customContent"] = m_resumeCustomContentInput ? m_resumeCustomContentInput->toPlainText().trimmed() : QString();
    options["sectionTitleIntent"] = QString("求职意向");
    options["sectionTitleEducation"] = QString("教育背景");
    options["sectionTitleSkills"] = QString("技能特长");
    options["sectionTitleProjects"] = QString("项目经验");
    options["sectionTitleInternship"] = QString("实习经历");
    options["sectionTitleAwards"] = QString("竞赛获奖");
    options["sectionTitleCustom"] = QString("自我评价");
    options["includeEducation"] = m_resumeEducationCheck->isChecked();
    options["includeExperience"] = m_resumeExperienceCheck->isChecked();
    options["includeAchievements"] = m_resumeAchievementCheck->isChecked();
    options["includeRoles"] = m_resumeRoleCheck->isChecked();
    options["includeActivities"] = m_resumeActivityCheck->isChecked();
    return options;
}

void MainWindow::resetResumeOptions()
{
    const QJsonObject options = defaultResumeOptions();
    if (!m_resumeNameInput) {
        return;
    }

    const QSignalBlocker b1(m_resumeNameInput);
    const QSignalBlocker b2(m_resumeTitleInput);
    const QSignalBlocker b3(m_resumeEmailInput);
    const QSignalBlocker b4(m_resumePhoneInput);
    const QSignalBlocker b5(m_resumeSummaryInput);
    const QSignalBlocker b6(m_resumeCustomContentInput);
    const QSignalBlocker b7(m_resumeAgeInput);
    const QSignalBlocker b8(m_resumeCityInput);
    const QSignalBlocker b9(m_resumeIntentInput);
    const QSignalBlocker b10(m_resumeSchoolInput);
    const QSignalBlocker b11(m_resumeMajorInput);
    const QSignalBlocker b12(m_resumeDegreeInput);
    const QSignalBlocker b13(m_resumeEducationBodyInput);
    const QSignalBlocker b14(m_resumeSkillsBodyInput);
    const QSignalBlocker b15(m_resumeProjectNameInput);
    const QSignalBlocker b16(m_resumeProjectBodyInput);
    const QSignalBlocker b17(m_resumeEducationCheck);
    const QSignalBlocker b18(m_resumeExperienceCheck);
    const QSignalBlocker b19(m_resumeAchievementCheck);
    const QSignalBlocker b20(m_resumeRoleCheck);
    const QSignalBlocker b21(m_resumeActivityCheck);

    m_resumeNameInput->setText(options["name"].toString());
    m_resumeTitleInput->setText(options["title"].toString());
    if (m_resumeAgeInput) m_resumeAgeInput->setText(options["age"].toString());
    if (m_resumeCityInput) m_resumeCityInput->setText(options["city"].toString());
    m_resumeEmailInput->setText(options["email"].toString());
    m_resumePhoneInput->setText(options["phone"].toString());
    if (m_resumeIntentInput) m_resumeIntentInput->setText(options["intent"].toString());
    if (m_resumeSchoolInput) m_resumeSchoolInput->setText(options["school"].toString());
    if (m_resumeMajorInput) m_resumeMajorInput->setText(options["major"].toString());
    if (m_resumeDegreeInput) m_resumeDegreeInput->setText(options["degree"].toString());
    m_resumeSummaryInput->setPlainText(options["summary"].toString());
    if (m_resumeEducationBodyInput) m_resumeEducationBodyInput->setPlainText(options["educationBody"].toString());
    if (m_resumeSkillsBodyInput) m_resumeSkillsBodyInput->setPlainText(options["skillsBody"].toString());
    if (m_resumeProjectNameInput) m_resumeProjectNameInput->setText(options["projectName"].toString());
    if (m_resumeProjectBodyInput) m_resumeProjectBodyInput->setPlainText(options["projectBody"].toString());
    if (m_resumeCustomContentInput) {
        m_resumeCustomContentInput->setPlainText(options["customContent"].toString());
    }
    m_resumeEducationCheck->setChecked(options["includeEducation"].toBool(true));
    m_resumeExperienceCheck->setChecked(options["includeExperience"].toBool(true));
    m_resumeAchievementCheck->setChecked(options["includeAchievements"].toBool(true));
    m_resumeRoleCheck->setChecked(options["includeRoles"].toBool(true));
    m_resumeActivityCheck->setChecked(options["includeActivities"].toBool(false));
    refreshResumeCandidates();
    refreshResume();
}

void MainWindow::applyAiToResumeSummary()
{
    // This method is now handled by AiPanelWidget signal connection in setupUi
    // Keeping for backward compatibility but functionality moved to setupUi
}

void MainWindow::createGoalFromAiSuggestion()
{
    // This method is now handled by AiPanelWidget signal connection in setupUi
    // Keeping for backward compatibility but functionality moved to setupUi
}

void MainWindow::exportResumeJson()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "导出 JSON 简历", QDir::homePath() + "/resume.json", "JSON Files (*.json)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ToastNotification::display(this, "无法写入 JSON 文件。");
        return;
    }
    file.write(ResumeService::exportJson(currentResumeOptions()));
    file.close();
    ToastNotification::display(this, "JSON 简历已导出。");
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::exportResumeHtml()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "导出 HTML 简历", QDir::homePath() + "/resume.html", "HTML Files (*.html)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ToastNotification::display(this, "无法写入 HTML 文件。");
        return;
    }
    file.write(ResumeService::exportHtml(currentResumeOptions()));
    file.close();
    ToastNotification::display(this, "HTML 简历已导出。");
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::exportResumePdf()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "导出 PDF 简历", QDir::homePath() + "/resume.pdf", "PDF Files (*.pdf)");
    if (path.isEmpty()) {
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(QString::fromUtf8(ResumeService::exportHtml(currentResumeOptions())));
    doc.setDocumentMargin(20);
    doc.print(&printer);

    ToastNotification::display(this, "PDF 简历已导出。");
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::runAiAnalysis(const QString& type)
{
    // This method is now handled by AiPanelWidget signal connection in setupUi
    if (m_aiPanel) {
        emit m_aiPanel->analysisRequested(type);
    }
}

void MainWindow::sendAiChat()
{
    // This method is now handled by AiPanelWidget internally
    // Keeping for backward compatibility
}

void MainWindow::refreshActivities() {
    QList<Activity> list = ActivityService::getAll();
    QString kw = m_activitySearchInput ? m_activitySearchInput->text().trimmed().toLower() : "";
    QString cat = m_activityCategoryFilter ? m_activityCategoryFilter->currentData().toString().toLower() : "";
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
    m_activityTotalValue->setText(QString::number(totalAct));
    m_activityFavoriteValue->setText(QString::number(favAct));
    m_activityActiveValue->setText(QString::number(actAct));
    
    m_activityList->clear();
    for (auto& a : filtered) {
        QString timeRange = a.endDate.isEmpty() ? (a.startDate + (a.isActive ? "至今" : "")) : (a.startDate + " - " + a.endDate);
        QString txt = QString("%1 %2\n%3\n%4")
            .arg(a.isFavorite ? "★" : "").arg(a.name)
            .arg(a.category + " | " + timeRange)
            .arg(a.description);
        QListWidgetItem* item = new QListWidgetItem(txt, m_activityList);
        item->setData(Qt::UserRole, a.id);
    }
    if (m_activityList->count() == 0) {
        setupEmptyState(m_activityList, "暂无课外活动记录");
    }
    m_activitySummaryLabel->setText(QString("显示 %1 / %2 项活动记录").arg(filtered.size()).arg(list.size()));
}

void MainWindow::addActivity() {
    ActivityEditorDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        Activity act = dlg.activity();
        ActivityService::create(act);
        refreshActivities();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "活动已创建。");
    }
}

void MainWindow::editSelectedActivity() {
    if (!m_activityList->currentItem()) return;
    int id = m_activityList->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0) return;
    Activity a = ActivityService::getById(id);
    ActivityEditorDialog dlg(this);
    dlg.setActivity(a);
    if(dlg.exec() == QDialog::Accepted) {
        Activity act = dlg.activity();
        ActivityService::update(id, act);
        refreshActivities();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "活动已更新。");
    }
}

void MainWindow::removeSelectedActivity() {
    if (!m_activityList->currentItem()) return;
    int id = m_activityList->currentItem()->data(Qt::UserRole).toInt();
    if (id > 0 && QMessageBox::question(this, "删除活动", "确定要删除该活动记录吗？此操作会同步影响总览和时间轴。") == QMessageBox::Yes) {
        ActivityService::remove(id);
        refreshActivities();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "活动已删除。");
    }
}

void MainWindow::refreshJobs() {
    QList<Job> list = JobService::getAll();
    QString kw = m_jobSearchInput ? m_jobSearchInput->text().trimmed().toLower() : "";
    QString stat = m_jobStatusInput ? m_jobStatusInput->text().trimmed().toLower() : "";
    QList<Job> filtered;
    for (auto& j : list) {
        if (kw.isEmpty() || j.title.toLower().contains(kw) || j.company.toLower().contains(kw) || j.location.toLower().contains(kw)) {
            if (stat.isEmpty() || (j.isActive && stat == "active") || (!j.isActive && stat == "inactive")) {
                filtered.append(j);
            }
        }
    }
    
    int totalJob = list.size();
    int actJob = 0;
    double totalRatio = 0.0;
    for (const auto& j : list) {
        if (j.isActive) actJob++;
        if (!j.requirements.isEmpty()) {
            int metCount = 0;
            for (const auto& req : j.requirements) {
                if (req.met) metCount++;
            }
            totalRatio += (double)metCount / j.requirements.size();
        }
    }
    double avgRatio = totalJob > 0 ? (totalRatio / totalJob) : 0.0;
    
    m_jobTotalValue->setText(QString::number(totalJob));
    m_jobActiveValue->setText(QString::number(actJob));
    m_jobRequirementValue->setText(QString("%1%").arg(avgRatio * 100.0, 0, 'f', 1));
    
    m_jobList->clear();
    for (auto& j : filtered) {
        QString txt = QString("%1\n%2 - %3\n优先级: %4")
            .arg(j.title)
            .arg(j.company, j.location)
            .arg(j.priority);
        QListWidgetItem* item = new QListWidgetItem(txt, m_jobList);
        item->setData(Qt::UserRole, j.id);
    }
    if (m_jobList->count() == 0) {
        setupEmptyState(m_jobList, "暂无目标岗位数据");
    }
    m_jobSummaryLabel->setText(QString("显示 %1 / %2 项目标岗位").arg(filtered.size()).arg(list.size()));
    if(m_jobRequirementList) m_jobRequirementList->clear();
    if(m_jobRequirementSummaryLabel) m_jobRequirementSummaryLabel->setText("请在左侧选择岗位");
}

void MainWindow::addJob() {
    JobEditorDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        Job jb = dlg.job();
        JobService::create(jb);
        refreshJobs();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "岗位已创建。");
    }
}

void MainWindow::editSelectedJob() {
    if (!m_jobList->currentItem()) return;
    int id = m_jobList->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0) return;
    Job j = JobService::getById(id);
    JobEditorDialog dlg(this);
    dlg.setJob(j);
    if(dlg.exec() == QDialog::Accepted) {
        Job jb = dlg.job();
        JobService::update(id, jb);
        refreshJobs();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "岗位已更新。");
    }
}

void MainWindow::removeSelectedJob() {
    if (!m_jobList->currentItem()) return;
    int id = m_jobList->currentItem()->data(Qt::UserRole).toInt();
    if (id > 0 && QMessageBox::question(this, "删除岗位", "确定要删除该目标岗位吗？此操作会同步影响总览和时间轴。") == QMessageBox::Yes) {
        JobService::remove(id);
        refreshJobs();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "岗位已删除。");
    }
}

void MainWindow::toggleSelectedJobRequirement() {
    if (!m_jobList->currentItem() || !m_jobRequirementList->currentItem()) return;
    int id = m_jobList->currentItem()->data(Qt::UserRole).toInt();
    int reqIdx = m_jobRequirementList->currentItem()->data(Qt::UserRole).toInt();
    if (id <= 0 || reqIdx < 0) return;
    Job j = JobService::getById(id);
    if (reqIdx < j.requirements.size()) {
        j.requirements[reqIdx].met = !j.requirements[reqIdx].met;
        JobService::update(id, j);
        
        // just partial refresh UI
        m_jobRequirementList->currentItem()->setText(QString("[%1] %2").arg(j.requirements[reqIdx].met ? "x" : " ").arg(j.requirements[reqIdx].text));
        int metCount = 0;
        for (const auto& r : j.requirements) { if (r.met) metCount++; }
        m_jobRequirementSummaryLabel->setText(QString("此岗位共有 %1 项要求，已匹配 %2 项。").arg(j.requirements.size()).arg(metCount));
    }
}

void MainWindow::refreshAnalysis() {
    QJsonObject report = AnalyticsService::generateReport();
    QJsonArray semesters = report["semesters"].toArray();
    QJsonArray strengths = report["strengths"].toArray();
    QJsonArray risks = report["risks"].toArray();
    QJsonArray suggestions = report["suggestions"].toArray();

    m_analysisSemesterValue->setText(QString::number(semesters.size()));
    m_analysisSuggestionValue->setText(QString::number(suggestions.size() + strengths.size() + risks.size()));

    m_analysisSemesterTable->setRowCount(semesters.size());
    for (int i = 0; i < semesters.size(); ++i) {
        QJsonObject s = semesters[i].toObject();
        m_analysisSemesterTable->setItem(i, 0, new QTableWidgetItem(s["semester"].toString()));
        m_analysisSemesterTable->setItem(i, 1, new QTableWidgetItem(QString::number(s["credits"].toDouble(), 'f', 1)));
        m_analysisSemesterTable->setItem(i, 2, new QTableWidgetItem(QString::number(s["gpa"].toDouble(), 'f', 2)));
        m_analysisSemesterTable->setItem(i, 3, new QTableWidgetItem(s["rank"].toString()));
    }
    if (semesters.isEmpty()) {
        m_analysisSemesterTable->setRowCount(1);
        m_analysisSemesterTable->setSpan(0, 0, 1, 4);
        auto* emptyItem = new QTableWidgetItem("\n暂无学期对比数据\n添加课程后自动生成\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(11); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_analysisSemesterTable->setItem(0, 0, emptyItem);
    }

    QList<PeerBenchmark> peers = PeerBenchmarkService::getAll();
    m_analysisPeerValue->setText(QString::number(peers.size()));
    m_analysisPeerTable->setRowCount(peers.size());
    for(int i = 0; i < peers.size(); ++i) {
        m_analysisPeerTable->setItem(i, 0, new QTableWidgetItem(peers[i].name));
        m_analysisPeerTable->setItem(i, 1, new QTableWidgetItem(peers[i].major));
        m_analysisPeerTable->setItem(i, 2, new QTableWidgetItem(peers[i].semester));
        m_analysisPeerTable->setItem(i, 3, new QTableWidgetItem(QString::number(peers[i].gpa, 'f', 2)));
        m_analysisPeerTable->setItem(i, 4, new QTableWidgetItem(QString::number(peers[i].achievementsCount)));
        m_analysisPeerTable->setItem(i, 5, new QTableWidgetItem(QString::number(peers[i].experiencesCount)));
        m_analysisPeerTable->item(i, 0)->setData(Qt::UserRole, peers[i].id);
    }
    if (peers.isEmpty()) {
        m_analysisPeerTable->setRowCount(1);
        m_analysisPeerTable->setSpan(0, 0, 1, 6);
        auto* emptyItem = new QTableWidgetItem("\n暂无同学对照数据\n点击[新增对照]添加同学信息\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(11); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_analysisPeerTable->setItem(0, 0, emptyItem);
    }
    
    m_analysisStrengthList->clear();
    for (auto v : strengths) m_analysisStrengthList->addItem(v.toString());
    m_analysisRiskList->clear();
    for (auto v : risks) m_analysisRiskList->addItem(v.toString());
    m_analysisSuggestionList->clear();
    for (auto v : suggestions) m_analysisSuggestionList->addItem(v.toString());
    
    m_analysisSummaryLabel->setText("报告生成成功，已评估各类维度的学习成果表现与差距。");
}

void MainWindow::addPeer() {
    PeerEditorDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        PeerBenchmark pb = dlg.peer();
        PeerBenchmarkService::create(pb);
        refreshAnalysis();
        ToastNotification::display(this, "对照同学已添加。");
    }
}

void MainWindow::editSelectedPeer() {
    if(!m_analysisPeerTable || m_analysisPeerTable->currentRow() < 0) return;
    int id = m_analysisPeerTable->item(m_analysisPeerTable->currentRow(), 0)->data(Qt::UserRole).toInt();
    PeerBenchmark p = PeerBenchmarkService::getById(id);
    PeerEditorDialog dlg(this);
    dlg.setPeer(p);
    if(dlg.exec() == QDialog::Accepted) {
        PeerBenchmark pb = dlg.peer();
        PeerBenchmarkService::update(id, pb);
        refreshAnalysis();
        ToastNotification::display(this, "对照同学信息已更新。");
    }
}

void MainWindow::removeSelectedPeer() {
    if(!m_analysisPeerTable || m_analysisPeerTable->currentRow() < 0) return;
    int id = m_analysisPeerTable->item(m_analysisPeerTable->currentRow(), 0)->data(Qt::UserRole).toInt();
    if(id > 0 && QMessageBox::question(this, "删除对照同学", "确定要删除这名对照同学记录吗？删除后将无法恢复。") == QMessageBox::Yes) {
        PeerBenchmarkService::remove(id);
        refreshAnalysis();
        ToastNotification::display(this, "对照同学已删除。");
    }
}

void MainWindow::chooseImportFile() {
    QString path = QFileDialog::getOpenFileName(this, "选择数据文件", QDir::homePath(), "CSV 文件 (*.csv)");
    if (!path.isEmpty()) {
        m_importFilePath = path;
        m_importFileLabel->setText(path);
    }
}

void MainWindow::runImport() {
    if (m_importFilePath.isEmpty()) { 
        ToastNotification::display(this, "请先选择数据源文件！");
        return; 
    }
    QFile file(m_importFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        ToastNotification::display(this, "无法读取文件，请检查文件权限。");
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    
    QString entity = m_importEntityCombo->currentData().toString();
    QJsonObject result = ImportService::importData(entity, data, m_importFilePath);
    
    if (result.contains("error") && result["error"].toBool()) {
        ToastNotification::display(this, "❌ 导入失败: " + result["message"].toString());
        return;
    }
    
    m_importResultImportedValue->setText(QString::number(result["imported"].toInt()));
    m_importResultFailedValue->setText(QString::number(result["failed"].toInt()));
    
    QJsonArray errors = result["errors"].toArray();
    m_importErrorTable->setRowCount(errors.size());
    for(int i = 0; i < errors.size(); ++i) {
        QJsonObject e = errors[i].toObject();
        m_importErrorTable->setItem(i, 0, new QTableWidgetItem(QString("行 %1").arg(e["row"].toInt())));
        m_importErrorTable->setItem(i, 1, new QTableWidgetItem(e["error"].toString()));
    }
    
    m_importSummaryLabel->setText(QString("文件处理完成：成功导入 %1 条，并刷新了各系统缓存。").arg(result["imported"].toInt()));
    
    // Refresh all pages after import!
    refreshOverview();
    refreshCourses();
    refreshRoles();
    refreshAchievements();
    refreshExperiences();
    refreshActivities();
    refreshGoals();
    refreshJobs();
    refreshTimeline();
    
    ToastNotification::display(this, "共导入了 " + QString::number(result["imported"].toInt()) + " 条数据，已触发全系统数据刷新。");
}


void MainWindow::onBackendStarted()
{
    m_serverReady = true;
    m_statusLabel->setText("后端服务运行中，原生页面已可直接读取数据。");
        updateBackendBadge(true, "端口 8080");
    m_progressBar->hide();

    if (m_trayIcon) {
        m_trayIcon->showMessage("学业发展规划系统", "C++ 后端已启动，可以使用原生界面或网页预览。");
    }

    refreshCurrentPage();
}

void MainWindow::onBackendError(const QString& error)
{
    m_serverReady = false;
    m_statusLabel->setText("后端启动失败: " + error);
    updateBackendBadge(false, error);
    m_progressBar->hide();
    QMessageBox::critical(this, "后端错误", "后端服务启动失败：\n" + error);
}

void MainWindow::onNavigationChanged(int row)
{
    if (row >= 0 && row < m_stack->count()) {
        QWidget* widget = m_stack->widget(row);
        if (widget) {
            if (widget->graphicsEffect()) {
                widget->setGraphicsEffect(nullptr);
            }
            QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(eff);
            QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity");
            anim->setDuration(150);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            connect(anim, &QPropertyAnimation::finished, widget, [widget]() {
                widget->setGraphicsEffect(nullptr);
            });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
        m_stack->setCurrentIndex(row);
        
        const int kResumeEditorPageIndex = 10;
        const bool isResumeEditor = (row == kResumeEditorPageIndex);
        
        if (m_mainInner) {
            if (isResumeEditor) {
                m_mainInner->setMaximumWidth(16777215);
            } else {
                m_mainInner->setMaximumWidth(kMaxContentWidth);
            }
        }
        
        if (m_leftStretchSpacer) {
            m_leftStretchSpacer->changeSize(
                0, 0,
                isResumeEditor ? QSizePolicy::Fixed : QSizePolicy::Expanding,
                QSizePolicy::Minimum
            );
        }
        if (m_rightStretchSpacer) {
            m_rightStretchSpacer->changeSize(
                0, 0,
                isResumeEditor ? QSizePolicy::Fixed : QSizePolicy::Expanding,
                QSizePolicy::Minimum
            );
        }
        if (m_mainInner && m_mainInner->parentWidget()) {
            m_mainInner->parentWidget()->layout()->invalidate();
        }
        
        refreshCurrentPage();
    }
}

void MainWindow::refreshCurrentPage()
{
    const int index = m_stack ? m_stack->currentIndex() : 0;
    switch (index) {
    case 0:
        if (m_overviewPage) m_overviewPage->refresh();
        break;
    case 1:
        if (m_coursesPage) m_coursesPage->refresh();
        break;
    case 2:
        if (m_rolesPage) m_rolesPage->refresh();
        break;
    case 3:
        if (m_achievementsPage) m_achievementsPage->refresh();
        break;
    case 4:
        if (m_experiencesPage) m_experiencesPage->refresh();
        break;
    case 5:
        if (m_activitiesPage) m_activitiesPage->refresh();
        break;
    case 6:
        if (m_goalsPage) m_goalsPage->refresh();
        break;
    case 7:
        if (m_jobsPage) m_jobsPage->refresh();
        break;
    case 8:
        if (m_analysisPage) m_analysisPage->refresh();
        break;
    case 9:
        if (m_timelinePage) m_timelinePage->refresh();
        break;
    case 10:
        if (m_resumePage) m_resumePage->refresh();
        break;
    case 11:
        if (m_importsPage) m_importsPage->refresh();
        break;
    default:
        break;
    }
    // Refresh widgets
    if (m_aiPanel) {
        m_aiPanel->refreshStatus();
        if (m_aiPanel->currentOutput().isEmpty()) {
            m_aiPanel->setOutput("点击\"综合分析 / 课程建议 / 经历建议 / 目标建议\"，或直接在下方输入问题。");
        }
    }
    if (m_sidebar) {
        m_sidebar->refreshData();
    }
}

void MainWindow::onOpenBrowser()
{
    if (!m_serverReady && m_frontendPath.isEmpty()) {
        ToastNotification::display(this, "当前没有可用的网页预览资源。");
        return;
    }
    openBrowser();
}

void MainWindow::onAboutTriggered()
{
    QMessageBox::about(
        this,
        "关于",
        QString("学业发展规划系统 - Qt 桌面版 v%1\n\n"
                "当前阶段：\n"
                "1. C++ 后端已独立运行\n"
                "2. 九个核心页面都已有原生 Qt 界面\n"
                "3. 当前继续沿用前端的卡片式知识库设计逻辑\n"
                "4. 原网页前端保留为辅助预览与对照版本")
            .arg(PDP_VERSION));
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::onQuitTriggered()
{
    const auto reply = QMessageBox::question(
        this, "确认退出", "确定要退出学业发展规划系统吗？", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_trayIcon) {
            m_trayIcon->hide();
        }
        qApp->quit();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
    qApp->quit();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    // Handle text selection in content area for AI context
    if (event->type() == QEvent::MouseButtonRelease) {
        // Check if any text is selected in any child widget
        QWidget* focusWidget = QApplication::focusWidget();
        if (focusWidget) {
            QString selectedText;
            if (auto* textEdit = qobject_cast<QTextEdit*>(focusWidget)) {
                selectedText = textEdit->textCursor().selectedText().trimmed();
            } else if (auto* listWidget = qobject_cast<QListWidget*>(focusWidget)) {
                if (listWidget->currentItem()) {
                    selectedText = listWidget->currentItem()->text().trimmed();
                }
            }
            if (!selectedText.isEmpty() && selectedText.length() < 2000 && selectedText.length() > 5) {
                if (m_aiPanel) {
                    m_aiPanel->setContext("选中内容", selectedText);
                }
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

HttpServerThread::HttpServerThread(QObject* parent)
    : QThread(parent)
{}

void HttpServerThread::run()
{
    Logger::info("后端服务线程启动");

    HttpServer server;
    if (!server.start(8080)) {
                emit serverError("无法绑定端口 8080");
        return;
    }

    emit serverStarted();

    while (m_running) {
        msleep(100);
    }

    server.stop();
    Logger::info("后端服务线程退出");
}

void HttpServerThread::stop()
{
    m_running = false;
}

void MainWindow::insertSampleDataIfNeeded()
{
    QSettings settings;
    if (settings.value("sampleDataInserted", false).toBool()) {
        return;
    }

    QStringList semesters = {"2023-2024-1", "2023-2024-2", "2024-2025-1", "2024-2025-2"};
    QStringList categories = {"必修", "选修", "通识", "实践"};
    QStringList statuses = {"Completed", "In Progress", "Planned"};
    
    for (int i = 1; i <= 20; ++i) {
        Course c;
        c.name = QString("课程%1").arg(i);
        c.code = QString("CS%1").arg(1000 + i);
        c.credits = 2.0 + (i % 4);
        c.semester = semesters[i % 4];
        c.category = categories[i % 4];
        c.score = 70 + (i % 30);
        c.status = statuses[i % 3];
        c.teacher = QString("教师%1").arg(i);
        c.location = QString("教学楼%1教室").arg(i % 5 + 1);
        c.description = QString("这是课程%1的描述信息，包含课程的主要内容和教学目标。").arg(i);
        c.tags = "专业课程,核心必修";
        CourseService::create(c);
    }

    QStringList roleTypes = {"学生干部", "社团负责人", "志愿者", "助教"};
    for (int i = 1; i <= 20; ++i) {
        Role r;
        r.title = QString("角色%1").arg(i);
        r.type = roleTypes[i % 4];
        r.organization = QString("组织%1").arg(i);
        r.description = QString("这是角色%1的详细描述，包括主要职责和工作内容。").arg(i);
        r.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        r.endDate = i % 3 == 0 ? "" : QString("2025-%1-01").arg(i % 12 + 1);
        r.isActive = (i % 3 != 0);
        r.achievements = QString("成就1,成就2,成就3");
        r.contact = QString("contact%1@example.com").arg(i);
        r.supervisor = QString("指导老师%1").arg(i);
        RoleService::create(r);
    }

    QStringList achTypes = {"竞赛", "证书", "项目", "论文"};
    QStringList levels = {"国家级", "省级", "校级", "院级"};
    for (int i = 1; i <= 20; ++i) {
        Achievement a;
        a.title = QString("成果%1").arg(i);
        a.type = achTypes[i % 4];
        a.level = levels[i % 4];
        a.organization = QString("颁发机构%1").arg(i);
        a.description = QString("这是成果%1的详细描述，包括获奖原因和主要内容。").arg(i);
        a.date = QString("2024-%1-15").arg(i % 12 + 1);
        a.certificate = QString("证书编号%1").arg(10000 + i);
        a.relatedCourse = QString("课程%1").arg(i % 10 + 1);
        a.teamMembers = QString("成员1,成员2,成员3");
        a.ranking = QString("第%1名").arg(i % 10 + 1);
        a.prize = i % 3 == 0 ? "一等奖" : (i % 3 == 1 ? "二等奖" : "三等奖");
        a.verified = (i % 2 == 0);
        AchievementService::create(a);
    }

    QStringList expTypes = {"实习", "项目", "研究", "竞赛"};
    for (int i = 1; i <= 20; ++i) {
        Experience e;
        e.title = QString("经历%1").arg(i);
        e.type = expTypes[i % 4];
        e.organization = QString("单位%1").arg(i);
        e.role = QString("角色%1").arg(i);
        e.description = QString("这是经历%1的详细描述，包括主要工作内容和成果。").arg(i);
        e.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        e.endDate = i % 3 == 0 ? "" : QString("2025-%1-01").arg(i % 12 + 1);
        e.isOngoing = (i % 3 == 0);
        e.technologies = QString("技术1,技术2,技术3");
        e.achievements = QString("成果1,成果2,成果3");
        e.supervisor = QString("导师%1").arg(i);
        e.contact = QString("contact%1@example.com").arg(i);
        e.location = QString("地点%1").arg(i);
        e.url = QString("https://example.com/exp%1").arg(i);
        ExperienceService::create(e);
    }

    QStringList actCategories = {"学术", "体育", "艺术", "社交", "志愿"};
    for (int i = 1; i <= 20; ++i) {
        Activity a;
        a.name = QString("活动%1").arg(i);
        a.description = QString("这是活动%1的详细描述，包括活动内容和参与方式。").arg(i);
        a.category = actCategories[i % 5];
        a.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        a.endDate = QString("2024-%1-15").arg(i % 12 + 1);
        a.isFavorite = (i % 5 == 0);
        a.isActive = (i % 3 != 0);
        a.tags = QString("标签1,标签2,标签3");
        ActivityService::create(a);
    }

    QStringList goalCategories = {"学业", "技能", "健康", "社交", "职业"};
    QStringList priorities = {"High", "Medium", "Low"};
    QStringList goalStatuses = {"In Progress", "Completed", "Pending"};
    for (int i = 1; i <= 20; ++i) {
        Goal g;
        g.title = QString("目标%1").arg(i);
        g.category = goalCategories[i % 5];
        g.description = QString("这是目标%1的详细描述，包括目标内容和实现路径。").arg(i);
        g.targetValue = 100;
        g.currentValue = i * 5;
        g.unit = "%";
        g.deadline = QString("2025-%1-01").arg(i % 12 + 1);
        g.priority = priorities[i % 3];
        g.status = goalStatuses[i % 3];
        g.milestones = QString("里程碑1,里程碑2,里程碑3");
        GoalService::create(g);
    }

    QStringList jobStatuses = {"收藏", "已投递", "面试中", "已拒绝", "已录用"};
    for (int i = 1; i <= 20; ++i) {
        Job j;
        j.title = QString("职位%1").arg(i);
        j.company = QString("公司%1").arg(i);
        j.location = QString("城市%1").arg(i);
        j.salaryRange = QString("%1-%2K").arg(10 + i).arg(20 + i);
        j.description = QString("这是职位%1的详细描述，包括岗位职责和任职要求。").arg(i);
        JobRequirement req1, req2;
        req1.text = QString("要求1：熟练掌握技能%1").arg(i);
        req1.met = (i % 2 == 0);
        req2.text = QString("要求2：具备相关经验%1年").arg(i % 5);
        req2.met = (i % 3 == 0);
        j.requirements = {req1, req2};
        j.isActive = true;
        j.priority = i % 5;
        j.source = "招聘网站";
        j.url = QString("https://example.com/job%1").arg(i);
        j.status = jobStatuses[i % 5];
        j.appliedDate = QString("2024-%1-01").arg(i % 12 + 1);
        JobService::create(j);
    }

    settings.setValue("sampleDataInserted", true);
    Logger::info("已插入虚拟数据");
}
