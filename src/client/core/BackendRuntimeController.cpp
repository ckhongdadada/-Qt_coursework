#include "BackendRuntimeController.h"
#include "server/HttpServer.h"

#include <atomic>
#include "service/CourseService.h"
#include "service/RoleService.h"
#include "service/AchievementService.h"
#include "service/ExperienceService.h"
#include "service/ActivityService.h"
#include "service/GoalService.h"
#include "service/JobService.h"
#include "util/Logger.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QThread>
#include <QStyle>

class HttpServerThread : public QThread {
    Q_OBJECT

public:
    explicit HttpServerThread(QObject* parent = nullptr) : QThread(parent) {}

    void stop() { m_running.store(false); }

signals:
    void serverStarted();
    void serverError(const QString& error);

protected:
    void run() override {
        Logger::info("后端服务线程启动");
        HttpServer server;
        if (!server.start(8080)) {
            emit serverError("无法绑定端口 8080");
            return;
        }
        emit serverStarted();
        while (m_running.load()) {
            msleep(100);
        }
        server.stop();
        Logger::info("后端服务线程退出");
    }

private:
    std::atomic<bool> m_running{true};
};

BackendRuntimeController::BackendRuntimeController(QObject* parent)
    : QObject(parent)
{
    m_serverUrl = "http://localhost:8080";
}

void BackendRuntimeController::bindWidgets(QLabel* statusLabel, QProgressBar* progressBar, QSystemTrayIcon* trayIcon)
{
    m_statusLabel = statusLabel;
    m_progressBar = progressBar;
    m_trayIcon = trayIcon;
}

void BackendRuntimeController::startBackendServer()
{
    if (m_statusLabel) {
        m_statusLabel->setText("正在启动后端服务...");
    }
    updateBackendBadge(false, "启动中");

    m_serverThread = new HttpServerThread(this);
    connect(m_serverThread, &HttpServerThread::serverStarted, this, &BackendRuntimeController::onBackendStarted);
    connect(m_serverThread, &HttpServerThread::serverError, this, &BackendRuntimeController::onBackendError);
    m_serverThread->start();
}

void BackendRuntimeController::stopBackendServer()
{
    if (m_serverThread) {
        m_serverThread->stop();
        m_serverThread->wait(3000);
        m_serverThread->deleteLater();
        m_serverThread = nullptr;
    }
    m_serverReady = false;
}

void BackendRuntimeController::checkFrontendExists()
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

void BackendRuntimeController::openBrowser()
{
    const QString url = m_serverReady ? m_serverUrl : "file:///" + m_frontendPath + "/index.html";
    Logger::info("打开网页预览: " + url);
    QDesktopServices::openUrl(QUrl(url));
}

void BackendRuntimeController::onBackendStarted()
{
    m_serverReady = true;
    if (m_statusLabel) {
        m_statusLabel->setText("后端服务运行中，原生页面已可直接读取数据。");
    }
    updateBackendBadge(true, "端口 8080");
    if (m_progressBar) {
        m_progressBar->hide();
    }
    if (m_trayIcon) {
        m_trayIcon->showMessage("学业发展规划系统", "C++ 后端已启动，可以使用原生界面或网页预览。");
    }
    emit backendReady();
}

void BackendRuntimeController::onBackendError(const QString& error)
{
    m_serverReady = false;
    if (m_statusLabel) {
        m_statusLabel->setText("后端启动失败: " + error);
    }
    updateBackendBadge(false, error);
    if (m_progressBar) {
        m_progressBar->hide();
    }
    emit serverError(error);
}

void BackendRuntimeController::updateBackendBadge(bool ready, const QString& detail)
{
    const QString state = ready ? "运行中" : "未就绪";
    const QString extra = detail.isEmpty() ? QString() : QString(" · %1").arg(detail);
    if (m_statusLabel) {
        m_statusLabel->setText(QString("后端状态：%1%2").arg(state, extra));
    }
}

bool BackendRuntimeController::isServerReady() const { return m_serverReady; }
QString BackendRuntimeController::serverUrl() const { return m_serverUrl; }
QString BackendRuntimeController::frontendPath() const { return m_frontendPath; }

