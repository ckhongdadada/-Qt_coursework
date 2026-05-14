#include "ResumeCandidatePanel.h"
#include "service/CourseService.h"
#include "service/ExperienceService.h"
#include "service/AchievementService.h"
#include "service/RoleService.h"
#include "service/ActivityService.h"
#include "utils/UiHelpers.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ResumeCandidatePanel::ResumeCandidatePanel(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("contentCard");
    setMinimumWidth(240);
    setMaximumWidth(320);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setupUi();
}

void ResumeCandidatePanel::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    QLabel* candidateTitle = new QLabel("备选素材", this);
    candidateTitle->setObjectName("sectionTitle");
    layout->addWidget(candidateTitle);

    QLabel* candidateHint = new QLabel("从已有课程、经历、成果和活动中挑选内容，点击即可插入到补充内容区，再按你的需要微调。", this);
    candidateHint->setObjectName("pageSubtitle");
    candidateHint->setWordWrap(true);
    layout->addWidget(candidateHint);

    m_candidateTypeCombo = new QComboBox(this);
    m_candidateTypeCombo->addItem("课程亮点", "course");
    m_candidateTypeCombo->addItem("实践经历", "experience");
    m_candidateTypeCombo->addItem("成果记录", "achievement");
    m_candidateTypeCombo->addItem("角色职责", "role");
    m_candidateTypeCombo->addItem("课外活动", "activity");
    layout->addWidget(m_candidateTypeCombo);

    m_candidateList = new QListWidget(this);
    m_candidateList->setObjectName("plainList");
    m_candidateList->setWordWrap(true);
    m_candidateList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_candidateList, 1);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(8);
    m_insertButton = new QPushButton("插入到教育背景", this);
    m_insertButton->setStyleSheet("QPushButton { background: transparent; border: 1px solid #d8deea; border-radius: 8px; padding: 6px 12px; color: #42526b; } QPushButton:hover { background: #f4f8ff; }");
    actionLayout->addWidget(m_insertButton);
    layout->addLayout(actionLayout);

    QPushButton* clearCustomContentButton = new QPushButton("清空补充内容", this);
    layout->addWidget(clearCustomContentButton);
    layout->addStretch(0);

    connect(m_candidateTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        updateInsertButton();
        refreshCandidates();
    });
    connect(m_candidateList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            emit candidateClicked(item->data(Qt::UserRole).toString().trimmed());
        }
    });
    connect(m_insertButton, &QPushButton::clicked, this, [this]() {
        const QString type = m_candidateTypeCombo->currentData().toString();
        QString section;
        if (type == "course") section = "education";
        else if (type == "experience") section = "project";
        else if (type == "achievement") section = "awards";
        else section = "custom";
        emit insertToSectionRequested(section);
    });
    connect(clearCustomContentButton, &QPushButton::clicked, this, &ResumeCandidatePanel::clearCustomContentRequested);
    
    updateInsertButton();
}

