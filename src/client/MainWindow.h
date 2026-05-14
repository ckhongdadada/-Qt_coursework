#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QProgressBar>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QToolBar>
#include <QWidget>

class SidebarWidget;
class AiPanelWidget;

class CoursesPage;
class RolesPage;
class AchievementsPage;
class ExperiencesPage;
class ActivitiesPage;
class GoalsPage;
class JobsPage;
class AnalysisPage;
class TimelinePage;
class ResumePage;
class OverviewPage;
class ImportsPage;

class AppShellController;
class DataRefreshCoordinator;
class BackendRuntimeController;
class AiContextMediator;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void startBackendServer();
    void openBrowser();

private slots:
    void onBackendReady();
    void onBackendError(const QString& error);
    void onOpenBrowser();
    void onAboutTriggered();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onQuitTriggered();
    void onNavigationChanged(int row);
    void refreshCurrentPage();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupSystemTray();
    void applyWindowStyle();

    QProgressBar* m_progressBar = nullptr;
    QLabel* m_statusLabel = nullptr;
    QToolBar* m_toolBar = nullptr;

    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_trayMenu = nullptr;
    QAction* m_openBrowserAction = nullptr;
    QAction* m_quitAction = nullptr;
    QAction* m_refreshAction = nullptr;

    SidebarWidget* m_sidebar = nullptr;
    AiPanelWidget* m_aiPanel = nullptr;

    OverviewPage* m_overviewPage = nullptr;
    CoursesPage* m_coursesPage = nullptr;
    RolesPage* m_rolesPage = nullptr;
    AchievementsPage* m_achievementsPage = nullptr;
    ExperiencesPage* m_experiencesPage = nullptr;
    ActivitiesPage* m_activitiesPage = nullptr;
    GoalsPage* m_goalsPage = nullptr;
    JobsPage* m_jobsPage = nullptr;
    AnalysisPage* m_analysisPage = nullptr;
    TimelinePage* m_timelinePage = nullptr;
    ResumePage* m_resumePage = nullptr;
    ImportsPage* m_importsPage = nullptr;

    QStackedWidget* m_stack = nullptr;
    QWidget* m_mainInner = nullptr;
    QSpacerItem* m_leftStretchSpacer = nullptr;
    QSpacerItem* m_rightStretchSpacer = nullptr;
    QLabel* m_topbarKicker = nullptr;
    QLabel* m_topbarTitle = nullptr;
    QLabel* m_topbarPill = nullptr;

    AppShellController* m_shellController = nullptr;
    DataRefreshCoordinator* m_refreshCoordinator = nullptr;
    BackendRuntimeController* m_backendController = nullptr;
    AiContextMediator* m_aiMediator = nullptr;
};

#endif // MAINWINDOW_H
