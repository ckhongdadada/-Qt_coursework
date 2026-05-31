#include "ResumeEditorPanel.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>

ResumeEditorPanel::ResumeEditorPanel(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("contentCard");
    setMinimumWidth(480);
    setMaximumWidth(860);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setupUi();
}

void ResumeEditorPanel::setupUi()
{
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget* content = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(12);

    QLabel* configTitle = new QLabel("简历配置", content);
    configTitle->setObjectName("sectionTitle");
    layout->addWidget(configTitle);

    QLabel* configHint = new QLabel("这组字段就是简历页面的单一数据源。预览、导出与后续 AI 优化都会基于这里的配置。", content);
    configHint->setObjectName("pageSubtitle");
    configHint->setWordWrap(true);
    layout->addWidget(configHint);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(10);

    m_nameInput = new QLineEdit(content);
    m_titleInput = new QLineEdit(content);
    m_emailInput = new QLineEdit(content);
    m_phoneInput = new QLineEdit(content);
    m_ageInput = new QLineEdit(content);
    m_cityInput = new QLineEdit(content);
    m_intentInput = new QLineEdit(content);
    m_schoolInput = new QLineEdit(content);
    m_majorInput = new QLineEdit(content);
    m_degreeInput = new QLineEdit(content);
    m_summaryInput = new QTextEdit(content);
    m_customContentInput = new QTextEdit(content);
    m_educationBodyInput = new QTextEdit(content);
    m_skillsBodyInput = new QTextEdit(content);
    m_projectNameInput = new QLineEdit(content);
    m_projectBodyInput = new QTextEdit(content);
    m_internshipInput = new QTextEdit(content);
    m_awardsInput = new QTextEdit(content);

    m_summaryInput->setObjectName("richCardText");
    m_customContentInput->setObjectName("richCardText");
    m_educationBodyInput->setObjectName("richCardText");
    m_skillsBodyInput->setObjectName("richCardText");
    m_projectBodyInput->setObjectName("richCardText");
    m_internshipInput->setObjectName("richCardText");
    m_awardsInput->setObjectName("richCardText");

    m_summaryInput->setMinimumHeight(80);
    m_customContentInput->setMinimumHeight(80);
    m_educationBodyInput->setMinimumHeight(80);
    m_skillsBodyInput->setMinimumHeight(80);
    m_projectBodyInput->setMinimumHeight(80);
    m_internshipInput->setMinimumHeight(60);
    m_awardsInput->setMinimumHeight(60);

    m_summaryInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_customContentInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_educationBodyInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_skillsBodyInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_projectBodyInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_internshipInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_awardsInput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_summaryInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_customContentInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_educationBodyInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_skillsBodyInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_projectBodyInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_internshipInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_awardsInput->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    m_nameInput->setPlaceholderText("例如：张三");
    m_titleInput->setPlaceholderText("例如：个人成长规划简历");
    m_emailInput->setPlaceholderText("例如：name@example.com");
    m_phoneInput->setPlaceholderText("例如：13800000000");
    m_ageInput->setPlaceholderText("例如：21岁");
    m_cityInput->setPlaceholderText("例如：北京");
    m_intentInput->setPlaceholderText("例如：大数据工程师");
    m_schoolInput->setPlaceholderText("例如：对外经济贸易大学");
    m_majorInput->setPlaceholderText("例如：数据科学与大数据技术");
    m_degreeInput->setPlaceholderText("例如：本科");
    m_summaryInput->setPlaceholderText("用 2-4 句话概括你的学习方向、实践重点和成长亮点。");
    m_customContentInput->setPlaceholderText("这里用于手动补充简历亮点。右侧候选素材支持点击插入，你也可以直接编辑。");
    m_educationBodyInput->setPlaceholderText("例如：• GPA：85/100，专业排名前列\n• 核心课程：机器学习、数据分析、数据库系统");
    m_skillsBodyInput->setPlaceholderText("例如：• 熟悉 Python、C++、SQL 与数据分析流程");
    m_projectNameInput->setPlaceholderText("例如：个人发展规划系统");
    m_projectBodyInput->setPlaceholderText("例如：• 负责系统设计、模块拆分与界面实现");
    m_internshipInput->setPlaceholderText("实习经历描述...");
    m_awardsInput->setPlaceholderText("竞赛获奖描述...");

    formLayout->addRow("姓名", m_nameInput);
    formLayout->addRow("身份标题", m_titleInput);
    formLayout->addRow("邮箱", m_emailInput);
    formLayout->addRow("电话", m_phoneInput);
    formLayout->addRow("年龄", m_ageInput);
    formLayout->addRow("城市", m_cityInput);
    formLayout->addRow("求职意向", m_intentInput);
    formLayout->addRow("学校", m_schoolInput);
    formLayout->addRow("专业", m_majorInput);
    formLayout->addRow("学历", m_degreeInput);
    formLayout->addRow("个人摘要", m_summaryInput);
    formLayout->addRow("教育背景", m_educationBodyInput);
    formLayout->addRow("技能特长", m_skillsBodyInput);
    formLayout->addRow("项目名称", m_projectNameInput);
    formLayout->addRow("项目经验", m_projectBodyInput);
    formLayout->addRow("实习经历", m_internshipInput);
    formLayout->addRow("竞赛获奖", m_awardsInput);
    formLayout->addRow("补充内容", m_customContentInput);
    layout->addLayout(formLayout);

    QLabel* sectionHint = new QLabel("分区开关", content);
    sectionHint->setObjectName("sectionTitle");
    layout->addWidget(sectionHint);

    m_educationCheck = new QCheckBox("教育经历", content);
    m_experienceCheck = new QCheckBox("实践经历", content);
    m_achievementCheck = new QCheckBox("成果记录", content);
    m_roleCheck = new QCheckBox("角色任职", content);
    m_activityCheck = new QCheckBox("活动参与", content);
    layout->addWidget(m_educationCheck);
    layout->addWidget(m_experienceCheck);
    layout->addWidget(m_achievementCheck);
    layout->addWidget(m_roleCheck);
    layout->addWidget(m_activityCheck);
    layout->addStretch();

    scroll->setWidget(content);
    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);

    auto emitChanged = [this]() { emit optionsChanged(); };
    connect(m_nameInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_titleInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_emailInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_phoneInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_ageInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_cityInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_intentInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_schoolInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_majorInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_degreeInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_projectNameInput, &QLineEdit::textChanged, this, emitChanged);
    connect(m_summaryInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_customContentInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_educationBodyInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_skillsBodyInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_projectBodyInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_internshipInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_awardsInput, &QTextEdit::textChanged, this, emitChanged);
    connect(m_educationCheck, &QCheckBox::checkStateChanged, this, emitChanged);
    connect(m_experienceCheck, &QCheckBox::checkStateChanged, this, emitChanged);
    connect(m_achievementCheck, &QCheckBox::checkStateChanged, this, emitChanged);
    connect(m_roleCheck, &QCheckBox::checkStateChanged, this, emitChanged);
    connect(m_activityCheck, &QCheckBox::checkStateChanged, this, emitChanged);
}

