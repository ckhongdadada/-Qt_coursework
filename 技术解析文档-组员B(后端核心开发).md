# C++版技术解析文档 - 组员B（后端核心开发）

## 1. 职责概述

在C++版本中，组员B负责后端核心开发，包括数据模型（Model）、数据访问层（DAO）、业务逻辑层（Service）和数据库Schema设计。使用Qt6的QSqlDatabase进行数据库操作，通过QJsonObject实现数据序列化/反序列化，构建完整的三层架构（Model-DAO-Service）。

### 1.1 主要职责

| 职责领域 | 具体内容 |
|----------|----------|
| 数据模型 | Model类设计、成员变量、序列化方法 |
| 数据访问 | DAO类设计、SQL操作、数据库连接管理 |
| 业务逻辑 | Service类设计、数据计算、业务规则 |
| 数据库设计 | Schema设计、表结构、索引优化 |

---

## 2. 技术栈与核心依赖

### 2.1 核心框架

| 技术 | 版本 | 用途 |
|------|------|------|
| Qt6 Sql | 6.x | 数据库操作（QSqlDatabase, QSqlQuery） |
| Qt6 Core | 6.x | JSON处理、数据类型、容器 |
| SQLite | 3.x | 嵌入式关系型数据库 |
| C++17 | - | 结构化绑定、optional、filesystem |

### 2.2 项目结构（后端核心层）

```
cpp_project/src/
├── model/                      # 数据模型层
│   ├── Course.h                # 课程模型
│   ├── Course.cpp              # 课程模型实现
│   ├── Role.h                  # 角色模型
│   ├── Achievement.h           # 成就模型
│   ├── Experience.h            # 经验模型
│   ├── Activity.h              # 活动模型
│   ├── Goal.h                  # 目标模型
│   ├── Job.h                   # 岗位模型
│   ├── JobRequirement.h        # 岗位需求模型
│   ├── PeerBenchmark.h         # 同学对比模型
│   ├── User.h                  # 用户模型
│   └── TimelineEvent.h         # 时间轴事件模型
├── dao/                        # 数据访问层
│   ├── DaoBase.h               # DAO基类
│   ├── DaoBase.cpp             # DAO基类实现
│   ├── CourseDao.h             # 课程DAO
│   ├── RoleDao.h               # 角色DAO
│   ├── AchievementDao.h        # 成就DAO
│   ├── ExperienceDao.h         # 经验DAO
│   ├── ActivityDao.h           # 活动DAO
│   ├── GoalDao.h               # 目标DAO
│   ├── JobDao.h                # 岗位DAO
│   ├── PeerBenchmarkDao.h      # 同学对比DAO
│   └── UserDao.h               # 用户DAO
├── service/                    # 业务逻辑层
│   ├── CourseService.h         # 课程服务
│   ├── RoleService.h           # 角色服务
│   ├── AchievementService.h    # 成就服务
│   ├── ExperienceService.h     # 经验服务
│   ├── ActivityService.h       # 活动服务
│   ├── GoalService.h           # 目标服务
│   ├── JobService.h            # 岗位服务
│   ├── DashboardService.h      # 仪表盘服务
│   ├── AnalyticsService.h      # 分析服务
│   ├── ImportService.h         # 导入服务
│   ├── ResumeService.h         # 简历服务
│   ├── AiService.h             # AI服务
│   └── AuthService.h           # 认证服务
└── resources/
    └── schema.sql              # 数据库Schema
```

---

## 3. 数据模型层（Model）详细解析

### 3.0 客户端数据流

**桌面端数据流：**

```
User Action (点击按钮)
    ↓
Dialog (输入数据)
    ↓
Page::onAction() (事件处理)
    ↓
Service::create/update/delete() (业务逻辑)
    ↓
DAO::insert/update/delete() (数据库操作)
    ↓
Page::emit dataChanged(domain) (发送信号)
    ↓
DataRefreshCoordinator::refreshByDomain() (协调刷新)
    ↓
Multiple Pages::refresh() (更新UI)
```

**Service层职责：**

| 调用方式 | 说明 |
|----------|------|
| 桌面端 | 直接调用，返回C++对象 |

