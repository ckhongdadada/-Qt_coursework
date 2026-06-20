#include "CoursesPage.h"
#include "dialogs/CourseEditorDialog.h"
#include "service/CourseService.h"
#include "widgets/ToastNotification.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QHeaderView>

CoursesPage::CoursesPage(QWidget* parent)
    : BasePage(parent)
{
    setupUi();
    refresh();
}

void CoursesPage::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    QLabel* title = new QLabel("课程库", this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    m_summaryLabel = new QLabel("正在读取课程数据...", this);
    m_summaryLabel->setObjectName("pageSubtitle");
    layout->addWidget(m_summaryLabel);

    QFrame* filterCard = new QFrame(this);
    filterCard->setObjectName("contentCard");
    QGridLayout* filterLayout = new QGridLayout(filterCard);
    filterLayout->setContentsMargins(12, 12, 12, 12);
    filterLayout->setHorizontalSpacing(10);
    filterLayout->setVerticalSpacing(10);

    m_searchInput = new QLineEdit(filterCard);
    m_searchInput->setPlaceholderText("搜索课程名称 / 代码 / 教师");
    m_statusInput = new QLineEdit(filterCard);
    m_statusInput->setPlaceholderText("状态过滤，例如 Completed");
    m_categoryInput = new QLineEdit(filterCard);
    m_categoryInput->setPlaceholderText("类别过滤，例如 Required");
    m_sortInput = new QLineEdit(filterCard);
    m_sortInput->setPlaceholderText("排序：updated / semester / credits / score / gpa / name");

    filterLayout->addWidget(m_searchInput, 0, 0);
    filterLayout->addWidget(m_statusInput, 0, 1);
    filterLayout->addWidget(m_categoryInput, 1, 0);
    filterLayout->addWidget(m_sortInput, 1, 1);

    connect(m_searchInput, &QLineEdit::textChanged, this, &CoursesPage::refresh);
    connect(m_statusInput, &QLineEdit::textChanged, this, &CoursesPage::refresh);
    connect(m_categoryInput, &QLineEdit::textChanged, this, &CoursesPage::refresh);
    connect(m_sortInput, &QLineEdit::textChanged, this, &CoursesPage::refresh);
    layout->addWidget(filterCard);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    QPushButton* addButton = new QPushButton("新增课程", this);
    QPushButton* editButton = new QPushButton("编辑选中课程", this);
    QPushButton* removeButton = new QPushButton("删除选中课程", this);
    removeButton->setProperty("danger", true);

    connect(addButton, &QPushButton::clicked, this, &CoursesPage::onAddClicked);
    connect(editButton, &QPushButton::clicked, this, &CoursesPage::onEditClicked);
    connect(removeButton, &QPushButton::clicked, this, &CoursesPage::onRemoveClicked);
    addButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    QFrame* tableCard = new QFrame(this);
    tableCard->setObjectName("contentCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(12, 12, 12, 12);
    cardLayout->setSpacing(10);

    QLabel* helper = new QLabel("课程库现在支持原生 Qt 录入与编辑。双击任意一行也可以直接打开编辑弹窗。", tableCard);
    helper->setObjectName("pageSubtitle");
    helper->setWordWrap(true);
    cardLayout->addWidget(helper);

    m_table = new QTableWidget(tableCard);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"课程名称", "代码", "学期", "学分", "分数", "绩点", "状态"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &CoursesPage::onCellDoubleClicked);
    cardLayout->addWidget(m_table);

    layout->addWidget(tableCard, 1);
}

void CoursesPage::refresh()
{
    QList<Course> courses = CourseService::getAll();
    const QString keyword = m_searchInput ? m_searchInput->text().trimmed().toLower() : QString();
    const QString statusKeyword = m_statusInput ? m_statusInput->text().trimmed().toLower() : QString();
    const QString categoryKeyword = m_categoryInput ? m_categoryInput->text().trimmed().toLower() : QString();
    const QString sortKey = m_sortInput ? m_sortInput->text().trimmed().toLower() : QString("updated");

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

    m_table->setRowCount(filteredCourses.size());

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
            m_table->setItem(row, column, item);
        }
    }

    if (!filteredCourses.isEmpty()) {
        m_table->selectRow(0);
    } else {
        m_table->setRowCount(1);
        m_table->setSpan(0, 0, 1, 7);
        auto* emptyItem = new QTableWidgetItem("\n\n暂无课程数据\n点击[新增课程]添加第一门课程\n");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        QFont f = emptyItem->font(); f.setPointSize(12); emptyItem->setFont(f);
        emptyItem->setForeground(QBrush(QColor("#a8a096")));
        emptyItem->setFlags(Qt::NoItemFlags);
        m_table->setItem(0, 0, emptyItem);
    }

    m_summaryLabel->setText(
        QString("当前显示 %1 / %2 门课程，其中已完成 %3 门。支持搜索、状态筛选和排序，并会同步影响总览、时间轴与简历导出。")
            .arg(filteredCourses.size())
            .arg(courses.size())
            .arg(completedCount));
}

void CoursesPage::onAddClicked()
{
    CourseEditorDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Course course = dlg.course();
        CourseService::create(course);
        refresh();
        emit dataChanged(DataDomain::Courses);
        ToastNotification::display(this, "课程已添加。");
    }
}

void CoursesPage::onEditClicked()
{
    int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0)) {
        ToastNotification::display(this, "请先选择一门课程。");
        return;
    }
    int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    Course course = CourseService::getById(id);
    CourseEditorDialog dlg(this);
    dlg.setCourse(course);
    if (dlg.exec() == QDialog::Accepted) {
        Course updated = dlg.course();
        updated.id = id;
        CourseService::update(id, updated);
        refresh();
        emit dataChanged(DataDomain::Courses);
        ToastNotification::display(this, "课程已更新。");
    }
}

void CoursesPage::onRemoveClicked()
{
    int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0)) {
        ToastNotification::display(this, "请先选择一门课程。");
        return;
    }
    int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this, "删除课程", "确定要删除这门课程吗？") == QMessageBox::Yes) {
        CourseService::remove(id);
        refresh();
        emit dataChanged(DataDomain::Courses);
        ToastNotification::display(this, "课程已删除。");
    }
}

void CoursesPage::onCellDoubleClicked(int row, int col)
{
    Q_UNUSED(col);
    if (row >= 0 && m_table->item(row, 0)) {
        onEditClicked();
    }
}
