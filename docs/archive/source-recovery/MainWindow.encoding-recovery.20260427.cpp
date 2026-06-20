#include "MainWindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
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
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
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
#include <QScrollArea>

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
#include "util/Logger.h"

namespace {

constexpr int kSidebarExpandedWidth = 260;
constexpr int kSidebarCollapsedWidth = 56;
constexpr int kAiSidebarWidth = 360;
constexpr int kAiCollapsedWidth = 50;
constexpr int kMaxContentWidth = 1200;

QString safeText(const QString& value, const QString& fallback = "鏈～鍐?)
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
    return QString("鈥?%1").arg(value);
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
    return "鏃堕棿鏈～鍐?;
}

QJsonObject defaultResumeOptions()
{
    QSettings settings;
    QJsonObject options;
    options["name"] = settings.value("profile/name", "涓汉鍙戝睍妗ｆ").toString();
    options["title"] = settings.value("profile/title", "涓汉鎴愰暱瑙勫垝绠€鍘?).toString();
    options["email"] = settings.value("profile/email", "").toString();
    options["phone"] = settings.value("profile/phone", "").toString();
    options["summary"] = settings.value("profile/summary", "鍩轰簬课程銆佺粡鍘嗐€佹垚鏋滀笌鐩爣鑷姩鐢熸垚鐨勭患鍚堢畝鍘嗛瑙堛€?).toString();
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

class ToastNotification : public QWidget {
public:
    static void display(QWidget* parent, const QString& message) {
        if (!parent) return;
        auto* toast = new ToastNotification(parent, message);
        toast->show();
        toast->raise();
    }
private:
    ToastNotification(QWidget* parent, const QString& message) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        QLabel* label = new QLabel(message, this);
        label->setStyleSheet("background: rgba(43, 92, 93, 0.95); color: white; padding: 12px 24px; border-radius: 8px; font-size: 14px; font-weight: bold; border: 1px solid #1a3c3c;");
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        layout->addWidget(label);
        adjustSize();
        move((parent->width() - width()) / 2, 40);
        
        QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(eff);
        QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity");
        anim->setDuration(250);
        anim->setStartValue(0);
        anim->setEndValue(1);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
        
        QTimer::singleShot(2500, this, [this, eff]() {
            QPropertyAnimation* fadeOut = new QPropertyAnimation(eff, "opacity");
            fadeOut->setDuration(400);
            fadeOut->setStartValue(1);
            fadeOut->setEndValue(0);
            connect(fadeOut, &QPropertyAnimation::finished, this, &QObject::deleteLater);
            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
};

static void setupEmptyState(QListWidget* list, const QString& hint = "杩欓噷绌虹┖濡備篃锛屽揩鍘绘坊鍔犵涓€绗旇褰曞惂~") {
    auto* item = new QListWidgetItem("\n\n\n馃摥\n" + hint + "\n\n\n");
    item->setTextAlignment(Qt::AlignCenter);
    QFont f = item->font(); f.setPointSize(12); item->setFont(f);
    item->setForeground(QBrush(QColor("#a8a096")));
    item->setFlags(Qt::NoItemFlags);
    list->addItem(item);
}

class ProfileEditorDialog : public QDialog {
public:
    explicit ProfileEditorDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Edit Profile");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        resize(340, 360);
        QFrame* container = new QFrame(this);
        container->setObjectName("profilePopup");
        container->setStyleSheet(
            "#profilePopup {"
            "  background: #ffffff; border: 1px solid rgba(67,57,43,0.12); border-radius: 12px;"
            "  padding: 16px;"
            "}"
        );
        QVBoxLayout* outer = new QVBoxLayout(this);
        outer->setContentsMargins(8, 8, 8, 8);
        outer->addWidget(container);

        QVBoxLayout* form = new QVBoxLayout(container);
        form->setContentsMargins(16, 14, 16, 10);
        form->setSpacing(9);

        QLabel* headerLabel = new QLabel("Personal Info", container);
        QFont hf = headerLabel->font(); hf.setPointSize(13); hf.setBold(true); headerLabel->setFont(hf);
        headerLabel->setStyleSheet("color: #24211d; padding-bottom: 6px;");
        form->addWidget(headerLabel);

        QGridLayout* grid = new QGridLayout();
        grid->setHorizontalSpacing(10);
        grid->setVerticalSpacing(7);
        m_nameEdit = new QLineEdit(container); m_nameEdit->setPlaceholderText("Name");
        m_sidEdit = new QLineEdit(container); m_sidEdit->setPlaceholderText("Student ID");
        m_deptEdit = new QLineEdit(container); m_deptEdit->setPlaceholderText("Department");
        m_emailEdit = new QLineEdit(container); m_emailEdit->setPlaceholderText("Email");
        m_phoneEdit = new QLineEdit(container); m_phoneEdit->setPlaceholderText("Phone");
        QString inputStyle = "QLineEdit { border: 1px solid #ddd3c6; border-radius: 6px; padding: 7px 10px; font-size: 12px; background: #faf8f4; } QLineEdit:focus { border-color: #2b5c5d; }";
        m_nameEdit->setStyleSheet(inputStyle);
        m_sidEdit->setStyleSheet(inputStyle);
        m_deptEdit->setStyleSheet(inputStyle);
        m_emailEdit->setStyleSheet(inputStyle);
        m_phoneEdit->setStyleSheet(inputStyle);
        grid->addWidget(new QLabel("Name"), 0, 0); grid->addWidget(m_nameEdit, 0, 1);
        grid->addWidget(new QLabel("ID"), 1, 0); grid->addWidget(m_sidEdit, 1, 1);
        grid->addWidget(new QLabel("Dept"), 2, 0); grid->addWidget(m_deptEdit, 2, 1);
        grid->addWidget(new QLabel("Email"), 3, 0); grid->addWidget(m_emailEdit, 3, 1);
        grid->addWidget(new QLabel("Phone"), 4, 0); grid->addWidget(m_phoneEdit, 4, 1);
        for (int i = 0; i < 5; ++i) {
            if (auto* lbl = qobject_cast<QLabel*>(grid->itemAtPosition(i, 0)->widget())) {
                lbl->setStyleSheet("color: #888; font-size: 11px;");
            }
        }
        form->addLayout(grid);

        QHBoxLayout* btnRow = new QHBoxLayout();
        QPushButton* saveBtn = new QPushButton("Save", container);
        QPushButton* cancelBtn = new QPushButton("Cancel", container);
        saveBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setCursor(Qt::PointingHandCursor);
        saveBtn->setStyleSheet(
            "QPushButton { background: #2b5c5d; color: white; border-radius: 6px; padding: 7px 20px; font-size: 12px; font-weight: bold; border: none; }"
            "QPushButton:hover { background: #234a4b; }"
        );
        cancelBtn->setStyleSheet(
            "QPushButton { background: transparent; color: #666; border: 1px solid #ddd3c6; border-radius: 6px; padding: 7px 20px; font-size: 12px; }"
            "QPushButton:hover { background: #f0eee8; }"
        );
        connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        btnRow->addStretch();
        btnRow->addWidget(saveBtn);
        btnRow->addWidget(cancelBtn);
        form->addLayout(btnRow);

        QSettings settings;
        m_nameEdit->setText(settings.value("profile/name", "").toString());
        m_sidEdit->setText(settings.value("profile/studentId", "").toString());
        m_deptEdit->setText(settings.value("profile/department", "").toString());
        m_emailEdit->setText(settings.value("profile/email", "").toString());
        m_phoneEdit->setText(settings.value("profile/phone", "").toString());
    }

    void showNear(QWidget* anchor) {
        QPoint globalPos = anchor->mapToGlobal(QPoint(anchor->width() - width() - 8, -height() - 8));
        move(globalPos);
    }

    void save() {
        QSettings settings;
        settings.setValue("profile/name", m_nameEdit->text().trimmed());
        settings.setValue("profile/studentId", m_sidEdit->text().trimmed());
        settings.setValue("profile/department", m_deptEdit->text().trimmed());
        settings.setValue("profile/email", m_emailEdit->text().trimmed());
        settings.setValue("profile/phone", m_phoneEdit->text().trimmed());
    }
private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_sidEdit;
    QLineEdit* m_deptEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_phoneEdit;
};

class CourseEditorDialog : public QDialog {
public:
    explicit CourseEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("课程缂栬緫");
        resize(520, 640);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("鍦?Qt 鍘熺敓鐣岄潰涓洿鎺ョ淮鎶よ绋嬫暟鎹紝淇濆瓨鍚庝細鍚屾鍒锋柊鎬昏銆佹椂闂磋酱鍜岀畝鍘嗐€?, this);
        intro->setWordWrap(true);
        intro->setObjectName("pageSubtitle");
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_nameEdit = new QLineEdit(this);
        m_codeEdit = new QLineEdit(this);
        m_semesterEdit = new QLineEdit(this);
        m_semesterEdit->setPlaceholderText("渚嬪 2026-鏄ュ");

        m_creditsSpin = new QDoubleSpinBox(this);
        m_creditsSpin->setRange(0.0, 40.0);
        m_creditsSpin->setDecimals(1);
        m_creditsSpin->setSingleStep(0.5);

        m_scoreSpin = new QDoubleSpinBox(this);
        m_scoreSpin->setRange(0.0, 100.0);
        m_scoreSpin->setDecimals(1);
        m_scoreSpin->setSingleStep(1.0);
        m_scoreSpin->setSpecialValueText("鏈～鍐?);

        m_categoryCombo = new QComboBox(this);
        m_categoryCombo->addItems({"Required", "Elective", "General", "Other"});

        m_statusCombo = new QComboBox(this);
        m_statusCombo->addItems({"Planned", "In Progress", "Completed"});

        m_teacherEdit = new QLineEdit(this);
        m_locationEdit = new QLineEdit(this);
        m_tagsEdit = new QLineEdit(this);
        m_tagsEdit->setPlaceholderText("鐢ㄩ€楀彿鍒嗛殧锛屼緥濡傦細鏍稿績璇? 鏁版嵁缁撴瀯");

        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setPlaceholderText("璁板綍课程閲嶇偣銆佷釜浜烘敹鑾锋垨琛ュ厖璇存槑");
        m_descriptionEdit->setMinimumHeight(120);
        m_descriptionEdit->setObjectName("richCardText");

        form->addRow("课程鍚嶇О", m_nameEdit);
        form->addRow("课程浠ｇ爜", m_codeEdit);
        form->addRow("瀛︽湡", m_semesterEdit);
        form->addRow("瀛﹀垎", m_creditsSpin);
        form->addRow("鍒嗘暟", m_scoreSpin);
        form->addRow("课程绫诲埆", m_categoryCombo);
        form->addRow("鐘舵€?, m_statusCombo);
        form->addRow("鎺堣鏁欏笀", m_teacherEdit);
        form->addRow("涓婅鍦扮偣", m_locationEdit);
        form->addRow("鏍囩", m_tagsEdit);
        form->addRow("课程璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setCourse(const Course& course)
    {
        m_courseId = course.id;
        m_nameEdit->setText(course.name);
        m_codeEdit->setText(course.code);
        m_semesterEdit->setText(course.semester);
        m_creditsSpin->setValue(course.credits);
        m_scoreSpin->setValue(course.score > 0 ? course.score : 0.0);
        setComboValue(m_categoryCombo, course.category);
        setComboValue(m_statusCombo, course.status);
        m_teacherEdit->setText(course.teacher);
        m_locationEdit->setText(course.location);
        m_tagsEdit->setText(course.tags);
        m_descriptionEdit->setPlainText(course.description);
    }

    Course course() const
    {
        Course course;
        course.id = m_courseId;
        course.name = m_nameEdit->text().trimmed();
        course.code = m_codeEdit->text().trimmed();
        course.semester = m_semesterEdit->text().trimmed();
        course.credits = m_creditsSpin->value();
        course.score = m_scoreSpin->value();
        course.category = m_categoryCombo->currentText();
        course.status = m_statusCombo->currentText();
        course.teacher = m_teacherEdit->text().trimmed();
        course.location = m_locationEdit->text().trimmed();
        course.tags = m_tagsEdit->text().trimmed();
        course.description = m_descriptionEdit->toPlainText().trimmed();
        return course;
    }

private:
    static void setComboValue(QComboBox* combo, const QString& value)
    {
        const int index = combo->findText(value);
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    }

    void validateAndAccept()
    {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓课程鍚嶇О銆?);
            return;
        }
        if (m_creditsSpin->value() <= 0.0) {
            ToastNotification::display(this, "瀛﹀垎闇€瑕佸ぇ浜?0銆?);
            return;
        }
        if (m_statusCombo->currentText() == "Completed" && m_scoreSpin->value() <= 0.0) {
            const auto result = QMessageBox::question(
                this,
                "缂哄皯鎴愮哗",
                "褰撳墠课程鏍囪涓哄凡瀹屾垚锛屼絾灏氭湭濉啓鍒嗘暟銆傛槸鍚︿粛鐒剁户缁繚瀛橈紵");
            if (result != QMessageBox::Yes) {
                return;
            }
        }
        accept();
    }

    int m_courseId = 0;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_codeEdit = nullptr;
    QLineEdit* m_semesterEdit = nullptr;
    QDoubleSpinBox* m_creditsSpin = nullptr;
    QDoubleSpinBox* m_scoreSpin = nullptr;
    QComboBox* m_categoryCombo = nullptr;
    QComboBox* m_statusCombo = nullptr;
    QLineEdit* m_teacherEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLineEdit* m_tagsEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class GoalEditorDialog : public QDialog {
public:
    explicit GoalEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("鐩爣缂栬緫");
        resize(520, 620);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("杩欓噷缁存姢鐩爣鏍囬銆佽繘搴︿笌閲岀▼纰戯紝淇濆瓨鍚庝細绔嬪嵆褰卞搷鎬昏銆佹椂闂磋酱鍜?AI 建议銆?, this);
        intro->setWordWrap(true);
        intro->setObjectName("pageSubtitle");
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_titleEdit = new QLineEdit(this);
        m_categoryEdit = new QLineEdit(this);
        m_categoryEdit->setPlaceholderText("渚嬪锛氬涔犮€佺珵璧涖€佹眰鑱屻€佹垚闀?);

        m_targetSpin = new QDoubleSpinBox(this);
        m_targetSpin->setRange(0.0, 1000000.0);
        m_targetSpin->setDecimals(1);
        m_targetSpin->setSingleStep(1.0);

        m_currentSpin = new QDoubleSpinBox(this);
        m_currentSpin->setRange(0.0, 1000000.0);
        m_currentSpin->setDecimals(1);
        m_currentSpin->setSingleStep(1.0);

        m_unitEdit = new QLineEdit(this);
        m_deadlineEdit = new QLineEdit(this);
        m_deadlineEdit->setPlaceholderText("渚嬪锛?026-06-30");

        m_priorityCombo = new QComboBox(this);
        m_priorityCombo->addItems({"Low", "Medium", "High", "Critical"});

        m_statusCombo = new QComboBox(this);
        m_statusCombo->addItems({"Not Started", "In Progress", "Completed", "Paused"});

        m_milestonesEdit = new QLineEdit(this);
        m_milestonesEdit->setPlaceholderText("鐢ㄩ€楀彿鍒嗛殧閲岀▼纰?);

        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setMinimumHeight(120);
        m_descriptionEdit->setPlaceholderText("璁板綍鐩爣鑳屾櫙銆佽鍔ㄨ鍒掍笌璇勪及鏂瑰紡");
        m_descriptionEdit->setObjectName("richCardText");

        form->addRow("鐩爣鏍囬", m_titleEdit);
        form->addRow("鍒嗙被", m_categoryEdit);
        form->addRow("鐩爣鍊?, m_targetSpin);
        form->addRow("褰撳墠鍊?, m_currentSpin);
        form->addRow("鍗曚綅", m_unitEdit);
        form->addRow("鎴鏃堕棿", m_deadlineEdit);
        form->addRow("浼樺厛绾?, m_priorityCombo);
        form->addRow("鐘舵€?, m_statusCombo);
        form->addRow("閲岀▼纰?, m_milestonesEdit);
        form->addRow("璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setGoal(const Goal& goal)
    {
        m_goalId = goal.id;
        m_titleEdit->setText(goal.title);
        m_categoryEdit->setText(goal.category);
        m_targetSpin->setValue(goal.targetValue);
        m_currentSpin->setValue(goal.currentValue);
        m_unitEdit->setText(goal.unit);
        m_deadlineEdit->setText(goal.deadline);
        setComboValue(m_priorityCombo, goal.priority);
        setComboValue(m_statusCombo, goal.status);
        m_milestonesEdit->setText(goal.milestones);
        m_descriptionEdit->setPlainText(goal.description);
    }

    Goal goal() const
    {
        Goal goal;
        goal.id = m_goalId;
        goal.title = m_titleEdit->text().trimmed();
        goal.category = m_categoryEdit->text().trimmed();
        goal.targetValue = m_targetSpin->value();
        goal.currentValue = m_currentSpin->value();
        goal.unit = m_unitEdit->text().trimmed();
        goal.deadline = m_deadlineEdit->text().trimmed();
        goal.priority = m_priorityCombo->currentText();
        goal.status = m_statusCombo->currentText();
        goal.milestones = m_milestonesEdit->text().trimmed();
        goal.description = m_descriptionEdit->toPlainText().trimmed();
        return goal;
    }

private:
    static void setComboValue(QComboBox* combo, const QString& value)
    {
        const int index = combo->findText(value);
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    }

    void validateAndAccept()
    {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓鐩爣鏍囬銆?);
            return;
        }
        if (m_targetSpin->value() <= 0.0) {
            ToastNotification::display(this, "鐩爣鍊奸渶瑕佸ぇ浜?0銆?);
            return;
        }
        if (m_currentSpin->value() > m_targetSpin->value()) {
            const auto result = QMessageBox::question(
                this,
                "褰撳墠鍊艰秴杩囩洰鏍囧€?,
                "褰撳墠鍊煎凡缁忓ぇ浜庣洰鏍囧€笺€傛槸鍚︿粛鐒剁户缁繚瀛橈紵");
            if (result != QMessageBox::Yes) {
                return;
            }
        }
        accept();
    }

    int m_goalId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
    QDoubleSpinBox* m_targetSpin = nullptr;
    QDoubleSpinBox* m_currentSpin = nullptr;
    QLineEdit* m_unitEdit = nullptr;
    QLineEdit* m_deadlineEdit = nullptr;
    QComboBox* m_priorityCombo = nullptr;
    QComboBox* m_statusCombo = nullptr;
    QLineEdit* m_milestonesEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class RoleEditorDialog : public QDialog {
public:
    explicit RoleEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("瑙掕壊缂栬緫");
        resize(520, 620);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("缁存姢浠昏亴銆佺彮濮斻€佸洟闃熻亴璐ｇ瓑瑙掕壊淇℃伅锛屼繚瀛樺悗浼氬悓姝ュ弬涓庢椂闂磋酱鍜岀畝鍘嗙敓鎴愩€?, this);
        intro->setObjectName("pageSubtitle");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_titleEdit = new QLineEdit(this);
        m_typeEdit = new QLineEdit(this);
        m_orgEdit = new QLineEdit(this);
        m_startEdit = new QLineEdit(this);
        m_endEdit = new QLineEdit(this);
        m_activeCheck = new QCheckBox("褰撳墠浠嶅湪杩涜", this);
        m_achievementEdit = new QLineEdit(this);
        m_achievementEdit->setPlaceholderText("鐢ㄩ€楀彿鍒嗛殧涓昏鎴愭灉");
        m_contactEdit = new QLineEdit(this);
        m_supervisorEdit = new QLineEdit(this);
        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setObjectName("richCardText");
        m_descriptionEdit->setMinimumHeight(120);

        form->addRow("瑙掕壊鍚嶇О", m_titleEdit);
        form->addRow("瑙掕壊绫诲瀷", m_typeEdit);
        form->addRow("鎵€灞炵粍缁?, m_orgEdit);
        form->addRow("寮€濮嬫椂闂?, m_startEdit);
        form->addRow("缁撴潫鏃堕棿", m_endEdit);
        form->addRow("", m_activeCheck);
        form->addRow("成果摘要", m_achievementEdit);
        form->addRow("鑱旂郴浜?, m_contactEdit);
        form->addRow("鎸囧鑰佸笀", m_supervisorEdit);
        form->addRow("鑱岃矗璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setRole(const Role& role)
    {
        m_roleId = role.id;
        m_titleEdit->setText(role.title);
        m_typeEdit->setText(role.type);
        m_orgEdit->setText(role.organization);
        m_startEdit->setText(role.startDate);
        m_endEdit->setText(role.endDate);
        m_activeCheck->setChecked(role.isActive);
        m_achievementEdit->setText(role.achievements);
        m_contactEdit->setText(role.contact);
        m_supervisorEdit->setText(role.supervisor);
        m_descriptionEdit->setPlainText(role.description);
    }

    Role role() const
    {
        Role role;
        role.id = m_roleId;
        role.title = m_titleEdit->text().trimmed();
        role.type = m_typeEdit->text().trimmed();
        role.organization = m_orgEdit->text().trimmed();
        role.startDate = m_startEdit->text().trimmed();
        role.endDate = m_endEdit->text().trimmed();
        role.isActive = m_activeCheck->isChecked();
        role.achievements = m_achievementEdit->text().trimmed();
        role.contact = m_contactEdit->text().trimmed();
        role.supervisor = m_supervisorEdit->text().trimmed();
        role.description = m_descriptionEdit->toPlainText().trimmed();
        return role;
    }

private:
    void validateAndAccept()
    {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓瑙掕壊鍚嶇О銆?);
            return;
        }
        accept();
    }

    int m_roleId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_typeEdit = nullptr;
    QLineEdit* m_orgEdit = nullptr;
    QLineEdit* m_startEdit = nullptr;
    QLineEdit* m_endEdit = nullptr;
    QCheckBox* m_activeCheck = nullptr;
    QLineEdit* m_achievementEdit = nullptr;
    QLineEdit* m_contactEdit = nullptr;
    QLineEdit* m_supervisorEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class AchievementEditorDialog : public QDialog {
public:
    explicit AchievementEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("鎴愭灉缂栬緫");
        resize(540, 660);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("杩欓噷璁板綍绔炶禌銆佽瘉涔︺€佸椤瑰拰鑽ｈ獕锛屼繚瀛樺悗浼氬悓姝ュ埌鎬昏銆佹椂闂磋酱鍜岀畝鍘嗐€?, this);
        intro->setObjectName("pageSubtitle");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_titleEdit = new QLineEdit(this);
        m_typeEdit = new QLineEdit(this);
        m_levelEdit = new QLineEdit(this);
        m_orgEdit = new QLineEdit(this);
        m_dateEdit = new QLineEdit(this);
        m_certificateEdit = new QLineEdit(this);
        m_relatedCourseEdit = new QLineEdit(this);
        m_teamEdit = new QLineEdit(this);
        m_rankingEdit = new QLineEdit(this);
        m_prizeEdit = new QLineEdit(this);
        m_verifiedCheck = new QCheckBox("宸叉牳楠?鏈夎瘉鏄庢潗鏂?, this);
        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setObjectName("richCardText");
        m_descriptionEdit->setMinimumHeight(120);

        form->addRow("鎴愭灉鏍囬", m_titleEdit);
        form->addRow("鎴愭灉绫诲瀷", m_typeEdit);
        form->addRow("绾у埆", m_levelEdit);
        form->addRow("鎺堜簣鏈烘瀯", m_orgEdit);
        form->addRow("鏃ユ湡", m_dateEdit);
        form->addRow("证书缂栧彿", m_certificateEdit);
        form->addRow("鍏宠仈课程", m_relatedCourseEdit);
        form->addRow("鍥㈤槦鎴愬憳", m_teamEdit);
        form->addRow("鎺掑悕", m_rankingEdit);
        form->addRow("奖项", m_prizeEdit);
        form->addRow("", m_verifiedCheck);
        form->addRow("鎴愭灉璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setAchievement(const Achievement& achievement)
    {
        m_achievementId = achievement.id;
        m_titleEdit->setText(achievement.title);
        m_typeEdit->setText(achievement.type);
        m_levelEdit->setText(achievement.level);
        m_orgEdit->setText(achievement.organization);
        m_dateEdit->setText(achievement.date);
        m_certificateEdit->setText(achievement.certificate);
        m_relatedCourseEdit->setText(achievement.relatedCourse);
        m_teamEdit->setText(achievement.teamMembers);
        m_rankingEdit->setText(achievement.ranking);
        m_prizeEdit->setText(achievement.prize);
        m_verifiedCheck->setChecked(achievement.verified);
        m_descriptionEdit->setPlainText(achievement.description);
    }

    Achievement achievement() const
    {
        Achievement achievement;
        achievement.id = m_achievementId;
        achievement.title = m_titleEdit->text().trimmed();
        achievement.type = m_typeEdit->text().trimmed();
        achievement.level = m_levelEdit->text().trimmed();
        achievement.organization = m_orgEdit->text().trimmed();
        achievement.date = m_dateEdit->text().trimmed();
        achievement.certificate = m_certificateEdit->text().trimmed();
        achievement.relatedCourse = m_relatedCourseEdit->text().trimmed();
        achievement.teamMembers = m_teamEdit->text().trimmed();
        achievement.ranking = m_rankingEdit->text().trimmed();
        achievement.prize = m_prizeEdit->text().trimmed();
        achievement.verified = m_verifiedCheck->isChecked();
        achievement.description = m_descriptionEdit->toPlainText().trimmed();
        return achievement;
    }

private:
    void validateAndAccept()
    {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓鎴愭灉鏍囬銆?);
            return;
        }
        accept();
    }

    int m_achievementId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_typeEdit = nullptr;
    QLineEdit* m_levelEdit = nullptr;
    QLineEdit* m_orgEdit = nullptr;
    QLineEdit* m_dateEdit = nullptr;
    QLineEdit* m_certificateEdit = nullptr;
    QLineEdit* m_relatedCourseEdit = nullptr;
    QLineEdit* m_teamEdit = nullptr;
    QLineEdit* m_rankingEdit = nullptr;
    QLineEdit* m_prizeEdit = nullptr;
    QCheckBox* m_verifiedCheck = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class ExperienceEditorDialog : public QDialog {
public:
    explicit ExperienceEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("经历缂栬緫");
        resize(540, 680);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("缁存姢椤圭洰銆佸疄涔犮€佺鐮旂瓑瀹炶返经历銆傝繖閲岀殑鏁版嵁浼氳繘鍏ユ椂闂磋酱銆佺畝鍘嗗拰 AI 鍒嗘瀽銆?, this);
        intro->setObjectName("pageSubtitle");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_titleEdit = new QLineEdit(this);
        m_typeEdit = new QLineEdit(this);
        m_orgEdit = new QLineEdit(this);
        m_roleEdit = new QLineEdit(this);
        m_startEdit = new QLineEdit(this);
        m_endEdit = new QLineEdit(this);
        m_ongoingCheck = new QCheckBox("褰撳墠浠嶅湪杩涜", this);
        m_techEdit = new QLineEdit(this);
        m_achievementEdit = new QLineEdit(this);
        m_supervisorEdit = new QLineEdit(this);
        m_contactEdit = new QLineEdit(this);
        m_locationEdit = new QLineEdit(this);
        m_urlEdit = new QLineEdit(this);
        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setObjectName("richCardText");
        m_descriptionEdit->setMinimumHeight(120);

        form->addRow("经历鏍囬", m_titleEdit);
        form->addRow("经历绫诲瀷", m_typeEdit);
        form->addRow("鏈烘瀯 / 鍥㈤槦", m_orgEdit);
        form->addRow("鎷呬换瑙掕壊", m_roleEdit);
        form->addRow("寮€濮嬫椂闂?, m_startEdit);
        form->addRow("缁撴潫鏃堕棿", m_endEdit);
        form->addRow("", m_ongoingCheck);
        form->addRow("鎶€鏈爤", m_techEdit);
        form->addRow("鎴愭灉鍏抽敭璇?, m_achievementEdit);
        form->addRow("鎸囧浜?, m_supervisorEdit);
        form->addRow("鑱旂郴鏂瑰紡", m_contactEdit);
        form->addRow("鍦扮偣", m_locationEdit);
        form->addRow("閾炬帴", m_urlEdit);
        form->addRow("经历璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setExperience(const Experience& experience)
    {
        m_experienceId = experience.id;
        m_titleEdit->setText(experience.title);
        m_typeEdit->setText(experience.type);
        m_orgEdit->setText(experience.organization);
        m_roleEdit->setText(experience.role);
        m_startEdit->setText(experience.startDate);
        m_endEdit->setText(experience.endDate);
        m_ongoingCheck->setChecked(experience.isOngoing);
        m_techEdit->setText(experience.technologies);
        m_achievementEdit->setText(experience.achievements);
        m_supervisorEdit->setText(experience.supervisor);
        m_contactEdit->setText(experience.contact);
        m_locationEdit->setText(experience.location);
        m_urlEdit->setText(experience.url);
        m_descriptionEdit->setPlainText(experience.description);
    }

    Experience experience() const
    {
        Experience experience;
        experience.id = m_experienceId;
        experience.title = m_titleEdit->text().trimmed();
        experience.type = m_typeEdit->text().trimmed();
        experience.organization = m_orgEdit->text().trimmed();
        experience.role = m_roleEdit->text().trimmed();
        experience.startDate = m_startEdit->text().trimmed();
        experience.endDate = m_endEdit->text().trimmed();
        experience.isOngoing = m_ongoingCheck->isChecked();
        experience.technologies = m_techEdit->text().trimmed();
        experience.achievements = m_achievementEdit->text().trimmed();
        experience.supervisor = m_supervisorEdit->text().trimmed();
        experience.contact = m_contactEdit->text().trimmed();
        experience.location = m_locationEdit->text().trimmed();
        experience.url = m_urlEdit->text().trimmed();
        experience.description = m_descriptionEdit->toPlainText().trimmed();
        return experience;
    }

private:
    void validateAndAccept()
    {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓经历鏍囬銆?);
            return;
        }
        accept();
    }

    int m_experienceId = 0;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_typeEdit = nullptr;
    QLineEdit* m_orgEdit = nullptr;
    QLineEdit* m_roleEdit = nullptr;
    QLineEdit* m_startEdit = nullptr;
    QLineEdit* m_endEdit = nullptr;
    QCheckBox* m_ongoingCheck = nullptr;
    QLineEdit* m_techEdit = nullptr;
    QLineEdit* m_achievementEdit = nullptr;
    QLineEdit* m_supervisorEdit = nullptr;
    QLineEdit* m_contactEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLineEdit* m_urlEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class ActivityEditorDialog : public QDialog {
public:
    explicit ActivityEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("娲诲姩缂栬緫");
        resize(520, 600);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("鐢ㄤ簬缁存姢璇惧娲诲姩銆佺珵璧涖€侀」鐩拰蹇楁効鏈嶅姟璁板綍銆備繚瀛樺悗浼氬悓姝ュ埛鏂版€昏銆佹椂闂磋酱鍜岀畝鍘嗐€?, this);
        intro->setObjectName("pageSubtitle");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_nameEdit = new QLineEdit(this);
        m_categoryEdit = new QLineEdit(this);
        m_startEdit = new QLineEdit(this);
        m_endEdit = new QLineEdit(this);
        m_tagsEdit = new QLineEdit(this);
        m_tagsEdit->setPlaceholderText("鐢ㄩ€楀彿鍒嗛殧鏍囩");
        m_favoriteCheck = new QCheckBox("鏍囪涓洪噸鐐规椿鍔?, this);
        m_activeCheck = new QCheckBox("褰撳墠浠嶅湪杩涜", this);
        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setObjectName("richCardText");
        m_descriptionEdit->setMinimumHeight(120);

        form->addRow("娲诲姩鍚嶇О", m_nameEdit);
        form->addRow("娲诲姩绫诲埆", m_categoryEdit);
        form->addRow("寮€濮嬫椂闂?, m_startEdit);
        form->addRow("缁撴潫鏃堕棿", m_endEdit);
        form->addRow("娲诲姩鏍囩", m_tagsEdit);
        form->addRow("", m_favoriteCheck);
        form->addRow("", m_activeCheck);
        form->addRow("娲诲姩璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setActivity(const Activity& activity)
    {
        m_activityId = activity.id;
        m_nameEdit->setText(activity.name);
        m_categoryEdit->setText(activity.category);
        m_startEdit->setText(activity.startDate);
        m_endEdit->setText(activity.endDate);
        m_tagsEdit->setText(activity.tags);
        m_favoriteCheck->setChecked(activity.isFavorite);
        m_activeCheck->setChecked(activity.isActive);
        m_descriptionEdit->setPlainText(activity.description);
    }

    Activity activity() const
    {
        Activity activity;
        activity.id = m_activityId;
        activity.name = m_nameEdit->text().trimmed();
        activity.category = m_categoryEdit->text().trimmed();
        activity.startDate = m_startEdit->text().trimmed();
        activity.endDate = m_endEdit->text().trimmed();
        activity.tags = m_tagsEdit->text().trimmed();
        activity.isFavorite = m_favoriteCheck->isChecked();
        activity.isActive = m_activeCheck->isChecked();
        activity.description = m_descriptionEdit->toPlainText().trimmed();
        return activity;
    }

private:
    void validateAndAccept()
    {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓娲诲姩鍚嶇О銆?);
            return;
        }
        accept();
    }

    int m_activityId = 0;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
    QLineEdit* m_startEdit = nullptr;
    QLineEdit* m_endEdit = nullptr;
    QLineEdit* m_tagsEdit = nullptr;
    QCheckBox* m_favoriteCheck = nullptr;
    QCheckBox* m_activeCheck = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class JobEditorDialog : public QDialog {
public:
    explicit JobEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("宀椾綅缂栬緫");
        resize(560, 700);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("鐢ㄤ簬缁存姢鐩爣宀椾綅銆佽姹傚尮閰嶅拰鏉ユ簮淇℃伅銆傚矖浣嶈姹傛瘡琛屽～鍐欎竴鏉★紝淇濆瓨鍚庝細鏄剧ず鍖归厤杩涘害銆?, this);
        intro->setObjectName("pageSubtitle");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_titleEdit = new QLineEdit(this);
        m_companyEdit = new QLineEdit(this);
        m_locationEdit = new QLineEdit(this);
        m_salaryEdit = new QLineEdit(this);
        m_sourceEdit = new QLineEdit(this);
        m_urlEdit = new QLineEdit(this);
        m_prioritySpin = new QSpinBox(this);
        m_prioritySpin->setRange(0, 5);
        m_activeCheck = new QCheckBox("缁х画鍏虫敞璇ュ矖浣?, this);
        m_requirementsEdit = new QPlainTextEdit(this);
        m_requirementsEdit->setObjectName("richCardText");
        m_requirementsEdit->setMinimumHeight(140);
        m_requirementsEdit->setPlaceholderText("姣忚涓€鏉″矖浣嶈姹?);
        m_descriptionEdit = new QPlainTextEdit(this);
        m_descriptionEdit->setObjectName("richCardText");
        m_descriptionEdit->setMinimumHeight(120);

        form->addRow("宀椾綅鍚嶇О", m_titleEdit);
        form->addRow("鐩爣鍏徃", m_companyEdit);
        form->addRow("鍩庡競 / 鍦扮偣", m_locationEdit);
        form->addRow("钖祫鑼冨洿", m_salaryEdit);
        form->addRow("鎶曢€掓潵婧?, m_sourceEdit);
        form->addRow("浼樺厛绾?, m_prioritySpin);
        form->addRow("宀椾綅閾炬帴", m_urlEdit);
        form->addRow("", m_activeCheck);
        form->addRow("宀椾綅瑕佹眰", m_requirementsEdit);
        form->addRow("宀椾綅璇存槑", m_descriptionEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setJob(const Job& job)
    {
        m_jobId = job.id;
        m_existingRequirements = job.requirements;
        m_titleEdit->setText(job.title);
        m_companyEdit->setText(job.company);
        m_locationEdit->setText(job.location);
        m_salaryEdit->setText(job.salaryRange);
        m_sourceEdit->setText(job.source);
        m_urlEdit->setText(job.url);
        m_prioritySpin->setValue(job.priority);
        m_activeCheck->setChecked(job.isActive);
        QStringList lines;
        for (const auto& requirement : job.requirements) {
            lines << requirement.text;
        }
        m_requirementsEdit->setPlainText(lines.join('\n'));
        m_descriptionEdit->setPlainText(job.description);
    }

    Job job() const
    {
        Job job;
        job.id = m_jobId;
        job.title = m_titleEdit->text().trimmed();
        job.company = m_companyEdit->text().trimmed();
        job.location = m_locationEdit->text().trimmed();
        job.salaryRange = m_salaryEdit->text().trimmed();
        job.source = m_sourceEdit->text().trimmed();
        job.url = m_urlEdit->text().trimmed();
        job.priority = m_prioritySpin->value();
        job.isActive = m_activeCheck->isChecked();
        job.description = m_descriptionEdit->toPlainText().trimmed();

        const QStringList lines = m_requirementsEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
        for (const QString& rawLine : lines) {
            const QString line = rawLine.trimmed();
            if (line.isEmpty()) {
                continue;
            }
            JobRequirement requirement;
            requirement.text = line;
            for (const auto& existing : m_existingRequirements) {
                if (existing.text.trimmed() == line) {
                    requirement.met = existing.met;
                    break;
                }
            }
            job.requirements.append(requirement);
        }
        return job;
    }

private:
    void validateAndAccept()
    {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓宀椾綅鍚嶇О銆?);
            return;
        }
        accept();
    }

    int m_jobId = 0;
    QList<JobRequirement> m_existingRequirements;
    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_companyEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLineEdit* m_salaryEdit = nullptr;
    QLineEdit* m_sourceEdit = nullptr;
    QLineEdit* m_urlEdit = nullptr;
    QSpinBox* m_prioritySpin = nullptr;
    QCheckBox* m_activeCheck = nullptr;
    QPlainTextEdit* m_requirementsEdit = nullptr;
    QPlainTextEdit* m_descriptionEdit = nullptr;
};

class PeerEditorDialog : public QDialog {
public:
    explicit PeerEditorDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("鍚屽瀵规瘮鏁版嵁");
        resize(520, 620);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto* intro = new QLabel("鐢ㄤ簬琛ュ厖鍚屽鎴栨爣鏉嗗璞＄殑鏁版嵁锛屽府鍔╃敓鎴愭洿鐪熷疄鐨勬í鍚戝垎鏋愭姤鍛娿€?, this);
        intro->setObjectName("pageSubtitle");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        m_nameEdit = new QLineEdit(this);
        m_majorEdit = new QLineEdit(this);
        m_semesterEdit = new QLineEdit(this);
        m_semesterEdit->setPlaceholderText("渚嬪 2026-鏄ュ");
        m_gpaSpin = new QDoubleSpinBox(this);
        m_gpaSpin->setRange(0.0, 5.0);
        m_gpaSpin->setDecimals(2);
        m_creditsSpin = new QDoubleSpinBox(this);
        m_creditsSpin->setRange(0.0, 300.0);
        m_creditsSpin->setDecimals(1);
        m_achievementSpin = new QSpinBox(this);
        m_achievementSpin->setRange(0, 999);
        m_experienceSpin = new QSpinBox(this);
        m_experienceSpin->setRange(0, 999);
        m_noteEdit = new QPlainTextEdit(this);
        m_noteEdit->setObjectName("richCardText");
        m_noteEdit->setMinimumHeight(120);

        form->addRow("濮撳悕", m_nameEdit);
        form->addRow("涓撲笟", m_majorEdit);
        form->addRow("瀛︽湡", m_semesterEdit);
        form->addRow("GPA", m_gpaSpin);
        form->addRow("瀛﹀垎", m_creditsSpin);
        form->addRow("鎴愭灉鏁伴噺", m_achievementSpin);
        form->addRow("经历鏁伴噺", m_experienceSpin);
        form->addRow("澶囨敞", m_noteEdit);
        layout->addLayout(form);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }

    void setPeer(const PeerBenchmark& peer)
    {
        m_peerId = peer.id;
        m_nameEdit->setText(peer.name);
        m_majorEdit->setText(peer.major);
        m_semesterEdit->setText(peer.semester);
        m_gpaSpin->setValue(peer.gpa);
        m_creditsSpin->setValue(peer.credits);
        m_achievementSpin->setValue(peer.achievementsCount);
        m_experienceSpin->setValue(peer.experiencesCount);
        m_noteEdit->setPlainText(peer.note);
    }

    PeerBenchmark peer() const
    {
        PeerBenchmark peer;
        peer.id = m_peerId;
        peer.name = m_nameEdit->text().trimmed();
        peer.major = m_majorEdit->text().trimmed();
        peer.semester = m_semesterEdit->text().trimmed();
        peer.gpa = m_gpaSpin->value();
        peer.credits = m_creditsSpin->value();
        peer.achievementsCount = m_achievementSpin->value();
        peer.experiencesCount = m_experienceSpin->value();
        peer.note = m_noteEdit->toPlainText().trimmed();
        return peer;
    }

private:
    void validateAndAccept()
    {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            ToastNotification::display(this, "璇峰厛濉啓濮撳悕銆?);
            return;
        }
        accept();
    }

    int m_peerId = 0;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_majorEdit = nullptr;
    QLineEdit* m_semesterEdit = nullptr;
    QDoubleSpinBox* m_gpaSpin = nullptr;
    QDoubleSpinBox* m_creditsSpin = nullptr;
    QSpinBox* m_achievementSpin = nullptr;
    QSpinBox* m_experienceSpin = nullptr;
    QPlainTextEdit* m_noteEdit = nullptr;
};

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_serverUrl("http://127.0.0.1:5000/")
{
    setWindowTitle("学业发展规划系统 - Qt 妗岄潰鐗?);
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
    
    QPushButton* aiToggleBtn = new QPushButton("鉁?AI 鍔╂墜", topbar);
    aiToggleBtn->setCursor(Qt::PointingHandCursor);
    aiToggleBtn->setStyleSheet("background: #eef2f6; border-radius: 12px; padding: 4px 12px; color: #555; border: 1px solid #d0d7de; font-size: 12px; font-weight: bold; margin-left: 10px;");
    connect(aiToggleBtn, &QPushButton::clicked, this, [this]() {
        QWidget* aiSidebar = this->findChild<QWidget*>("aiSidebar");
        if (!aiSidebar) return;
        QWidget* strip = aiSidebar->findChild<QWidget*>("aiCollapsedStrip");
        QWidget* panel = aiSidebar->findChild<QWidget*>("aiPanelContent");
        bool isCollapsed = (aiSidebar->maximumWidth() <= kAiCollapsedWidth);
        QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

        if (isCollapsed) {
            // Expand
            if (strip) strip->hide();
            if (panel) { panel->show(); panel->setFixedWidth(kAiSidebarWidth); }
            aiSidebar->setMinimumWidth(kAiCollapsedWidth);
            aiSidebar->setMaximumWidth(kAiSidebarWidth);
            auto* animMin = new QPropertyAnimation(aiSidebar, "minimumWidth");
            animMin->setDuration(240); animMin->setStartValue(kAiCollapsedWidth); animMin->setEndValue(kAiSidebarWidth);
            animMin->setEasingCurve(QEasingCurve::InOutQuad);
            group->addAnimation(animMin);
        } else {
            // Collapse
            auto* animMax = new QPropertyAnimation(aiSidebar, "maximumWidth");
            animMax->setDuration(240); animMax->setStartValue(kAiSidebarWidth); animMax->setEndValue(kAiCollapsedWidth);
            animMax->setEasingCurve(QEasingCurve::InOutQuad);
            group->addAnimation(animMax);
            connect(group, &QParallelAnimationGroup::finished, this, [this, aiSidebar]() {
                QWidget* s = aiSidebar->findChild<QWidget*>("aiCollapsedStrip");
                QWidget* p = aiSidebar->findChild<QWidget*>("aiPanelContent");
                if (s) s->show();
                if (p) p->hide();
                aiSidebar->setFixedWidth(kAiCollapsedWidth);
            });
        }
        connect(group, &QParallelAnimationGroup::finished, group, &QObject::deleteLater);
        group->start();
    });
    topbarLayout->addWidget(aiToggleBtn, 0, Qt::AlignVCenter);
    
    contentLayout->addWidget(topbar);

    QWidget* mainShell = new QWidget(contentShell);
    QVBoxLayout* mainShellLayout = new QVBoxLayout(mainShell);
    mainShellLayout->setContentsMargins(0, 0, 0, 0);
    mainShellLayout->setSpacing(0);
    mainShellLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QWidget* mainInner = new QWidget(mainShell);
    mainInner->setMaximumWidth(kMaxContentWidth);
    QVBoxLayout* mainInnerLayout = new QVBoxLayout(mainInner);
    mainInnerLayout->setContentsMargins(24, 24, 28, 32);
    mainInnerLayout->setSpacing(14);

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
    m_stack->addWidget(createResumePage());
    m_stack->addWidget(createImportsPage());
    mainInnerLayout->addWidget(m_stack, 1);

    QScrollArea* scrollArea = new QScrollArea(mainShell);
    scrollArea->setObjectName("mainScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(mainInner);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    mainShellLayout->addWidget(scrollArea, 1);
    contentLayout->addWidget(mainShell, 1);

    rootLayout->addWidget(contentShell, 1);
    rootLayout->addWidget(createAiPage());
    setCentralWidget(centralWidget);

    connect(m_navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);
}

QWidget* MainWindow::createSidebar()
{
    QFrame* sidebar = new QFrame(this);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(kSidebarExpandedWidth);

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(16);

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

    QPushButton* toggleButton = new QPushButton("鈽?, topRow);
    toggleButton->setObjectName("sidebarToggle");
    toggleButton->setFlat(true);
    topRowLayout->addWidget(toggleButton, 0, Qt::AlignTop);
    layout->addWidget(topRow);

    m_navList = new QListWidget(sidebar);
    m_navList->setObjectName("navList");
    m_navList->setSpacing(2);
    m_navList->addItems({
        " 馃彔    鎬昏",
        " 馃摉    课程搴?,
        " 馃幁    瑙掕壊鑱岃矗",
        " 馃弳    成果记录",
        " 馃捈    经历妗ｆ",
        " 馃搮    璇惧娲诲姩",
        " 馃幆    鐩爣杩借釜",
        " 馃捈    鐩爣宀椾綅",
        " 馃搳    鍒嗘瀽鎶ュ憡",
        " 馃晵    鏃堕棿杞?,
        " 馃搫    绠€鍘嗗鍑?,
        " 馃摛    数据导入"
    });
    layout->addWidget(m_navList, 1);

    // Spacer to push bottom cards down (Vue: margin-top: auto)
    layout->addStretch(0);

    QFrame* timeCard = new QFrame(sidebar);
    timeCard->setObjectName("sidebarInfoCard");
    QHBoxLayout* timeLayout = new QHBoxLayout(timeCard);
    timeLayout->setContentsMargins(8, 8, 8, 8);
    timeLayout->setSpacing(10);
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
    m_timeDetailLabel->setWordWrap(true);
    timeTextLayout->addWidget(m_timeDetailLabel);
    timeLayout->addWidget(timeText, 1);
    layout->addWidget(timeCard);

    QPushButton* studentCard = new QPushButton(sidebar);
    studentCard->setObjectName("sidebarInfoCard");
    studentCard->setCursor(Qt::PointingHandCursor);
    studentCard->setStyleSheet("#sidebarInfoCard { border: 1px solid transparent; background: transparent; border-radius: 14px; text-align: left; } #sidebarInfoCard:hover { background: #f0ddd1; }");
    QHBoxLayout* studentLayout = new QHBoxLayout(studentCard);
    studentLayout->setContentsMargins(8, 8, 8, 8);
    studentLayout->setSpacing(10);
    QLabel* studentAvatar = new QLabel("S", studentCard);
    studentAvatar->setObjectName("sidebarInfoAvatar");
    studentAvatar->setAttribute(Qt::WA_TransparentForMouseEvents);
    studentLayout->addWidget(studentAvatar, 0, Qt::AlignTop);
    QWidget* studentText = new QWidget(studentCard);
    studentText->setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout* studentTextLayout = new QVBoxLayout(studentText);
    studentTextLayout->setContentsMargins(0, 0, 0, 0);
    studentTextLayout->setSpacing(2);
    QLabel* studentKicker = new QLabel("Student", studentText);
    studentKicker->setObjectName("sidebarInfoKicker");
    studentTextLayout->addWidget(studentKicker);
    m_studentNameLabel = new QLabel("璇峰～鍐欏鍚?, studentText);
    m_studentNameLabel->setObjectName("sidebarInfoTitle");
    studentTextLayout->addWidget(m_studentNameLabel);
    m_studentMetaLabel = new QLabel("璇峰～鍐欏鍙?路 璇峰～鍐欓櫌绯?, studentText);
    m_studentMetaLabel->setObjectName("sidebarInfoDetail");
    m_studentMetaLabel->setWordWrap(true);
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


    connect(toggleButton, &QPushButton::clicked, this, [sidebar, brandMark, brandText, timeCard, timeText, studentCard, studentText, timeAvatar, studentAvatar, toggleButton, this]() {
        bool isExpanded = (sidebar->width() > kSidebarCollapsedWidth);
        QPropertyAnimation* anim1 = new QPropertyAnimation(sidebar, "minimumWidth");
        QPropertyAnimation* anim2 = new QPropertyAnimation(sidebar, "maximumWidth");
        anim1->setDuration(220);
        anim2->setDuration(220);
        anim1->setEasingCurve(QEasingCurve::InOutQuad);
        anim2->setEasingCurve(QEasingCurve::InOutQuad);

        if (isExpanded) {
            // Collapse - show only icons
            anim1->setStartValue(kSidebarExpandedWidth); anim1->setEndValue(kSidebarCollapsedWidth);
            anim2->setStartValue(kSidebarExpandedWidth); anim2->setEndValue(kSidebarCollapsedWidth);
            brandMark->hide();
            brandText->hide();
            timeText->hide();
            studentText->hide();
            // Center avatars when collapsed
            QHBoxLayout* tl = qobject_cast<QHBoxLayout*>(timeCard->layout());
            if (tl) tl->setAlignment(timeAvatar, Qt::AlignHCenter);
            QHBoxLayout* sl = qobject_cast<QHBoxLayout*>(studentCard->layout());
            if (sl) sl->setAlignment(studentAvatar, Qt::AlignHCenter);
            // Show only emoji icon for each nav item
            for (int i = 0; i < m_navList->count(); ++i) {
                if (auto* item = m_navList->item(i)) {
                    QString fullText = item->text().trimmed();
                    // Extract first emoji character
                    QString iconOnly;
                    for (int c = 0; c < fullText.length(); ) {
                        uint ucs4 = fullText.at(c).unicode();
                        if (ucs4 >= 0x1F000 || ucs4 >= 0x2600 && ucs4 <= 0x27BF || ucs4 == 0x200D) {
                            iconOnly += fullText.at(c);
                            if (c + 1 < fullText.length() && fullText.at(c+1).isHighSurrogate()) { ++c; iconOnly += fullText.at(c); }
                            break;
                        }
                        ++c;
                    }
                    item->setText(iconOnly.isEmpty() ? QString("鈥?) : iconOnly);
                    item->setTextAlignment(Qt::AlignCenter);
                }
            }
            toggleButton->setText("鈥?);
        } else {
            // Expand - restore full text
            anim1->setStartValue(kSidebarCollapsedWidth); anim1->setEndValue(kSidebarExpandedWidth);
            anim2->setStartValue(kSidebarCollapsedWidth); anim2->setEndValue(kSidebarExpandedWidth);
            brandMark->show();
            brandText->show();
            timeText->show();
            studentText->show();
            // Restore left alignment
            QHBoxLayout* tl = qobject_cast<QHBoxLayout*>(timeCard->layout());
            if (tl) tl->setAlignment(timeAvatar, Qt::AlignTop);
            QHBoxLayout* sl = qobject_cast<QHBoxLayout*>(studentCard->layout());
            if (sl) sl->setAlignment(studentAvatar, Qt::AlignTop);
            // Restore full nav labels
            QStringList fullLabels = {
                " 馃彔    鎬昏",
                " 馃摉    课程搴?,
                " 馃幁    瑙掕壊鑱岃矗",
                " 馃弳    成果记录",
                " 馃捈    经历妗ｆ",
                " 馃搮    璇惧娲诲姩",
                " 馃幆    鐩爣杩借釜",
                " 馃捈    鐩爣宀椾綅",
                " 馃搳    鍒嗘瀽鎶ュ憡",
                " 馃晵    鏃堕棿杞?,
                " 馃搫    绠€鍘嗗鍑?,
                " 馃摛    数据导入"
            };
            for (int i = 0; i < qMin(m_navList->count(), fullLabels.size()); ++i) {
                if (auto* item = m_navList->item(i)) {
                    item->setText(fullLabels[i]);
                    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                }
            }
            toggleButton->setText("鈽?);
        }
        anim1->start(QAbstractAnimation::DeleteWhenStopped);
        anim2->start(QAbstractAnimation::DeleteWhenStopped);
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
        "蹇€熸煡鐪嬩釜浜哄彂灞曠姸鎬佸拰杩戞湡建议銆?,
        page);
    subtitle->setObjectName("pageSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    QGridLayout* metrics = new QGridLayout();
    metrics->setHorizontalSpacing(14);
    metrics->setVerticalSpacing(14);
    metrics->addWidget(createMetricCard("课程鎬绘暟", &m_totalCoursesValue), 0, 0);
    metrics->addWidget(createMetricCard("褰撳墠 GPA", &m_gpaValue), 0, 1);
    metrics->addWidget(createMetricCard("鐩爣骞冲潎杩涘害", &m_goalProgressValue), 0, 2);
    metrics->addWidget(createMetricCard("鎴愭灉鏁伴噺", &m_achievementValue), 0, 3);
    metrics->addWidget(createMetricCard("经历鏁伴噺", &m_experienceValue), 1, 0);
    metrics->addWidget(createMetricCard("瑙掕壊鏁伴噺", &m_roleValue), 1, 1);
    metrics->addWidget(createMetricCard("娲诲姩鏁伴噺", &m_activityValue), 1, 2);
    metrics->addWidget(
        createMetricCard("宸蹭慨瀛﹀垎", &m_creditValue, "鍩轰簬课程瀹屾垚鐘舵€佽嚜鍔ㄧ粺璁?), 1, 3);
    layout->addLayout(metrics);

    QHBoxLayout* lowerLayout = new QHBoxLayout();
    lowerLayout->setSpacing(14);

    QFrame* recommendationCard = new QFrame(page);
    recommendationCard->setObjectName("contentCard");
    QVBoxLayout* recommendationLayout = new QVBoxLayout(recommendationCard);
    recommendationLayout->setContentsMargins(16, 14, 16, 14);
    recommendationLayout->setSpacing(10);
    QLabel* recommendationTitle = new QLabel("杩戞湡建议", recommendationCard);
    recommendationTitle->setObjectName("sectionTitle");
    recommendationLayout->addWidget(recommendationTitle);
    m_recommendationList = new QListWidget(recommendationCard);
    m_recommendationList->setObjectName("plainList");
    m_recommendationList->setWordWrap(true);
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

    QLabel* title = new QLabel("课程搴?, page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_courseSummaryLabel = new QLabel("姝ｅ湪璇诲彇课程鏁版嵁...", page);
    m_courseSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_courseSummaryLabel);

    QFrame* filterCard = new QFrame(page);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);
    m_courseSearchInput = new QLineEdit(filterCard);
    m_courseSearchInput->setPlaceholderText("鎼滅储课程鍚嶇О / 浠ｇ爜 / 鏁欏笀");
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
    QPushButton* addButton = new QPushButton("鏂板课程", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑课程", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑课程", page);
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

    QLabel* helper = new QLabel("课程搴撶幇鍦ㄦ敮鎸佸師鐢?Qt 褰曞叆涓庣紪杈戙€傚弻鍑讳换鎰忎竴琛屼篃鍙互鐩存帴鎵撳紑缂栬緫寮圭獥銆?, tableCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    cardLayout->addWidget(helper);

    m_courseTable = new QTableWidget(tableCard);
    m_courseTable->setColumnCount(7);
    m_courseTable->setHorizontalHeaderLabels(
        {"课程鍚嶇О", "浠ｇ爜", "瀛︽湡", "瀛﹀垎", "鍒嗘暟", "缁╃偣", "鐘舵€?});
    m_courseTable->horizontalHeader()->setStretchLastSection(true);
    m_courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    m_roleSearchInput->setPlaceholderText("鎼滅储瑙掕壊鍚嶇О / 缁勭粐 / 鎻忚堪");
    m_roleTypeFilter = new QComboBox(roleFilterCard);
    m_roleTypeFilter->addItem("全部类型", "");
    m_roleTypeFilter->addItem("浠昏亴", "浠昏亴");
    m_roleTypeFilter->addItem("鐝", "鐝");
    m_roleTypeFilter->addItem("绀惧洟", "绀惧洟");
    m_roleTypeFilter->addItem("鍥㈤槦", "鍥㈤槦");
    m_roleTypeFilter->addItem("瀹炰範", "瀹炰範");
    m_roleTypeFilter->addItem("其他", "其他");
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

    QLabel* title = new QLabel("成果记录", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_achievementSummaryLabel = new QLabel("正在读取成果数据...", page);
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
    m_achievementTypeFilter->addItem("全部类型", "");
    m_achievementTypeFilter->addItem("证书", "证书");
    m_achievementTypeFilter->addItem("竞赛", "竞赛");
    m_achievementTypeFilter->addItem("奖项", "奖项");
    m_achievementTypeFilter->addItem("课程成果", "课程成果");
    m_achievementTypeFilter->addItem("开源贡献, "开源贡献);
    m_achievementTypeFilter->addItem("论文报告", "论文报告");
    m_achievementTypeFilter->addItem("其他", "其他");
    m_achievementLevelFilter = new QComboBox(achFilterCard);
    m_achievementLevelFilter->addItem("全部级别", "");
    m_achievementLevelFilter->addItem("鍥藉绾?, "鍥藉绾?);
    m_achievementLevelFilter->addItem("鐪佺骇", "鐪佺骇");
    m_achievementLevelFilter->addItem("鏍＄骇", "鏍＄骇");
    m_achievementLevelFilter->addItem("闄㈢骇", "闄㈢骇");
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
    QPushButton* addButton = new QPushButton("鏂板鎴愭灉", page);
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
    metrics->addWidget(createMetricCard("已验证成果数, &m_achievementVerifiedValue), 0, 1);
    metrics->addWidget(createMetricCard("成果级别数, &m_achievementLevelValue, "鎸?level 瀛楁缁熻"), 0, 2);
    metrics->addWidget(createMetricCard("成果类型数, &m_achievementTypeValue, "鎸?type 瀛楁缁熻"), 0, 3);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("成果时间线, listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("绔炶禌銆佽瘉涔︺€佸椤归兘鍙互鐩存帴鍦ㄨ繖閲岀淮鎶ゃ€傚弻鍑绘潯鐩繘鍏ュ師鐢熺紪杈戝脊绐椼€?, listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_achievementList = new QListWidget(listCard);
    m_achievementList->setObjectName("plainList");
    m_achievementList->setWordWrap(true);
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

    QLabel* title = new QLabel("经历妗ｆ", page);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_experienceSummaryLabel = new QLabel("姝ｅ湪璇诲彇经历鏁版嵁...", page);
    m_experienceSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_experienceSummaryLabel);

    QFrame* expFilterCard = new QFrame(page);
    expFilterCard->setObjectName("contentCard");
    QGridLayout* expFilterLayout = new QGridLayout(expFilterCard);
    expFilterLayout->setContentsMargins(12, 12, 12, 12);
    expFilterLayout->setHorizontalSpacing(10);
    expFilterLayout->setVerticalSpacing(10);
    m_experienceSearchInput = new QLineEdit(expFilterCard);
    m_experienceSearchInput->setPlaceholderText("鎼滅储经历鏍囬 / 缁勭粐 / 鎻忚堪");
    m_experienceTypeFilter = new QComboBox(expFilterCard);
    m_experienceTypeFilter->addItem("全部类型", "");
    m_experienceTypeFilter->addItem("椤圭洰", "椤圭洰");
    m_experienceTypeFilter->addItem("瀹炰範", "瀹炰範");
    m_experienceTypeFilter->addItem("绉戠爺", "绉戠爺");
    m_experienceTypeFilter->addItem("蹇楁効", "蹇楁効");
    m_experienceTypeFilter->addItem("绔炶禌", "绔炶禌");
    m_experienceTypeFilter->addItem("其他", "其他");
    expFilterLayout->addWidget(m_experienceSearchInput, 0, 0);
    expFilterLayout->addWidget(m_experienceTypeFilter, 0, 1);
    connect(m_experienceSearchInput, &QLineEdit::textChanged, this, [this]() { refreshExperiences(); });
    connect(m_experienceTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshExperiences(); });
    layout->addWidget(expFilterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("鏂板经历", page);
    QPushButton* editButton = new QPushButton("缂栬緫閫変腑经历", page);
    QPushButton* removeButton = new QPushButton("鍒犻櫎閫変腑经历", page);
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
    metrics->addWidget(createMetricCard("经历鎬绘暟", &m_experienceTotalValue), 0, 0);
    metrics->addWidget(createMetricCard("杩涜涓粡鍘?, &m_experienceOngoingValue), 0, 1);
    metrics->addWidget(createMetricCard("经历绫诲瀷鏁?, &m_experienceTypeValue, "鎸?type 瀛楁缁熻"), 0, 2);
    layout->addLayout(metrics);

    QFrame* listCard = new QFrame(page);
    listCard->setObjectName("contentCard");
    QVBoxLayout* listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 14, 16, 14);
    listLayout->setSpacing(10);
    QLabel* listTitle = new QLabel("经历鏃堕棿绾?, listCard);
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    QLabel* helper = new QLabel("椤圭洰銆佸疄涔犮€佺鐮斿拰蹇楁効经历閮藉彲浠ュ湪鍘熺敓鐣岄潰涓淮鎶ゃ€傚弻鍑绘潯鐩嵆鍙紪杈戙€?, listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_experienceList = new QListWidget(listCard);
    m_experienceList->setObjectName("plainList");
    m_experienceList->setWordWrap(true);
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
    m_activitySearchInput->setPlaceholderText("鎼滅储娲诲姩鍚嶇О / 鎻忚堪");
    m_activityCategoryInput = new QLineEdit(filterCard);
    m_activityCategoryInput->setPlaceholderText("鍒嗙被杩囨护");
    filterLayout->addWidget(m_activitySearchInput, 0, 0);
    filterLayout->addWidget(m_activityCategoryInput, 0, 1);
    connect(m_activitySearchInput, &QLineEdit::textChanged, this, [this]() { refreshActivities(); });
    connect(m_activityCategoryInput, &QLineEdit::textChanged, this, [this]() { refreshActivities(); });
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
    QLabel* helper = new QLabel("璇惧鎷撳睍銆佸織鎰挎湇鍔′笌鏃ュ父娲诲姩銆傚弻鍑荤紪杈戙€?, listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_activityList = new QListWidget(listCard);
    m_activityList->setObjectName("plainList");
    m_activityList->setWordWrap(true);
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

    QLabel* title = new QLabel("鐩爣瑙勫垝", page);
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
    m_goalStatusInput = new QLineEdit(filterCard);
    m_goalStatusInput->setPlaceholderText("鐘舵€佽繃婊わ紝渚嬪 In Progress");
    m_goalPriorityInput = new QLineEdit(filterCard);
    m_goalPriorityInput->setPlaceholderText("浼樺厛绾ц繃婊わ紝渚嬪 High");
    m_goalSortInput = new QLineEdit(filterCard);
    m_goalSortInput->setPlaceholderText("鎺掑簭锛歱rogress / deadline / title / priority");
    filterLayout->addWidget(m_goalSearchInput, 0, 0);
    filterLayout->addWidget(m_goalStatusInput, 0, 1);
    filterLayout->addWidget(m_goalPriorityInput, 1, 0);
    filterLayout->addWidget(m_goalSortInput, 1, 1);
    connect(m_goalSearchInput, &QLineEdit::textChanged, this, [this]() { refreshGoals(); });
    connect(m_goalStatusInput, &QLineEdit::textChanged, this, [this]() { refreshGoals(); });
    connect(m_goalPriorityInput, &QLineEdit::textChanged, this, [this]() { refreshGoals(); });
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
    QLabel* helper = new QLabel("鐩爣鍙互鐩存帴鍦ㄥ師鐢熺獥鍙ｄ腑缁存姢锛屼繚瀛樺悗浼氳仈鍔ㄦ洿鏂版€昏銆佹椂闂磋酱鍜岀畝鍘嗗垎鏋愩€?, listCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    listLayout->addWidget(helper);
    m_goalList = new QListWidget(listCard);
    m_goalList->setObjectName("plainList");
    m_goalList->setWordWrap(true);
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
    metrics->addWidget(createMetricCard("鎬荤粨建议鏉℃暟", &m_analysisSuggestionValue), 0, 2);
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
    m_analysisSemesterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tLayout->addWidget(m_analysisSemesterTable, 1);

    QLabel* peerLabel = new QLabel("妯悜鍚屽瀵规瘮", tableCard);
    peerLabel->setObjectName("sectionTitle");
    tLayout->addWidget(peerLabel);
    m_analysisPeerTable = new QTableWidget(tableCard);
    m_analysisPeerTable->setColumnCount(6);
    m_analysisPeerTable->setHorizontalHeaderLabels({"濮撳悕", "涓撲笟", "瀛︽湡", "GPA", "鎴愭灉", "经历"});
    m_analysisPeerTable->horizontalHeader()->setStretchLastSection(true);
    m_analysisPeerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    sLayout->addWidget(m_analysisStrengthList, 1);

    QLabel* rskLabel = new QLabel("娼滃湪椋庨櫓", suggestionCard);
    rskLabel->setObjectName("sectionTitle");
    sLayout->addWidget(rskLabel);
    m_analysisRiskList = new QListWidget(suggestionCard);
    m_analysisRiskList->setObjectName("plainList");
    m_analysisRiskList->setWordWrap(true);
    sLayout->addWidget(m_analysisRiskList, 1);

    QLabel* sugLabel = new QLabel("鍙戝睍建议", suggestionCard);
    sugLabel->setObjectName("sectionTitle");
    sLayout->addWidget(sugLabel);
    m_analysisSuggestionList = new QListWidget(suggestionCard);
    m_analysisSuggestionList->setObjectName("plainList");
    m_analysisSuggestionList->setWordWrap(true);
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
    timelineLayout->addWidget(m_timelineList);
    bottom->addWidget(timelineCard, 2);

    QFrame* suggestionCard = new QFrame(page);
    suggestionCard->setObjectName("contentCard");
    QVBoxLayout* suggestionLayout = new QVBoxLayout(suggestionCard);
    suggestionLayout->setContentsMargins(16, 14, 16, 14);
    suggestionLayout->setSpacing(10);
    QLabel* suggestionTitle = new QLabel("闃舵建议", suggestionCard);
    suggestionTitle->setObjectName("sectionTitle");
    suggestionLayout->addWidget(suggestionTitle);
    m_timelineSuggestionList = new QListWidget(suggestionCard);
    m_timelineSuggestionList->setObjectName("plainList");
    m_timelineSuggestionList->setWordWrap(true);
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

    QLabel* title = new QLabel("绠€鍘嗙敓鎴?, page);
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
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshResume);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetResumeOptions);
    connect(exportJsonButton, &QPushButton::clicked, this, &MainWindow::exportResumeJson);
    connect(exportHtmlButton, &QPushButton::clicked, this, &MainWindow::exportResumeHtml);
    actionLayout->addWidget(refreshButton);
    actionLayout->addWidget(resetButton);
    actionLayout->addWidget(exportJsonButton);
    actionLayout->addWidget(exportHtmlButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    QFrame* configCard = new QFrame(page);
    configCard->setObjectName("contentCard");
    configCard->setMinimumWidth(340);
    configCard->setMaximumWidth(380);
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
    m_resumeSummaryInput->setObjectName("richCardText");
    m_resumeSummaryInput->setMinimumHeight(120);

    formLayout->addRow("濮撳悕 / 鏍囬", m_resumeNameInput);
    formLayout->addRow("韬唤璇存槑", m_resumeTitleInput);
    formLayout->addRow("閭", m_resumeEmailInput);
    formLayout->addRow("鐢佃瘽", m_resumePhoneInput);
    formLayout->addRow("涓汉鎽樿", m_resumeSummaryInput);
    configLayout->addLayout(formLayout);

    QLabel* sectionHint = new QLabel("鍒嗗尯寮€鍏?, configCard);
    sectionHint->setObjectName("sectionTitle");
    configLayout->addWidget(sectionHint);

    m_resumeEducationCheck = new QCheckBox("鏁欒偛经历", configCard);
    m_resumeExperienceCheck = new QCheckBox("瀹炶返经历", configCard);
    m_resumeAchievementCheck = new QCheckBox("成就徽章", configCard);
    m_resumeRoleCheck = new QCheckBox("瑙掕壊浠昏亴", configCard);
    m_resumeActivityCheck = new QCheckBox("娲诲姩鍙備笌", configCard);
    configLayout->addWidget(m_resumeEducationCheck);
    configLayout->addWidget(m_resumeExperienceCheck);
    configLayout->addWidget(m_resumeAchievementCheck);
    configLayout->addWidget(m_resumeRoleCheck);
    configLayout->addWidget(m_resumeActivityCheck);
    configLayout->addStretch();

    QFrame* previewCard = new QFrame(page);
    previewCard->setObjectName("contentCard");
    QVBoxLayout* previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(16, 14, 16, 14);
    previewLayout->setSpacing(10);
    QLabel* previewTitle = new QLabel("绠€鍘嗛瑙?, previewCard);
    previewTitle->setObjectName("sectionTitle");
    previewLayout->addWidget(previewTitle);
    m_resumePreview = new QTextBrowser(previewCard);
    m_resumePreview->setObjectName("richCardText");
    previewLayout->addWidget(m_resumePreview);
    bodyLayout->addWidget(configCard);
    bodyLayout->addWidget(previewCard, 1);
    layout->addLayout(bodyLayout, 1);
    resetResumeOptions();

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

    m_importSummaryLabel = new QLabel("鏀寔浠庡閮ㄧ郴缁熸壒閲忓鍏ラ€氱敤课程銆佽鑹层€佹垚灏辩瓑璁板綍銆?, page);
    m_importSummaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_importSummaryLabel);

    QFrame* controlCard = new QFrame(page);
    controlCard->setObjectName("contentCard");
    QVBoxLayout* cl = new QVBoxLayout(controlCard);
    cl->setContentsMargins(16, 14, 16, 14);
    cl->setSpacing(12);

    QFormLayout* form = new QFormLayout();
    m_importEntityCombo = new QComboBox(controlCard);
    m_importEntityCombo->addItem("课程鏁版嵁 (courses)", "courses");
    m_importEntityCombo->addItem("瑙掕壊鑱岃矗 (roles)", "roles");
    m_importEntityCombo->addItem("成果记录 (achievements)", "achievements");
    m_importEntityCombo->addItem("瀹炶返经历 (experiences)", "experiences");
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
    m_importErrorTable->setHorizontalHeaderLabels({"鍑哄彂琛屽彿", "閿欒鍘熷洜"});
    m_importErrorTable->horizontalHeader()->setStretchLastSection(true);
    m_importErrorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    QLabel* stripIcon = new QLabel("AI", collapsedStrip);
    stripIcon->setFixedSize(28, 28);
    stripIcon->setAlignment(Qt::AlignCenter);
    stripIcon->setStyleSheet(
        "background: #fffdf9; border: 0.5px solid rgba(67,57,43,0.16); border-radius: 6px;"
        "font-size: 11px; font-weight: 500; color: #24211d;"
    );
    stripLayout->addWidget(stripIcon, 0, Qt::AlignHCenter);

    QLabel* stripText = new QLabel("AI, collapsedStrip);
    stripText->setAlignment(Qt::AlignCenter);
    stripText->setStyleSheet("font-size: 14px; font-weight: 500; color: #6a6258; letter-spacing: 2px;");
    stripLayout->addWidget(stripText, 0, Qt::AlignHCenter);
    stripLayout->addStretch();

    // Click on strip to expand
    collapsedStrip->installEventFilter(this);

    rootAiLayout->addWidget(collapsedStrip);

    // === Main Panel Content (visible when expanded) ===
    QWidget* panelContent = new QWidget(page);
    panelContent->setObjectName("aiPanelContent");
    QVBoxLayout* layout = new QVBoxLayout(panelContent);
    layout->setContentsMargins(18, 24, 18, 18);
    layout->setSpacing(16);

    // Title Block
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* title = new QLabel("AI 建议", panelContent);
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #333;");

    QPushButton* closeBtn = new QPushButton("脳", panelContent);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  font-size: 16px; color: #6a6258; background: transparent; "
        "  border: 0.5px solid transparent; border-radius: 6px;"
        "}"
        "QPushButton:hover { background: #f0eee8; border-color: rgba(67,57,43,0.16); color: #24211d; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        QWidget* aiSidebar = this->findChild<QWidget*>("aiSidebar");
        if (!aiSidebar) return;
        QWidget* strip = aiSidebar->findChild<QWidget*>("aiCollapsedStrip");
        QWidget* panel = aiSidebar->findChild<QWidget*>("aiPanelContent");
        aiSidebar->setMinimumWidth(kAiCollapsedWidth);
        QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
        auto* animMax = new QPropertyAnimation(aiSidebar, "maximumWidth");
        animMax->setDuration(240); animMax->setStartValue(kAiSidebarWidth); animMax->setEndValue(kAiCollapsedWidth);
        animMax->setEasingCurve(QEasingCurve::InOutQuad);
        group->addAnimation(animMax);
        connect(group, &QParallelAnimationGroup::finished, this, [strip, panel, aiSidebar]() {
            if (strip) strip->show();
            if (panel) panel->hide();
            aiSidebar->setFixedWidth(kAiCollapsedWidth);
        });
        connect(group, &QParallelAnimationGroup::finished, group, &QObject::deleteLater);
        group->start();
    });

    titleLayout->addWidget(title);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);
    layout->addLayout(titleLayout);

    // Status Tags
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(8);
    m_aiModeValue = new QLabel("离线模式", panelContent);
    m_aiStatusValue = new QLabel("加载完成", panelContent);
    m_aiModelValue = new QLabel("Internal", panelContent);
    m_aiModelValue->hide();

    QString tagStyle = "background: #f4f6f8; color: #555; padding: 4px 8px; border-radius: 4px; font-size: 11px;";
    m_aiModeValue->setStyleSheet(tagStyle);
    m_aiStatusValue->setStyleSheet(tagStyle);

    statusLayout->addWidget(m_aiModeValue);
    statusLayout->addWidget(m_aiStatusValue);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);

    // Toggle Buttons
    QHBoxLayout* toggleLayout = new QHBoxLayout();
    toggleLayout->setSpacing(0);
    QPushButton* chatTabBtn = new QPushButton("对话", panelContent);
    QPushButton* suggTabBtn = new QPushButton("建议", panelContent);
    chatTabBtn->setCursor(Qt::PointingHandCursor);
    suggTabBtn->setCursor(Qt::PointingHandCursor);
    QString activeTabStyle = "background: #2b5c5d; color: #fff; padding: 10px; border: none; font-weight: bold;";
    QString inactiveTabStyle = "background: #f0f2f5; color: #666; padding: 10px; border: none; font-weight: bold;";
    chatTabBtn->setStyleSheet(activeTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
    suggTabBtn->setStyleSheet(inactiveTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
    connect(chatTabBtn, &QPushButton::clicked, this, [chatTabBtn, suggTabBtn, activeTabStyle, inactiveTabStyle, this]() {
        chatTabBtn->setStyleSheet(activeTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
        suggTabBtn->setStyleSheet(inactiveTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
        if (m_aiOutput) m_aiOutput->setPlainText("AI鍔╂墜宸插氨缁€俓n鍙互鍦ㄤ笅鏂圭洿鎺ヨ緭鍏ラ棶棰樿繘琛屽璇濄€?);
    });
    connect(suggTabBtn, &QPushButton::clicked, this, [chatTabBtn, suggTabBtn, activeTabStyle, inactiveTabStyle, this]() {
        suggTabBtn->setStyleSheet(activeTabStyle + "border-top-right-radius: 6px; border-bottom-right-radius: 6px;");
        chatTabBtn->setStyleSheet(inactiveTabStyle + "border-top-left-radius: 6px; border-bottom-left-radius: 6px;");
        if (m_aiOutput) m_aiOutput->setPlainText("鐐瑰嚮涓婃柟蹇嵎鎸夐挳鑾峰彇閽堝鎬у缓璁細\n\n鈥?综合 鈥?鍏ㄩ潰瀛︿笟鍒嗘瀽\n鈥?课程 鈥?课程瑙勫垝建议\n鈥?经历 鈥?瀹炶返经历鎻愬崌\n鈥?鐩爣 鈥?鐩爣瀹屾垚绛栫暐");
    });
    toggleLayout->addWidget(chatTabBtn, 1);
    toggleLayout->addWidget(suggTabBtn, 1);
    layout->addLayout(toggleLayout);

    // Quick Action Chips
    QGridLayout* actionGrid = new QGridLayout();
    actionGrid->setSpacing(8);
    QPushButton* btn1 = new QPushButton("综合", panelContent);
    QPushButton* btn2 = new QPushButton("课程", panelContent);
    QPushButton* btn3 = new QPushButton("经历", panelContent);
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
    m_aiOutput = new QTextEdit(panelContent);
    m_aiOutput->setReadOnly(true);
    m_aiOutput->setStyleSheet("border: none; background: transparent; font-size: 13px; color: #333;");
    m_aiOutput->setPlainText("AI鍔╂墜宸插氨缁€俓n鍙互鐐瑰嚮涓婃柟鐨勫揩鎹锋寜閽幏鍙栧缓璁紝鎴栧湪涓嬫柟鐩存帴鍚戞垜鎻愰棶銆?);
    layout->addWidget(m_aiOutput, 1);

    // Quick Apply buttons
    QHBoxLayout* refillLayout = new QHBoxLayout();
    refillLayout->setSpacing(8);
    m_aiToResumeButton = new QPushButton("回填摘要", panelContent);
    m_aiToGoalButton = new QPushButton("杞负鐩爣", panelContent);
    m_aiToResumeButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    m_aiToGoalButton->setStyleSheet("background: #f0f2f5; color: #444; border-radius: 4px; padding: 6px; border: 1px solid #e0e0e0;");
    connect(m_aiToResumeButton, &QPushButton::clicked, this, &MainWindow::applyAiToResumeSummary);
    connect(m_aiToGoalButton, &QPushButton::clicked, this, &MainWindow::createGoalFromAiSuggestion);
    refillLayout->addWidget(m_aiToResumeButton, 1);
    refillLayout->addWidget(m_aiToGoalButton, 1);
    layout->addLayout(refillLayout);

    // Input Area
    QHBoxLayout* chatLayout = new QHBoxLayout();
    chatLayout->setSpacing(8);
    chatLayout->setContentsMargins(0, 10, 0, 0);
    m_aiChatInput = new QLineEdit(panelContent);
    m_aiChatInput->setPlaceholderText("杈撳叆闂...");
    m_aiChatInput->setStyleSheet("background: #f5f7fa; border: none; border-radius: 6px; padding: 10px; color: #333;");
    QPushButton* sendButton = new QPushButton("鍙戦€?, panelContent);
    sendButton->setStyleSheet("background: #88a7aa; color: white; border: none; border-radius: 6px; padding: 10px 16px; font-weight: bold;");
    sendButton->setCursor(Qt::PointingHandCursor);

    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendAiChat);
    connect(m_aiChatInput, &QLineEdit::returnPressed, this, &MainWindow::sendAiChat);

    chatLayout->addWidget(m_aiChatInput, 1);
    chatLayout->addWidget(sendButton);
    layout->addLayout(chatLayout);

    rootAiLayout->addWidget(panelContent, 1);

    // Start collapsed
    panelContent->hide();
    page->setFixedWidth(kAiCollapsedWidth);

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
    m_trayIcon->setToolTip("学业发展规划系统");

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
        #navList::item {
            padding: 10px 12px;
            margin: 2px 0;
            border-radius: 10px;
            color: #5d5146;
        }
        #navList::item:selected {
            background: #fffdf9;
            border: 1px solid #d7cab8;
            color: #2f261e;
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
        QComboBox::drop-down {
            border: none;
            width: 20px;
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
        setupEmptyState(m_recommendationList, "鏆傛棤建议");
    }

    m_semesterList->clear();
    const QJsonArray semesters = CourseService::getSemesterStatistics();
    for (const auto& semesterValue : semesters) {
        const QJsonObject semester = semesterValue.toObject();
        m_semesterList->addItem(
            QString("%1  路 GPA %2 路 骞冲潎鍒?%3")
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
        auto* emptyItem = new QTableWidgetItem("\n\n馃摥 鏆傛棤课程鏁版嵁\n鐐瑰嚮[鏂板课程]娣诲姞绗竴闂ㄨ绋媆n\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(12); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_courseTable->setItem(0, 0, emptyItem);
    }

    m_courseSummaryLabel->setText(
        QString("褰撳墠灞曠ず %1 / %2 闂ㄨ绋嬶紝鍏朵腑宸插畬鎴?%3 闂ㄣ€傛敮鎸佹悳绱€佺姸鎬佺瓫閫夊拰鎺掑簭锛屽苟浼氬悓姝ュ奖鍝嶆€昏銆佹椂闂磋酱涓庣畝鍘嗗鍑恒€?)
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
        QString("灞曠ず %1 / %2 涓鑹诧紝鍏朵腑杩涜涓?%3 涓€備富瑕佺被鍨嬶細%4銆傛敮鎸佹悳绱㈠拰绫诲瀷绛涢€夈€?)
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
            : QString("%1 路 %2").arg(safeText(achievement.date, "鏃ユ湡鏈～鍐?), achievement.level);
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
        setupEmptyState(m_achievementList, "鏆傛棤成果记录鏁版嵁");
    } else {
        m_achievementList->setCurrentRow(0);
    }

    const QString mainLevel = levelBreakdown.isEmpty() ? "鏈垎绾? : levelBreakdown.keys().first();
    m_achievementSummaryLabel->setText(
        QString("灞曠ず %1 / %2 椤规垚鏋滐紝宸查獙璇?%3 椤广€備富瑕佺骇鍒細%4銆傛敮鎸佹悳绱€佺被鍨嬪拰绾у埆绛涢€夈€?)
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
            experience.isOngoing ? "褰撳墠经历浠嶅湪杩涜涓€? : "璇ョ粡鍘嗛樁娈靛凡瀹屾垚銆?);
        m_experienceList->addItem(
            QString("%1\n%2 路 %3%4\n%5")
                .arg(safeText(experience.title))
                .arg(joinDateRange(experience.startDate, experience.endDate, experience.isOngoing, "鑷充粖"))
                .arg(org)
                .arg(roleText)
                .arg(body));
        m_experienceList->item(m_experienceList->count() - 1)->setData(Qt::UserRole, experience.id);
    }
    if (m_experienceList->count() == 0) {
        setupEmptyState(m_experienceList, "鏆傛棤经历妗ｆ鏁版嵁");
    } else {
        m_experienceList->setCurrentRow(0);
    }

    m_experienceSummaryLabel->setText(
        QString("灞曠ず %1 / %2 娈电粡鍘嗭紝鍏朵腑杩涜涓?%3 娈点€傛敮鎸佹悳绱㈠拰绫诲瀷绛涢€夈€?)
            .arg(filteredExperiences.size())
            .arg(experiences.size())
            .arg(stats["ongoingExperiences"].toInt()));
}

void MainWindow::refreshGoals()
{
    QList<Goal> goals = GoalService::getAll();
    const QJsonObject stats = GoalService::getStatistics();
    const QString keyword = m_goalSearchInput ? m_goalSearchInput->text().trimmed().toLower() : QString();
    const QString statusKeyword = m_goalStatusInput ? m_goalStatusInput->text().trimmed().toLower() : QString();
    const QString priorityKeyword = m_goalPriorityInput ? m_goalPriorityInput->text().trimmed().toLower() : QString();
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
        const QString deadline = safeText(goal.deadline, "鎴鏃堕棿鏈～鍐?);
        const QString body = shortBody(
            goal.description,
            QString("鐩爣鍊?%1 %2锛屽綋鍓嶅€?%3 %4銆?)
                .arg(goal.targetValue, 0, 'f', 1)
                .arg(safeText(goal.unit, ""))
                .arg(goal.currentValue, 0, 'f', 1)
                .arg(safeText(goal.unit, "")));
        m_goalList->addItem(
            QString("%1\n%2 路 鎴 %3\n%4")
                .arg(safeText(goal.title))
                .arg(progress)
                .arg(deadline)
                .arg(body));
        m_goalList->item(m_goalList->count() - 1)->setData(Qt::UserRole, goal.id);
    }
    if (m_goalList->count() == 0) {
        setupEmptyState(m_goalList, "鏆傛棤鐩爣瑙勫垝鏁版嵁");
    } else {
        m_goalList->setCurrentRow(0);
    }

    m_goalSummaryLabel->setText(
        QString("褰撳墠灞曠ず %1 / %2 涓洰鏍囷紝宸插畬鎴?%3 涓紝骞冲潎杩涘害 %4%銆傛敮鎸佹悳绱€佺瓫閫夊拰鎺掑簭銆?)
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
        m_timelineSuggestionList->addItem(QString("建议锛?1").arg(item.toString()));
    }
    if (m_timelineSuggestionList->count() == 0) {
        setupEmptyState(m_timelineSuggestionList, "鏆傛棤闃舵建议");
    }

    m_timelineSummaryLabel->setText(
        QString("鏃堕棿杞翠粠课程銆佽鑹层€佹垚鏋溿€佺粡鍘嗗拰鐩爣涓彁鍙栦簨浠讹紝骞堕厤濂楃敓鎴愰樁娈垫€у垎鏋愩€?));
}

void MainWindow::refreshResume()
{
    const QJsonObject options = currentResumeOptions();
    const QJsonObject resume = ResumeService::generate(options);
    const QByteArray htmlBytes = ResumeService::exportHtml(options);
    const QString html = QString::fromUtf8(htmlBytes);
    m_resumePreview->setHtml(html);

    const QJsonArray sections = resume["sections"].toArray();
    m_resumeSectionCountValue->setText(QString::number(sections.size()));
    m_resumeIdentityValue->setText(safeText(resume["title"].toString(), "涓汉鎴愰暱瑙勫垝绠€鍘?));
    m_resumeSummaryLabel->setText(
        QString("褰撳墠棰勮鍩轰簬 C++ 鏁版嵁灞傝嚜鍔ㄧ敓鎴愶紝鍏?%1 涓畝鍘嗗垎鍖恒€傞厤缃潰鏉裤€侀瑙堝拰瀵煎嚭鍏辩敤鍚屼竴濂楁暟鎹簮銆?)
            .arg(sections.size()));
}

void MainWindow::refreshAiStatus()
{
    const QJsonObject status = AiService::checkStatus();
    m_aiModeValue->setText(safeText(status["mode"].toString(), "unknown"));
    m_aiModelValue->setText(safeText(status["model"].toString(), "local-rule-based"));
    m_aiStatusValue->setText(status["available"].toBool() ? "鍙敤" : "涓嶅彲鐢?);
}

void MainWindow::updateBackendBadge(bool ready, const QString& detail)
{
    const QString state = ready ? "杩愯涓? : "鏈氨缁?;
    const QString extra = detail.isEmpty() ? QString() : QString(" 路 %1").arg(detail);
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
    const QString detail = QString("%1 路 %2")
        .arg(now.date().toString("yyyy-MM-dd"))
        .arg(now.time().toString("HH:mm"));

    if (m_timeSemesterLabel) {
        m_timeSemesterLabel->setText(semester);
    }
    if (m_timeDetailLabel) {
        m_timeDetailLabel->setText(QString("%1 %2").arg(now.date().toString("yyyy-MM-dd")).arg(now.time().toString("HH:mm")));
    }
    QSettings settings;
    QString profileName = settings.value("profile/name", "璇风偣鍑昏缃鍚?).toString();
    if (m_studentNameLabel) {
        m_studentNameLabel->setText(profileName);
    }
    if (m_studentMetaLabel) {
        QString sid = settings.value("profile/studentId", "鏈～鍐欏鍙?).toString();
        QString dept = settings.value("profile/department", "鏈～鍐欓櫌绯?).toString();
        m_studentMetaLabel->setText(QString("%1 路 %2\n%3").arg(sid, dept, weekday));
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
    dialog.setWindowTitle("鏂板经历");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience experience = dialog.experience();
    const Experience created = ExperienceService::create(experience);
    if (created.id == 0) {
        ToastNotification::display(this, "经历鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "经历宸插垱寤恒€?);
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
    dialog.setWindowTitle("缂栬緫经历");
    dialog.setExperience(experience);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Experience updated = dialog.experience();
    const Experience saved = ExperienceService::update(experienceId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "经历鏇存柊澶辫触銆?);
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "经历宸叉洿鏂般€?);
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
    const auto result = QMessageBox::question(this, "鍒犻櫎经历",
        QString("纭畾瑕佸垹闄ょ粡鍘嗏€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆椂闂磋酱銆佺畝鍘嗗拰 AI 鍒嗘瀽銆?).arg(title));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!ExperienceService::remove(experienceId)) {
        ToastNotification::display(this, "经历鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshExperiences();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "经历宸插垹闄ゃ€?);
}

void MainWindow::addCourse()
{
    CourseEditorDialog dialog(this);
    dialog.setWindowTitle("鏂板课程");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Course course = dialog.course();
    const Course created = CourseService::create(course);
    if (created.id == 0) {
        ToastNotification::display(this, "课程鏈兘鎴愬姛鍐欏叆鏁版嵁搴撱€?);
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "课程宸插垱寤恒€?);
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
    dialog.setWindowTitle("缂栬緫课程");
    dialog.setCourse(course);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Course updated = dialog.course();
    const Course saved = CourseService::update(courseId, updated);
    if (saved.id == 0) {
        ToastNotification::display(this, "课程鏇存柊澶辫触銆?);
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "课程宸叉洿鏂般€?);
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
        "鍒犻櫎课程",
        QString("纭畾瑕佸垹闄よ绋嬧€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆€昏銆佹椂闂磋酱鍜岀畝鍘嗗鍑恒€?).arg(courseName));
    if (result != QMessageBox::Yes) {
        return;
    }

    if (!CourseService::remove(courseId)) {
        ToastNotification::display(this, "课程鍒犻櫎澶辫触锛岃绋嶅悗鍐嶈瘯銆?);
        return;
    }

    refreshCourses();
    refreshOverview();
    refreshTimeline();
    refreshResume();
    ToastNotification::display(this, "课程宸插垹闄ゃ€?);
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
        QString("纭畾瑕佸垹闄ょ洰鏍団€?1鈥濆悧锛熸鎿嶄綔浼氬奖鍝嶆€昏銆佹椂闂磋酱鍜?AI 建议銆?).arg(title));
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
    options["email"] = m_resumeEmailInput->text().trimmed();
    options["phone"] = m_resumePhoneInput->text().trimmed();
    options["summary"] = shortBody(m_resumeSummaryInput->toPlainText(), "鍩轰簬课程銆佺粡鍘嗐€佹垚鏋滀笌鐩爣鑷姩鐢熸垚鐨勭患鍚堢畝鍘嗛瑙堛€?);
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

    m_resumeNameInput->setText(options["name"].toString());
    m_resumeTitleInput->setText(options["title"].toString());
    m_resumeEmailInput->setText(options["email"].toString());
    m_resumePhoneInput->setText(options["phone"].toString());
    m_resumeSummaryInput->setPlainText(options["summary"].toString());
    m_resumeEducationCheck->setChecked(options["includeEducation"].toBool(true));
    m_resumeExperienceCheck->setChecked(options["includeExperience"].toBool(true));
    m_resumeAchievementCheck->setChecked(options["includeAchievements"].toBool(true));
    m_resumeRoleCheck->setChecked(options["includeRoles"].toBool(true));
    m_resumeActivityCheck->setChecked(options["includeActivities"].toBool(false));
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
        ToastNotification::display(this, "璇峰厛鐢熸垚涓€娈?AI 建议銆?);
        return;
    }

    m_resumeSummaryInput->setPlainText(suggestion);
    if (m_navList) {
        m_navList->setCurrentRow(10);
    }
    refreshResume();
    ToastNotification::display(this, "宸插皢 AI 建议鍐欏叆绠€鍘嗘憳瑕侊紝浣犲彲浠ョ户缁湪绠€鍘嗛〉寰皟銆?);
}

void MainWindow::createGoalFromAiSuggestion()
{
    const QString suggestion = m_aiOutput ? m_aiOutput->toPlainText().trimmed() : QString();
    if (suggestion.isEmpty()) {
        ToastNotification::display(this, "璇峰厛鐢熸垚涓€娈?AI 建议銆?);
        return;
    }

    GoalEditorDialog dialog(this);
    dialog.setWindowTitle("浠?AI 建议鐢熸垚鐩爣");
    Goal draft;
    draft.title = "AI 建议璺熻繘鐩爣";
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
        ToastNotification::display(this, "鏈兘鏍规嵁 AI 建议鍒涘缓鐩爣銆?);
        return;
    }

    if (m_navList) {
        m_navList->setCurrentRow(6);
    }
    refreshGoals();
    refreshOverview();
    refreshTimeline();
    ToastNotification::display(this, "宸叉牴鎹?AI 建议鐢熸垚鐩爣鑽夌銆?);
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
    ToastNotification::display(this, "鉁?JSON 绠€鍘嗗凡瀵煎嚭銆?);
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
    ToastNotification::display(this, "鉁?HTML 绠€鍘嗗凡瀵煎嚭銆?);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::runAiAnalysis(const QString& type)
{
    m_aiOutput->setPlainText("鈴?姝ｅ湪鍒嗘瀽涓紝璇风◢鍊欌€︹€?);
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
        lines << "建议锛?;
        for (const auto& item : suggestions) {
            lines << QString("- %1").arg(item.toString());
        }
    } else if (result.contains("reply")) {
        lines << "";
        lines << result["reply"].toString();
    } else {
        lines << "";
        lines << "褰撳墠娌℃湁杩斿洖建议鍐呭銆?;
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

    m_aiOutput->setPlainText("鈴?AI 姝ｅ湪鎬濊€冧腑鈥︹€?);
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
    QString cat = m_activityCategoryInput ? m_activityCategoryInput->text().trimmed().toLower() : "";
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
    m_activitySummaryLabel->setText(QString("灞曠ず %1 / %2 椤规椿鍔ㄨ褰?).arg(filtered.size()).arg(list.size()));
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
    m_jobSummaryLabel->setText(QString("灞曠ず %1 / %2 椤圭洰鏍囧矖浣?).arg(filtered.size()).arg(list.size()));
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
        auto* emptyItem = new QTableWidgetItem("\n馃摥 鏆傛棤瀛︽湡瀵规瘮鏁版嵁\n娣诲姞课程鍚庤嚜鍔ㄧ敓鎴怽n");
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
        auto* emptyItem = new QTableWidgetItem("\n馃摥 鏆傛棤鍚屽瀵圭収鏁版嵁\n鐐瑰嚮[鏂板瀵圭収]娣诲姞鍚屽淇℃伅\n");
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
        ToastNotification::display(this, "鈿狅笍 璇峰厛閫夋嫨鏁版嵁婧愭枃浠讹紒"); 
        return; 
    }
    QFile file(m_importFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        ToastNotification::display(this, "鉂?鏃犳硶璇诲彇鏂囦欢锛岃妫€鏌ユ枃浠舵潈闄愩€?);
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
    
    m_importSummaryLabel->setText(QString("鏂囦欢澶勭悊瀹屾垚锛氭垚鍔熷鍏?%1 鏉″苟鍒锋柊浜嗗悇绯荤粺缂撳瓨銆?).arg(result["imported"].toInt()));
    
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
    
    ToastNotification::display(this, "鉁?鍏卞鍏ヤ簡 " + QString::number(result["imported"].toInt()) + " 鏉℃暟鎹紝宸茶Е鍙戝叏绯荤粺鏁版嵁鍒锋柊銆?);
}


void MainWindow::onBackendStarted()
{
    m_serverReady = true;
    m_statusLabel->setText("鍚庣鏈嶅姟杩愯涓紝鍘熺敓椤甸潰宸插彲鐩存帴璇诲彇鏁版嵁銆?);
    updateBackendBadge(true, "绔彛 5000");
    m_progressBar->hide();

    if (m_trayIcon) {
        m_trayIcon->showMessage("学业发展规划系统", "C++ 鍚庣宸插惎鍔紝鍙互浣跨敤鍘熺敓鐣岄潰鎴栫綉椤甸瑙堛€?);
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
        m_aiOutput->setPlainText("鐐瑰嚮鈥滅患鍚堝垎鏋?/ 课程建议 / 经历建议 / 鐩爣建议鈥濓紝鎴栫洿鎺ュ湪涓嬫柟杈撳叆闂銆?);
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
        "学业发展规划系统 - Qt 妗岄潰鐗圽n\n"
        "褰撳墠闃舵锛歕n"
        "1. C++ 鍚庣宸茬嫭绔嬭繍琛孿n"
        "2. 涔濅釜鏍稿績椤甸潰閮藉凡鏈夊師鐢?Qt 鐣岄潰\n"
        "3. 褰撳墠缁х画娌跨敤鍓嶇鐨勫崱鐗囧紡鐭ヨ瘑搴撹璁￠€昏緫\n"
        "4. 鍘熺綉椤靛墠绔繚鐣欎负杈呭姪棰勮涓庡鐓х増鏈?);
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
    hide();
    event->ignore();

    if (m_trayIcon) {
        m_trayIcon->showMessage("学业发展规划系统", "绋嬪簭宸叉渶灏忓寲鍒扮郴缁熸墭鐩樸€?);
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget* strip = qobject_cast<QWidget*>(watched);
        if (strip && strip->objectName() == "aiCollapsedStrip") {
            QWidget* aiSidebar = this->findChild<QWidget*>("aiSidebar");
            if (aiSidebar) {
                QWidget* panel = aiSidebar->findChild<QWidget*>("aiPanelContent");
                strip->hide();
                if (panel) { panel->show(); panel->setFixedWidth(kAiSidebarWidth); }
                aiSidebar->setMaximumWidth(kAiSidebarWidth);
                aiSidebar->setMinimumWidth(kAiCollapsedWidth);
                QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
                auto* animMin = new QPropertyAnimation(aiSidebar, "minimumWidth");
                animMin->setDuration(240); animMin->setStartValue(kAiCollapsedWidth); animMin->setEndValue(kAiSidebarWidth);
                animMin->setEasingCurve(QEasingCurve::InOutQuad);
                group->addAnimation(animMin);
                connect(group, &QParallelAnimationGroup::finished, group, &QObject::deleteLater);
                connect(group, &QParallelAnimationGroup::finished, [aiSidebar]() {
                    aiSidebar->setFixedWidth(kAiSidebarWidth);
                });
                group->start();
            }
            return true;
        }
    }

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
    if (!server.start(5000)) {
        emit serverError("鏃犳硶缁戝畾绔彛 5000");
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