void BackendRuntimeController::insertSampleDataIfNeeded()
{
    QSettings settings;
    if (settings.value("sampleDataInserted", false).toBool()) {
        return;
    }

    QStringList semesters = {"2023-2024-1", "2023-2024-2", "2024-2025-1", "2024-2025-2"};
    QStringList categories = {"必修", "选修", "通识", "实践"};
    QStringList statuses = {"Completed", "In Progress", "Planned"};
    QStringList courseNames = {
        "高等数学", "线性代数", "概率论与数理统计", "计算机组成原理", "数据结构",
        "算法设计与分析", "操作系统", "数据库系统", "机器学习", "深度学习",
        "数据挖掘", "人工智能导论", "软件工程", "计算机网络", "C++程序设计",
        "Python编程", "数据可视化", "自然语言处理", "分布式系统", "云计算基础"
    };
    QStringList courseTeachers = {
        "张明教授", "李华副教授", "王芳教授", "陈伟教授", "刘洋副教授",
        "赵静教授", "孙磊副教授", "周婷教授", "吴强副教授", "郑浩教授",
        "黄丽副教授", "林涛教授", "何欣副教授", "谢敏教授", "杨军副教授",
        "许峰教授", "徐萌副教授", "马亮教授", "高伟副教授", "朱琳教授"
    };
    QStringList courseDescriptions = {
        "涵盖极限、导数、积分等高等数学核心内容，培养数学思维能力。",
        "学习向量、矩阵、线性变换等内容，为机器学习奠定数学基础。",
        "概率论基础、随机变量、假设检验等统计方法与应用。",
        "计算机硬件组成、指令系统、CPU架构等底层原理。",
        "线性表、树、图等数据结构及其操作实现。",
        "经典算法设计思想与复杂度分析，包括排序、查找、图算法等。",
        "进程管理、内存管理、文件系统等操作系统核心概念。",
        "关系数据库原理、SQL语言、数据库设计与优化。",
        "监督学习、无监督学习、模型评估等机器学习基础。",
        "神经网络架构、深度学习框架、CNN/RNN等高级模型。",
        "数据预处理、特征工程、聚类分析、关联规则挖掘。",
        "AI发展历程、知识表示、推理方法、智能体设计。",
        "软件开发流程、需求分析、设计模式、项目管理。",
        "TCP/IP协议、网络层、传输层、应用层协议原理。",
        "C++面向对象编程、STL库、内存管理、模板编程。",
        "Python基础语法、数据处理、科学计算库、Web开发。",
        "数据可视化原理、图表设计、可视化工具与最佳实践。",
        "文本处理、情感分析、语言模型、Transformer架构。",
        "分布式系统原理、一致性协议、分布式存储与计算。",
        "云服务模型、虚拟化技术、容器化部署、云原生架构。"
    };

    for (int i = 1; i <= 20; ++i) {
        Course c;
        c.name = courseNames[i-1];
        c.code = QString("CS%1").arg(1000 + i);
        c.credits = 2.0 + (i % 4);
        c.semester = semesters[i % 4];
        c.category = categories[i % 4];
        c.score = 70 + (i % 30);
        c.status = statuses[i % 3];
        c.teacher = courseTeachers[i-1];
        c.location = QString("教学楼%1教室").arg(i % 5 + 1);
        c.description = courseDescriptions[i-1];
        c.tags = "专业课程,核心必修";
        CourseService::create(c);
    }

    QStringList roleTypes = {"学生干部", "社团负责人", "志愿者", "助教"};
    QStringList roleTitles = {
        "班长", "学习委员", "生活委员", "文体委员", "宣传委员",
        "学生会主席", "社团社长", "志愿者队长", "课程助教", "实验室助理",
        "团支部书记", "组织委员", "纪律委员", "心理委员", "就业指导助理",
        "科研助理", "图书馆志愿者", "运动会志愿者", "迎新志愿者", "校友联络官"
    };
    QStringList roleOrgs = {
        "计算机学院", "软件学院", "人工智能学院", "校学生会", "院学生会",
        "ACM编程社", "AI研习社", "志愿者协会", "教务处", "科研实验室",
        "团委", "学生事务中心", "图书馆", "体育中心", "就业指导中心",
        "创新中心", "创业孵化基地", "校园文化中心", "校友办公室", "国际交流处"
    };
    QStringList roleDescriptions = {
        "负责班级日常管理，组织班会，协调同学关系，传达学院通知。",
        "负责班级学习事务，组织学习小组，收集学习问题并反馈给老师。",
        "负责班级生活事务，管理班费，组织班级活动，关心同学生活。",
        "负责班级文体活动，组织运动会、文艺晚会等文体赛事。",
        "负责班级宣传工作，制作班级宣传材料，管理班级公众号。",
        "负责学生会全面工作，组织校园活动，代表学生与学校沟通。",
        "负责社团日常运营，组织社团活动，发展社团成员。",
        "组织志愿者活动，协调志愿者资源，服务校园和社区。",
        "协助教师授课，批改作业，辅导学生学习，解答课程疑问。",
        "协助实验室日常管理，维护实验设备，参与科研项目。",
        "负责团支部建设，组织团日活动，发展团员，收缴团费。",
        "负责组织班级活动，增强班级凝聚力，促进同学交流。",
        "维护班级纪律，记录考勤，协助老师处理违纪事件。",
        "关注同学心理健康，组织心理健康活动，提供心理支持。",
        "协助就业指导工作，组织招聘会，整理就业信息。",
        "参与科研项目，协助数据收集与分析，撰写研究报告。",
        "协助图书馆日常工作，整理图书，引导读者借阅。",
        "协助运动会组织工作，服务运动员，维持赛场秩序。",
        "协助新生报到工作，引导新生熟悉校园，解答新生疑问。",
        "负责校友联络工作，组织校友活动，维护校友关系。"
    };
    for (int i = 1; i <= 20; ++i) {
        Role r;
        r.title = roleTitles[i-1];
        r.type = roleTypes[i % 4];
        r.organization = roleOrgs[i-1];
        r.description = roleDescriptions[i-1];
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
    QStringList achievementTitles = {
        "全国大学生数学建模竞赛一等奖", "计算机二级证书", "个人发展规划系统开发", "深度学习在图像识别中的应用",
        "蓝桥杯软件大赛二等奖", "英语六级证书", "智能校园导航系统", "基于机器学习的成绩预测研究",
        "ACM程序设计竞赛三等奖", "普通话二级甲等", "在线教育平台设计", "社交网络分析与推荐算法",
        "挑战杯创业大赛金奖", "数据库工程师认证", "电商推荐系统", "自然语言处理情感分析",
        "数学竞赛省级一等奖", "网络工程师证书", "医疗数据分析平台", "区块链技术应用研究",
        "物理竞赛省级二等奖", "软件设计师证书", "智慧城市管理系统", "强化学习算法优化"
    };
    QStringList achievementOrgs = {
        "教育部", "教育部考试中心", "学校创新创业中心", "计算机学院",
        "工业和信息化部", "教育部考试中心", "信息学院", "人工智能实验室",
        "ACM协会", "国家语言文字工作委员会", "软件工程系", "数据科学研究中心",
        "共青团中央", "工信部教育考试中心", "商学院", "自然语言处理实验室",
        "省教育厅", "工信部教育考试中心", "医学院", "区块链研究中心",
        "省教育厅", "工信部教育考试中心", "城市规划学院", "智能系统实验室"
    };
    QStringList achievementDescriptions = {
        "团队协作完成数学建模论文，获得全国一等奖，展示了优秀的数学建模能力。",
        "通过计算机二级考试，掌握办公软件和基础编程技能。",
        "独立完成个人发展规划系统的设计与开发，包含课程管理、目标追踪等功能。",
        "研究深度学习算法在图像识别中的应用，发表学术论文。",
        "在蓝桥杯软件大赛中获得二等奖，展现了扎实的编程功底。",
        "通过英语六级考试，具备良好的英语读写能力。",
        "参与智能校园导航系统开发，负责前端界面设计与实现。",
        "基于机器学习算法进行成绩预测研究，取得较好预测效果。",
        "在ACM程序设计竞赛中获得三等奖，锻炼了算法设计能力。",
        "普通话水平达到二级甲等，具备良好的口语表达能力。",
        "参与在线教育平台设计，负责用户管理模块开发。",
        "研究社交网络分析算法，提出改进的推荐算法。",
        "挑战杯创业大赛金奖项目负责人，展示了创新创业能力。",
        "获得数据库工程师认证，具备数据库设计与优化能力。",
        "参与电商推荐系统开发，负责推荐算法实现。",
        "研究自然语言处理情感分析技术，发表相关论文。",
        "数学竞赛省级一等奖获得者，数学基础扎实。",
        "获得网络工程师证书，具备网络规划与维护能力。",
        "参与医疗数据分析平台开发，负责数据可视化模块。",
        "研究区块链技术应用，探索去中心化应用场景。",
        "物理竞赛省级二等奖，展现了良好的物理素养。",
        "获得软件设计师证书，具备软件开发与设计能力。",
        "参与智慧城市管理系统开发，负责数据处理模块。",
        "研究强化学习算法优化，提升算法收敛速度。"
    };
    for (int i = 1; i <= 20; ++i) {
        Achievement a;
        a.title = achievementTitles[i-1];
        a.type = achTypes[i % 4];
        a.level = levels[i % 4];
        a.organization = achievementOrgs[i-1];
        a.description = achievementDescriptions[i-1];
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
    QStringList experienceTitles = {
        "字节跳动算法实习生", "电商推荐系统开发", "深度学习研究助理", "ICPC国际大学生程序设计竞赛",
        "阿里巴巴后端开发实习生", "智慧校园管理平台", "计算机视觉研究", "全国大学生数学建模竞赛",
        "腾讯产品运营实习生", "在线教育平台开发", "自然语言处理研究", "蓝桥杯全国软件大赛",
        "百度AI研发实习生", "医疗健康大数据平台", "强化学习研究", "挑战杯创业计划竞赛",
        "华为软件开发实习生", "智能家居控制系统", "数据挖掘研究", "ACM区域赛",
        "美团数据分析实习生", "物流配送优化系统", "知识图谱研究", "数学竞赛省级赛"
    };
    QStringList experienceOrgs = {
        "字节跳动", "学校创新创业中心", "人工智能实验室", "ICPC组织委员会",
        "阿里巴巴", "信息学院", "计算机视觉实验室", "教育部",
        "腾讯", "教育科技公司", "自然语言处理实验室", "工业和信息化部",
        "百度", "医疗科技公司", "强化学习研究中心", "共青团中央",
        "华为", "智能家居公司", "数据挖掘实验室", "ACM协会",
        "美团", "物流科技公司", "知识图谱研究中心", "省教育厅"
    };
    QStringList experienceRoles = {
        "算法工程师", "后端开发工程师", "研究助理", "参赛队员",
        "后端开发工程师", "全栈开发工程师", "研究员", "参赛队员",
        "产品运营专员", "前端开发工程师", "研究员", "参赛队员",
        "AI研发工程师", "数据工程师", "研究员", "项目负责人",
        "软件开发工程师", "嵌入式开发工程师", "研究员", "参赛队员",
        "数据分析工程师", "算法工程师", "研究员", "参赛队员"
    };
    QStringList experienceDescriptions = {
        "参与推荐算法优化，提升推荐准确率15%，处理日均千万级请求。",
        "负责电商推荐系统后端开发，使用Spring Boot框架，实现商品推荐功能。",
        "协助导师进行深度学习模型训练，参与论文撰写，发表SCI论文一篇。",
        "参加ICPC国际大学生程序设计竞赛，团队进入亚洲区域赛。",
        "参与阿里巴巴核心业务系统开发，负责订单模块优化，提升系统性能。",
        "开发智慧校园管理平台，包含教务管理、选课系统等功能模块。",
        "研究计算机视觉算法，实现图像分类和目标检测功能。",
        "团队协作完成数学建模论文，获得全国一等奖。",
        "负责产品运营工作，策划运营活动，用户活跃度提升30%。",
        "开发在线教育平台前端界面，使用Vue.js框架，实现响应式设计。",
        "研究自然语言处理技术，实现文本分类和情感分析功能。",
        "参加蓝桥杯软件大赛，获得全国二等奖。",
        "参与百度AI研发项目，负责深度学习模型部署，优化模型推理速度。",
        "开发医疗健康大数据平台，实现数据采集、存储和分析功能。",
        "研究强化学习算法，应用于游戏AI和机器人控制。",
        "挑战杯创业大赛金奖项目负责人，完成商业计划书撰写和路演展示。",
        "参与华为软件产品开发，负责通信模块设计，保障系统稳定性。",
        "开发智能家居控制系统，实现设备联动和远程控制功能。",
        "研究数据挖掘算法，实现用户行为分析和精准营销。",
        "参加ACM区域赛，获得铜牌。",
        "负责美团数据分析工作，构建数据报表，支持业务决策。",
        "开发物流配送优化系统，使用遗传算法优化配送路线。",
        "研究知识图谱技术，实现智能问答和知识推理功能。",
        "参加数学竞赛省级赛，获得一等奖。"
    };
    QStringList experienceTechs = {
        "Python, TensorFlow, PyTorch", "Java, Spring Boot, MySQL", "Python, PyTorch, CUDA", "C++, STL, Algorithm",
        "Java, Spring Cloud, Redis", "Vue.js, Node.js, MongoDB", "Python, OpenCV, TensorFlow", "Matlab, Python, Latex",
        "Excel, SQL, Tableau", "Vue.js, React, TypeScript", "Python, HuggingFace, PyTorch", "C++, Python, Data Structure",
        "Python, TensorFlow, ONNX", "Python, Spark, Hadoop", "Python, PyTorch, Gym", "PPT, Excel, Business Model",
        "C++, Qt, SQL", "C, Embedded, MQTT", "Python, Scikit-learn, Pandas", "C++, Algorithm, Data Structure",
        "Python, SQL, Tableau", "Python, OR-Tools, Gurobi", "Python, Neo4j, PyTorch", "Python, Math, Latex"
    };
    for (int i = 1; i <= 20; ++i) {
        Experience e;
        e.title = experienceTitles[i-1];
        e.type = expTypes[i % 4];
        e.organization = experienceOrgs[i-1];
        e.role = experienceRoles[i-1];
        e.description = experienceDescriptions[i-1];
        e.startDate = QString("2024-%1-01").arg(i % 12 + 1);
        e.endDate = i % 3 == 0 ? "" : QString("2025-%1-01").arg(i % 12 + 1);
        e.isOngoing = (i % 3 == 0);
        e.technologies = experienceTechs[i-1];
        e.achievements = QString("成果1,成果2,成果3");
        e.supervisor = QString("导师%1").arg(i);
        e.contact = QString("contact%1@example.com").arg(i);
        e.location = QString("地点%1").arg(i);
        e.url = QString("https://example.com/exp%1").arg(i);
        ExperienceService::create(e);
    }

    QStringList actCategories = {"学术", "体育", "艺术", "社交", "志愿"};
    QStringList activityNames = {
        "学术讲座：人工智能前沿", "校运会田径比赛", "校园音乐节", "迎新晚会", "社区志愿服务",
        "技术分享会：大数据应用", "篮球联赛", "话剧表演", "校友交流会", "环保志愿者活动",
        "科研论坛：深度学习", "羽毛球比赛", "舞蹈大赛", "社团招新", "敬老院慰问",
        "行业沙龙：互联网发展", "足球比赛", "歌唱比赛", "毕业生欢送会", "支教活动",
        "学术年会", "乒乓球比赛", "书法展览", "中秋晚会", "图书馆整理志愿"
    };
    QStringList activityDescriptions = {
        "邀请知名专家分享人工智能最新研究成果和发展趋势。",
        "一年一度的校运会，包含田径、跳远、跳高多个项目。",
        "校园音乐盛会，展示学生才艺，丰富校园文化生活。",
        "迎接新生的盛大晚会，帮助新生快速融入校园生活。",
        "走进社区，为居民提供志愿服务，传递温暖。",
        "技术分享会，探讨大数据技术在各行业的应用案例。",
        "学院篮球联赛，展现团队协作精神和体育竞技风采。",
        "精彩话剧表演，展现学生表演才华和艺术创造力。",
        "校友交流活动，促进校友与在校生之间的联系与交流。",
        "环保志愿者活动，倡导绿色生活，保护校园环境。",
        "科研论坛，分享深度学习最新研究成果和应用经验。",
        "羽毛球比赛，锻炼身体，增进同学友谊。",
        "舞蹈大赛，展示各类舞蹈形式，丰富校园文化。",
        "社团招新活动，为新生提供了解和加入社团的机会。",
        "敬老院慰问活动，关爱孤寡老人，传递爱心。",
        "行业沙龙，探讨互联网行业发展趋势和就业前景。",
        "足球比赛，展现团队配合和拼搏精神。",
        "歌唱比赛，展示学生歌唱才华，发现校园好声音。",
        "毕业生欢送会，为毕业生送上美好祝福。",
        "支教活动，为偏远地区学生提供教育支持。",
        "学术年会，总结学术成果，展望未来发展。",
        "乒乓球比赛，锻炼反应能力，增进同学交流。",
        "书法展览，展示学生书法作品，弘扬传统文化。",
        "中秋晚会，共度传统佳节，增进师生情谊。",
        "图书馆整理志愿，帮助整理图书，方便师生借阅。"
    };
    for (int i = 1; i <= 20; ++i) {
        Activity a;
        a.name = activityNames[i-1];
        a.description = activityDescriptions[i-1];
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
    QStringList goalTitles = {
        "GPA达到3.8以上", "掌握Python编程", "每周运动3次", "参加10场社交活动", "拿到大厂实习offer",
        "完成毕业论文", "学习机器学习", "保持早睡早起", "加入技术社团", "通过英语六级",
        "专业排名前10%", "掌握深度学习框架", "学会游泳", "组织班级活动", "获得奖学金",
        "发表一篇论文", "掌握数据可视化", "减肥10斤", "认识20位新朋友", "考取专业证书",
        "完成所有必修课程", "掌握云计算技术", "跑步5公里", "参加创业比赛", "找到理想工作"
    };
    QStringList goalDescriptions = {
        "努力学习，提高各科成绩，争取GPA达到3.8以上。",
        "系统学习Python编程，掌握基础语法和常用库的使用。",
        "每周坚持运动3次，保持身体健康。",
        "积极参加各类社交活动，拓展人脉资源。",
        "努力准备面试，争取拿到大厂实习offer。",
        "完成毕业论文选题、调研和写作，顺利通过答辩。",
        "学习机器学习基础知识，掌握常用算法和模型。",
        "养成良好作息习惯，保持早睡早起。",
        "加入技术社团，参与技术交流和项目实践。",
        "认真备考，通过大学英语六级考试。",
        "努力学习，争取专业排名进入前10%。",
        "掌握TensorFlow、PyTorch等深度学习框架。",
        "学习游泳技能，掌握基本泳姿。",
        "组织班级活动，增强班级凝聚力。",
        "努力学习，争取获得奖学金。",
        "参与科研项目，发表一篇学术论文。",
        "掌握数据可视化技术，制作专业图表。",
        "通过合理饮食和运动，减肥10斤。",
        "积极参加各类活动，认识20位新朋友。",
        "备考并考取相关专业证书，提升竞争力。",
        "顺利完成所有必修课程学习。",
        "学习云计算技术，了解云服务平台使用。",
        "坚持跑步锻炼，目标每天5公里。",
        "组建团队参加创业比赛，锻炼创新创业能力。",
        "通过校招找到理想工作。"
    };
    for (int i = 1; i <= 20; ++i) {
        Goal g;
        g.title = goalTitles[i-1];
        g.category = goalCategories[i % 5];
        g.description = goalDescriptions[i-1];
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
    QStringList jobTitles = {
        "算法工程师", "后端开发工程师", "前端开发工程师", "数据分析师", "产品经理",
        "人工智能工程师", "测试工程师", "运维工程师", "安全工程师", "嵌入式工程师",
        "大数据工程师", "云计算工程师", "移动端开发工程师", "游戏开发工程师", "技术顾问",
        "研发工程师", "软件工程师", "数据科学家", "机器学习工程师", "深度学习工程师"
    };
    QStringList jobCompanies = {
        "字节跳动", "阿里巴巴", "腾讯", "百度", "美团",
        "华为", "京东", "网易", "小米", "滴滴",
        "快手", "拼多多", "小红书", "B站", "蚂蚁集团",
        "京东科技", "阿里云", "腾讯云", "百度智能云", "字节跳动抖音"
    };
    QStringList jobLocations = {
        "北京", "上海", "深圳", "杭州", "广州",
        "成都", "武汉", "南京", "西安", "苏州",
        "北京", "上海", "深圳", "杭州", "北京",
        "上海", "杭州", "深圳", "北京", "上海"
    };
    QStringList jobDescriptions = {
        "负责推荐算法优化，提升推荐系统性能和用户体验。",
        "负责后端服务开发，保障系统稳定性和高并发处理能力。",
        "负责前端页面开发，实现用户界面交互和响应式设计。",
        "负责数据收集、清洗和分析，为业务决策提供数据支持。",
        "负责产品规划和设计，推动产品迭代和用户增长。",
        "负责人工智能算法研发，实现AI应用落地。",
        "负责软件测试，保障产品质量和稳定性。",
        "负责系统运维，保障服务持续稳定运行。",
        "负责网络安全和数据安全，保护公司信息资产。",
        "负责嵌入式系统开发，实现硬件软件协同工作。",
        "负责大数据平台开发，处理海量数据存储和分析。",
        "负责云计算平台建设，提供弹性计算服务。",
        "负责移动端应用开发，支持iOS和Android平台。",
        "负责游戏开发，实现游戏玩法和画面效果。",
        "负责技术咨询和方案设计，为客户提供技术支持。",
        "负责研发新技术和解决方案，推动技术创新。",
        "负责软件产品开发，实现业务需求和功能。",
        "负责数据科学研究，挖掘数据价值和洞察。",
        "负责机器学习模型开发，实现智能决策和预测。",
        "负责深度学习模型训练，实现复杂AI任务。"
    };
    QStringList jobRequirements = {
        "熟练掌握Python、机器学习算法", "熟练掌握Java、Spring Boot", "熟练掌握Vue.js、React", "熟练掌握SQL、Python数据分析", "熟悉产品设计方法论",
        "掌握深度学习框架、PyTorch", "熟练掌握测试工具、自动化测试", "熟悉Linux运维、Docker", "掌握网络安全知识", "熟练掌握C/C++、嵌入式开发",
        "熟练掌握Hadoop、Spark", "熟悉云平台、Kubernetes", "熟练掌握Flutter、React Native", "熟悉Unity、Unreal Engine", "具备技术方案设计能力",
        "掌握多语言开发、架构设计", "熟练掌握Java/C++、软件工程", "掌握机器学习、统计学", "熟练掌握TensorFlow、PyTorch", "掌握深度学习、NLP/CV"
    };
    for (int i = 1; i <= 20; ++i) {
        Job j;
        j.title = jobTitles[i-1];
        j.company = jobCompanies[i-1];
        j.location = jobLocations[i-1];
        j.salaryRange = QString("%1-%2K").arg(10 + i).arg(20 + i);
        j.description = jobDescriptions[i-1];
        JobRequirement req1, req2;
        req1.text = QString("要求1：%1").arg(jobRequirements[i-1]);
        req1.met = (i % 2 == 0);
        req2.text = QString("要求2：具备%1年相关工作经验").arg(i % 5 + 1);
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

#include "BackendRuntimeController.moc"
