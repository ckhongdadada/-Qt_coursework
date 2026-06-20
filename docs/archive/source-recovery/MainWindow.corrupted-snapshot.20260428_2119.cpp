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
#include "client/dialogs/AchievementEditorDialog.h"
#include "client/dialogs/ActivityEditorDialog.h"
#include "client/dialogs/CourseEditorDialog.h"
#include "client/dialogs/ExperienceEditorDialog.h"
#include "client/dialogs/GoalEditorDialog.h"
#include "client/dialogs/JobEditorDialog.h"
#include "client/dialogs/PeerEditorDialog.h"
#include "client/dialogs/ProfileEditorDialog.h"
#include "client/dialogs/RoleEditorDialog.h"
#include "client/utils/ResumeHelpers.h"
#include "client/utils/UiHelpers.h"
#include "client/widgets/ToastNotification.h"
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
        "鎬昏",
        "璇剧▼搴?,
        "瑙掕壊鑱岃矗",
        "鎴愭灉璁板綍",
        "缁忓巻妗ｆ",
        "璇惧娲诲姩",
        "鐩爣杩借釜",
        "鐩爣宀椾綅",
        "鍒嗘瀽鎶ュ憡",
        "鏃堕棿杞?,
        "绠€鍘嗗鍑?,
        "鏁版嵁瀵煎叆"
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

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_serverUrl("http://127.0.0.1:8080/")
{
    setWindowTitle(QString("瀛︿笟鍙戝睍瑙勫垝绯荤粺 - Qt 妗岄潰鐗?v%1").arg(PDP_VERSION));
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
    refreshSidebarCards();

    m_navList->setCurrentRow(0);
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

    rootLayout->addWidget(createSidebar());

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

    m_topbarTitle = new QLabel("涓汉鍙戝睍瑙勫垝宸ヤ綔鍙?, topbarLeft);
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
    m_stack->addWidget(createOverviewPage());
    m_stack->addWidget(createCoursesPage());
    m_stack->addWidget(createRolesPage());
    m_stack->addWidget(createAchievementsPage());
    m_stack->addWidget(createExperiencesPage());
    m_stack->addWidget(createActivitiesPage());
    m_stack->addWidget(createGoalsPage());
    m_stack->addWidget(createJobsPage());
    m_stack->addWidget(createAnalysisPage());
    m_stack->addWidget(createTimelinePage());
    m_stack->addWidget(createResumeEditorPage());
    m_stack->addWidget(createImportsPage());
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
    rootLayout->addWidget(createAiPage());
    setCentralWidget(centralWidget);
    centralWidget->installEventFilter(this);

    connect(m_navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);
    connect(m_navList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item || !m_navList) {
            return;
        }
        const int row = m_navList->row(item);
        if (row >= 0 && m_navList->currentRow() != row) {
            m_navList->setCurrentRow(row);
        } else {
            m_navList->viewport()->update();
        }
    });
}