void ResumeEditorPanel::loadOptions(const QJsonObject& options)
{
    const QSignalBlocker b1(m_nameInput);
    const QSignalBlocker b2(m_titleInput);
    const QSignalBlocker b3(m_emailInput);
    const QSignalBlocker b4(m_phoneInput);
    const QSignalBlocker b5(m_summaryInput);
    const QSignalBlocker b6(m_customContentInput);
    const QSignalBlocker b7(m_ageInput);
    const QSignalBlocker b8(m_cityInput);
    const QSignalBlocker b9(m_intentInput);
    const QSignalBlocker b10(m_schoolInput);
    const QSignalBlocker b11(m_majorInput);
    const QSignalBlocker b12(m_degreeInput);
    const QSignalBlocker b13(m_educationBodyInput);
    const QSignalBlocker b14(m_skillsBodyInput);
    const QSignalBlocker b15(m_projectNameInput);
    const QSignalBlocker b16(m_projectBodyInput);
    const QSignalBlocker b17(m_educationCheck);
    const QSignalBlocker b18(m_experienceCheck);
    const QSignalBlocker b19(m_achievementCheck);
    const QSignalBlocker b20(m_roleCheck);
    const QSignalBlocker b21(m_activityCheck);
    const QSignalBlocker b22(m_internshipInput);
    const QSignalBlocker b23(m_awardsInput);

    m_nameInput->setText(options["name"].toString());
    m_titleInput->setText(options["title"].toString());
    m_ageInput->setText(options["age"].toString());
    m_cityInput->setText(options["city"].toString());
    m_emailInput->setText(options["email"].toString());
    m_phoneInput->setText(options["phone"].toString());
    m_intentInput->setText(options["intent"].toString());
    m_schoolInput->setText(options["school"].toString());
    m_majorInput->setText(options["major"].toString());
    m_degreeInput->setText(options["degree"].toString());
    m_summaryInput->setPlainText(options["summary"].toString());
    m_educationBodyInput->setPlainText(options["educationBody"].toString());
    m_skillsBodyInput->setPlainText(options["skillsBody"].toString());
    m_projectNameInput->setText(options["projectName"].toString());
    m_projectBodyInput->setPlainText(options["projectBody"].toString());
    m_internshipInput->setPlainText(options["internship"].toString());
    m_awardsInput->setPlainText(options["awards"].toString());
    m_customContentInput->setPlainText(options["customContent"].toString());
    m_educationCheck->setChecked(options["includeEducation"].toBool(true));
    m_experienceCheck->setChecked(options["includeExperience"].toBool(true));
    m_achievementCheck->setChecked(options["includeAchievements"].toBool(true));
    m_roleCheck->setChecked(options["includeRoles"].toBool(true));
    m_activityCheck->setChecked(options["includeActivities"].toBool(false));
}

