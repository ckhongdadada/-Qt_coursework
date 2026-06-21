#include "MainWindow.h"
#include "Version.h"

#include <QApplication>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QSettings>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QToolBar>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "client/widgets/SidebarWidget.h"
#include "client/widgets/NavigationListWidget.h"
#include "client/widgets/AiPanelWidget.h"
#include "client/widgets/ToastNotification.h"
#include "client/pages/OverviewPage.h"
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
#include "client/pages/ImportsPage.h"
#include "client/core/AppShellController.h"
#include "client/core/DataRefreshCoordinator.h"
#include "client/core/BackendRuntimeController.h"
#include "client/core/AiContextMediator.h"
#include "client/core/DataDomain.h"
#include "service/AiService.h"
#include "service/GoalService.h"
#include "client/dialogs/GoalEditorDialog.h"
#include "model/Goal.h"
#include "util/Logger.h"

namespace {
constexpr int kMaxContentWidth = 1080;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("学业发展规划系统 - Qt 桌面版 v%1").arg(PDP_VERSION));
    resize(1360, 860);
    setMinimumSize(1200, 780);

    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());

    m_shellController = new AppShellController(this);
    m_refreshCoordinator = new DataRefreshCoordinator(this);
    m_backendController = new BackendRuntimeController(this);
    m_aiMediator = new AiContextMediator(this);

    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupSystemTray();
    applyWindowStyle();

    m_backendController->bindWidgets(m_statusLabel, m_progressBar, m_trayIcon);

    m_backendController->checkFrontendExists();
    m_backendController->startBackendServer();
    m_sidebar->refreshData();

    m_sidebar->navigationList()->setCurrentRow(0);
    refreshCurrentPage();
    m_backendController->insertSampleDataIfNeeded();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());

    m_backendController->stopBackendServer();

    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

void MainWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_sidebar = new SidebarWidget(this);
    rootLayout->addWidget(m_sidebar);

    QWidget* contentShell = new QWidget(this);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentShell);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // --- 顶栏：统一中文，Pill改为AI面板切换按钮 ---
    QFrame* topbar = new QFrame(contentShell);
    topbar->setObjectName("topbar");
    QHBoxLayout* topbarLayout = new QHBoxLayout(topbar);
    topbarLayout->setContentsMargins(32, 10, 32, 10);
    topbarLayout->setSpacing(16);

    QWidget* topbarLeft = new QWidget(topbar);
    QVBoxLayout* topbarTextLayout = new QVBoxLayout(topbarLeft);
    topbarTextLayout->setContentsMargins(0, 0, 0, 0);
    topbarTextLayout->setSpacing(2);

    m_topbarKicker = new QLabel("学业规划 · 知识库", topbarLeft);
    m_topbarKicker->setObjectName("topbarKicker");
    topbarTextLayout->addWidget(m_topbarKicker);

    m_topbarTitle = new QLabel("个人发展规划工作台", topbarLeft);
    m_topbarTitle->setObjectName("topbarTitle");
    topbarTextLayout->addWidget(m_topbarTitle);
    topbarLayout->addWidget(topbarLeft);
    topbarLayout->addStretch();

    contentLayout->addWidget(topbar);

    // --- 内容区：简化布局层级，去掉多余的mainShell包裹 ---
    QScrollArea* scrollArea = new QScrollArea(contentShell);
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

    m_mainInner = new QWidget(scrollViewport);
    m_mainInner->setMaximumWidth(kMaxContentWidth);
    m_mainInner->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QVBoxLayout* mainInnerLayout = new QVBoxLayout(m_mainInner);
    mainInnerLayout->setContentsMargins(32, 28, 32, 36);
    mainInnerLayout->setSpacing(16);

    m_stack = new QStackedWidget(this);

    // --- 页面注册表：替代 switch 硬编码 ---
    m_overviewPage = new OverviewPage(this);
    m_pageRefreshers[m_stack->addWidget(m_overviewPage)] = [this]() { m_overviewPage->refresh(); };

    m_coursesPage = new CoursesPage(this);
    m_pageRefreshers[m_stack->addWidget(m_coursesPage)] = [this]() { m_coursesPage->refresh(); };

    m_rolesPage = new RolesPage(this);
    m_pageRefreshers[m_stack->addWidget(m_rolesPage)] = [this]() { m_rolesPage->refresh(); };

    m_achievementsPage = new AchievementsPage(this);
    m_pageRefreshers[m_stack->addWidget(m_achievementsPage)] = [this]() { m_achievementsPage->refresh(); };

    m_experiencesPage = new ExperiencesPage(this);
    m_pageRefreshers[m_stack->addWidget(m_experiencesPage)] = [this]() { m_experiencesPage->refresh(); };

    m_activitiesPage = new ActivitiesPage(this);
    m_pageRefreshers[m_stack->addWidget(m_activitiesPage)] = [this]() { m_activitiesPage->refresh(); };

    m_goalsPage = new GoalsPage(this);
    m_pageRefreshers[m_stack->addWidget(m_goalsPage)] = [this]() { m_goalsPage->refresh(); };

    m_jobsPage = new JobsPage(this);
    m_pageRefreshers[m_stack->addWidget(m_jobsPage)] = [this]() { m_jobsPage->refresh(); };

    m_analysisPage = new AnalysisPage(this);
    m_pageRefreshers[m_stack->addWidget(m_analysisPage)] = [this]() { m_analysisPage->refresh(); };

    m_timelinePage = new TimelinePage(this);
    m_pageRefreshers[m_stack->addWidget(m_timelinePage)] = [this]() { m_timelinePage->refresh(); };

    m_resumePage = new ResumePage(this);
    m_pageRefreshers[m_stack->addWidget(m_resumePage)] = [this]() { m_resumePage->refresh(); };

    m_importsPage = new ImportsPage(this);
    m_pageRefreshers[m_stack->addWidget(m_importsPage)] = [this]() { m_importsPage->refresh(); };

    mainInnerLayout->addWidget(m_stack, 1);

    scrollViewportLayout->addSpacerItem(m_leftStretchSpacer);
    scrollViewportLayout->addWidget(m_mainInner, 0, Qt::AlignTop);
    scrollViewportLayout->addSpacerItem(m_rightStretchSpacer);
    scrollArea->setWidget(scrollViewport);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    contentLayout->addWidget(scrollArea, 1);

    rootLayout->addWidget(contentShell, 1);

    // --- AI面板：默认收起 ---
    m_aiPanel = new AiPanelWidget(this);
    m_aiPanel->setMinimumWidth(50);
    m_aiPanel->setMaximumWidth(50);
    // 直接设置初始收起状态（隐藏内容区，显示折叠条）
    auto* collapsedStrip = m_aiPanel->findChild<QWidget*>("aiCollapsedStrip");
    auto* panelContent = m_aiPanel->findChild<QWidget*>("aiPanelContent");
    if (collapsedStrip) collapsedStrip->show();
    if (panelContent) panelContent->hide();
    rootLayout->addWidget(m_aiPanel);

    setCentralWidget(centralWidget);

    m_shellController->attach(m_stack, m_mainInner, m_leftStretchSpacer, m_rightStretchSpacer,
                               m_topbarKicker, m_topbarTitle, nullptr);

    m_refreshCoordinator->bindPages(
        m_overviewPage, m_coursesPage, m_rolesPage, m_achievementsPage,
        m_experiencesPage, m_activitiesPage, m_goalsPage, m_jobsPage,
        m_analysisPage, m_timelinePage, m_resumePage, m_importsPage);
    m_refreshCoordinator->connectSignals();

    connect(m_backendController, &BackendRuntimeController::backendReady, this, &MainWindow::onBackendReady);
    connect(m_backendController, &BackendRuntimeController::serverError, this, &MainWindow::onBackendError);

    m_aiMediator->bindRootWidget(centralWidget);
    m_aiMediator->attachPanel(m_aiPanel);

    connect(m_sidebar->navigationList(), &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);

    connect(m_aiPanel, &AiPanelWidget::chatMessageSent, this, [this](const QString& message) {
        m_aiPanel->setOutput("AI 正在思考中...");
        QApplication::processEvents();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QJsonObject payload;
        payload["message"] = message;
        const QJsonObject result = AiService::chat(payload);
        QApplication::restoreOverrideCursor();

        QString reply = result["reply"].toString().trimmed();
        if (reply.isEmpty()) {
            reply = result["analysis"].toString().trimmed();
        }
        if (reply.isEmpty() && result.contains("error")) {
            reply = QString("大模型暂时没有返回有效答复。\n错误信息：%1").arg(result["error"].toString());
        }
        if (reply.isEmpty()) {
            reply = "大模型暂时没有返回有效答复，请稍后重试。";
        }

        QString displayMessage = message;
        const int selectionMarker = displayMessage.indexOf("选中文本：\n");
        if (selectionMarker >= 0) {
            QString extracted = displayMessage.mid(selectionMarker + 6).trimmed();
            displayMessage = QString("基于选中文本生成建议：\n\"%1\"").arg(extracted);
        }

        QString output = QString("问题：%1\n\n答复：\n%2")
            .arg(displayMessage, reply);
        m_aiPanel->setOutput(output);
        m_aiPanel->refreshStatus();
    });
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
    m_toolBar = addToolBar("主工具栏");
    m_toolBar->setMovable(false);
    m_toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload), "刷新", this, &MainWindow::refreshCurrentPage);
    m_toolBar->addAction(QApplication::style()->standardIcon(QStyle::SP_CommandLink), "网页预览", this, &MainWindow::onOpenBrowser);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("后端状态：未就绪", this);
    m_statusLabel->setObjectName("connectionLabel");
    statusBar()->addWidget(m_statusLabel, 1);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(140);
    m_progressBar->setRange(0, 0);
    m_progressBar->setTextVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->hide();
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

void MainWindow::startBackendServer()
{
    m_backendController->startBackendServer();
}