QWidget* MainWindow::createSidebar()
{
    QFrame* sidebar = new QFrame(this);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(kSidebarExpandedWidth);

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    QWidget* topRow = new QWidget(sidebar);
    QHBoxLayout* topRowLayout = new QHBoxLayout(topRow);
    topRowLayout->setContentsMargins(0, 0, 0, 0);
    topRowLayout->setSpacing(8);

    QLabel* brandMark = new QLabel("P", topRow);
    brandMark->setObjectName("brandMark");
    topRowLayout->addWidget(brandMark, 0, Qt::AlignTop);

    QWidget* brandText = new QWidget(topRow);
    QVBoxLayout* brandTextLayout = new QVBoxLayout(brandText);
    brandTextLayout->setContentsMargins(0, 0, 0, 0);
    brandTextLayout->setSpacing(2);

    QLabel* title = new QLabel("Personal Planner", brandText);
    title->setObjectName("sidebarTitle");
    brandTextLayout->addWidget(title);

    QLabel* subtitle = new QLabel("涓汉鍙戝睍鐭ヨ瘑搴?, brandText);
    subtitle->setObjectName("sidebarSubtitle");
    brandTextLayout->addWidget(subtitle);
    topRowLayout->addWidget(brandText, 1);

    QPushButton* toggleButton = new QPushButton(QString(QChar(0x2261)), topRow);
    toggleButton->setObjectName("sidebarToggle");
    toggleButton->setFlat(true);
    topRowLayout->addWidget(toggleButton, 0, Qt::AlignTop);
    layout->addWidget(topRow);

    const QStringList fullLabels = navExpandedLabels();
    const QStringList compactLabels = navCollapsedIcons();
    const QStringList navLabels = navBaseLabels();

    m_navList = new QListWidget(sidebar);
    m_navList->setObjectName("navList");
    m_navList->setProperty("collapsed", false);
    m_navList->setSpacing(0);
    m_navList->setIconSize(QSize(20, 20));
    m_navList->setViewMode(QListView::ListMode);
    m_navList->setMovement(QListView::Static);
    m_navList->setResizeMode(QListView::Adjust);
    m_navList->setWrapping(false);
    for (int i = 0; i < fullLabels.size(); ++i) {
        auto* item = new QListWidgetItem(fullLabels.at(i), m_navList);
        item->setIcon(navIconForIndex(i));
        item->setToolTip(navLabels.value(i));
        item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        item->setSizeHint(QSize(0, 42));
    }
    layout->addWidget(m_navList, 1);

    // Spacer to push bottom cards down (Vue: margin-top: auto)
    layout->addStretch(0);

    QFrame* timeCard = new QFrame(sidebar);
    timeCard->setObjectName("sidebarInfoCard");
    timeCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    timeCard->setMinimumHeight(72);
    QHBoxLayout* timeLayout = new QHBoxLayout(timeCard);
    timeLayout->setContentsMargins(8, 6, 8, 6);
    timeLayout->setSpacing(8);
    QLabel* timeAvatar = new QLabel("T", timeCard);
    timeAvatar->setObjectName("sidebarInfoAvatar");
    timeLayout->addWidget(timeAvatar, 0, Qt::AlignTop);
    QWidget* timeText = new QWidget(timeCard);
    QVBoxLayout* timeTextLayout = new QVBoxLayout(timeText);
    timeTextLayout->setContentsMargins(0, 0, 0, 0);
    timeTextLayout->setSpacing(2);
    QLabel* timeKicker = new QLabel("Time", timeText);
    timeKicker->setObjectName("sidebarInfoKicker");
    timeTextLayout->addWidget(timeKicker);
    m_timeSemesterLabel = new QLabel("--", timeText);
    m_timeSemesterLabel->setObjectName("sidebarInfoTitle");
    timeTextLayout->addWidget(m_timeSemesterLabel);
    m_timeDetailLabel = new QLabel("--", timeText);
    m_timeDetailLabel->setObjectName("sidebarInfoDetail");
    m_timeDetailLabel->setWordWrap(false);
    timeTextLayout->addWidget(m_timeDetailLabel);
    timeLayout->addWidget(timeText, 1);
    layout->addWidget(timeCard);

    QPushButton* studentCard = new QPushButton(sidebar);
    studentCard->setObjectName("sidebarInfoCard");
    studentCard->setCursor(Qt::PointingHandCursor);
    studentCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    studentCard->setMinimumHeight(72);
    studentCard->setStyleSheet("#sidebarInfoCard { border: 1px solid transparent; background: transparent; border-radius: 14px; text-align: left; } #sidebarInfoCard:hover { background: #f0ddd1; }");
    QHBoxLayout* studentLayout = new QHBoxLayout(studentCard);
    studentLayout->setContentsMargins(8, 6, 8, 6);
    studentLayout->setSpacing(8);
    QLabel* studentAvatar = new QLabel("S", studentCard);
    studentAvatar->setObjectName("sidebarInfoAvatar");
    studentAvatar->setAttribute(Qt::WA_TransparentForMouseEvents);
    studentLayout->addWidget(studentAvatar, 0, Qt::AlignTop);
    QWidget* studentText = new QWidget(studentCard);
    studentText->setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout* studentTextLayout = new QVBoxLayout(studentText);
    studentTextLayout->setContentsMargins(0, 0, 0, 0);
    studentTextLayout->setSpacing(1);
    QLabel* studentKicker = new QLabel("Student", studentText);
    studentKicker->setObjectName("sidebarInfoKicker");
    studentTextLayout->addWidget(studentKicker);
    m_studentNameLabel = new QLabel("璇峰～鍐欏鍚?, studentText);
    m_studentNameLabel->setObjectName("sidebarInfoTitle");
    studentTextLayout->addWidget(m_studentNameLabel);
    m_studentMetaLabel = new QLabel("璇峰～鍐欏鍙?路 璇峰～鍐欓櫌绯?, studentText);
    m_studentMetaLabel->setObjectName("sidebarInfoDetail");
    m_studentMetaLabel->setWordWrap(false);
    studentTextLayout->addWidget(m_studentMetaLabel);
    studentLayout->addWidget(studentText, 1);
    layout->addWidget(studentCard);

    connect(studentCard, &QPushButton::clicked, this, [this, studentCard]() {
        ProfileEditorDialog dialog(this);
        if (dialog.showNear(studentCard), dialog.exec() == QDialog::Accepted) {
            dialog.save();
            refreshSidebarCards();
        }
    });


    connect(toggleButton, &QPushButton::clicked, this, [sidebar, brandMark, brandText, timeCard, timeText, studentCard, studentText, timeAvatar, studentAvatar, toggleButton, fullLabels, compactLabels, this]() {
        bool isExpanded = (sidebar->width() > kSidebarCollapsedWidth);
        auto* group = new QParallelAnimationGroup(this);
        auto* anim1 = new QPropertyAnimation(sidebar, "minimumWidth");
        auto* anim2 = new QPropertyAnimation(sidebar, "maximumWidth");
        anim1->setDuration(220);
        anim2->setDuration(220);
        anim1->setEasingCurve(QEasingCurve::InOutQuad);
        anim2->setEasingCurve(QEasingCurve::InOutQuad);
        group->addAnimation(anim1);
        group->addAnimation(anim2);

        if (isExpanded) {
            // Collapse - show only icons
            anim1->setStartValue(kSidebarExpandedWidth); anim1->setEndValue(kSidebarCollapsedWidth);
            anim2->setStartValue(kSidebarExpandedWidth); anim2->setEndValue(kSidebarCollapsedWidth);
            brandMark->hide();
            brandText->hide();
            timeText->hide();
            studentText->hide();
            timeCard->setMinimumHeight(44);
            studentCard->setMinimumHeight(44);
            QHBoxLayout* tl = qobject_cast<QHBoxLayout*>(timeCard->layout());
            if (tl) {
                tl->setAlignment(timeAvatar, Qt::AlignHCenter);
                tl->setContentsMargins(0, 6, 0, 6);
            }
            QHBoxLayout* sl = qobject_cast<QHBoxLayout*>(studentCard->layout());
            if (sl) {
                sl->setAlignment(studentAvatar, Qt::AlignHCenter);
                sl->setContentsMargins(0, 6, 0, 6);
            }
            for (int i = 0; i < m_navList->count(); ++i) {
                if (auto* item = m_navList->item(i)) {
                    item->setText(QString());
                    item->setIcon(navIconForIndex(i));
                    item->setTextAlignment(Qt::AlignCenter);
                    item->setSizeHint(QSize(0, 36));
                }
            }
            m_navList->setViewMode(QListView::IconMode);
            m_navList->setFlow(QListView::TopToBottom);
            m_navList->setGridSize(QSize(40, 40));
            m_navList->setSpacing(4);
            m_navList->setProperty("collapsed", true);
            m_navList->style()->unpolish(m_navList);
            m_navList->style()->polish(m_navList);
            if (m_navList->currentItem()) {
                m_navList->setCurrentItem(m_navList->currentItem());
            }
            m_navList->viewport()->update();
            toggleButton->setText(QString(QChar(0x203A)));
        } else {
            anim1->setStartValue(kSidebarCollapsedWidth); anim1->setEndValue(kSidebarExpandedWidth);
            anim2->setStartValue(kSidebarCollapsedWidth); anim2->setEndValue(kSidebarExpandedWidth);
            brandMark->show();
            brandText->show();
            timeText->show();
            studentText->show();
            timeCard->setMinimumHeight(72);
            studentCard->setMinimumHeight(72);
            QHBoxLayout* tl = qobject_cast<QHBoxLayout*>(timeCard->layout());
            if (tl) {
                tl->setAlignment(timeAvatar, Qt::AlignTop);
                tl->setContentsMargins(8, 6, 8, 6);
            }
            QHBoxLayout* sl = qobject_cast<QHBoxLayout*>(studentCard->layout());
            if (sl) {
                sl->setAlignment(studentAvatar, Qt::AlignTop);
                sl->setContentsMargins(8, 6, 8, 6);
            }
            for (int i = 0; i < qMin(m_navList->count(), fullLabels.size()); ++i) {
                if (auto* item = m_navList->item(i)) {
                    item->setText(fullLabels[i]);
                    item->setIcon(navIconForIndex(i));
                    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                    item->setSizeHint(QSize(0, 42));
                }
            }
            m_navList->setViewMode(QListView::ListMode);
            m_navList->setGridSize(QSize());
            m_navList->setSpacing(0);
            m_navList->setProperty("collapsed", false);
            m_navList->style()->unpolish(m_navList);
            m_navList->style()->polish(m_navList);
            if (m_navList->currentItem()) {
                m_navList->setCurrentItem(m_navList->currentItem());
            }
            m_navList->viewport()->update();
            toggleButton->setText(QString(QChar(0x2261)));
        }
        connect(group, &QParallelAnimationGroup::finished, group, &QObject::deleteLater);
        group->start();
    });

    return sidebar;
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

    QLabel* title = new QLabel("鎬昏", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(
        "蹇€熸煡鐪嬩釜浜哄彂灞曠姸鎬佸拰杩戞湡寤鸿銆?,
        page);
    subtitle->setObjectName("pageSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("璇剧▼鎬绘暟", &m_totalCoursesValue), 0, 0);
    metrics->addWidget(createMetricCard("褰撳墠 GPA", &m_gpaValue), 0, 1);
    metrics->addWidget(createMetricCard("鐩爣骞冲潎杩涘害", &m_goalProgressValue), 0, 2);
    metrics->addWidget(createMetricCard("鎴愭灉鏁伴噺", &m_achievementValue), 0, 3);
    metrics->addWidget(createMetricCard("缁忓巻鏁伴噺", &m_experienceValue), 1, 0);
    metrics->addWidget(createMetricCard("瑙掕壊鏁伴噺", &m_roleValue), 1, 1);
    metrics->addWidget(createMetricCard("娲诲姩鏁伴噺", &m_activityValue), 1, 2);
    metrics->addWidget(
        createMetricCard("宸蹭慨瀛﹀垎", &m_creditValue, "鍩轰簬璇剧▼瀹屾垚鐘舵€佽嚜鍔ㄧ粺璁?), 1, 3);
    layout->addLayout(metrics);

    QHBoxLayout* lowerLayout = new QHBoxLayout();
    lowerLayout->setSpacing(14);

    QFrame* recommendationCard = new QFrame(page);
    recommendationCard->setObjectName("contentCard");
    QVBoxLayout* recommendationLayout = new QVBoxLayout(recommendationCard);
    recommendationLayout->setContentsMargins(16, 14, 16, 14);
    recommendationLayout->setSpacing(10);
    QLabel* recommendationTitle = new QLabel("杩戞湡寤鸿", recommendationCard);
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
    QLabel* semesterTitle = new QLabel("瀛︽湡璧板娍", semesterCard);
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

    QLabel* title = new QLabel("璇剧▼搴?, page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_courseSummaryLabel = new QLabel("姝ｅ湪璇诲彇璇剧▼鏁版嵁...", page);
    m_courseSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_courseSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_courseSearchInput = new QLineEdit(filterCard);
    m_courseSearchInput->setPlaceholderText("鎼滅储璇剧▼鍚嶇О / 浠ｇ爜 / 鏁欏笀");
    m_courseStatusInput = new QLineEdit(filterCard);
    m_courseStatusInput->setPlaceholderText("鐘舵€佽繃婊わ紝渚嬪 Completed");
    m_courseCategoryInput = new QLineEdit(filterCard);
    m_courseCategoryInput->setPlaceholderText("绫诲埆杩囨护锛屼緥濡?Required");
    m_courseSortInput = new QLineEdit(filterCard);
    m_courseSortInput->setPlaceholderText("鎺掑簭锛歶pdated / semester / credits / score / gpa / name");
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
    QPushButton* addButton = new QPushButton("鏂板璇剧▼", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑璇剧▼", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑璇剧▼", page);
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

    QLabel* helper = new QLabel("璇剧▼搴撶幇鍦ㄦ敮鎸佸師鐢?Qt 褰曞叆涓庣紪杈戙€傚弻鍑讳换鎰忎竴琛屼篃鍙互鐩存帴鎵撳紑缂栬緫寮圭獥銆?, tableCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    cardLayout->addWidget(helper);

    m_courseTable = new QTableWidget(tableCard);
    m_courseTable->setColumnCount(7);
    m_courseTable->setHorizontalHeaderLabels(
        {"璇剧▼鍚嶇О", "浠ｇ爜", "瀛︽湡", "瀛﹀垎", "鍒嗘暟", "缁╃偣", "鐘舵€?});
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

    QLabel* title = new QLabel("瑙掕壊鑱岃矗", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_roleSummaryLabel = new QLabel("姝ｅ湪璇诲彇瑙掕壊鏁版嵁...", page);
    m_roleSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_roleSummaryLabel);

    QFrame* roleFilterCard = new QFrame(page);
    roleFilterCard->setObjectName("contentCard");
    QGridLayout* roleFilterLayout = new QGridLayout(roleFilterCard);
    roleFilterLayout->setContentsMargins(12, 12, 12, 12);
    roleFilterLayout->setHorizontalSpacing(10);
    roleFilterLayout->setVerticalSpacing(10);
    m_roleSearchInput = new QLineEdit(roleFilterCard);
    m_roleSearchInput->setPlaceholderText("鎼滅储瑙掕壊鍚嶇О / 缁勭粐 / 鎻忚堪");
    m_roleTypeFilter = new QComboBox(roleFilterCard);
    m_roleTypeFilter->addItem("鍏ㄩ儴绫诲瀷", "");
    m_roleTypeFilter->addItem("浠昏亴", "浠昏亴");
    m_roleTypeFilter->addItem("鐝", "鐝");
    m_roleTypeFilter->addItem("绀惧洟", "绀惧洟");
    m_roleTypeFilter->addItem("鍥㈤槦", "鍥㈤槦");
    m_roleTypeFilter->addItem("瀹炰範", "瀹炰範");
    m_roleTypeFilter->addItem("鍏朵粬", "鍏朵粬");
    roleFilterLayout->addWidget(m_roleSearchInput, 0, 0);
    roleFilterLayout->addWidget(m_roleTypeFilter, 0, 1);
    connect(m_roleSearchInput, &QLineEdit::textChanged, this, [this]() { refreshRoles(); });
    connect(m_roleTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshRoles(); });
    layout->addWidget(roleFilterCard);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("瑙掕壊鎬绘暟", &m_rolesTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("杩涜涓鑹?, &m_rolesActiveValue), 0, 1);
    metrics->addWidget(createMetricCard("瑙掕壊绫诲瀷鏁?, &m_rolesTypeValue, "鎸?type 瀛楁缁熻"), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板瑙掕壊", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑瑙掕壊", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑瑙掕壊", page);
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
    QLabel* listTitle = new QLabel("瑙掕壊鏃堕棿绾?, listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("鏀寔鍘熺敓 Qt 缁存姢浠昏亴鍜岃亴璐ｈ褰曘€傚弻鍑绘潯鐩嵆鍙洿鎺ョ紪杈戙€?, listCard);
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

    QLabel* title = new QLabel("锟缴癸拷锟斤拷录", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_achievementSummaryLabel = new QLabel("锟斤拷锟节讹拷取锟缴癸拷锟斤拷锟斤拷...", page);
    m_achievementSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_achievementSummaryLabel);

    QFrame* achFilterCard = new QFrame(page);
    achFilterCard->setObjectName("contentCard");
    QGridLayout* achFilterLayout = new QGridLayout(achFilterCard);
    achFilterLayout->setContentsMargins(12, 12, 12, 12);
    achFilterLayout->setHorizontalSpacing(10);
    achFilterLayout->setVerticalSpacing(10);
    m_achievementSearchInput = new QLineEdit(achFilterCard);
    m_achievementSearchInput->setPlaceholderText("鎼滅储鎴愭灉鏍囬 / 鏈烘瀯 / 鎻忚堪");
    m_achievementTypeFilter = new QComboBox(achFilterCard);
    m_achievementTypeFilter->addItem("鍏ㄩ儴绫诲瀷", "");
    m_achievementTypeFilter->addItem("璇佷功", "璇佷功");
    m_achievementTypeFilter->addItem("绔炶禌", "绔炶禌");
    m_achievementTypeFilter->addItem("濂栭」", "濂栭」");
    m_achievementTypeFilter->addItem("璇剧▼鎴愭灉", "璇剧▼鎴愭灉");
    m_achievementTypeFilter->addItem("寮€婧愯础鐚?, "寮€婧愯础鐚?);
    m_achievementTypeFilter->addItem("璁烘枃鎶ュ憡", "璁烘枃鎶ュ憡");
    m_achievementTypeFilter->addItem("鍏朵粬", "鍏朵粬");
    m_achievementLevelFilter = new QComboBox(achFilterCard);
    m_achievementLevelFilter->addItem("鍏ㄩ儴绾у埆", "");
    m_achievementLevelFilter->addItem("鍥藉绾?, "鍥藉绾?);
    m_achievementLevelFilter->addItem("鐪佺骇", "鐪佺骇");
    m_achievementLevelFilter->addItem("鏍＄骇", "鏍＄骇");
    m_achievementLevelFilter->addItem("闄㈢骇", "闄㈢骇");
    m_achievementLevelFilter->addItem("鍏朵粬", "鍏朵粬");
    achFilterLayout->addWidget(m_achievementSearchInput, 0, 0);
    achFilterLayout->addWidget(m_achievementTypeFilter, 0, 1);
    achFilterLayout->addWidget(m_achievementLevelFilter, 0, 2);
    connect(m_achievementSearchInput, &QLineEdit::textChanged, this, [this]() { refreshAchievements(); });
    connect(m_achievementTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshAchievements(); });
    connect(m_achievementLevelFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshAchievements(); });
    layout->addWidget(achFilterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板鎴愭灉", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑鎴愭灉", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑鎴愭灉", page);
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
    metrics->addWidget(createMetricCard("鎴愭灉鎬绘暟", &m_achievementTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("宸查獙璇佹垚鏋滄暟", &m_achievementVerifiedValue), 0, 1);
    metrics->addWidget(createMetricCard("鎴愭灉绾у埆鏁?, &m_achievementLevelValue, "鎸?level 瀛楁缁熻"), 0, 2);
    metrics->addWidget(createMetricCard("鎴愭灉绫诲瀷鏁?, &m_achievementTypeValue, "鎸?type 瀛楁缁熻"), 0, 3);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("鎴愭灉鏃堕棿绾?, listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("绔炶禌銆佽瘉涔﹀拰濂栭」閮藉彲浠ョ洿鎺ュ湪杩欓噷缁存姢銆傚弻鍑绘潯鐩繘鍏ュ師鐢熺紪杈戝脊绐椼€?, listCard);
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

    QLabel* title = new QLabel("缁忓巻妗ｆ", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_experienceSummaryLabel = new QLabel("姝ｅ湪璇诲彇缁忓巻鏁版嵁...", page);
    m_experienceSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_experienceSummaryLabel);

    QFrame* expFilterCard = new QFrame(page);
    expFilterCard->setObjectName("contentCard");
    QGridLayout* expFilterLayout = new QGridLayout(expFilterCard);
    expFilterLayout->setContentsMargins(12, 12, 12, 12);
    expFilterLayout->setHorizontalSpacing(10);
    expFilterLayout->setVerticalSpacing(10);
    m_experienceSearchInput = new QLineEdit(expFilterCard);
    m_experienceSearchInput->setPlaceholderText("鎼滅储缁忓巻鏍囬 / 缁勭粐 / 鎻忚堪");
    m_experienceTypeFilter = new QComboBox(expFilterCard);
    m_experienceTypeFilter->addItem("鍏ㄩ儴绫诲瀷", "");
    m_experienceTypeFilter->addItem("椤圭洰", "椤圭洰");
    m_experienceTypeFilter->addItem("瀹炰範", "瀹炰範");
    m_experienceTypeFilter->addItem("绉戠爺", "绉戠爺");
    m_experienceTypeFilter->addItem("蹇楁効", "蹇楁効");
    m_experienceTypeFilter->addItem("绔炶禌", "绔炶禌");
    m_experienceTypeFilter->addItem("鍏朵粬", "鍏朵粬");
    expFilterLayout->addWidget(m_experienceSearchInput, 0, 0);
    expFilterLayout->addWidget(m_experienceTypeFilter, 0, 1);
    connect(m_experienceSearchInput, &QLineEdit::textChanged, this, [this]() { refreshExperiences(); });
    connect(m_experienceTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshExperiences(); });
    layout->addWidget(expFilterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板缁忓巻", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑缁忓巻", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑缁忓巻", page);
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
    metrics->addWidget(createMetricCard("缁忓巻鎬绘暟", &m_experienceTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("杩涜涓粡鍘?, &m_experienceOngoingValue), 0, 1);
    metrics->addWidget(createMetricCard("缁忓巻绫诲瀷鏁?, &m_experienceTypeValue, "鎸?type 瀛楁缁熻"), 0, 2);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("缁忓巻鏃堕棿绾?, listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("椤圭洰銆佸疄涔犮€佺鐮斿拰蹇楁効缁忓巻閮藉彲浠ュ湪鍘熺敓鐣岄潰涓淮鎶ゃ€傚弻鍑绘潯鐩嵆鍙紪杈戙€?, listCard);
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

    QLabel* title = new QLabel("璇惧娲诲姩", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_activitySummaryLabel = new QLabel("姝ｅ湪璇诲彇娲诲姩鏁版嵁...", page);
    m_activitySummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_activitySummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_activitySearchInput = new QLineEdit(filterCard);
    m_activitySearchInput->setPlaceholderText("鎼滅储娲诲姩鍚嶇О / 鎻忚堪");
    m_activityCategoryFilter = new QComboBox(filterCard);
    m_activityCategoryFilter->addItem("鍏ㄩ儴鍒嗙被", "");
    m_activityCategoryFilter->addItem("瀛︽湳", "瀛︽湳");
    m_activityCategoryFilter->addItem("鏂囦綋", "鏂囦綋");
    m_activityCategoryFilter->addItem("蹇楁効", "蹇楁効");
    m_activityCategoryFilter->addItem("绀惧洟", "绀惧洟");
    m_activityCategoryFilter->addItem("绔炶禌", "绔炶禌");
    m_activityCategoryFilter->addItem("鍏朵粬", "鍏朵粬");
    filterLayout->addWidget(m_activitySearchInput, 0, 0);
    filterLayout->addWidget(m_activityCategoryFilter, 0, 1);
    connect(m_activitySearchInput, &QLineEdit::textChanged, this, [this]() { refreshActivities(); });
    connect(m_activityCategoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshActivities(); });
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板娲诲姩", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑娲诲姩", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑娲诲姩", page);
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
    metrics->addWidget(createMetricCard("娲诲姩鎬绘暟", &m_activityTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("閲嶇偣娲诲姩", &m_activityFavoriteValue), 0, 1);
    metrics->addWidget(createMetricCard("杩涜涓椿鍔?, &m_activityActiveValue), 0, 2);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("娲诲姩璁板綍", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("璇惧鎷撳睍銆佸織鎰挎湇鍔′笌鏃ュ父娲诲姩銆傛敮鎸佸弻鍑荤紪杈戙€?, listCard);
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

    QLabel* title = new QLabel("鐩爣杩借釜", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_goalSummaryLabel = new QLabel("姝ｅ湪璇诲彇鐩爣鏁版嵁...", page);
    m_goalSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_goalSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_goalSearchInput = new QLineEdit(filterCard);
    m_goalSearchInput->setPlaceholderText("鎼滅储鐩爣鏍囬 / 鎻忚堪");
    m_goalStatusFilter = new QComboBox(filterCard);
    m_goalStatusFilter->addItem("鍏ㄩ儴鐘舵€?, "");
    m_goalStatusFilter->addItem("杩涜涓?, "In Progress");
    m_goalStatusFilter->addItem("宸插畬鎴?, "Completed");
    m_goalStatusFilter->addItem("宸叉殏鍋?, "On Hold");
    m_goalStatusFilter->addItem("宸插彇娑?, "Cancelled");
    m_goalPriorityFilter = new QComboBox(filterCard);
    m_goalPriorityFilter->addItem("鍏ㄩ儴浼樺厛绾?, "");
    m_goalPriorityFilter->addItem("楂樹紭鍏堢骇", "High");
    m_goalPriorityFilter->addItem("涓紭鍏堢骇", "Medium");
    m_goalPriorityFilter->addItem("浣庝紭鍏堢骇", "Low");
    m_goalSortInput = new QLineEdit(filterCard);
    m_goalSortInput->setPlaceholderText("鎺掑簭锛歱rogress / deadline / title / priority");
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
    metrics->addWidget(createMetricCard("鐩爣鎬绘暟", &m_goalTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("宸插畬鎴愮洰鏍?, &m_goalCompletedValue), 0, 1);
    metrics->addWidget(createMetricCard("骞冲潎杩涘害", &m_goalProgressMetricValue, "鍩轰簬 target/current 鑷姩璁＄畻"), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板鐩爣", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑鐩爣", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑鐩爣", page);
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
    QLabel* listTitle = new QLabel("鐩爣娓呭崟", listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("鐩爣鍙互鐩存帴鍦ㄥ師鐢熺獥鍙ｄ腑缁存姢锛屼繚瀛樺悗浼氳仈鍔ㄥ埛鏂版€昏銆佹椂闂磋酱鍜岀畝鍘嗗垎鏋愩€?, listCard);
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

    QLabel* title = new QLabel("鐩爣宀椾綅", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_jobSummaryLabel = new QLabel("姝ｅ湪璇诲彇宀椾綅鏁版嵁...", page);
    m_jobSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_jobSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_jobSearchInput = new QLineEdit(filterCard);
    m_jobSearchInput->setPlaceholderText("鎼滅储宀椾綅鍚嶇О / 鍏徃 / 鍩庡競");
    m_jobStatusInput = new QLineEdit(filterCard);
    m_jobStatusInput->setPlaceholderText("杩囨护婵€娲荤姸鎬?);
    filterLayout->addWidget(m_jobSearchInput, 0, 0);
    filterLayout->addWidget(m_jobStatusInput, 0, 1);
    connect(m_jobSearchInput, &QLineEdit::textChanged, this, [this]() { refreshJobs(); });
    connect(m_jobStatusInput, &QLineEdit::textChanged, this, [this]() { refreshJobs(); });
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板宀椾綅", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑宀椾綅", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑宀椾綅", page);
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
    metrics->addWidget(createMetricCard("宀椾綅鎬绘暟", &m_jobTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("鍏虫敞涓?, &m_jobActiveValue), 0, 1);
    metrics->addWidget(createMetricCard("骞冲潎瑕佹眰鍖归厤鐜?, &m_jobRequirementValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("宀椾綅鍒楄〃", listCard);
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
                m_jobRequirementSummaryLabel->setText(QString("姝ゅ矖浣嶅叡鏈?%1 椤硅姹傦紝宸插尮閰?%2 椤广€?).arg(job.requirements.size()).arg(metCount));
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
    QLabel* reqTitle = new QLabel("宀椾綅瑕佹眰鍖归厤", reqCard);
    reqTitle->setObjectName("sectionTitle");
    reqLayout->addWidget(reqTitle);
    m_jobRequirementSummaryLabel = new QLabel("璇峰湪宸︿晶閫夋嫨宀椾綅", reqCard);
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

    QLabel* title = new QLabel("鍒嗘瀽鎶ュ憡", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_analysisSummaryLabel = new QLabel("姝ｅ湪鐢熸垚鏁版嵁鍒嗘瀽鎶ュ憡...", page);
    m_analysisSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_analysisSummaryLabel);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addPeerBtn = new QPushButton("褰曞叆瀵圭収鍚屽鏁版嵁", page);
    QPushButton* editPeerBtn = new QPushButton("缂栬緫鍚屽鏁版嵁", page);
    QPushButton* delPeerBtn = new QPushButton("鍒犻櫎鍚屽鏁版嵁", page);
    QPushButton* refreshBtn = new QPushButton("閲嶆柊鐢熸垚鎶ュ憡", page);
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
    metrics->addWidget(createMetricCard("鍒嗘瀽瀛︽湡鏁?, &m_analysisSemesterValue), 0, 0);
    metrics->addWidget(createMetricCard("瀵规瘮瀵硅薄鏁?, &m_analysisPeerValue), 0, 1);
    metrics->addWidget(createMetricCard("鎬荤粨寤鸿鏉℃暟", &m_analysisSuggestionValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* tableCard = new QFrame(page);
    tableCard->setObjectName("contentCard");
    QVBoxLayout* tLayout = new QVBoxLayout(tableCard);
    tLayout->setContentsMargins(16, 14, 16, 14);
    tLayout->setSpacing(10);

    QLabel* semLabel = new QLabel("瀛︽湡瓒嬪娍琛ㄧ幇", tableCard);
    semLabel->setObjectName("sectionTitle");
    tLayout->addWidget(semLabel);
    m_analysisSemesterTable = new QTableWidget(tableCard);
    m_analysisSemesterTable->setColumnCount(4);
    m_analysisSemesterTable->setHorizontalHeaderLabels({"瀛︽湡", "淇瀛﹀垎", "鍔犳潈缁╃偣", "鎺掑悕"});
    m_analysisSemesterTable->horizontalHeader()->setStretchLastSection(true);
    m_analysisSemesterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_analysisSemesterTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_analysisSemesterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tLayout->addWidget(m_analysisSemesterTable, 1);

    QLabel* peerLabel = new QLabel("妯悜鍚屽瀵规瘮", tableCard);
    peerLabel->setObjectName("sectionTitle");
    tLayout->addWidget(peerLabel);
    m_analysisPeerTable = new QTableWidget(tableCard);
    m_analysisPeerTable->setColumnCount(6);
    m_analysisPeerTable->setHorizontalHeaderLabels({"濮撳悕", "涓撲笟", "瀛︽湡", "GPA", "鎴愭灉", "缁忓巻"});
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
    QLabel* strLabel = new QLabel("鏍稿績浼樺娍", suggestionCard);
    strLabel->setObjectName("sectionTitle");
    sLayout->addWidget(strLabel);
    m_analysisStrengthList = new QListWidget(suggestionCard);
    m_analysisStrengthList->setObjectName("plainList");
    m_analysisStrengthList->setWordWrap(true);
    m_analysisStrengthList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sLayout->addWidget(m_analysisStrengthList, 1);

    QLabel* rskLabel = new QLabel("娼滃湪椋庨櫓", suggestionCard);
    rskLabel->setObjectName("sectionTitle");
    sLayout->addWidget(rskLabel);
    m_analysisRiskList = new QListWidget(suggestionCard);
    m_analysisRiskList->setObjectName("plainList");
    m_analysisRiskList->setWordWrap(true);
    m_analysisRiskList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sLayout->addWidget(m_analysisRiskList, 1);

    QLabel* sugLabel = new QLabel("鍙戝睍寤鸿", suggestionCard);
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

    QLabel* title = new QLabel("鏃堕棿杞?, page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_timelineSummaryLabel = new QLabel("姝ｅ湪鐢熸垚鎴愰暱鏃堕棿杞?..", page);
    m_timelineSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_timelineSummaryLabel);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("浜嬩欢鎬绘暟", &m_timelineEventCountValue), 0, 0);
    metrics->addWidget(createMetricCard("浼樺娍鏉℃暟", &m_timelineStrengthValue), 0, 1);
    metrics->addWidget(createMetricCard("椋庨櫓鏉℃暟", &m_timelineRiskValue), 0, 2);
    layout->addLayout(metrics);

    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->setSpacing(14);

    QFrame* timelineCard = new QFrame(page);
    timelineCard->setObjectName("contentCard");
    QVBoxLayout* timelineLayout = new QVBoxLayout(timelineCard);
    timelineLayout->setContentsMargins(16, 14, 16, 14);
    timelineLayout->setSpacing(10);
    QLabel* eventTitle = new QLabel("鎴愰暱浜嬩欢", timelineCard);
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
    QLabel* suggestionTitle = new QLabel("闃舵寤鸿", suggestionCard);
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

    QLabel* title = new QLabel("绠€鍘嗗鍑?, page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_resumeSummaryLabel = new QLabel("姝ｅ湪鐢熸垚鍘熺敓绠€鍘嗛瑙?..", page);
    m_resumeSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_resumeSummaryLabel);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("绠€鍘嗗垎鍖烘暟", &m_resumeSectionCountValue), 0, 0);
    metrics->addWidget(createMetricCard("韬唤鏍囬", &m_resumeIdentityValue, "鏉ヨ嚜绠€鍘嗙敓鎴愰厤缃?), 0, 1);
    layout->addLayout(metrics);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* refreshButton = new QPushButton("鍒锋柊棰勮", page);
    QPushButton* resetButton = new QPushButton("鎭㈠榛樿閰嶇疆", page);
    QPushButton* exportJsonButton = new QPushButton("瀵煎嚭 JSON", page);
    QPushButton* exportHtmlButton = new QPushButton("瀵煎嚭 HTML", page);
    QPushButton* copyToClipboardButton = new QPushButton("澶嶅埗鍒板壀璐存澘", page);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshResume);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetResumeOptions);
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::exportResumeJson);
    connect(exportHtmlButton, &QPushButton::clicked, this, &MainWindow::exportResumeHtml);
    connect(copyToClipboardButton, &QPushButton::clicked, this, [this]() {
        QJsonObject options = currentResumeOptions();
        QString html = QString::fromUtf8(ResumeService::exportHtml(options));
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(html);
        ToastNotification::display(this, "绠€鍘?HTML 宸插鍒跺埌鍓创鏉裤€?);
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

    QLabel* configTitle = new QLabel("绠€鍘嗛厤缃?, configCard);
    configTitle->setObjectName("sectionTitle");
    configLayout->addWidget(configTitle);

    QLabel* configHint = new QLabel("杩欑粍瀛楁灏辨槸绠€鍘嗛〉闈㈢殑鍗曚竴鏁版嵁婧愩€傞瑙堛€佸鍑轰笌鍚庣画 AI 浼樺寲閮戒細鍩轰簬杩欓噷鐨勯厤缃€?, configCard);
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
    m_resumeNameInput->setPlaceholderText("渚嬪锛氬紶涓?);
    m_resumeTitleInput->setPlaceholderText("渚嬪锛氫釜浜烘垚闀胯鍒掔畝鍘?);
    m_resumeEmailInput->setPlaceholderText("渚嬪锛歯ame@example.com");
    m_resumePhoneInput->setPlaceholderText("渚嬪锛?3800000000");
    m_resumeSummaryInput->setPlaceholderText("鐢?2-4 鍙ヨ瘽姒傛嫭浣犵殑瀛︿範鏂瑰悜銆佸疄璺甸噸鐐瑰拰鎴愰暱浜偣銆?);
    m_resumeCustomContentInput->setPlaceholderText("杩欓噷鐢ㄤ簬鎵嬪姩琛ュ厖绠€鍘嗕寒鐐广€傚彸渚у€欓€夌礌鏉愭敮鎸佺偣鍑绘彃鍏ワ紝浣犱篃鍙互鐩存帴缂栬緫銆?);

    formLayout->addRow("濮撳悕", m_resumeNameInput);
    formLayout->addRow("韬唤鏍囬", m_resumeTitleInput);
    formLayout->addRow("閭", m_resumeEmailInput);
    formLayout->addRow("鐢佃瘽", m_resumePhoneInput);
    formLayout->addRow("涓汉鎽樿", m_resumeSummaryInput);
    formLayout->addRow("琛ュ厖鍐呭", m_resumeCustomContentInput);
    configLayout->addLayout(formLayout);

    QLabel* sectionHint = new QLabel("鍒嗗尯寮€鍏?, configCard);
    sectionHint->setObjectName("sectionTitle");
    configLayout->addWidget(sectionHint);

    m_resumeEducationCheck = new QCheckBox("鏁欒偛缁忓巻", configCard);
    m_resumeExperienceCheck = new QCheckBox("瀹炶返缁忓巻", configCard);
    m_resumeAchievementCheck = new QCheckBox("鎴愭灉璁板綍", configCard);
    m_resumeRoleCheck = new QCheckBox("瑙掕壊浠昏亴", configCard);
    m_resumeActivityCheck = new QCheckBox("娲诲姩鍙備笌", configCard);
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

    QLabel* candidateTitle = new QLabel("澶囬€夌礌鏉?, candidateCard);
    candidateTitle->setObjectName("sectionTitle");
    candidateLayout->addWidget(candidateTitle);

    QLabel* candidateHint = new QLabel("浠庡凡鏈夎绋嬨€佺粡鍘嗐€佹垚鏋滃拰娲诲姩涓寫閫夊唴瀹癸紝鐐瑰嚮鍗冲彲鎻掑叆鍒拌ˉ鍏呭唴瀹瑰尯锛屽啀鎸変綘鐨勯渶瑕佸井璋冦€?, candidateCard);
    candidateHint->setObjectName("pageSubtitle");
    candidateHint->setWordWrap(true);
    candidateLayout->addWidget(candidateHint);

    m_resumeCandidateTypeCombo = new QComboBox(candidateCard);
    m_resumeCandidateTypeCombo->addItem("璇剧▼浜偣", "course");
    m_resumeCandidateTypeCombo->addItem("瀹炶返缁忓巻", "experience");
    m_resumeCandidateTypeCombo->addItem("鎴愭灉璁板綍", "achievement");
    m_resumeCandidateTypeCombo->addItem("瑙掕壊鑱岃矗", "role");
    m_resumeCandidateTypeCombo->addItem("璇惧娲诲姩", "activity");
    candidateLayout->addWidget(m_resumeCandidateTypeCombo);

    m_resumeCandidateList = new QListWidget(candidateCard);
    m_resumeCandidateList->setObjectName("plainList");
    m_resumeCandidateList->setWordWrap(true);
    m_resumeCandidateList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    candidateLayout->addWidget(m_resumeCandidateList, 1);

    QHBoxLayout* candidateActionLayout = new QHBoxLayout();
    candidateActionLayout->setSpacing(8);
    QPushButton* insertToSummaryButton = new QPushButton("杩藉姞鍒版憳瑕?, candidateCard);
    QPushButton* replaceSummaryButton = new QPushButton("鏇挎崲鎽樿", candidateCard);
    insertToSummaryButton->setStyleSheet("QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 8px; padding: 6px 12px; color: #42526b; } QPushButton:hover { background: #f4f8ff; }");
    replaceSummaryButton->setStyleSheet("QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 8px; padding: 6px 12px; color: #42526b; } QPushButton:hover { background: #f4f8ff; }");
    candidateActionLayout->addWidget(insertToSummaryButton);
    candidateActionLayout->addWidget(replaceSummaryButton);
    candidateLayout->addLayout(candidateActionLayout);

    QPushButton* clearCustomContentButton = new QPushButton("娓呯┖琛ュ厖鍐呭", candidateCard);
    candidateLayout->addWidget(clearCustomContentButton);
    candidateLayout->addStretch(0);

    connect(insertToSummaryButton, &QPushButton::clicked, this, [this]() {
        if (!m_resumeCandidateList || !m_resumeSummaryInput) return;
        QListWidgetItem* item = m_resumeCandidateList->currentItem();
        if (!item) {
            ToastNotification::display(this, "璇峰厛閫夋嫨涓€鏉＄礌鏉愩€?);
            return;
        }
        const QString snippet = item->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) return;
        QString current = m_resumeSummaryInput->toPlainText().trimmed();
        if (!current.isEmpty()) current += "\n";
        current += snippet;
        m_resumeSummaryInput->setPlainText(current);
        ToastNotification::display(this, "宸茶拷鍔犲埌涓汉鎽樿銆?);
    });
    connect(replaceSummaryButton, &QPushButton::clicked, this, [this]() {
        if (!m_resumeCandidateList || !m_resumeSummaryInput) return;
        QListWidgetItem* item = m_resumeCandidateList->currentItem();
        if (!item) {
            ToastNotification::display(this, "璇峰厛閫夋嫨涓€鏉＄礌鏉愩€?);
            return;
        }
        const QString snippet = item->data(Qt::UserRole).toString().trimmed();
        if (snippet.isEmpty()) return;
        m_resumeSummaryInput->setPlainText(snippet);
        ToastNotification::display(this, "宸叉浛鎹釜浜烘憳瑕併€?);
    });

    QFrame* previewCard = new QFrame(page);
    previewCard->setObjectName("contentCard");
    previewCard->setMinimumWidth(460);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(16, 14, 16, 14);
    previewLayout->setSpacing(10);
    QLabel* previewTitle = new QLabel("绠€鍘嗛瑙?, previewCard);
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
            ToastNotification::display(this, "宸叉彃鍏ュ埌琛ュ厖鍐呭鍖猴紝浣犲彲浠ョ户缁慨鏀广€?);
        } else {
            ToastNotification::display(this, "杩欐潯绱犳潗宸茬粡鍦ㄨˉ鍏呭唴瀹瑰尯涓簡銆?);
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

    QLabel* title = new QLabel("绠€鍘嗗鍑?, page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_resumeSummaryLabel = new QLabel("宸︿晶濉啓绠€鍘嗕俊鎭紝鍙充晶閫夋嫨澶囬€夌礌鏉愩€傜偣鍑汇€岄瑙堢畝鍘嗐€嶆煡鐪嬫晥鏋溿€?, page);
    m_resumeSummaryLabel->setObjectName("pageSubtitle");
    m_resumeSummaryLabel->setWordWrap(true);
    layout->addWidget(m_resumeSummaryLabel);

    QFrame* toolbarCard = new QFrame(page);
    toolbarCard->setObjectName("contentCard");
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbarCard);
    toolbarLayout->setContentsMargins(16, 12, 16, 12);
    toolbarLayout->setSpacing(10);

    QLabel* logo = new QLabel("绠€鍘嗗伐鍧?, toolbarCard);
    logo->setStyleSheet("font-size: 16px; font-weight: 700; color: #2f67ff;");
    toolbarLayout->addWidget(logo);

    QPushButton* addModuleButton = new QPushButton("娣诲姞妯″潡", toolbarCard);
    toolbarLayout->addWidget(addModuleButton);
    toolbarLayout->addStretch();

    const QString toolButtonStyle =
        "QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 10px; padding: 7px 14px; color: #42526b; }"
        "QPushButton:hover { background: #f4f8ff; border-color: #9db8ff; }";
    const QString activeButtonStyle =
        "QPushButton { background: #2f67ff; border: 1px solid #2f67ff; border-radius: 10px; padding: 7px 16px; color: white; font-weight: 600; }";

    QPushButton* previewButton = new QPushButton("棰勮绠€鍘?, toolbarCard);
    QPushButton* resetTopButton = new QPushButton("閲嶇疆", toolbarCard);
    QPushButton* exportJsonButton = new QPushButton("瀵煎嚭 JSON", toolbarCard);
    QPushButton* exportHtmlButton = new QPushButton("瀵煎嚭 HTML", toolbarCard);
    QPushButton* exportPdfButton = new QPushButton("瀵煎嚭 PDF", toolbarCard);
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

    QLabel* editorTitle = new QLabel("绠€鍘嗕俊鎭?, editorCard);
    editorTitle->setObjectName("sectionTitle");
    editorLayout->addWidget(editorTitle);

    m_resumeEditorTabs = new QTabWidget(editorCard);
    QWidget* contentTab = new QWidget(m_resumeEditorTabs);
    QWidget* styleTab = new QWidget(m_resumeEditorTabs);
    m_resumeEditorTabs->addTab(contentTab, "鍐呭缂栬緫");
    m_resumeEditorTabs->addTab(styleTab, "鏍峰紡璁剧疆");
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

    formLayout->addRow("濮撳悕", m_resumeNameInput);
    formLayout->addRow("韬唤鏍囬", m_resumeTitleInput);
    formLayout->addRow("骞撮緞", m_resumeAgeInput);
    formLayout->addRow("鍩庡競", m_resumeCityInput);
    formLayout->addRow("閭", m_resumeEmailInput);
    formLayout->addRow("鐢佃瘽", m_resumePhoneInput);
    formLayout->addRow("姹傝亴鎰忓悜", m_resumeIntentInput);
    formLayout->addRow("瀛︽牎鍚嶇О", m_resumeSchoolInput);
    formLayout->addRow("涓撲笟鍚嶇О", m_resumeMajorInput);
    formLayout->addRow("瀛﹀巻瀛︿綅", m_resumeDegreeInput);
    formLayout->addRow("涓汉绠€浠?, m_resumeSummaryInput);
    formLayout->addRow("鏁欒偛鎻忚堪", m_resumeEducationBodyInput);
    formLayout->addRow("鎶€鑳界壒闀?, m_resumeSkillsBodyInput);
    formLayout->addRow("椤圭洰鍚嶇О", m_resumeProjectNameInput);
    formLayout->addRow("椤圭洰缁忛獙", m_resumeProjectBodyInput);
    formLayout->addRow("瀹炰範缁忓巻", m_resumeInternshipInput);
    formLayout->addRow("绔炶禌鑾峰", m_resumeAwardsInput);
    formLayout->addRow("鑷垜璇勪环", m_resumeCustomContentInput);
    contentTabLayout->addLayout(formLayout);

    QVBoxLayout* styleTabLayout = new QVBoxLayout(styleTab);
    styleTabLayout->setContentsMargins(16, 12, 16, 16);
    styleTabLayout->setSpacing(12);
    m_resumeSectionVisibleCheck = new QCheckBox("鏄剧ず褰撳墠妯″潡", styleTab);
    styleTabLayout->addWidget(m_resumeSectionVisibleCheck);
    m_resumeEducationCheck = new QCheckBox("鏄剧ず鏁欒偛鑳屾櫙", styleTab);
    m_resumeExperienceCheck = new QCheckBox("鏄剧ず椤圭洰缁忛獙", styleTab);
    m_resumeAchievementCheck = new QCheckBox("鏄剧ず鎶€鑳界壒闀?, styleTab);
    m_resumeRoleCheck = new QCheckBox("鏄剧ず姹傝亴鎰忓悜", styleTab);
    m_resumeActivityCheck = new QCheckBox("鏄剧ず鑷垜璇勪环", styleTab);
    styleTabLayout->addWidget(m_resumeEducationCheck);
    styleTabLayout->addWidget(m_resumeExperienceCheck);
    styleTabLayout->addWidget(m_resumeAchievementCheck);
    styleTabLayout->addWidget(m_resumeRoleCheck);
    styleTabLayout->addWidget(m_resumeActivityCheck);
    QLabel* styleHint = new QLabel("鍕鹃€夐渶瑕佹樉绀虹殑绠€鍘嗘ā鍧椼€?, styleTab);
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

    QLabel* candidateTitle = new QLabel("澶囬€夌礌鏉?, candidateCard);
    candidateTitle->setObjectName("sectionTitle");
    candidateLayout->addWidget(candidateTitle);

    QLabel* candidateHint = new QLabel("浠庡凡鏈夋暟鎹腑閫夋嫨绱犳潗锛岀偣鍑诲嵆鍙彃鍏ュ埌瀵瑰簲缂栬緫鍖恒€?, candidateCard);
    candidateHint->setObjectName("pageSubtitle");
    candidateHint->setWordWrap(true);
    candidateLayout->addWidget(candidateHint);

    m_resumeCandidateTypeCombo = new QComboBox(candidateCard);
    m_resumeCandidateTypeCombo->addItem("璇剧▼浜偣", "course");
    m_resumeCandidateTypeCombo->addItem("瀹炶返缁忓巻", "experience");
    m_resumeCandidateTypeCombo->addItem("鎴愭灉璁板綍", "achievement");
    m_resumeCandidateTypeCombo->addItem("瑙掕壊鑱岃矗", "role");
    m_resumeCandidateTypeCombo->addItem("璇惧娲诲姩", "activity");
    candidateLayout->addWidget(m_resumeCandidateTypeCombo);

    m_resumeCandidateList = new QListWidget(candidateCard);
    m_resumeCandidateList->setObjectName("plainList");
    m_resumeCandidateList->setWordWrap(true);
    m_resumeCandidateList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    candidateLayout->addWidget(m_resumeCandidateList, 1);

    QHBoxLayout* candidateActionLayout = new QHBoxLayout();
    QPushButton* clearCustomContentButton = new QPushButton("娓呯┖琛ュ厖", candidateCard);
    QPushButton* insertSummaryButton = new QPushButton("鎻掑叆鍒扮畝浠?, candidateCard);
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
            m_resumeCustomContentInput->append("鏂板妯″潡锛氳鍦ㄨ繖閲岃ˉ鍏呮柊鐨勬ā鍧楀唴瀹广€?);
        }
    });
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::exportResumeJson);
    connect(exportHtmlButton, &QPushButton::clicked, this, &MainWindow::exportResumeHtml);
    connect(exportPdfButton, &QPushButton::clicked, this, &MainWindow::exportResumePdf);
    connect(resetTopButton, &QPushButton::clicked, this, &MainWindow::resetResumeOptions);
    connect(previewButton, &QPushButton::clicked, this, [this]() {
        refreshResume();
        QDialog* previewDialog = new QDialog(this);
        previewDialog->setWindowTitle("绠€鍘嗛瑙?);
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
        QLabel* titleLabel = new QLabel("馃搫 绠€鍘嗛瑙?, previewDialog);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #1f2937;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();
        
        m_resumePageLabel = new QLabel("绗?1 / 1 椤?, previewDialog);
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
        
        QPushButton* prevBtn = new QPushButton("鈼€ 涓婁竴椤?, previewDialog);
        QPushButton* nextBtn = new QPushButton("涓嬩竴椤?鈻?, previewDialog);
        QPushButton* closeBtn = new QPushButton("鍏抽棴", previewDialog);
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
                m_resumePageLabel->setText(QString("绗?%1 / %2 椤?).arg(currentPage).arg(totalPages));
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

    QLabel* title = new QLabel("鏁版嵁瀵煎叆", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_importSummaryLabel = new QLabel("鏀寔浠庡閮ㄧ郴缁熸壒閲忓鍏ラ€氱敤璇剧▼銆佽鑹层€佹垚鏋滅瓑璁板綍銆?, page);
    m_importSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_importSummaryLabel);

    QFrame* controlCard = new QFrame(page);
    controlCard->setObjectName("contentCard");
    QVBoxLayout* cl = new QVBoxLayout(controlCard);
    cl->setContentsMargins(16, 14, 16, 14);
    cl->setSpacing(12);

    QFormLayout* form = new QFormLayout();
    m_importEntityCombo = new QComboBox(controlCard);
    m_importEntityCombo->addItem("璇剧▼鏁版嵁 (courses)", "courses");
    m_importEntityCombo->addItem("瑙掕壊鑱岃矗 (roles)", "roles");
    m_importEntityCombo->addItem("鎴愭灉璁板綍 (achievements)", "achievements");
    m_importEntityCombo->addItem("瀹炶返缁忓巻 (experiences)", "experiences");
    m_importEntityCombo->addItem("璇惧娲诲姩 (activities)", "activities");
    m_importEntityCombo->addItem("鐩爣鏁版嵁 (goals)", "goals");
    m_importEntityCombo->addItem("瀵规爣鍚屽 (peers)", "peers");
    form->addRow("閫夋嫨瑕佸鍏ョ殑鏁版嵁绫诲埆锛?, m_importEntityCombo);
    
    m_importFileLabel = new QLabel("灏氭湭閫夋嫨鏂囦欢", controlCard);
    m_importFileLabel->setObjectName("richCardText");
    QPushButton* pickBtn = new QPushButton("閫夋嫨 CSV 鏂囦欢", controlCard);
    connect(pickBtn, &QPushButton::clicked, this, &MainWindow::chooseImportFile);
    
    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(m_importFileLabel, 1);
    fileRow->addWidget(pickBtn);
    form->addRow("閫夋嫨鏁版嵁婧愭枃浠讹細", fileRow);
    cl->addLayout(form);

    QPushButton* runBtn = new QPushButton("寮€濮嬪鍏?, controlCard);
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::runImport);
    cl->addWidget(runBtn, 0, Qt::AlignRight);
    layout->addWidget(controlCard);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("鎴愬姛瀵煎叆鏉℃暟", &m_importResultImportedValue), 0, 0);
    metrics->addWidget(createMetricCard("瀵煎叆澶辫触鏉℃暟", &m_importResultFailedValue), 0, 1);
    layout->addLayout(metrics);

    QFrame* errCard = new QFrame(page);
    errCard->setObjectName("contentCard");
    QVBoxLayout* el = new QVBoxLayout(errCard);
    el->setContentsMargins(16, 14, 16, 14);
    QLabel* elTitle = new QLabel("瀵煎叆澶辫触鏄庣粏", errCard);
    elTitle->setObjectName("sectionTitle");
    el->addWidget(elTitle);
    m_importErrorTable = new QTableWidget(errCard);
    m_importErrorTable->setColumnCount(2);
    m_importErrorTable->setHorizontalHeaderLabels({"鍑洪敊琛屽彿", "閿欒鍘熷洜"});
    m_importErrorTable->horizontalHeader()->setStretchLastSection(true);
    m_importErrorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_importErrorTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    el->addWidget(m_importErrorTable, 1);
    layout->addWidget(errCard, 1);

    return page;
}

QWidget* MainWindow::createAiPage()
{
    QFrame* page = new QFrame(this);
    page->setObjectName("aiSidebar");
    page->setStyleSheet("#aiSidebar { background: #faf8f4; border-left: 1px solid #ddd3c6; }");

    QHBoxLayout* rootAiLayout = new QHBoxLayout(page);
    rootAiLayout->setContentsMargins(0, 0, 0, 0);
    rootAiLayout->setSpacing(0);

    // === Collapsed Strip (visible when collapsed) ===
    QWidget* collapsedStrip = new QWidget(page);
    collapsedStrip->setObjectName("aiCollapsedStrip");
    collapsedStrip->setFixedWidth(kAiCollapsedWidth);
    collapsedStrip->setCursor(Qt::PointingHandCursor);
    QVBoxLayout* stripLayout = new QVBoxLayout(collapsedStrip);
    stripLayout->setContentsMargins(0, 16, 0, 16);
    stripLayout->setSpacing(12);
    stripLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QPushButton* stripButton = new QPushButton("AI", collapsedStrip);
    stripButton->setFixedSize(44, 44);
    stripButton->setCursor(Qt::PointingHandCursor);
    stripButton->setStyleSheet(
        "background: #2b5c5d; border: none; border-radius: 10px;"
        "font-size: 14px; font-weight: 700; color: white;"
        "QPushButton:hover { background: #1e4a4b; }"
    );
    stripLayout->addWidget(stripButton, 0, Qt::AlignHCenter);
    stripLayout->addStretch();

    rootAiLayout->addWidget(collapsedStrip);

    // === Main Panel Content (visible when expanded) ===
    QWidget* panelContent = new QWidget(page);
    panelContent->setObjectName("aiPanelContent");
    QVBoxLayout* layout = new QVBoxLayout(panelContent);
    layout->setContentsMargins(18, 20, 18, 18);
    layout->setSpacing(14);

    // Title Block
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* title = new QLabel("AI 寤鸿", panelContent);
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #333;");

    titleLayout->addWidget(title);
    titleLayout->addStretch();
    QPushButton* refreshAiButton = new QPushButton("鍒锋柊", panelContent);
    refreshAiButton->setCursor(Qt::PointingHandCursor);
    refreshAiButton->setFixedHeight(28);
    refreshAiButton->setToolTip("鍒锋柊 AI 鏈嶅姟鐘舵€?);
    refreshAiButton->setStyleSheet(
        "background: transparent; border: 1px solid rgba(67,57,43,0.14); border-radius: 8px;"
        "font-size: 12px; color: #5b4e43; padding: 0 10px;"
        "QPushButton:hover { background: #f4ede2; }"
    );
    connect(refreshAiButton, &QPushButton::clicked, this, [this]() {
        AiService::resetAiServerCheck();
        refreshAiStatus();
        if (m_aiOutput) {
            m_aiOutput->setPlainText("AI 鏈嶅姟鐘舵€佸凡鍒锋柊銆俓n\n鐐瑰嚮涓婃柟鎸夐挳寮€濮嬪垎鏋愩€?);
        }
    });
    titleLayout->addWidget(refreshAiButton, 0, Qt::AlignRight);
    QPushButton* collapseButton = new QPushButton("鏀惰捣", panelContent);
    collapseButton->setCursor(Qt::PointingHandCursor);
    collapseButton->setFixedHeight(28);
    collapseButton->setStyleSheet(
        "background: transparent; border: 1px solid rgba(67,57,43,0.14); border-radius: 8px;"
        "font-size: 12px; color: #5b4e43; padding: 0 10px;"
        "QPushButton:hover { background: #f4ede2; }"
    );
    titleLayout->addWidget(collapseButton, 0, Qt::AlignRight);
    layout->addLayout(titleLayout);

    QLabel* panelSubtitle = new QLabel("鍩轰簬璇剧▼銆佺粡鍘嗐€佺洰鏍囦笌绠€鍘嗛厤缃敓鎴愬缓璁紝骞舵敮鎸佸洖濉埌褰撳墠宸ヤ綔娴併€?, panelContent);
    panelSubtitle->setObjectName("pageSubtitle");
    panelSubtitle->setWordWrap(true);
    layout->addWidget(panelSubtitle);

    // Status Tags
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(8);
    m_aiModeValue = new QLabel("绂荤嚎妯″紡", panelContent);
    m_aiStatusValue = new QLabel("鍔犺浇瀹屾垚", panelContent);
    m_aiModelValue = new QLabel("Internal", panelContent);

    QString tagStyle = "background: #f4f6f8; color: #555; padding: 4px 8px; border-radius: 999px; font-size: 11px;";
    m_aiModeValue->setStyleSheet(tagStyle);
    m_aiStatusValue->setStyleSheet(tagStyle);
    m_aiModelValue->setStyleSheet(tagStyle);

    statusLayout->addWidget(m_aiModeValue);
    statusLayout->addWidget(m_aiStatusValue);
    statusLayout->addWidget(m_aiModelValue);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);

    // Toggle Buttons
    QHBoxLayout* toggleLayout = new QHBoxLayout();
    toggleLayout->setSpacing(0);
    QPushButton* chatTabBtn = new QPushButton("瀵硅瘽", panelContent);
    QPushButton* suggTabBtn = new QPushButton("寤鸿", panelContent);
    chatTabBtn->setCursor(Qt::PointingHandCursor);
    suggTabBtn->setCursor(Qt::PointingHandCursor);
    QString activeTabStyle = "background: #2b5c5d; color: #fff; padding: 10px; border: none; font-weight: bold;";
    QString inactiveTabStyle = "background: #f0f2f5; color: #666; padding: 10px; border: none; font-weight: bold;";
    chatTabBtn->setStyleSheet(activeTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
    suggTabBtn->setStyleSheet(inactiveTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
    connect(chatTabBtn, &QPushButton::clicked, this, [chatTabBtn, suggTabBtn, activeTabStyle, inactiveTabStyle, this]() {
        chatTabBtn->setStyleSheet(activeTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
        suggTabBtn->setStyleSheet(inactiveTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
        if (m_aiOutput) m_aiOutput->setPlainText("AI 鍔╂墜宸插氨缁€俓n鍙互鍦ㄤ笅鏂圭洿鎺ヨ緭鍏ラ棶棰樿繘琛屽璇濄€?);
    });
    connect(suggTabBtn, &QPushButton::clicked, this, [chatTabBtn, suggTabBtn, activeTabStyle, inactiveTabStyle, this]() {
        suggTabBtn->setStyleSheet(activeTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
        chatTabBtn->setStyleSheet(inactiveTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
        if (m_aiOutput) m_aiOutput->setPlainText("鐐瑰嚮涓婃柟蹇嵎鎸夐挳鑾峰彇閽堝鎬у缓璁細\n\n- 缁煎悎锛氬叏闈㈠涓氬垎鏋怽n- 璇剧▼锛氳绋嬭鍒掑缓璁甛n- 缁忓巻锛氬疄璺电粡鍘嗘彁鍗嘰n- 鐩爣锛氱洰鏍囧畬鎴愮瓥鐣?);
    });
    toggleLayout->addWidget(chatTabBtn, 1);
    toggleLayout->addWidget(suggTabBtn, 1);
    layout->addLayout(toggleLayout);

    // Quick Action Chips
    QGridLayout* actionGrid = new QGridLayout();
    actionGrid->setSpacing(8);
    QPushButton* btn1 = new QPushButton("缁煎悎", panelContent);
    QPushButton* btn2 = new QPushButton("璇剧▼", panelContent);
    QPushButton* btn3 = new QPushButton("缁忓巻", panelContent);
    QPushButton* btn4 = new QPushButton("鐩爣", panelContent);
    QString secondaryStyle = "background: #eef2f6; color: #444; border: none; border-radius: 4px; padding: 6px;";
    btn1->setStyleSheet(secondaryStyle); btn2->setStyleSheet(secondaryStyle);
    btn3->setStyleSheet(secondaryStyle); btn4->setStyleSheet(secondaryStyle);
    connect(btn1, &QPushButton::clicked, this, [this]() { runAiAnalysis("general"); });
    connect(btn2, &QPushButton::clicked, this, [this]() { runAiAnalysis("course"); });
    connect(btn3, &QPushButton::clicked, this, [this]() { runAiAnalysis("career"); });
    connect(btn4, &QPushButton::clicked, this, [this]() { runAiAnalysis("goal"); });
    actionGrid->addWidget(btn1, 0, 0);
    actionGrid->addWidget(btn2, 0, 1);
    actionGrid->addWidget(btn3, 0, 2);
    actionGrid->addWidget(btn4, 0, 3);
    layout->addLayout(actionGrid);

    QHBoxLayout* helperActionLayout = new QHBoxLayout();
    helperActionLayout->setSpacing(8);
    QPushButton* rerunButton = new QPushButton("閲嶆柊鐢熸垚", panelContent);
    QPushButton* clearContextButton = new QPushButton("娓呯┖涓婁笅鏂?, panelContent);
    rerunButton->setStyleSheet(secondaryStyle);
    clearContextButton->setStyleSheet(secondaryStyle);
    connect(rerunButton, &QPushButton::clicked, this, [this]() { runAiAnalysis("general"); });
    connect(clearContextButton, &QPushButton::clicked, this, [this]() {
        m_selectedContext.clear();
        if (m_aiContextLabel) {
            m_aiContextLabel->hide();
        }
        if (m_aiOutput) {
            m_aiOutput->setPlainText("涓婁笅鏂囧凡娓呯┖銆俓n鍙互閲嶆柊閫夋嫨鍐呭锛屾垨鐩存帴鐐瑰嚮涓婃柟鎸夐挳鐢熸垚鏂板缓璁€?);
        }
    });
    helperActionLayout->addWidget(rerunButton);
    helperActionLayout->addWidget(clearContextButton);
    helperActionLayout->addStretch();
    layout->addLayout(helperActionLayout);

    // Context preview (for text selection)
    m_aiContextLabel = new QLabel(panelContent);
    m_aiContextLabel->setObjectName("aiContextLabel");
    m_aiContextLabel->setWordWrap(true);
    m_aiContextLabel->setMaximumHeight(80);
    m_aiContextLabel->setStyleSheet(
        "background: #eef7f5; border: 0.5px solid rgba(15,111,120,0.28); border-radius: 6px;"
        "padding: 8px 10px; font-size: 12px; color: #24211d;"
    );
    m_aiContextLabel->hide();
    layout->addWidget(m_aiContextLabel);

    // Chat Output
    QFrame* outputCard = new QFrame(panelContent);
    outputCard->setObjectName("contentCard");
    QVBoxLayout* outputLayout = new QVBoxLayout(outputCard);
    outputLayout->setContentsMargins(14, 12, 14, 12);
    outputLayout->setSpacing(8);
    QLabel* outputTitle = new QLabel("鍒嗘瀽缁撴灉", outputCard);
    outputTitle->setObjectName("sectionTitle");
    outputLayout->addWidget(outputTitle);
    m_aiOutput = new QTextEdit(outputCard);
    m_aiOutput->setReadOnly(true);
    m_aiOutput->setAcceptRichText(false);
    m_aiOutput->setLineWrapMode(QTextEdit::WidgetWidth);
    m_aiOutput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aiOutput->setStyleSheet("border: none; background: transparent; font-size: 13px; color: #333;");
    m_aiOutput->setPlainText("AI 鍔╂墜宸插氨缁€俓n鍙互鐐瑰嚮涓婃柟鐨勫揩鎹锋寜閽幏鍙栧缓璁紝鎴栧湪涓嬫柟鐩存帴鍚戞垜鎻愰棶銆?);
    outputLayout->addWidget(m_aiOutput, 1);
    layout->addWidget(outputCard, 1);

    // Quick Apply buttons
    QHBoxLayout* refillLayout = new QHBoxLayout();
    refillLayout->setSpacing(8);
    m_aiToResumeButton = new QPushButton("鍥炲～鎽樿", panelContent);
    m_aiToGoalButton = new QPushButton("杞负鐩爣", panelContent);
    m_aiToResumeButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    m_aiToGoalButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    connect(m_aiToResumeButton, &QPushButton::clicked, this, &MainWindow::applyAiToResumeSummary);
    connect(m_aiToGoalButton, &QPushButton::clicked, this, &MainWindow::createGoalFromAiSuggestion);
    refillLayout->addWidget(m_aiToResumeButton, 1);
    refillLayout->addWidget(m_aiToGoalButton, 1);
    layout->addLayout(refillLayout);

    // Input Area
    QFrame* inputCard = new QFrame(panelContent);
    inputCard->setObjectName("contentCard");
    QHBoxLayout* chatLayout = new QHBoxLayout(inputCard);
    chatLayout->setSpacing(8);
    chatLayout->setContentsMargins(12, 12, 12, 12);
    m_aiChatInput = new QLineEdit(inputCard);
    m_aiChatInput->setPlaceholderText("杈撳叆闂...");
    m_aiChatInput->setStyleSheet("background: #f5f7fa; border: none; border-radius: 10px; padding: 10px; color: #333;");
    QPushButton* sendButton = new QPushButton("鍙戦€?, inputCard);
    sendButton->setStyleSheet("background: #88a7aa; color: white; border: none; border-radius: 6px; padding: 10px 16px; font-weight: bold;");
    sendButton->setCursor(Qt::PointingHandCursor);

    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendAiChat);
    connect(m_aiChatInput, &QLineEdit::returnPressed, this, &MainWindow::sendAiChat);

    chatLayout->addWidget(m_aiChatInput, 1);
    chatLayout->addWidget(sendButton);
    layout->addWidget(inputCard);

    rootAiLayout->addWidget(panelContent, 1);

    auto* panelOpacity = new QGraphicsOpacityEffect(panelContent);
    panelContent->setGraphicsEffect(panelOpacity);
    panelOpacity->setOpacity(1.0);

    connect(stripButton, &QPushButton::clicked, this, [this, page, collapsedStrip, panelContent, panelOpacity]() {
        collapsedStrip->hide();
        panelContent->show();
        page->setMinimumWidth(kAiCollapsedWidth);
        page->setMaximumWidth(kAiCollapsedWidth);

        auto* group = new QParallelAnimationGroup(page);
        auto* minAnim = new QPropertyAnimation(page, "minimumWidth", group);
        auto* maxAnim = new QPropertyAnimation(page, "maximumWidth", group);
        auto* opacityAnim = new QPropertyAnimation(panelOpacity, "opacity", group);

        minAnim->setDuration(220);
        maxAnim->setDuration(220);
        opacityAnim->setDuration(180);
        minAnim->setStartValue(kAiCollapsedWidth);
        minAnim->setEndValue(kAiSidebarWidth);
        maxAnim->setStartValue(kAiCollapsedWidth);
        maxAnim->setEndValue(kAiSidebarWidth);
        opacityAnim->setStartValue(0.0);
        opacityAnim->setEndValue(1.0);
        minAnim->setEasingCurve(QEasingCurve::InOutQuad);
        maxAnim->setEasingCurve(QEasingCurve::InOutQuad);
        opacityAnim->setEasingCurve(QEasingCurve::OutCubic);

        group->addAnimation(minAnim);
        group->addAnimation(maxAnim);
        group->addAnimation(opacityAnim);
        connect(group, &QParallelAnimationGroup::finished, page, [page, panelOpacity, group]() {
            page->setMinimumWidth(kAiSidebarWidth);
            page->setMaximumWidth(kAiSidebarWidth);
            panelOpacity->setOpacity(1.0);
            group->deleteLater();
        });
        group->start();
    });
    connect(collapseButton, &QPushButton::clicked, this, [this, page, collapsedStrip, panelContent, panelOpacity]() {
        page->setMinimumWidth(kAiSidebarWidth);
        page->setMaximumWidth(kAiSidebarWidth);

        auto* group = new QParallelAnimationGroup(page);
        auto* minAnim = new QPropertyAnimation(page, "minimumWidth", group);
        auto* maxAnim = new QPropertyAnimation(page, "maximumWidth", group);
        auto* opacityAnim = new QPropertyAnimation(panelOpacity, "opacity", group);

        minAnim->setDuration(220);
        maxAnim->setDuration(220);
        opacityAnim->setDuration(140);
        minAnim->setStartValue(kAiSidebarWidth);
        minAnim->setEndValue(kAiCollapsedWidth);
        maxAnim->setStartValue(kAiSidebarWidth);
        maxAnim->setEndValue(kAiCollapsedWidth);
        opacityAnim->setStartValue(1.0);
        opacityAnim->setEndValue(0.0);
        minAnim->setEasingCurve(QEasingCurve::InOutQuad);
        maxAnim->setEasingCurve(QEasingCurve::InOutQuad);
        opacityAnim->setEasingCurve(QEasingCurve::InCubic);

        group->addAnimation(minAnim);
        group->addAnimation(maxAnim);
        group->addAnimation(opacityAnim);
        connect(group, &QParallelAnimationGroup::finished, page, [page, collapsedStrip, panelContent, panelOpacity, group]() {
            panelContent->hide();
            collapsedStrip->show();
            page->setMinimumWidth(kAiCollapsedWidth);
            page->setMaximumWidth(kAiCollapsedWidth);
            panelOpacity->setOpacity(1.0);
            group->deleteLater();
        });
        group->start();
    });

    // Start expanded so AI assistant remains available after rollback.
    collapsedStrip->hide();
    panelContent->show();
    page->setFixedWidth(kAiSidebarWidth);

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
    QMenu* fileMenu = menuBar()->addMenu("鏂囦欢(&F)");
    QAction* openBrowserAction = fileMenu->addAction("鎵撳紑缃戦〉棰勮(&O)");
    openBrowserAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openBrowserAction, &QAction::triggered, this, &MainWindow::onOpenBrowser);

    QAction* refreshAction = fileMenu->addAction("鍒锋柊褰撳墠椤甸潰(&R)");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshCurrentPage);

    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("閫€鍑?&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onQuitTriggered);

    QMenu* helpMenu = menuBar()->addMenu("甯姪(&H)");
    QAction* aboutAction = helpMenu->addAction("鍏充簬(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutTriggered);
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar("宸ュ叿鏍?);
    m_toolBar->setMovable(false);

    QAction* openAction = m_toolBar->addAction("缃戦〉棰勮");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenBrowser);

    m_refreshAction = m_toolBar->addAction("鍒锋柊");
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshCurrentPage);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("姝ｅ湪鍚姩鍚庣鏈嶅姟...", this);
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
        Logger::warning("绯荤粺鎵樼洏涓嶅彲鐢?);
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("瀛︿笟鍙戝睍瑙勫垝绯荤粺");

    m_trayMenu = new QMenu(this);
    m_openBrowserAction = m_trayMenu->addAction("鎵撳紑缃戦〉棰勮");
    connect(m_openBrowserAction, &QAction::triggered, this, &MainWindow::onOpenBrowser);
    m_trayMenu->addSeparator();
    m_quitAction = m_trayMenu->addAction("閫€鍑?);
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
            Logger::info("鎵惧埌鍓嶇闈欐€佹枃浠? " + m_frontendPath);
            return;
        }
    }

    m_frontendPath.clear();
    Logger::warning("鏈壘鍒?frontend_dist锛岀綉椤甸瑙堝皢渚濊禆鍚庣鎵樼鎴栨湰鍦板紑鍙戞湇鍔°€?);
}

void MainWindow::startBackendServer()
{
    m_statusLabel->setText("姝ｅ湪鍚姩鍚庣鏈嶅姟...");
    updateBackendBadge(false, "鍚姩涓?);

    m_serverThread = new HttpServerThread(this);
    connect(m_serverThread, &HttpServerThread::serverStarted, this, &MainWindow::onBackendStarted);
    connect(m_serverThread, &HttpServerThread::serverError, this, &MainWindow::onBackendError);
    m_serverThread->start();
}

void MainWindow::openBrowser()
{
    const QString url = m_serverReady ? m_serverUrl : "file:///" + m_frontendPath + "/index.html";
        Logger::info("鎵撳紑缃戦〉棰勮: " + url);
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
        setupEmptyState(m_recommendationList, "鏆傛棤寤鸿");
    }

    m_semesterList->clear();
    const QJsonArray semesters = CourseService::getSemesterStatistics();
    for (const auto& semesterValue : semesters) {
        const QJsonObject semester = semesterValue.toObject();
        m_semesterList->addItem(
            QString("%1  璺?GPA %2 璺?骞冲潎鍒?%3")
                .arg(semester["semester"].toString())
                .arg(QString::number(semester["gpa"].toDouble(), 'f', 2))
                .arg(QString::number(semester["avgScore"].toDouble(), 'f', 1)));
    }
    if (m_semesterList->count() == 0) {
        setupEmptyState(m_semesterList, "鏃犲鏈熸暟鎹?);
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
        auto* emptyItem = new QTableWidgetItem("\n\n鏆傛棤璇剧▼鏁版嵁\n鐐瑰嚮[鏂板璇剧▼]娣诲姞绗竴闂ㄨ绋媆n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(12); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_courseTable->setItem(0, 0, emptyItem);
    }

    m_courseSummaryLabel->setText(
        QString("褰撳墠鏄剧ず %1 / %2 闂ㄨ绋嬶紝鍏朵腑宸插畬鎴?%3 闂ㄣ€傛敮鎸佹悳绱€佺姸鎬佺瓫閫夊拰鎺掑簭锛屽苟浼氬悓姝ュ奖鍝嶆€昏銆佹椂闂磋酱涓庣畝鍘嗗鍑恒€?)
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
        const QString summary = QString("%1\n%2 璺?%3\n%4")
            .arg(safeText(role.title))
            .arg(joinDateRange(role.startDate, role.endDate, role.isActive, "鑷充粖"))
            .arg(safeText(role.organization, safeText(role.type)))
            .arg(shortBody(role.description, role.isActive ? "褰撳墠瑙掕壊浠嶅湪杩涜涓€? : "璇ヨ鑹查樁娈靛凡瀹屾垚銆?));
        m_roleList->addItem(summary);
        m_roleList->item(m_roleList->count() - 1)->setData(Qt::UserRole, role.id);
    }
    if (m_roleList->count() == 0) {
        setupEmptyState(m_roleList, "鏆傛棤瑙掕壊鑱岃矗鏁版嵁");
    } else {
        m_roleList->setCurrentRow(0);
    }

    const QString dominantType = typeBreakdown.isEmpty() ? "鏈垎绫? : typeBreakdown.keys().first();
    m_roleSummaryLabel->setText(
        QString("鏄剧ず %1 / %2 涓鑹诧紝鍏朵腑杩涜涓?%3 涓€備富瑕佺被鍨嬶細%4銆傛敮鎸佹悳绱㈠拰绫诲瀷绛涢€夈€?)
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
            : QString("%1 璺?%2").arg(safeText(achievement.date, "鏃ユ湡鏈～鍐?), achievement.level);
        const QString detail = shortBody(
            achievement.description,
            achievement.organization.trimmed().isEmpty()
                ? "宸茶褰曚竴椤规柊鐨勬垚鏋溿€?
                : QString("褰掑睘鏈烘瀯锛?1").arg(achievement.organization));
        m_achievementList->addItem(QString("%1\n%2\n%3")
                                       .arg(safeText(achievement.title))
                                       .arg(meta)
                                       .arg(detail));
        m_achievementList->item(m_achievementList->count() - 1)->setData(Qt::UserRole, achievement.id);
    }
    if (m_achievementList->count() == 0) {
        setupEmptyState(m_achievementList, "鏆傛棤鎴愭灉璁板綍鏁版嵁");
    } else {
        m_achievementList->setCurrentRow(0);
    }

    const QString mainLevel = levelBreakdown.isEmpty() ? "鏈垎绾? : levelBreakdown.keys().first();
    m_achievementSummaryLabel->setText(
        QString("鏄剧ず %1 / %2 椤规垚鏋滐紝宸查獙璇?%3 椤广€備富瑕佺骇鍒細%4銆傛敮鎸佹悳绱€佺被鍨嬪拰绾у埆绛涢€夈€?)
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
        const QString roleText = experience.role.trimmed().isEmpty() ? QString() : QString(" 璺?%1").arg(experience.role);
        const QString body = shortBody(
            experience.description,
            experience.isOngoing ? "褰撳墠缁忓巻浠嶅湪杩涜涓€? : "璇ョ粡鍘嗛樁娈靛凡瀹屾垚銆?);
        m_experienceList->addItem(
            QString("%1\n%2 璺?%3%4\n%5")
                .arg(safeText(experience.title))
                .arg(joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, "鑷充粖"))
                .arg(org)
                .arg(roleText)
                .arg(body));
        m_experienceList->item(m_experienceList->count() - 1)->setData(Qt::UserRole, experience.id);
    }
    if (m_experienceList->count() == 0) {
        setupEmptyState(m_experienceList, "鏆傛棤缁忓巻妗ｆ鏁版嵁");
    } else {
        m_experienceList->setCurrentRow(0);
    }

    m_experienceSummaryLabel->setText(
        QString("鏄剧ず %1 / %2 娈电粡鍘嗭紝鍏朵腑杩涜涓?%3 娈点€傛敮鎸佹悳绱㈠拰绫诲瀷绛涢€夈€?)
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
        const QString progress = QString("%1% 璺?%2")
            .arg(goal.progress(), 0, 'f', 1)
            .arg(safeText(goal.status));
        const QString deadline = safeText(goal.deadline, "鎴鏃堕棿鏈～鍐?);
        const QString body = shortBody(
            goal.description,
            QString("鐩爣鍊?%1 %2锛屽綋鍓嶅€?%3 %4銆?)
                .arg(goal.targetValue, 0, 'f', 1)
                .arg(safeText(goal.unit, ""))
                .arg(goal.currentValue, 0, 'f', 1)
                .arg(safeText(goal.unit, "")));
        m_goalList->addItem(
            QString("%1\n%2 璺?鎴 %3\n%4")
                .arg(safeText(goal.title))
                .arg(progress)
                .arg(deadline)
                .arg(body));
        m_goalList->item(m_goalList->count() - 1)->setData(Qt::UserRole, goal.id);
    }
    if (m_goalList->count() == 0) {
        setupEmptyState(m_goalList, "鏆傛棤鐩爣杩借釜鏁版嵁");
    } else {
        m_goalList->setCurrentRow(0);
    }

    m_goalSummaryLabel->setText(
        QString("褰撳墠鏄剧ず %1 / %2 涓洰鏍囷紝宸插畬鎴?%3 涓紝骞冲潎杩涘害 %4%銆傛敮鎸佹悳绱€佺瓫閫夊拰鎺掑簭銆?)
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
            QString("%1\n%2 璺?%3\n%4")
                .arg(safeText(event["title"].toString()))
                .arg(safeText(event["date"].toString(), "鏃ユ湡鏈～鍐?))
                .arg(safeText(event["subtitle"].toString(), event["type"].toString()))
                .arg(shortBody(event["description"].toString(), "宸茶褰曟柊鐨勬垚闀胯妭鐐广€?)));
    }
    if (m_timelineList->count() == 0) {
        setupEmptyState(m_timelineList, "鏆傛棤鏃堕棿杞翠簨浠?);
    }

    m_timelineSuggestionList->clear();
    for (const auto& item : strengths) {
        m_timelineSuggestionList->addItem(QString("浼樺娍锛?1").arg(item.toString()));
    }
    for (const auto& item : risks) {
        m_timelineSuggestionList->addItem(QString("椋庨櫓锛?1").arg(item.toString()));
    }
    for (const auto& item : suggestions) {
        m_timelineSuggestionList->addItem(QString("寤鸿锛?1").arg(item.toString()));
    }
    if (m_timelineSuggestionList->count() == 0) {
        setupEmptyState(m_timelineSuggestionList, "鏆傛棤闃舵寤鸿");
    }

    m_timelineSummaryLabel->setText(
        QString("鏃堕棿杞翠粠璇剧▼銆佽鑹层€佹垚鏋溿€佺粡鍘嗗拰鐩爣涓彁鍙栦簨浠讹紝骞堕厤濂楃敓鎴愰樁娈垫€у垎鏋愩€?));
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
            const QString detail = QString("%1 路 %2 瀛﹀垎 路 %3")
                .arg(safeText(course.semester, "瀛︽湡寰呰ˉ鍏?))
                .arg(QString::number(course.credits, 'f', 1))
                .arg(safeText(course.status, "鐘舵€佸緟琛ュ厖"));
            const QString snippet = QString("璇剧▼浜偣锛?1锛?2锛夛紝瀛﹀垎 %3锛屽綋鍓嶇姸鎬佷负 %4銆?5")
                .arg(title)
                .arg(safeText(course.semester, "瀛︽湡寰呰ˉ鍏?))
                .arg(QString::number(course.credits, 'f', 1))
                .arg(safeText(course.status, "鐘舵€佸緟琛ュ厖"))
                .arg(shortBody(course.description, "鍙獊鍑鸿绋嬪涔犳垚鏋溿€佹柟娉曚笌鑳藉姏鎻愬崌銆?));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "experience") {
        const QList<Experience> experiences = ExperienceService::getAll();
        for (const Experience& experience : experiences) {
            const QString title = safeText(experience.title);
            const QString detail = QString("%1 路 %2")
                .arg(safeText(experience.organization, "缁勭粐寰呰ˉ鍏?))
                .arg(joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, "鑷充粖"));
            const QString snippet = QString("瀹炶返缁忓巻锛氬湪 %1 鍙備笌 %2锛屾媴浠?%3銆?4")
                .arg(safeText(experience.organization, "鐩稿叧缁勭粐"))
                .arg(title)
                .arg(safeText(experience.role, "鏍稿績鎴愬憳"))
                .arg(shortBody(experience.description, "鍙獊鍑洪」鐩亴璐ｃ€佹柟娉曚笌缁撴灉銆?));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "achievement") {
        const QList<Achievement> achievements = AchievementService::getAll();
        for (const Achievement& achievement : achievements) {
            const QString title = safeText(achievement.title);
            const QString detail = QString("%1 路 %2")
                .arg(safeText(achievement.level, "绾у埆寰呰ˉ鍏?))
                .arg(safeText(achievement.date, "鏃ユ湡寰呰ˉ鍏?));
            const QString snippet = QString("鎴愭灉璁板綍锛氳幏寰?%1锛?2锛?3锛夈€?4")
                .arg(title)
                .arg(safeText(achievement.level, "绾у埆寰呰ˉ鍏?))
                .arg(safeText(achievement.organization, "缁勭粐寰呰ˉ鍏?))
                .arg(shortBody(achievement.description, "鍙獊鍑烘垚鏋滀环鍊笺€佽础鐚拰褰卞搷銆?));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "role") {
        const QList<Role> roles = RoleService::getAll();
        for (const Role& role : roles) {
            const QString title = safeText(role.title);
            const QString detail = QString("%1 路 %2")
                .arg(safeText(role.organization, "缁勭粐寰呰ˉ鍏?))
                .arg(joinDateRange(role.startDate, role.endDate, role.isActive, "鑷充粖"));
            const QString snippet = QString("瑙掕壊鑱岃矗锛氬湪 %1 鎷呬换 %2銆?3")
                .arg(safeText(role.organization, "鐩稿叧缁勭粐"))
                .arg(title)
                .arg(shortBody(role.description, "鍙己璋冪粍缁囧崗璋冦€佹矡閫氭帹杩涗笌鎵ц鎴愭灉銆?));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "activity") {
        const QList<Activity> activities = ActivityService::getAll();
        for (const Activity& activity : activities) {
            const QString title = safeText(activity.name);
            const QString detail = QString("%1 路 %2")
                .arg(safeText(activity.category, "绫诲埆寰呰ˉ鍏?))
                .arg(joinDateRange(activity.startDate, activity.endDate, activity.isActive, "鑷充粖"));
            const QString snippet = QString("璇惧娲诲姩锛氬弬涓?%1锛?2锛夈€?3")
                .arg(title)
                .arg(safeText(activity.category, "绫诲埆寰呰ˉ鍏?))
                .arg(shortBody(activity.description, "鍙綋鐜伴暱鏈熸姇鍏ャ€佸崗浣滄柟寮忎笌鍏蜂綋璐＄尞銆?));
            appendCandidate(title, detail, snippet);
        }
    }

    if (m_resumeCandidateList->count() == 0) {
        setupEmptyState(m_resumeCandidateList, "褰撳墠鍒嗙被涓嬭繕娌℃湁鍙彃鍏ョ殑绱犳潗銆?);
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
        return QString("<div class='resume-actions'><a href='copy:%1'>澶嶅埗</a><a href='delete:%1'>鍒犻櫎</a></div>").arg(key);
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
    html += "<div class='avatar'>鐓?br/>鐗?/div>";
    html += "<div class='header-main'>";
    html += QString("<div class='name'>%1</div>").arg(safeText(options["name"].toString(), "涓汉鎴愰暱瑙勫垝绠€鍘?));
    html += QString("<div class='meta'><span class='meta-item'>馃搷 %1</span><span class='meta-item'>馃巶 %2</span><span class='meta-item'>馃摓 %3</span><span class='meta-item'>鉁夛笍 %4</span></div>")
        .arg(safeText(options["city"].toString(), "鍩庡競寰呰ˉ鍏?))
        .arg(safeText(options["age"].toString(), "骞撮緞寰呰ˉ鍏?))
        .arg(safeText(options["phone"].toString(), "鐢佃瘽寰呰ˉ鍏?))
        .arg(safeText(options["email"].toString(), "閭寰呰ˉ鍏?));
    html += QString("<div class='intent-row'><span class='intent-tag'>馃捈 %1</span><span class='intent-tag'>馃幆 %2</span></div>")
        .arg(safeText(options["title"].toString(), "鑱屼綅澶磋"))
        .arg(safeText(options["intent"].toString(), "姹傝亴鏂瑰悜"));
    html += "</div></div>";

    if (isVisibleSection("intent")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("intent"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:intent'>%1</a></div>").arg(intentTitle);
        html += "<div class='section-content'>";
        html += actionHtml("intent");
        html += QString("<div>%1</div>").arg(safeText(options["summary"].toString(), "璇疯ˉ鍏呬綘鐨勬眰鑱屾剰鍚戝拰涓汉绠€浠嬨€?));
        html += "</div></div>";
    }

    if (isVisibleSection("education")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("education"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:education'>%1</a></div>").arg(eduTitle);
        html += "<div class='section-content'>";
        html += actionHtml("education");
        html += QString("<div class='section-title-main'>%1</div>").arg(safeText(options["school"].toString(), "瀛︽牎寰呰ˉ鍏?));
        html += QString("<div class='section-sub'>%1 路 %2</div>").arg(safeText(options["major"].toString(), "涓撲笟寰呰ˉ鍏?)).arg(safeText(options["degree"].toString(), "瀛﹀巻寰呰ˉ鍏?));
        html += QString("<div>%1</div>").arg(safeText(options["educationBody"].toString(), "璇疯ˉ鍏呮暀鑲茶儗鏅弿杩般€?).replace("\n", "<br/>"));
        html += "</div></div>";
    }

    if (isVisibleSection("skills")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("skills"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:skills'>%1</a></div>").arg(skillsTitle);
        html += "<div class='section-content'>";
        html += actionHtml("skills");
        html += QString("<div>%1</div>").arg(safeText(options["skillsBody"].toString(), "璇疯ˉ鍏呮妧鑳界壒闀裤€?).replace("\n", "<br/>"));
        html += "</div></div>";
    }

    if (isVisibleSection("project")) {
        html += QString("<div class='resume-section%1'>").arg(selectedClass("project"));
        html += QString("<div class='section-label'><a class='section-anchor' href='section:project'>%1</a></div>").arg(projectTitle);
        html += "<div class='section-content'>";
        html += actionHtml("project");
        html += QString("<div class='section-title-main'>%1</div>").arg(safeText(options["projectName"].toString(), "椤圭洰鍚嶇О寰呰ˉ鍏?));
        html += QString("<div>%1</div>").arg(safeText(options["projectBody"].toString(), "璇疯ˉ鍏呴」鐩粡楠屻€?).replace("\n", "<br/>"));
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
        m_resumeIdentityValue->setText(safeText(options["title"].toString(), "涓汉鎴愰暱瑙勫垝绠€鍘?));
    }
    refreshResumeEditorPanel();
}

void MainWindow::refreshResumeEditorPanel()
{
    if (!m_resumeEditorTitleLabel || !m_resumeSectionVisibleCheck) {
        return;
    }

    bool visible = true;
    QString title = "鏁欒偛鑳屾櫙";
    if (m_resumeSelectedSection == "intent") {
        title = "姹傝亴鎰忓悜";
        visible = m_resumeRoleCheck ? m_resumeRoleCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "education") {
        title = "鏁欒偛鑳屾櫙";
        visible = m_resumeEducationCheck ? m_resumeEducationCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "skills") {
        title = "鎶€鑳界壒闀?;
        visible = m_resumeAchievementCheck ? m_resumeAchievementCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "project") {
        title = "椤圭洰缁忛獙";
        visible = m_resumeExperienceCheck ? m_resumeExperienceCheck->isChecked() : true;
    } else if (m_resumeSelectedSection == "custom") {
        title = "鑷垜璇勪环";
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

void MainWindow::refreshAiStatus()
{
    const QJsonObject status = AiService::checkStatus();
    const QString mode = safeText(status["mode"].toString(), "rule");
    const QString model = safeText(status["model"].toString(), "local-rule-based");
    const bool available = status["available"].toBool();
    m_aiModeValue->setText(QString("妯″紡锛?1").arg(mode));
    m_aiModelValue->setText(QString("妯″瀷锛?1").arg(model));
    m_aiStatusValue->setText(available ? "鐘舵€侊細鍙敤" : "鐘舵€侊細涓嶅彲鐢?);
}

void MainWindow::updateBackendBadge(bool ready, const QString& detail)
{
    const QString state = ready ? "杩愯涓? : "鏈氨缁?;
    const QString extra = detail.isEmpty() ? QString() : QString(" 璺?%1").arg(detail);
    if (m_statusLabel) {
        m_statusLabel->setText(QString("鍚庣鐘舵€侊細%1%2").arg(state, extra));
    }
}

void MainWindow::refreshSidebarCards()
{
    const QDateTime now = QDateTime::currentDateTime();
    const int month = now.date().month();
    int year = now.date().year();
    QString semester;
    if (month >= 2 && month <= 7) {
        semester = QString("%1 鏄ュ瀛︽湡").arg(year);
    } else if (month == 8) {
        semester = QString("%1 澶忓瀛︽湡").arg(year);
    } else {
        if (month == 1) {
            year -= 1;
        }
        semester = QString("%1 绉嬪瀛︽湡").arg(year);
    }

    const QString weekday = QStringList({"鍛ㄦ棩","鍛ㄤ竴","鍛ㄤ簩","鍛ㄤ笁","鍛ㄥ洓","鍛ㄤ簲","鍛ㄥ叚"}).at(now.date().dayOfWeek() % 7);
    const QString detail = QString("%1 璺?%2")
        .arg(now.date().toString("yyyy-MM-dd"))
        .arg(now.time().toString("HH:mm"));

    if (m_timeSemesterLabel) {
        m_timeSemesterLabel->setText(semester);
    }
    if (m_timeDetailLabel) {
        m_timeDetailLabel->setText(QString("%1 路 %2").arg(now.date().toString("yyyy-MM-dd")).arg(now.time().toString("HH:mm")));
    }
    QSettings settings;
    QString profileName = settings.value("profile/name", "璇风偣鍑昏缃鍚?).toString();
    if (m_studentNameLabel) {
        m_studentNameLabel->setText(profileName);
    }
    if (m_studentMetaLabel) {
        QString sid = settings.value("profile/studentId", "鏈～鍐欏鍙?).toString();
        QString dept = settings.value("profile/department", "鏈～鍐欓櫌绯?).toString();
        m_studentMetaLabel->setText(QString("%1 路 %2").arg(sid, dept));
    }
    Q_UNUSED(detail);
}

void MainWindow::addRole()
{
    RoleEditorDialog dialog(this);
    dialog.setWindowTitle("鏂板瑙掕壊");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Role role = dialog.role();
    const Role created = RoleService::create(role);
    if (created.id == 0) {
        ToastNotification::display(this, "瑙掕壊鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshRoles();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "瑙掕壊宸插垱寤恒€?);
}

void MainWindow::editSelectedRole()
{
    if (!m_roleList || !m_roleList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€涓鑹层€?);
        return;
    }

    const int roleId = m_roleList->currentItem()->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳界紪杈戙€?);
        return;
    }

    Role role = RoleService::getById(roleId);
    if (role.id == 0) {
        ToastNotification::display(this, "鏈壘鍒板搴旇鑹茶褰曘€?);
        return;
    }

    RoleEditorDialog dialog(this);
    dialog.setWindowTitle("缂栬緫瑙掕壊");
    dialog.setRole(role);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Role updated = dialog.role();
    const Role saved = RoleService::update(roleId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "瑙掕壊鏇存柊澶辫触銆?);
        return;
    }

    refreshRoles();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "瑙掕壊宸叉洿鏂般€?);
}

void MainWindow::removeSelectedRole()
{
    if (!m_roleList || !m_roleList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€涓鑹层€?);
        return;
    }

    const int roleId = m_roleList->currentItem()->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳藉垹闄ゃ€?);
        return;
    }

    const QString title = m_roleList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(this, "鍒犻櫎瑙掕壊",
        QString("纭畾瑕佸垹闄よ鑹测€?1鈥濆悧锛熸鎿嶄綔浼氬悓姝ュ奖鍝嶆椂闂磋酱鍜岀畝鍘嗐€?).arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!RoleService::remove(roleId)) {
        ToastNotification::display(this, "瑙掕壊鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshRoles();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "瑙掕壊宸插垹闄ゃ€?);
}

void MainWindow::addAchievement()
{
    AchievementEditorDialog dialog(this);
    dialog.setWindowTitle("鏂板鎴愭灉");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Achievement achievement = dialog.achievement();
    const Achievement created = AchievementService::create(achievement);
    if (created.id == 0) {
        ToastNotification::display(this, "鎴愭灉鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshAchievements();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "鎴愭灉宸插垱寤恒€?);
}

void MainWindow::editSelectedAchievement()
{
    if (!m_achievementList || !m_achievementList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€鏉℃垚鏋溿€?);
        return;
    }

    const int achievementId = m_achievementList->currentItem()->data(Qt::UserRole).toInt();
    if (achievementId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳界紪杈戙€?);
        return;
    }

    Achievement achievement = AchievementService::getById(achievementId);
    if (achievement.id == 0) {
        ToastNotification::display(this, "鏈壘鍒板搴旀垚鏋滆褰曘€?);
        return;
    }

    AchievementEditorDialog dialog(this);
    dialog.setWindowTitle("缂栬緫鎴愭灉");
    dialog.setAchievement(achievement);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Achievement updated = dialog.achievement();
    const Achievement saved = AchievementService::update(achievementId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "鎴愭灉鏇存柊澶辫触銆?);
        return;
    }

    refreshAchievements();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "鎴愭灉宸叉洿鏂般€?);
}

void MainWindow::removeSelectedAchievement()
{
    if (!m_achievementList || !m_achievementList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€鏉℃垚鏋溿€?);
        return;
    }

    const int achievementId = m_achievementList->currentItem()->data(Qt::UserRole).toInt();
    if (achievementId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳藉垹闄ゃ€?);
        return;
    }

    const QString title = m_achievementList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(this, "鍒犻櫎鎴愭灉",
        QString("纭畾瑕佸垹闄ゆ垚鏋溾€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆€昏銆佹椂闂磋酱鍜岀畝鍘嗐€?).arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!AchievementService::remove(achievementId)) {
        ToastNotification::display(this, "鎴愭灉鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshAchievements();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "鎴愭灉宸插垹闄ゃ€?);
}

void MainWindow::addExperience()
{
    ExperienceEditorDialog dialog(this);
    dialog.setWindowTitle("鏂板缁忓巻");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience experience = dialog.experience();
    const Experience created = ExperienceService::create(experience);
    if (created.id == 0) {
        ToastNotification::display(this, "缁忓巻鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "缁忓巻宸插垱寤恒€?);
}

void MainWindow::editSelectedExperience()
{
    if (!m_experienceList || !m_experienceList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€娈电粡鍘嗐€?);
        return;
    }

    const int experienceId = m_experienceList->currentItem()->data(Qt::UserRole).toInt();
    if (experienceId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳界紪杈戙€?);
        return;
    }

    Experience experience = ExperienceService::getById(experienceId);
    if (experience.id == 0) {
        ToastNotification::display(this, "鏈壘鍒板搴旂粡鍘嗚褰曘€?);
        return;
    }

    ExperienceEditorDialog dialog(this);
    dialog.setWindowTitle("缂栬緫缁忓巻");
    dialog.setExperience(experience);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience updated = dialog.experience();
    const Experience saved = ExperienceService::update(experienceId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "缁忓巻鏇存柊澶辫触銆?);
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "缁忓巻宸叉洿鏂般€?);
}

void MainWindow::removeSelectedExperience()
{
    if (!m_experienceList || !m_experienceList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€娈电粡鍘嗐€?);
        return;
    }

    const int experienceId = m_experienceList->currentItem()->data(Qt::UserRole).toInt();
    if (experienceId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳藉垹闄ゃ€?);
        return;
    }

    const QString title = m_experienceList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(this, "鍒犻櫎缁忓巻",
        QString("纭畾瑕佸垹闄ょ粡鍘嗏€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆椂闂磋酱銆佺畝鍘嗗拰 AI 鍒嗘瀽銆?).arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!ExperienceService::remove(experienceId)) {
        ToastNotification::display(this, "缁忓巻鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "缁忓巻宸插垹闄ゃ€?);
}

void MainWindow::addCourse()
{
    CourseEditorDialog dialog(this);
    dialog.setWindowTitle("鏂板璇剧▼");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Course course = dialog.course();
    const Course created = CourseService::create(course);
    if (created.id == 0) {
        ToastNotification::display(this, "璇剧▼鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "璇剧▼宸插垱寤恒€?);
}

void MainWindow::editSelectedCourse()
{
    if (!m_courseTable || m_courseTable->currentRow() < 0) {
        ToastNotification::display(this, "璇峰厛鍦ㄨ绋嬭〃涓€夋嫨涓€闂ㄨ绋嬨€?);
        return;
    }

    const QTableWidgetItem* idItem = m_courseTable->item(m_courseTable->currentRow(), 0);
    if (!idItem) {
        ToastNotification::display(this, "褰撳墠閫変腑琛屾病鏈夋湁鏁堣绋嬫暟鎹€?);
        return;
    }

    const int courseId = idItem->data(Qt::UserRole).toInt();
    Course course = CourseService::getById(courseId);
    if (course.id == 0) {
        ToastNotification::display(this, "鏈壘鍒板搴旇绋嬭褰曘€?);
        return;
    }

    CourseEditorDialog dialog(this);
    dialog.setWindowTitle("缂栬緫璇剧▼");
    dialog.setCourse(course);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Course updated = dialog.course();
    const Course saved = CourseService::update(courseId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "璇剧▼鏇存柊澶辫触銆?);
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "璇剧▼宸叉洿鏂般€?);
}

void MainWindow::removeSelectedCourse()
{
    if (!m_courseTable || m_courseTable->currentRow() < 0) {
        ToastNotification::display(this, "璇峰厛鍦ㄨ绋嬭〃涓€夋嫨涓€闂ㄨ绋嬨€?);
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
        "鍒犻櫎璇剧▼",
        QString("纭畾瑕佸垹闄よ绋嬧€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆€昏銆佹椂闂磋酱鍜岀畝鍘嗗鍑恒€?).arg(courseName));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!CourseService::remove(courseId)) {
        ToastNotification::display(this, "璇剧▼鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "璇剧▼宸插垹闄ゃ€?);
}

void MainWindow::addGoal()
{
    GoalEditorDialog dialog(this);
    dialog.setWindowTitle("鏂板鐩爣");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal goal = dialog.goal();
    const Goal created = GoalService::create(goal);
    if (created.id == 0) {
        ToastNotification::display(this, "鐩爣鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshGoals();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "鐩爣宸插垱寤恒€?);
}

void MainWindow::editSelectedGoal()
{
    if (!m_goalList || !m_goalList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€涓洰鏍囥€?);
        return;
    }

    const int goalId = m_goalList->currentItem()->data(Qt::UserRole).toInt();
    if (goalId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳界紪杈戙€?);
        return;
    }

    Goal goal = GoalService::getById(goalId);
    if (goal.id == 0) {
        ToastNotification::display(this, "鏈壘鍒板搴旂洰鏍囪褰曘€?);
        return;
    }

    GoalEditorDialog dialog(this);
    dialog.setWindowTitle("缂栬緫鐩爣");
    dialog.setGoal(goal);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal updated = dialog.goal();
    const Goal saved = GoalService::update(goalId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "鐩爣鏇存柊澶辫触銆?);
        return;
    }

    refreshGoals();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "鐩爣宸叉洿鏂般€?);
}

void MainWindow::removeSelectedGoal()
{
    if (!m_goalList || !m_goalList->currentItem()) {
        ToastNotification::display(this, "璇峰厛閫夋嫨涓€涓洰鏍囥€?);
        return;
    }

    const int goalId = m_goalList->currentItem()->data(Qt::UserRole).toInt();
    if (goalId <= 0) {
        ToastNotification::display(this, "褰撳墠椤规槸鍗犱綅淇℃伅锛屾殏鏃朵笉鑳藉垹闄ゃ€?);
        return;
    }

    const QString title = m_goalList->currentItem()->text().section('\n', 0, 0);
    const auto result = QMessageBox::question(
        this,
        "鍒犻櫎鐩爣",
        QString("纭畾瑕佸垹闄ょ洰鏍団€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆€昏銆佹椂闂磋酱鍜?AI 寤鸿銆?).arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!GoalService::remove(goalId)) {
        ToastNotification::display(this, "鐩爣鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshGoals();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "鐩爣宸插垹闄ゃ€?);
}

QJsonObject MainWindow::currentResumeOptions() const
{
    if (!m_resumeNameInput) {
        return defaultResumeOptions();
    }

    QJsonObject options;
    options["name"] = safeText(m_resumeNameInput->text(), "涓汉鍙戝睍妗ｆ");
    options["title"] = safeText(m_resumeTitleInput->text(), "涓汉鎴愰暱瑙勫垝绠€鍘?);
    options["age"] = safeText(m_resumeAgeInput ? m_resumeAgeInput->text() : QString(), "21宀?);
    options["city"] = safeText(m_resumeCityInput ? m_resumeCityInput->text() : QString(), "鍖椾含");
    options["email"] = m_resumeEmailInput->text().trimmed();
    options["phone"] = m_resumePhoneInput->text().trimmed();
    options["intent"] = safeText(m_resumeIntentInput ? m_resumeIntentInput->text() : QString(), "澶ф暟鎹伐绋嬪笀");
    options["school"] = safeText(m_resumeSchoolInput ? m_resumeSchoolInput->text() : QString(), "瀛︽牎寰呰ˉ鍏?);
    options["major"] = safeText(m_resumeMajorInput ? m_resumeMajorInput->text() : QString(), "涓撲笟寰呰ˉ鍏?);
    options["degree"] = safeText(m_resumeDegreeInput ? m_resumeDegreeInput->text() : QString(), "瀛﹀巻寰呰ˉ鍏?);
    options["summary"] = shortBody(m_resumeSummaryInput->toPlainText(), "鍩轰簬璇剧▼銆佺粡鍘嗐€佹垚鏋滀笌鐩爣鑷姩鐢熸垚鐨勭患鍚堢畝鍘嗛瑙堛€?);
    options["educationBody"] = m_resumeEducationBodyInput ? m_resumeEducationBodyInput->toPlainText().trimmed() : QString();
    options["skillsBody"] = m_resumeSkillsBodyInput ? m_resumeSkillsBodyInput->toPlainText().trimmed() : QString();
    options["projectName"] = safeText(m_resumeProjectNameInput ? m_resumeProjectNameInput->text() : QString(), "椤圭洰鍚嶇О寰呰ˉ鍏?);
    options["projectBody"] = m_resumeProjectBodyInput ? m_resumeProjectBodyInput->toPlainText().trimmed() : QString();
    options["internship"] = m_resumeInternshipInput ? m_resumeInternshipInput->toPlainText().trimmed() : QString();
    options["awards"] = m_resumeAwardsInput ? m_resumeAwardsInput->toPlainText().trimmed() : QString();
    options["customContent"] = m_resumeCustomContentInput ? m_resumeCustomContentInput->toPlainText().trimmed() : QString();
    options["sectionTitleIntent"] = QString("姹傝亴鎰忓悜");
    options["sectionTitleEducation"] = QString("鏁欒偛鑳屾櫙");
    options["sectionTitleSkills"] = QString("鎶€鑳界壒闀?);
    options["sectionTitleProjects"] = QString("椤圭洰缁忛獙");
    options["sectionTitleInternship"] = QString("瀹炰範缁忓巻");
    options["sectionTitleAwards"] = QString("绔炶禌鑾峰");
    options["sectionTitleCustom"] = QString("鑷垜璇勪环");
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
    if (!m_resumeSummaryInput) {
        ToastNotification::display(this, "绠€鍘嗛厤缃尯灏氭湭鍑嗗濂姐€?);
        return;
    }
    const QString suggestion = m_aiOutput ? m_aiOutput->toPlainText().trimmed() : QString();
    if (suggestion.isEmpty()) {
        ToastNotification::display(this, "璇峰厛鐢熸垚涓€娈?AI 寤鸿銆?);
        return;
    }

    m_resumeSummaryInput->setPlainText(suggestion);
    if (m_navList) {
        m_navList->setCurrentRow(10);
    }
    refreshResume();
    ToastNotification::display(this, "宸插皢 AI 寤鸿鍐欏叆绠€鍘嗘憳瑕侊紝浣犲彲浠ョ户缁湪绠€鍘嗛〉寰皟銆?);
}

void MainWindow::createGoalFromAiSuggestion()
{
    const QString suggestion = m_aiOutput ? m_aiOutput->toPlainText().trimmed() : QString();
    if (suggestion.isEmpty()) {
        ToastNotification::display(this, "璇峰厛鐢熸垚涓€娈?AI 寤鸿銆?);
        return;
    }

    GoalEditorDialog dialog(this);
    dialog.setWindowTitle("浠?AI 寤鸿鐢熸垚鐩爣");
    Goal draft;
    draft.title = "AI 寤鸿璺熻繘鐩爣";
    draft.category = "General";
    draft.description = suggestion;
    draft.targetValue = 1;
    draft.currentValue = 0;
    draft.unit = "椤?;
    draft.priority = "High";
    draft.status = "In Progress";
    dialog.setGoal(draft);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Goal goal = dialog.goal();
    const Goal created = GoalService::create(goal);
    if (created.id == 0) {
        ToastNotification::display(this, "鏈兘鏍规嵁 AI 寤鸿鍒涘缓鐩爣銆?);
        return;
    }

    if (m_navList) {
        m_navList->setCurrentRow(6);
    }
    refreshGoals();
    refreshOverview();
    refreshTimeline();
    ToastNotification::display(this, "宸叉牴鎹?AI 寤鸿鐢熸垚鐩爣鑽夌銆?);
}

void MainWindow::exportResumeJson()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "瀵煎嚭 JSON 绠€鍘?, QDir::homePath() + "/resume.json", "JSON Files (*.json)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ToastNotification::display(this, "鏃犳硶鍐欏叆 JSON 鏂囦欢銆?);
        return;
    }
    file.write(ResumeService::exportJson(currentResumeOptions()));
    file.close();
    ToastNotification::display(this, "JSON 绠€鍘嗗凡瀵煎嚭銆?);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::exportResumeHtml()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "瀵煎嚭 HTML 绠€鍘?, QDir::homePath() + "/resume.html", "HTML Files (*.html)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ToastNotification::display(this, "鏃犳硶鍐欏叆 HTML 鏂囦欢銆?);
        return;
    }
    file.write(ResumeService::exportHtml(currentResumeOptions()));
    file.close();
    ToastNotification::display(this, "HTML 绠€鍘嗗凡瀵煎嚭銆?);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::exportResumePdf()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "瀵煎嚭 PDF 绠€鍘?, QDir::homePath() + "/resume.pdf", "PDF Files (*.pdf)");
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

    ToastNotification::display(this, "PDF 绠€鍘嗗凡瀵煎嚭銆?);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::runAiAnalysis(const QString& type)
{
    m_aiOutput->setPlainText("姝ｅ湪鍒嗘瀽涓紝璇风◢鍊?..");
    QApplication::processEvents();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QJsonObject payload;
    payload["type"] = type;
    const QJsonObject result = AiService::analyze(payload);
    QApplication::restoreOverrideCursor();

    QStringList lines;
    lines << QString("鍒嗘瀽绫诲瀷锛?1").arg(type);
    lines << QString("AI 妯″紡锛?1").arg(result["aiPowered"].toBool() ? "妯″瀷鍒嗘瀽" : "瑙勫垯寮曟搸");

    const QJsonArray suggestions = result["suggestions"].toArray();
    if (!suggestions.isEmpty()) {
        lines << "";
        lines << "寤鸿锛?;
        for (const auto& item : suggestions) {
            lines << QString("- %1").arg(item.toString());
        }
    } else if (result.contains("reply")) {
        lines << "";
        lines << result["reply"].toString();
    } else {
        lines << "";
        lines << "褰撳墠娌℃湁杩斿洖寤鸿鍐呭銆?;
    }

    m_aiOutput->setPlainText(lines.join('\n'));
    refreshAiStatus();
}

void MainWindow::sendAiChat()
{
    QString message = m_aiChatInput->text().trimmed();
    if (message.isEmpty()) {
        ToastNotification::display(this, "璇峰厛杈撳叆涓€涓棶棰樸€?);
        return;
    }

    // Prepend selected context if available
    if (!m_selectedContext.isEmpty()) {
        message = QString("銆愬叧浜庝互涓嬪唴瀹广€慭n%1\n\n%2").arg(m_selectedContext, message);
        m_selectedContext.clear();
        if (m_aiContextLabel) m_aiContextLabel->hide();
    }

    m_aiOutput->setPlainText("AI 姝ｅ湪鎬濊€冧腑...");
    QApplication::processEvents();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QJsonObject payload;
    payload["message"] = message;
    const QJsonObject result = AiService::chat(payload);
    QApplication::restoreOverrideCursor();

    QString output = QString("闂锛?1\n\n绛斿锛歕n%2")
        .arg(message)
        .arg(result["reply"].toString());
    m_aiOutput->setPlainText(output);
    m_aiChatInput->clear();
    refreshAiStatus();
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
        QString timeRange = a.endDate.isEmpty() ? (a.startDate + (a.isActive ? "鑷充粖" : "")) : (a.startDate + " - " + a.endDate);
        QString txt = QString("%1 %2\n%3\n%4")
            .arg(a.isFavorite ? "鈽? : "").arg(a.name)
            .arg(a.category + " | " + timeRange)
            .arg(a.description);
        QListWidgetItem* item = new QListWidgetItem(txt, m_activityList);
        item->setData(Qt::UserRole, a.id);
    }
    if (m_activityList->count() == 0) {
        setupEmptyState(m_activityList, "鏆傛棤璇惧娲诲姩璁板綍");
    }
    m_activitySummaryLabel->setText(QString("鏄剧ず %1 / %2 椤规椿鍔ㄨ褰?).arg(filtered.size()).arg(list.size()));
}

void MainWindow::addActivity() {
    ActivityEditorDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        Activity act = dlg.activity();
        ActivityService::create(act);
        refreshActivities();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "娲诲姩宸插垱寤恒€?);
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
        ToastNotification::display(this, "娲诲姩宸叉洿鏂般€?);
    }
}

void MainWindow::removeSelectedActivity() {
    if (!m_activityList->currentItem()) return;
    int id = m_activityList->currentItem()->data(Qt::UserRole).toInt();
    if (id > 0 && QMessageBox::question(this, "鍒犻櫎娲诲姩", "纭畾瑕佸垹闄よ娲诲姩璁板綍鍚楋紵姝ゆ搷浣滀細鍚屾褰卞搷鎬昏鍜屾椂闂磋酱銆?) == QMessageBox::Yes) {
        ActivityService::remove(id);
        refreshActivities();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "娲诲姩宸插垹闄ゃ€?);
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
        QString txt = QString("%1\n%2 - %3\n浼樺厛绾? %4")
            .arg(j.title)
            .arg(j.company, j.location)
            .arg(j.priority);
        QListWidgetItem* item = new QListWidgetItem(txt, m_jobList);
        item->setData(Qt::UserRole, j.id);
    }
    if (m_jobList->count() == 0) {
        setupEmptyState(m_jobList, "鏆傛棤鐩爣宀椾綅鏁版嵁");
    }
    m_jobSummaryLabel->setText(QString("鏄剧ず %1 / %2 椤圭洰鏍囧矖浣?).arg(filtered.size()).arg(list.size()));
    if(m_jobRequirementList) m_jobRequirementList->clear();
    if(m_jobRequirementSummaryLabel) m_jobRequirementSummaryLabel->setText("璇峰湪宸︿晶閫夋嫨宀椾綅");
}

void MainWindow::addJob() {
    JobEditorDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        Job jb = dlg.job();
        JobService::create(jb);
        refreshJobs();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "宀椾綅宸插垱寤恒€?);
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
        ToastNotification::display(this, "宀椾綅宸叉洿鏂般€?);
    }
}

void MainWindow::removeSelectedJob() {
    if (!m_jobList->currentItem()) return;
    int id = m_jobList->currentItem()->data(Qt::UserRole).toInt();
    if (id > 0 && QMessageBox::question(this, "鍒犻櫎宀椾綅", "纭畾瑕佸垹闄よ鐩爣宀椾綅鍚楋紵姝ゆ搷浣滀細鍚屾褰卞搷鎬昏鍜屾椂闂磋酱銆?) == QMessageBox::Yes) {
        JobService::remove(id);
        refreshJobs();
        refreshOverview();
        refreshTimeline();
        ToastNotification::display(this, "宀椾綅宸插垹闄ゃ€?);
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
        m_jobRequirementSummaryLabel->setText(QString("姝ゅ矖浣嶅叡鏈?%1 椤硅姹傦紝宸插尮閰?%2 椤广€?).arg(j.requirements.size()).arg(metCount));
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
        auto* emptyItem = new QTableWidgetItem("\n鏆傛棤瀛︽湡瀵规瘮鏁版嵁\n娣诲姞璇剧▼鍚庤嚜鍔ㄧ敓鎴怽n");
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
        auto* emptyItem = new QTableWidgetItem("\n鏆傛棤鍚屽瀵圭収鏁版嵁\n鐐瑰嚮[鏂板瀵圭収]娣诲姞鍚屽淇℃伅\n");
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
    
    m_analysisSummaryLabel->setText("鎶ュ憡鐢熸垚鎴愬姛锛屽凡璇勪及鍚勭被缁村害鐨勫涔犳垚鏋滆〃鐜颁笌宸窛銆?);
}

void MainWindow::addPeer() {
    PeerEditorDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        PeerBenchmark pb = dlg.peer();
        PeerBenchmarkService::create(pb);
        refreshAnalysis();
        ToastNotification::display(this, "瀵圭収鍚屽宸叉坊鍔犮€?);
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
        ToastNotification::display(this, "瀵圭収鍚屽淇℃伅宸叉洿鏂般€?);
    }
}

void MainWindow::removeSelectedPeer() {
    if(!m_analysisPeerTable || m_analysisPeerTable->currentRow() < 0) return;
    int id = m_analysisPeerTable->item(m_analysisPeerTable->currentRow(), 0)->data(Qt::UserRole).toInt();
    if(id > 0 && QMessageBox::question(this, "鍒犻櫎瀵圭収鍚屽", "纭畾瑕佸垹闄よ繖鍚嶅鐓у悓瀛﹁褰曞悧锛熷垹闄ゅ悗灏嗘棤娉曟仮澶嶃€?) == QMessageBox::Yes) {
        PeerBenchmarkService::remove(id);
        refreshAnalysis();
        ToastNotification::display(this, "瀵圭収鍚屽宸插垹闄ゃ€?);
    }
}

void MainWindow::chooseImportFile() {
    QString path = QFileDialog::getOpenFileName(this, "閫夋嫨鏁版嵁鏂囦欢", QDir::homePath(), "CSV 鏂囦欢 (*.csv)");
    if (!path.isEmpty()) {
        m_importFilePath = path;
        m_importFileLabel->setText(path);
    }
}

void MainWindow::runImport() {
    if (m_importFilePath.isEmpty()) { 
        ToastNotification::display(this, "璇峰厛閫夋嫨鏁版嵁婧愭枃浠讹紒");
        return; 
    }
    QFile file(m_importFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        ToastNotification::display(this, "鏃犳硶璇诲彇鏂囦欢锛岃妫€鏌ユ枃浠舵潈闄愩€?);
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    
    QString entity = m_importEntityCombo->currentData().toString();
    QJsonObject result = ImportService::importData(entity, data, m_importFilePath);
    
    if (result.contains("error") && result["error"].toBool()) {
        ToastNotification::display(this, "鉂?瀵煎叆澶辫触: " + result["message"].toString());
        return;
    }
    
    m_importResultImportedValue->setText(QString::number(result["imported"].toInt()));
    m_importResultFailedValue->setText(QString::number(result["failed"].toInt()));
    
    QJsonArray errors = result["errors"].toArray();
    m_importErrorTable->setRowCount(errors.size());
    for(int i = 0; i < errors.size(); ++i) {
        QJsonObject e = errors[i].toObject();
        m_importErrorTable->setItem(i, 0, new QTableWidgetItem(QString("琛?%1").arg(e["row"].toInt())));
        m_importErrorTable->setItem(i, 1, new QTableWidgetItem(e["error"].toString()));
    }
    
    m_importSummaryLabel->setText(QString("鏂囦欢澶勭悊瀹屾垚锛氭垚鍔熷鍏?%1 鏉★紝骞跺埛鏂颁簡鍚勭郴缁熺紦瀛樸€?).arg(result["imported"].toInt()));
    
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
    
    ToastNotification::display(this, "鍏卞鍏ヤ簡 " + QString::number(result["imported"].toInt()) + " 鏉℃暟鎹紝宸茶Е鍙戝叏绯荤粺鏁版嵁鍒锋柊銆?);
}


void MainWindow::onBackendStarted()
{
    m_serverReady = true;
    m_statusLabel->setText("鍚庣鏈嶅姟杩愯涓紝鍘熺敓椤甸潰宸插彲鐩存帴璇诲彇鏁版嵁銆?);
        updateBackendBadge(true, "绔彛 8080");
    m_progressBar->hide();

    if (m_trayIcon) {
        m_trayIcon->showMessage("瀛︿笟鍙戝睍瑙勫垝绯荤粺", "C++ 鍚庣宸插惎鍔紝鍙互浣跨敤鍘熺敓鐣岄潰鎴栫綉椤甸瑙堛€?);
    }

    refreshCurrentPage();
}

void MainWindow::onBackendError(const QString& error)
{
    m_serverReady = false;
    m_statusLabel->setText("鍚庣鍚姩澶辫触: " + error);
    updateBackendBadge(false, error);
    m_progressBar->hide();
    QMessageBox::critical(this, "鍚庣閿欒", "鍚庣鏈嶅姟鍚姩澶辫触锛歕n" + error);
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
        refreshOverview();
        break;
    case 1:
        refreshCourses();
        break;
    case 2:
        refreshRoles();
        break;
    case 3:
        refreshAchievements();
        break;
    case 4:
        refreshExperiences();
        break;
    case 5:
        refreshActivities();
        break;
    case 6:
        refreshGoals();
        break;
    case 7:
        refreshJobs();
        break;
    case 8:
        refreshAnalysis();
        break;
    case 9:
        refreshTimeline();
        break;
    case 10:
        refreshResume();
        break;
    case 11:
        break;
    default:
        break;
    }
    refreshAiStatus();
    refreshSidebarCards();
    if (m_aiOutput && m_aiOutput->toPlainText().isEmpty()) {
        m_aiOutput->setPlainText("鐐瑰嚮鈥滅患鍚堝垎鏋?/ 璇剧▼寤鸿 / 缁忓巻寤鸿 / 鐩爣寤鸿鈥濓紝鎴栫洿鎺ュ湪涓嬫柟杈撳叆闂銆?);
    }
}

void MainWindow::onOpenBrowser()
{
    if (!m_serverReady && m_frontendPath.isEmpty()) {
        ToastNotification::display(this, "褰撳墠娌℃湁鍙敤鐨勭綉椤甸瑙堣祫婧愩€?);
        return;
    }
    openBrowser();
}

void MainWindow::onAboutTriggered()
{
    QMessageBox::about(
        this,
        "鍏充簬",
        QString("瀛︿笟鍙戝睍瑙勫垝绯荤粺 - Qt 妗岄潰鐗?v%1\n\n"
                "褰撳墠闃舵锛歕n"
                "1. C++ 鍚庣宸茬嫭绔嬭繍琛孿n"
                "2. 涔濅釜鏍稿績椤甸潰閮藉凡鏈夊師鐢?Qt 鐣岄潰\n"
                "3. 褰撳墠缁х画娌跨敤鍓嶇鐨勫崱鐗囧紡鐭ヨ瘑搴撹璁￠€昏緫\n"
                "4. 鍘熺綉椤靛墠绔繚鐣欎负杈呭姪棰勮涓庡鐓х増鏈?)
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
        this, "纭閫€鍑?, "纭畾瑕侀€€鍑哄涓氬彂灞曡鍒掔郴缁熷悧锛?, QMessageBox::Yes | QMessageBox::No);

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
                m_selectedContext = selectedText;
                if (m_aiContextLabel) {
                    m_aiContextLabel->setText(QString("閫変腑鐨勫唴瀹癸細%1").arg(selectedText.left(150)));
                    m_aiContextLabel->show();
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
    Logger::info("鍚庣鏈嶅姟绾跨▼鍚姩");

    HttpServer server;
    if (!server.start(8080)) {
                emit serverError("鏃犳硶缁戝畾绔彛 8080");
        return;
    }

    emit serverStarted();

    while (m_running) {
        msleep(100);
    }

    server.stop();
    Logger::info("鍚庣鏈嶅姟绾跨▼閫€鍑?);
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
    QStringList categories = {"蹇呬慨", "閫変慨", "閫氳瘑", "瀹炶返"};
    QStringList statuses = {"Completed", "In Progress", "Planned"};
    
    for (int i = 1; i <= 20; ++i) {
        Course c;
        c.name = QString("璇剧▼%1").arg(i);
        c.code = QString("CS%1").arg(1000 + i);
        c.credits = 2.0 + (i % 4);
        c.semester = semesters[i % 4];
        c.category = categories[i % 4];
        c.score = 70 + (i % 30);
        c.status = statuses[i % 3];
        c.teacher = QString("鏁欏笀%1").arg(i);
        c.location = QString("鏁欏妤?1鏁欏").arg(i % 5 + 1);
        c.description = QString("杩欐槸璇剧▼%1鐨勬弿杩颁俊鎭紝鍖呭惈璇剧▼鐨勪富瑕佸唴瀹瑰拰鏁欏鐩爣銆?).arg(i);
        c.tags = "涓撲笟璇剧▼,鏍稿績蹇呬慨";
        CourseService::create(c);
    }

    QStringList roleTypes = {"瀛︾敓骞查儴", "绀惧洟璐熻矗浜?, "蹇楁効鑰?, "鍔╂暀"};
    for (int i = 1; i <= 20; ++i) {
        Role r;
        r.title = QString("瑙掕壊%1").arg(i);
        r.type = roleTypes[i % 4];
        r.organization = QString("缁勭粐%1").arg(i);
        r.description = QString("杩欐槸瑙掕壊%1鐨勮缁嗘弿杩帮紝鍖呮嫭涓昏鑱岃矗鍜屽伐浣滃唴瀹广€?).arg(i);
        r.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        r.endDate = i % 3 == 0 ? "" : QString("2025-%1-01").arg(i % 12 + 1);
        r.isActive = (i % 3 != 0);
        r.achievements = QString("鎴愬氨1,鎴愬氨2,鎴愬氨3");
        r.contact = QString("contact%1@example.com").arg(i);
        r.supervisor = QString("鎸囧鑰佸笀%1").arg(i);
        RoleService::create(r);
    }

    QStringList achTypes = {"绔炶禌", "璇佷功", "椤圭洰", "璁烘枃"};
    QStringList levels = {"鍥藉绾?, "鐪佺骇", "鏍＄骇", "闄㈢骇"};
    for (int i = 1; i <= 20; ++i) {
        Achievement a;
        a.title = QString("鎴愭灉%1").arg(i);
        a.type = achTypes[i % 4];
        a.level = levels[i % 4];
        a.organization = QString("棰佸彂鏈烘瀯%1").arg(i);
        a.description = QString("杩欐槸鎴愭灉%1鐨勮缁嗘弿杩帮紝鍖呮嫭鑾峰鍘熷洜鍜屼富瑕佸唴瀹广€?).arg(i);
        a.date = QString("2024-%1-15").arg(i % 12 + 1);
        a.certificate = QString("璇佷功缂栧彿%1").arg(10000 + i);
        a.relatedCourse = QString("璇剧▼%1").arg(i % 10 + 1);
        a.teamMembers = QString("鎴愬憳1,鎴愬憳2,鎴愬憳3");
        a.ranking = QString("绗?1鍚?).arg(i % 10 + 1);
        a.prize = i % 3 == 0 ? "涓€绛夊" : (i % 3 == 1 ? "浜岀瓑濂? : "涓夌瓑濂?);
        a.verified = (i % 2 == 0);
        AchievementService::create(a);
    }

    QStringList expTypes = {"瀹炰範", "椤圭洰", "鐮旂┒", "绔炶禌"};
    for (int i = 1; i <= 20; ++i) {
        Experience e;
        e.title = QString("缁忓巻%1").arg(i);
        e.type = expTypes[i % 4];
        e.organization = QString("鍗曚綅%1").arg(i);
        e.role = QString("瑙掕壊%1").arg(i);
        e.description = QString("杩欐槸缁忓巻%1鐨勮缁嗘弿杩帮紝鍖呮嫭涓昏宸ヤ綔鍐呭鍜屾垚鏋溿€?).arg(i);
        e.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        e.endDate = i % 3 == 0 ? "" : QString("2025-%1-01").arg(i % 12 + 1);
        e.isOngoing = (i % 3 == 0);
        e.technologies = QString("鎶€鏈?,鎶€鏈?,鎶€鏈?");
        e.achievements = QString("鎴愭灉1,鎴愭灉2,鎴愭灉3");
        e.supervisor = QString("瀵煎笀%1").arg(i);
        e.contact = QString("contact%1@example.com").arg(i);
        e.location = QString("鍦扮偣%1").arg(i);
        e.url = QString("https://example.com/exp%1").arg(i);
        ExperienceService::create(e);
    }

    QStringList actCategories = {"瀛︽湳", "浣撹偛", "鑹烘湳", "绀句氦", "蹇楁効"};
    for (int i = 1; i <= 20; ++i) {
        Activity a;
        a.name = QString("娲诲姩%1").arg(i);
        a.description = QString("杩欐槸娲诲姩%1鐨勮缁嗘弿杩帮紝鍖呮嫭娲诲姩鍐呭鍜屽弬涓庢柟寮忋€?).arg(i);
        a.category = actCategories[i % 5];
        a.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        a.endDate = QString("2024-%1-15").arg(i % 12 + 1);
        a.isFavorite = (i % 5 == 0);
        a.isActive = (i % 3 != 0);
        a.tags = QString("鏍囩1,鏍囩2,鏍囩3");
        ActivityService::create(a);
    }

    QStringList goalCategories = {"瀛︿笟", "鎶€鑳?, "鍋ュ悍", "绀句氦", "鑱屼笟"};
    QStringList priorities = {"High", "Medium", "Low"};
    QStringList goalStatuses = {"In Progress", "Completed", "Pending"};
    for (int i = 1; i <= 20; ++i) {
        Goal g;
        g.title = QString("鐩爣%1").arg(i);
        g.category = goalCategories[i % 5];
        g.description = QString("杩欐槸鐩爣%1鐨勮缁嗘弿杩帮紝鍖呮嫭鐩爣鍐呭鍜屽疄鐜拌矾寰勩€?).arg(i);
        g.targetValue = 100;
        g.currentValue = i * 5;
        g.unit = "%";
        g.deadline = QString("2025-%1-01").arg(i % 12 + 1);
        g.priority = priorities[i % 3];
        g.status = goalStatuses[i % 3];
        g.milestones = QString("閲岀▼纰?,閲岀▼纰?,閲岀▼纰?");
        GoalService::create(g);
    }

    QStringList jobStatuses = {"鏀惰棌", "宸叉姇閫?, "闈㈣瘯涓?, "宸叉嫆缁?, "宸插綍鐢?};
    for (int i = 1; i <= 20; ++i) {
        Job j;
        j.title = QString("鑱屼綅%1").arg(i);
        j.company = QString("鍏徃%1").arg(i);
        j.location = QString("鍩庡競%1").arg(i);
        j.salaryRange = QString("%1-%2K").arg(10 + i).arg(20 + i);
        j.description = QString("杩欐槸鑱屼綅%1鐨勮缁嗘弿杩帮紝鍖呮嫭宀椾綅鑱岃矗鍜屼换鑱岃姹傘€?).arg(i);
        JobRequirement req1, req2;
        req1.text = QString("瑕佹眰1锛氱啛缁冩帉鎻℃妧鑳?1").arg(i);
        req1.met = (i % 2 == 0);
        req2.text = QString("瑕佹眰2锛氬叿澶囩浉鍏崇粡楠?1骞?).arg(i % 5);
        req2.met = (i % 3 == 0);
        j.requirements = {req1, req2};
        j.isActive = true;
        j.priority = i % 5;
        j.source = "鎷涜仒缃戠珯";
        j.url = QString("https://example.com/job%1").arg(i);
        j.status = jobStatuses[i % 5];
        j.appliedDate = QString("2024-%1-01").arg(i % 12 + 1);
        JobService::create(j);
    }

    settings.setValue("sampleDataInserted", true);
    Logger::info("宸叉彃鍏ヨ櫄鎷熸暟鎹?);
}