QJsonObject ResumeEditorPanel::collectOptions() const
{
    QJsonObject options;
    options["name"] = m_nameInput->text().trimmed();
    options["title"] = m_titleInput->text().trimmed();
    options["age"] = m_ageInput->text().trimmed();
    options["city"] = m_cityInput->text().trimmed();
    options["email"] = m_emailInput->text().trimmed();
    options["phone"] = m_phoneInput->text().trimmed();
    options["intent"] = m_intentInput->text().trimmed();
    options["school"] = m_schoolInput->text().trimmed();
    options["major"] = m_majorInput->text().trimmed();
    options["degree"] = m_degreeInput->text().trimmed();
    options["summary"] = m_summaryInput->toPlainText().trimmed();
    options["educationBody"] = m_educationBodyInput->toPlainText().trimmed();
    options["skillsBody"] = m_skillsBodyInput->toPlainText().trimmed();
    options["projectName"] = m_projectNameInput->text().trimmed();
    options["projectBody"] = m_projectBodyInput->toPlainText().trimmed();
    options["internship"] = m_internshipInput->toPlainText().trimmed();
    options["awards"] = m_awardsInput->toPlainText().trimmed();
    options["customContent"] = m_customContentInput->toPlainText().trimmed();
    options["includeEducation"] = m_educationCheck->isChecked();
    options["includeExperience"] = m_experienceCheck->isChecked();
    options["includeAchievements"] = m_achievementCheck->isChecked();
    options["includeRoles"] = m_roleCheck->isChecked();
    options["includeActivities"] = m_activityCheck->isChecked();
    return options;
}

void ResumeEditorPanel::setSelectedSection(const QString& key)
{
    emit sectionChanged(key);
}