void MainWindow::openBrowser()
{
    m_backendController->openBrowser();
}

void MainWindow::onBackendReady()
{
    refreshCurrentPage();
}

void MainWindow::onBackendError(const QString& error)
{
    QMessageBox::critical(this, "后端错误", "后端服务启动失败：\n" + error);
}

void MainWindow::onNavigationChanged(int row)
{
    if (row >= 0 && row < m_stack->count()) {
        m_stack->setCurrentIndex(row);
        m_shellController->onPageChanged(row);

        QWidget* widget = m_stack->widget(row);
        if (widget) {
            if (widget->graphicsEffect()) {
                widget->graphicsEffect()->setEnabled(false);
                widget->setGraphicsEffect(nullptr);
            }
            QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(eff);
            QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity");
            anim->setDuration(120);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            connect(anim, &QPropertyAnimation::finished, widget, [widget]() {
                widget->setGraphicsEffect(nullptr);
            });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
        QTimer::singleShot(0, this, &MainWindow::refreshCurrentPage);
    }
}

void MainWindow::refreshCurrentPage()
{
    const int index = m_stack ? m_stack->currentIndex() : 0;
    auto it = m_pageRefreshers.find(index);
    if (it != m_pageRefreshers.end()) {
        it.value()();
    }
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
    if (!m_backendController->isServerReady() && m_backendController->frontendPath().isEmpty()) {
        ToastNotification::display(this, "当前没有可用的网页预览资源。");
        return;
    }
    m_backendController->openBrowser();
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
    event->accept();
    qApp->quit();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    return QMainWindow::eventFilter(watched, event);
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
        #topbarPill:hover {
            background: #f4ede2;
            color: #2f8f86;
            border-color: #b8d4d2;
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
        #plainList::item:hover {
            background: #fbf7ef;
            border-color: #d6c7b7;
        }
        #plainList::item:selected {
            background: #fffaf1;
            border: 1px solid #d4c2ae;
            border-left: 5px solid #2f8f86;
            color: #2d241c;
        }
        QTableWidget {
            background: transparent;
            border: none;
            gridline-color: #e6dacb;
            color: #342b23;
        }
        QTableWidget::item {
            padding: 8px 10px;
            border-bottom: 1px solid #ede4d6;
        }
        QTableWidget::item:selected {
            background: #f4ede2;
            color: #2d241c;
        }
        QHeaderView::section {
            background: #f7f2ea;
            border: none;
            border-bottom: 2px solid #d7cab8;
            padding: 8px 10px;
            font-weight: 600;
            color: #5b4e43;
        }
        QLineEdit, QComboBox {
            padding: 8px 12px;
            border: 1px solid #d7cab8;
            border-radius: 8px;
            background: #fffdf9;
            color: #2d241c;
        }
        QComboBox {
            padding: 8px 40px 8px 12px;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 38px;
            border: none;
            background: transparent;
        }
        QComboBox::drop-down:hover {
            background: rgba(244, 237, 226, 0.58);
            border-radius: 6px;
        }
        QComboBox::down-arrow {
            image: url(:/icons/chevron-down.svg);
            width: 12px;
            height: 12px;
            margin-right: 13px;
        }
        QComboBox QAbstractItemView {
            outline: none;
            border: 1px solid #d7cab8;
            border-radius: 8px;
            background: #fffdf9;
            selection-background-color: #4a90e2;
            selection-color: #ffffff;
            padding: 4px;
        }
        QLineEdit:focus, QComboBox:focus {
            border-color: #b89e7e;
        }
        QTextEdit, QPlainTextEdit {
            padding: 8px;
            border: 1px solid #d7cab8;
            border-radius: 8px;
            background: #fffdf9;
            color: #2d241c;
        }
        QTextEdit:focus, QPlainTextEdit:focus {
            border-color: #b89e7e;
        }
        QPushButton {
            padding: 8px 16px;
            border: 1px solid #d7cab8;
            border-radius: 8px;
            background: #fffdf9;
            color: #2d241c;
            font-weight: 500;
        }
        QPushButton:hover {
            background: #f4ede2;
            border-color: #b89e7e;
        }
        QPushButton:pressed {
            background: #ede4d6;
        }
        QCheckBox {
            spacing: 8px;
            color: #2d241c;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid #d7cab8;
            border-radius: 4px;
            background: #fffdf9;
        }
        QCheckBox::indicator:checked {
            background: #8b7355;
            border-color: #8b7355;
        }
        QTabWidget::pane {
            border: 1px solid #d7cab8;
            border-radius: 8px;
            background: #fffdf9;
        }
        QTabBar::tab {
            padding: 8px 16px;
            border: 1px solid #d7cab8;
            border-bottom: none;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            background: #f4ede2;
            color: #5b4e43;
        }
        QTabBar::tab:selected {
            background: #fffdf9;
            color: #2d241c;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #d7cab8;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 8px;
            margin: 0;
        }
        QScrollBar::handle:horizontal {
            background: #d7cab8;
            border-radius: 4px;
            min-width: 30px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
        }
    )");
}
