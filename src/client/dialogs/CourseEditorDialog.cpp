#include "client/dialogs/CourseEditorDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "client/widgets/ToastNotification.h"

CourseEditorDialog::CourseEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("课程编辑"));
    resize(520, 640);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* intro = new QLabel(QStringLiteral("在 Qt 原生界面中直接维护课程数据，保存后会同步刷新总览、时间轴和简历。"), this);
    intro->setWordWrap(true);
    intro->setObjectName(QStringLiteral("pageSubtitle"));
    layout->addWidget(intro);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_codeEdit = new QLineEdit(this);
    m_semesterEdit = new QLineEdit(this);
    m_semesterEdit->setPlaceholderText(QStringLiteral("例如 2026-春季"));

    m_creditsSpin = new QDoubleSpinBox(this);
    m_creditsSpin->setRange(0.0, 40.0);
    m_creditsSpin->setDecimals(1);
    m_creditsSpin->setSingleStep(0.5);

    m_scoreSpin = new QDoubleSpinBox(this);
    m_scoreSpin->setRange(0.0, 100.0);
    m_scoreSpin->setDecimals(1);
    m_scoreSpin->setSingleStep(1.0);
    m_scoreSpin->setSpecialValueText(QStringLiteral("未填写"));

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems({QStringLiteral("Required"), QStringLiteral("Elective"), QStringLiteral("General"), QStringLiteral("Other")});

    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItems({QStringLiteral("Planned"), QStringLiteral("In Progress"), QStringLiteral("Completed")});

    m_teacherEdit = new QLineEdit(this);
    m_locationEdit = new QLineEdit(this);
    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText(QStringLiteral("用逗号分隔，例如：核心课，数据结构"));

    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setPlaceholderText(QStringLiteral("记录课程重点、个人收获或补充说明"));
    m_descriptionEdit->setMinimumHeight(120);
    m_descriptionEdit->setObjectName(QStringLiteral("richCardText"));

    form->addRow(QStringLiteral("课程名称"), m_nameEdit);
    form->addRow(QStringLiteral("课程代码"), m_codeEdit);
    form->addRow(QStringLiteral("学期"), m_semesterEdit);
    form->addRow(QStringLiteral("学分"), m_creditsSpin);
    form->addRow(QStringLiteral("分数"), m_scoreSpin);
    form->addRow(QStringLiteral("课程类别"), m_categoryCombo);
    form->addRow(QStringLiteral("状态"), m_statusCombo);
    form->addRow(QStringLiteral("授课教师"), m_teacherEdit);
    form->addRow(QStringLiteral("上课地点"), m_locationEdit);
    form->addRow(QStringLiteral("标签"), m_tagsEdit);
    form->addRow(QStringLiteral("课程说明"), m_descriptionEdit);
    layout->addLayout(form);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { validateAndAccept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void CourseEditorDialog::setCourse(const Course& course)
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

Course CourseEditorDialog::course() const
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

void CourseEditorDialog::setComboValue(QComboBox* combo, const QString& value)
{
    const int index = combo->findText(value);
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}

void CourseEditorDialog::validateAndAccept()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        ToastNotification::display(this, QStringLiteral("请先填写课程名称。"));
        return;
    }
    if (m_creditsSpin->value() <= 0.0) {
        ToastNotification::display(this, QStringLiteral("学分需要大于 0。"));
        return;
    }
    if (m_statusCombo->currentText() == QStringLiteral("Completed") && m_scoreSpin->value() <= 0.0) {
        const auto result = QMessageBox::question(
            this,
            QStringLiteral("缺少成绩"),
            QStringLiteral("当前课程标记为已完成，但尚未填写分数。是否仍然继续保存？"));
        if (result != QMessageBox::Yes) {
            return;
        }
    }
    accept();
}