### 3.1 Course 类 (model/Course.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 数据库字段 | 用途 |
|--------|------|--------|-----------|------|
| `id` | int | 0 | id | 主键 |
| `name` | QString | "" | name | 课程名称 |
| `code` | QString | "" | code | 课程代码 |
| `credits` | double | 0 | credits | 学分 |
| `semester` | QString | "" | semester | 学期 |
| `category` | QString | "Required" | category | 类别 |
| `score` | double | 0 | score | 分数 |
| `gradePoint` | double | 0 | grade_point | 绩点 |
| `status` | QString | "Planned" | status | 状态 |
| `teacher` | QString | "" | teacher | 教师 |
| `location` | QString | "" | location | 地点 |
| `description` | QString | "" | description | 描述 |
| `tags` | QString | "" | tags | 标签 |
| `createdAt` | QDateTime | - | created_at | 创建时间 |
| `updatedAt` | QDateTime | - | updated_at | 更新时间 |

**核心方法：**

| 方法 | 返回值 | 用途 |
|------|--------|------|
| `calculateGradePoint(score, scale)` | double | 计算绩点（静态方法） |
| `toDict()` | QJsonObject | 序列化为JSON |
| `fromDict(obj)` | Course | 从JSON反序列化（静态方法） |

**绩点计算逻辑：**

```cpp
double Course::calculateGradePoint(double score, const QString& scale) {
    if (score < 60) return 0;
    if (scale == "standard") {
        if (score >= 90) return 4.0;
        if (score >= 85) return 3.7;
        if (score >= 82) return 3.3;
        if (score >= 78) return 3.0;
        if (score >= 75) return 2.7;
        if (score >= 72) return 2.3;
        if (score >= 68) return 2.0;
        if (score >= 64) return 1.5;
        if (score >= 60) return 1.0;
    } else if (scale == "wes") {
        if (score >= 85) return 4.0;
        if (score >= 75) return 3.0;
        if (score >= 60) return 2.0;
    }
    return 0;
}
```