void ResumeCandidatePanel::refreshCandidates()
{
    if (!m_candidateList || !m_candidateTypeCombo) {
        return;
    }

    const QString type = m_candidateTypeCombo->currentData().toString();
    m_candidateList->clear();

    auto appendCandidate = [this](const QString& title, const QString& detail, const QString& snippet) {
        auto* item = new QListWidgetItem(QString("%1\n%2").arg(title, detail), m_candidateList);
        item->setData(Qt::UserRole, snippet);
        item->setToolTip(snippet);
        item->setSizeHint(QSize(0, 54));
    };

    if (type == "course") {
        const QList<Course> courses = CourseService::getAll();
        for (const Course& course : courses) {
            const QString title = UiHelpers::safeText(course.name);
            const QString detail = QString("%1 · %2 学分 · %3")
                .arg(UiHelpers::safeText(course.semester, "学期待补充"))
                .arg(QString::number(course.credits, 'f', 1))
                .arg(UiHelpers::safeText(course.status, "状态待补充"));
            const QString snippet = QString("课程亮点：%1（%2），学分 %3，当前状态为 %4。%5")
                .arg(title)
                .arg(UiHelpers::safeText(course.semester, "学期待补充"))
                .arg(QString::number(course.credits, 'f', 1))
                .arg(UiHelpers::safeText(course.status, "状态待补充"))
                .arg(UiHelpers::shortBody(course.description, "可突出课程学习成果、方法与能力提升。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "experience") {
        const QList<Experience> experiences = ExperienceService::getAll();
        for (const Experience& experience : experiences) {
            const QString title = UiHelpers::safeText(experience.title);
            const QString detail = QString("%1 · %2")
                .arg(UiHelpers::safeText(experience.organization, "组织待补充"))
                .arg(UiHelpers::joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, "至今"));
            const QString snippet = QString("实践经历：在 %1 参与 %2，担任 %3。%4")
                .arg(UiHelpers::safeText(experience.organization, "相关组织"))
                .arg(title)
                .arg(UiHelpers::safeText(experience.role, "核心成员"))
                .arg(UiHelpers::shortBody(experience.description, "可突出项目职责、方法与结果。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "achievement") {
        const QList<Achievement> achievements = AchievementService::getAll();
        for (const Achievement& achievement : achievements) {
            const QString title = UiHelpers::safeText(achievement.title);
            const QString detail = QString("%1 · %2")
                .arg(UiHelpers::safeText(achievement.level, "级别待补充"))
                .arg(UiHelpers::safeText(achievement.date, "日期待补充"));
            const QString snippet = QString("成果记录：获得 %1（%2，%3）。%4")
                .arg(title)
                .arg(UiHelpers::safeText(achievement.level, "级别待补充"))
                .arg(UiHelpers::safeText(achievement.organization, "组织待补充"))
                .arg(UiHelpers::shortBody(achievement.description, "可突出成果价值、贡献和影响。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "role") {
        const QList<Role> roles = RoleService::getAll();
        for (const Role& role : roles) {
            const QString title = UiHelpers::safeText(role.title);
            const QString detail = QString("%1 · %2")
                .arg(UiHelpers::safeText(role.organization, "组织待补充"))
                .arg(UiHelpers::joinDateRange(role.startDate, role.endDate, role.isActive, "至今"));
            const QString snippet = QString("角色职责：在 %1 担任 %2。%3")
                .arg(UiHelpers::safeText(role.organization, "相关组织"))
                .arg(title)
                .arg(UiHelpers::shortBody(role.description, "可强调组织协调、沟通推进与执行成果。"));
            appendCandidate(title, detail, snippet);
        }
    } else if (type == "activity") {
        const QList<Activity> activities = ActivityService::getAll();
        for (const Activity& activity : activities) {
            const QString title = UiHelpers::safeText(activity.name);
            const QString detail = QString("%1 · %2")
                .arg(UiHelpers::safeText(activity.category, "类别待补充"))
                .arg(UiHelpers::joinDateRange(activity.startDate, activity.endDate, activity.isActive, "至今"));
            const QString snippet = QString("课外活动：参与 %1（%2）。%3")
                .arg(title)
                .arg(UiHelpers::safeText(activity.category, "类别待补充"))
                .arg(UiHelpers::shortBody(activity.description, "可体现长期投入、协作方式与具体贡献。"));
            appendCandidate(title, detail, snippet);
        }
    }

    if (m_candidateList->count() == 0) {
        UiHelpers::setupEmptyState(m_candidateList, "当前分类下还没有可插入的素材。");
    } else {
        m_candidateList->setCurrentRow(0);
    }
}

QString ResumeCandidatePanel::currentSnippet() const
{
    if (!m_candidateList) return {};
    QListWidgetItem* item = m_candidateList->currentItem();
    return item ? item->data(Qt::UserRole).toString().trimmed() : QString();
}

QString ResumeCandidatePanel::currentCandidateType() const
{
    return m_candidateTypeCombo ? m_candidateTypeCombo->currentData().toString() : QString();
}

void ResumeCandidatePanel::updateInsertButton()
{
    if (!m_insertButton || !m_candidateTypeCombo) return;
    
    const QString type = m_candidateTypeCombo->currentData().toString();
    QString buttonText;
    
    if (type == "course") {
        buttonText = "插入到教育背景";
    } else if (type == "experience") {
        buttonText = "插入到项目经验";
    } else if (type == "achievement") {
        buttonText = "插入到竞赛获奖";
    } else if (type == "role") {
        buttonText = "插入到补充内容";
    } else if (type == "activity") {
        buttonText = "插入到补充内容";
    } else {
        buttonText = "插入到简历";
    }
    
    m_insertButton->setText(buttonText);
}