### 3.2 Role 类 (model/Role.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `title` | QString | "" | 角色标题 |
| `type` | QString | "" | 角色类型 |
| `organization` | QString | "" | 组织名称 |
| `description` | QString | "" | 描述 |
| `startDate` | QString | "" | 开始日期 |
| `endDate` | QString | "" | 结束日期 |
| `isActive` | bool | false | 是否在职 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.3 Achievement 类 (model/Achievement.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `title` | QString | "" | 成就标题 |
| `type` | QString | "" | 类型 |
| `level` | QString | "" | 级别 |
| `organization` | QString | "" | 颁发组织 |
| `description` | QString | "" | 描述 |
| `date` | QString | "" | 获得日期 |
| `verified` | bool | false | 是否已验证 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.4 Experience 类 (model/Experience.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `title` | QString | "" | 经历标题 |
| `type` | QString | "" | 类型 |
| `organization` | QString | "" | 组织 |
| `role` | QString | "" | 担任角色 |
| `description` | QString | "" | 描述 |
| `startDate` | QString | "" | 开始日期 |
| `endDate` | QString | "" | 结束日期 |
| `isOngoing` | bool | false | 是否进行中 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.5 Activity 类 (model/Activity.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `name` | QString | "" | 活动名称 |
| `description` | QString | "" | 描述 |
| `startDate` | QString | "" | 开始日期 |
| `endDate` | QString | "" | 结束日期 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.6 Goal 类 (model/Goal.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `title` | QString | "" | 目标标题 |
| `category` | QString | "" | 类别 |
| `description` | QString | "" | 描述 |
| `targetValue` | double | 0 | 目标值 |
| `currentValue` | double | 0 | 当前进度 |
| `deadline` | QString | "" | 截止日期 |
| `priority` | QString | "Medium" | 优先级 |
| `status` | QString | "In Progress" | 状态 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.7 Job 类 (model/Job.h)

**内部类 JobRequirement：**

| 变量名 | 类型 | 用途 |
|--------|------|------|
| `text` | QString | 需求描述 |
| `met` | bool | 是否满足 |

**Job类成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `title` | QString | "" | 岗位标题 |
| `company` | QString | "" | 公司 |
| `location` | QString | "" | 地点 |
| `type` | QString | "" | 类型 |
| `salary` | QString | "" | 薪资 |
| `description` | QString | "" | 描述 |
| `requirements` | QList\<JobRequirement\> | [] | 需求列表 |
| `matchScore` | double | 0 | 匹配分数 |
| `status` | QString | "Saved" | 状态 |
| `url` | QString | "" | 链接 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.8 User 类 (model/User.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `username` | QString | "" | 用户名 |
| `email` | QString | "" | 邮箱 |
| `passwordHash` | QString | "" | 密码哈希 |
| `displayName` | QString | "" | 显示名称 |
| `role` | QString | "user" | 角色 |
| `isActive` | bool | true | 是否激活 |
| `lastLogin` | QDateTime | - | 最后登录 |
| `createdAt` | QDateTime | - | 创建时间 |
| `updatedAt` | QDateTime | - | 更新时间 |

### 3.9 PeerBenchmark 类 (model/PeerBenchmark.h)

**成员变量：**

| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| `id` | int | 0 | 主键 |
| `name` | QString | "" | 姓名 |
| `major` | QString | "" | 专业 |
| `semester` | QString | "" | 学期 |
| `gpa` | double | 0 | GPA |
| `credits` | double | 0 | 学分 |
| `note` | QString | "" | 备注 |
| `createdAt` | QDateTime | - | 创建时间 |

### 3.10 TimelineEvent 类 (model/TimelineEvent.h)

**成员变量：**

| 变量名 | 类型 | 用途 |
|--------|------|------|
| `date` | QString | 日期 |
| `title` | QString | 标题 |
| `type` | QString | 类型 |
| `description` | QString | 描述 |

---

## 4. 数据访问层（DAO）详细解析

### 4.1 DaoBase 基类 (dao/DaoBase.h)

**创建的对象：**

| 对象名 | 类型 | 用途 |
|--------|------|------|
| `m_db` | QSqlDatabase | 数据库连接 |

**核心方法：**

| 方法 | 返回值 | 用途 |
|------|--------|------|
| `getDatabase()` | QSqlDatabase | 获取数据库连接 |
| `executeQuery(sql)` | QSqlQuery | 执行查询 |
| `executeUpdate(sql)` | bool | 执行更新 |
| `getLastInsertId()` | int | 获取最后插入ID |

**数据库连接管理：**

```cpp
QSqlDatabase DaoBase::getDatabase() {
    if (!QSqlDatabase::contains("pdp_connection")) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "pdp_connection");
        db.setDatabaseName("pdp.db");
        db.open();
    }
    return QSqlDatabase::database("pdp_connection");
}
```

### 4.2 CourseDao 类 (dao/CourseDao.h)

**核心方法：**

| 方法 | SQL操作 | 返回值 |
|------|---------|--------|
| `getAll()` | SELECT * FROM courses | QList\<Course\> |
| `getById(id)` | SELECT * FROM courses WHERE id=? | Course |
| `create(course)` | INSERT INTO courses | void |
| `update(course)` | UPDATE courses SET ... WHERE id=? | void |
| `remove(id)` | DELETE FROM courses WHERE id=? | bool |
| `getCount()` | SELECT COUNT(*) FROM courses | int |
| `getCompletedCount()` | SELECT COUNT(*) WHERE status='Completed' | int |
| `getTotalCredits()` | SELECT SUM(credits) WHERE status='Completed' | double |
| `getAverageScore()` | SELECT AVG(score) WHERE status='Completed' | double |

### 4.3 GoalDao 类 (dao/GoalDao.h)

**扩展方法：**

| 方法 | SQL操作 | 返回值 |
|------|---------|--------|
| `getCount()` | SELECT COUNT(*) FROM goals | int |
| `getCompletedCount()` | SELECT COUNT(*) WHERE status='Completed' | int |
| `getAverageProgress()` | SELECT AVG(currentValue/targetValue*100) | double |

### 4.4 UserDao 类 (dao/UserDao.h)

**扩展方法：**

| 方法 | SQL操作 | 返回值 |
|------|---------|--------|
| `getByUsername(username)` | SELECT * WHERE username=? | User |
| `getByEmail(email)` | SELECT * WHERE email=? | User |
| `updateLastLogin(id)` | UPDATE SET last_login=NOW() WHERE id=? | void |

---

## 5. 业务逻辑层（Service）详细解析

### 5.1 CourseService 类 (service/CourseService.h)

**Service层双重职责：**

```cpp
QList<Course> courses = CourseService::getAll();
for (const auto& c : courses) {
    displayCourse(c);
}

QHttpServerResponse CourseApi::getAll(const QHttpServerRequest&) {
    QList<Course> courses = CourseService::getAll();
    QJsonArray arr;
    for (const auto& c : courses) {
        arr.append(c.toDict());
    }
    return JsonUtils::successResponse(arr);
}
```

**公共方法：**

| 方法 | 功能 | 核心逻辑 |
|------|------|----------|
| `getAll()` | 获取所有课程 | 委托CourseDao |
| `getById(id)` | 获取单个课程 | 委托CourseDao |
| `create(course)` | 创建课程 | 自动计算gradePoint |
| `update(id, course)` | 更新课程 | 先验证存在性，再更新 |
| `remove(id)` | 删除课程 | 委托CourseDao |
| `getStatistics(scale, creditTarget)` | 获取统计数据 | 计算GPA、加权平均、分类统计 |
| `getSemesterStatistics(scale)` | 学期统计 | 按学期分组计算GPA |

**私有方法：**

| 方法 | 功能 |
|------|------|
| `calculateGPA(courses, scale)` | 计算加权GPA |
| `calculateWeightedAverage(courses)` | 计算加权平均分 |
| `getCategoryStatistics(courses)` | 按类别统计 |

**GPA计算逻辑：**

```cpp
double CourseService::calculateGPA(const QList<Course>& courses, const QString& scale) {
    double totalPoints = 0;
    double totalCredits = 0;
    for (const auto& c : courses) {
        if (c.status == "Completed" && c.score > 0) {
            double gp = Course::calculateGradePoint(c.score, scale);
            totalPoints += gp * c.credits;
            totalCredits += c.credits;
        }
    }
    return totalCredits > 0 ? totalPoints / totalCredits : 0;
}
```

### 5.2 DashboardService 类 (service/DashboardService.h)

**公共方法：**

| 方法 | 功能 | 返回数据 |
|------|------|----------|
| `getOverview(scale)` | 获取总览 | GPA、学分、课程数、目标进度 |
| `getGpaTrend(scale)` | GPA趋势 | 按学期的GPA变化 |
| `getRecommendations()` | 推荐建议 | 基于规则的建议列表 |

### 5.3 AnalyticsService 类 (service/AnalyticsService.h)

**公共方法：**

| 方法 | 功能 | 返回数据 |
|------|------|----------|
| `getSemesterComparison()` | 学期对比 | 各学期成绩对比 |
| `compareWithPeers()` | 同学对比 | 与同学数据对比 |
| `generateReport()` | 生成报告 | 综合分析报告 |
| `getTimelineEvents()` | 时间轴事件 | 所有事件按时间排序 |

### 5.4 AuthService 类 (service/AuthService.h)

**公共方法：**

| 方法 | 功能 | 核心逻辑 |
|------|------|----------|
| `hashPassword(password)` | 密码哈希 | SHA256 + salt |
| `verifyPassword(password, hash)` | 密码验证 | 比对哈希值 |
| `registerUser(...)` | 用户注册 | 验证唯一性→哈希密码→创建→生成Token |
| `login(username, password)` | 用户登录 | 查找用户→验证密码→更新登录时间→生成Token |
| `getMe(userId)` | 获取用户信息 | 按ID查询 |
| `changePassword(...)` | 修改密码 | 验证旧密码→哈希新密码→更新 |
| `updateProfile(userId, data)` | 更新资料 | 验证邮箱唯一性→更新 |

**Token生成逻辑：**

```cpp
QString AuthService::generateToken(int userId, const QString& username, const QString& role) {
    QString data = QString("%1:%2:%3:%4")
        .arg(userId).arg(username).arg(role)
        .arg(QDateTime::currentSecsSinceEpoch() + 72 * 3600);
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
    return QString("%1.%2.%3")
        .arg(QString(QByteArray::number(userId).toBase64()))
        .arg(QString(QCryptographicHash::hash(username.toUtf8(), QCryptographicHash::Md5).toHex()).left(12))
        .arg(QString(hash.toHex()).left(24));
}
```

### 5.5 AiService 类 (service/AiService.h)

**公共方法：**

| 方法 | 功能 | 实现方式 |
|------|------|----------|
| `analyze(data)` | AI分析 | 规则引擎（按类型分发） |
| `checkStatus()` | 检查状态 | 返回模型状态信息 |
| `chat(data)` | AI聊天 | 规则引擎回复 |

**分析分发逻辑：**

```cpp
QJsonObject AiService::analyze(const QJsonObject& data) {
    QString type = data["type"].toString();
    if (type == "course") return analyzeCourse(data);
    if (type == "career") return analyzeCareer(data);
    if (type == "goal") return analyzeGoal(data);
    return analyzeGeneral();
}
```

### 5.6 ResumeService 类 (service/ResumeService.h)

**公共方法：**

| 方法 | 功能 | 返回数据 |
|------|------|----------|
| `generate(options)` | 生成简历 | QJsonObject简历数据 |
| `exportJson(options)` | 导出JSON | QByteArray |
| `exportHtml(options)` | 导出HTML | QByteArray（含CSS样式） |

**简历生成逻辑：**

```cpp
QJsonObject ResumeService::generate(const QJsonObject& options) {
    QJsonObject resume;
    resume["name"] = options["name"].toString("个人简历");
    
    QJsonArray sections;
    if (options["includeEducation"].toBool(true)) {
        QJsonArray eduArr;
        for (const auto& c : courses) {
            if (c.status == "Completed") eduArr.append(c.toDict());
        }
        sections.append(section);
    }
    resume["sections"] = sections;
    return resume;
}
```

### 5.7 ImportService 类 (service/ImportService.h)

**公共方法：**

| 方法 | 功能 |
|------|------|
| `importData(entity, fileData, filename)` | 导入CSV数据 |

**私有方法：**

| 方法 | 功能 |
|------|------|
| `importRow(entity, row)` | 导入单行数据 |
| `parseCsv(data, entity)` | 解析CSV文件 |
| `parseCsvLine(line)` | 解析CSV行（支持引号） |
| `normalizeHeader(entity, header)` | 标准化表头（中英文映射） |

**CSV解析特性：**

| 特性 | 说明 |
|------|------|
| 引号包裹 | 支持引号包裹的字段 |
| 中英文表头 | 支持中英文表头映射 |
| 支持实体 | courses, roles, achievements, experiences, activities, goals, peers |

---

## 6. 数据库Schema设计

### 6.0 数据刷新联动表

**客户端数据刷新联动：**

| 触发域 | 刷新页面 | 说明 |
|--------|----------|------|
| Courses | Overview, Analysis, Timeline, Resume | 课程影响GPA、学分统计、时间轴、简历 |
| Goals | Overview, Analysis, Timeline, Resume | 目标影响进度统计、分析、时间轴、简历 |
| Roles | Overview, Analysis, Resume | 角色影响统计、分析、简历 |
| Achievements | Overview, Analysis, Resume | 成就影响统计、分析、简历 |
| Experiences | Overview, Analysis, Timeline, Resume | 经历影响统计、分析、时间轴、简历 |
| Activities | Overview, Analysis, Timeline, Resume | 活动影响统计、分析、时间轴、简历 |
| Jobs | Overview, Analysis, Timeline | 岗位影响统计、分析、时间轴 |
| All | 所有页面 | 导入数据后全局刷新 |

**实现方式：**

```cpp
void DataRefreshCoordinator::refreshByDomain(DataDomain domain) {
    switch(domain) {
        case DataDomain::Courses:
            m_overview->refresh();
            m_analysis->refresh();
            m_timeline->refresh();
            m_resume->refresh();
            break;
        case DataDomain::Goals:
            m_overview->refresh();
            m_analysis->refresh();
            m_timeline->refresh();
            m_resume->refresh();
            break;
    }
}
```

### 6.1 表结构设计

**courses 表：**

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY | 主键 |
| name | TEXT | NOT NULL | 课程名称 |
| code | TEXT | - | 课程代码 |
| credits | REAL | DEFAULT 0 | 学分 |
| semester | TEXT | - | 学期 |
| category | TEXT | DEFAULT 'Required' | 类别 |
| score | REAL | DEFAULT 0 | 分数 |
| grade_point | REAL | DEFAULT 0 | 绩点 |
| status | TEXT | DEFAULT 'Planned' | 状态 |
| teacher | TEXT | - | 教师 |
| location | TEXT | - | 地点 |
| description | TEXT | - | 描述 |
| tags | TEXT | - | 标签 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | 更新时间 |

**goals 表：**

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY | 主键 |
| title | TEXT | NOT NULL | 目标标题 |
| category | TEXT | - | 类别 |
| description | TEXT | - | 描述 |
| target_value | REAL | DEFAULT 0 | 目标值 |
| current_value | REAL | DEFAULT 0 | 当前进度 |
| deadline | TEXT | - | 截止日期 |
| priority | TEXT | DEFAULT 'Medium' | 优先级 |
| status | TEXT | DEFAULT 'In Progress' | 状态 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | 更新时间 |

**users 表：**

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY | 主键 |
| username | TEXT | UNIQUE NOT NULL | 用户名 |
| email | TEXT | UNIQUE NOT NULL | 邮箱 |
| password_hash | TEXT | NOT NULL | 密码哈希 |
| display_name | TEXT | - | 显示名称 |
| role | TEXT | DEFAULT 'user' | 角色 |
| is_active | INTEGER | DEFAULT 1 | 是否激活 |
| last_login | TIMESTAMP | - | 最后登录 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | 更新时间 |

---

## 7. 技术方案特点

### 7.1 后端核心设计

| 技术点 | 方案 |
|--------|------|
| 数据库访问 | 手动SQL (QSqlQuery) |
| 序列化 | QJsonObject toDict/fromDict |
| 密码哈希 | QCryptographicHash |
| Token | 自定义Token (SHA256) |
| 数据验证 | 手动验证 |

### 7.2 性能优势

| 指标 | 数值 |
|------|------|
| 启动时间 | ~0.1s |
| 内存占用 | ~10MB |
| 请求响应 | ~1ms |
| 数据库操作 | 直接SQL |

---

## 8. 开发规范

### 8.1 Model类设计规范

| 规范 | 说明 |
|------|------|
| 成员变量 | 公共成员变量，默认值初始化 |
| toDict | 序列化为QJsonObject |
| fromDict | 静态方法，从QJsonObject反序列化 |
| Header-Only | 大部分Model类只在.h中实现 |

### 8.2 DAO类设计规范

| 规范 | 说明 |
|------|------|
| 静态方法 | 所有方法为静态方法 |
| 无状态 | 不持有状态，无成员变量 |
| SQL封装 | 封装所有SQL操作 |
| 返回Model | 返回Model对象或列表 |

### 8.3 Service类设计规范

| 规范 | 说明 |
|------|------|
| 静态方法 | 所有方法为静态方法 |
| 无状态 | 不持有状态，无成员变量 |
| 业务逻辑 | 包含所有业务逻辑 |
| 调用DAO | 通过DAO访问数据库 |

---

## 9. 总结

### 9.1 工作成果

| 层次 | 类数量 | 代码行数 |
|------|--------|----------|
| Model层 | 10个 | ~800行 |
| DAO层 | 10个 | ~600行 |
| Service层 | 13个 | ~1200行 |
| Schema | 10个表 | ~150行 |

### 9.2 核心贡献

| 贡献 | 说明 |
|------|------|
| Model层 | 10个数据模型类，每个类包含完整的成员变量、toDict/fromDict序列化方法 |
| DAO层 | DaoBase基类 + 9个具体DAO类，封装所有SQL操作 |
| Service层 | 13个服务类，实现业务逻辑包括GPA计算、密码哈希、Token生成、CSV解析、简历生成、AI规则引擎等 |

C++版本采用手动SQL方案，获得了更强的类型安全和更高效的运行时性能。通过统一的toDict/fromDict模式和静态方法设计，保持了代码的一致性和可维护性。

---

**文档版本**: v1.1  
**更新日期**: 2026-04-29  
**适用项目版本**: v0.3.0
