#include "service/AiService.h"
#include "service/CourseService.h"
#include "service/GoalService.h"
#include "service/ExperienceService.h"
#include "service/AchievementService.h"
#include "dao/AchievementDao.h"
#include "dao/ExperienceDao.h"
#include "dao/RoleDao.h"
#include "util/Logger.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QDateTime>

QString AiService::m_aiServerUrl = "http://localhost:8001";
QString AiService::m_backendUrl = "http://127.0.0.1:8080";
bool AiService::m_aiServerChecked = false;
bool AiService::m_aiServerAvailable = false;
qint64 AiService::m_lastAiServerCheckMs = 0;

namespace {
constexpr int kAiHealthTimeoutMs = 2000;
constexpr int kAiGenerationTimeoutMs = 180000;
constexpr int kBackendApiTimeoutMs = 10000;
constexpr qint64 kAiAvailableCacheMs = 15000;
constexpr qint64 kAiUnavailableRetryMs = 3000;

bool hasAiError(const QJsonObject& result)
{
    if (!result.contains("error")) {
        return false;
    }
    const QJsonValue errorValue = result.value("error");
    if (errorValue.isBool()) {
        return errorValue.toBool();
    }
    return !errorValue.toString().trimmed().isEmpty();
}
}

void AiService::setAiServerUrl(const QString& url) {
    m_aiServerUrl = url;
    resetAiServerCheck();
}

QString AiService::aiServerUrl() {
    return m_aiServerUrl;
}

void AiService::setBackendUrl(const QString& url) {
    m_backendUrl = url;
}

QString AiService::backendUrl() {
    return m_backendUrl;
}

void AiService::resetAiServerCheck() {
    m_aiServerChecked = false;
    m_aiServerAvailable = false;
    m_lastAiServerCheckMs = 0;
}

bool AiService::isAiServerAvailable(bool forceRefresh) {
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const bool cacheActive = m_aiServerChecked
        && !forceRefresh
        && ((m_aiServerAvailable && (nowMs - m_lastAiServerCheckMs) < kAiAvailableCacheMs)
            || (!m_aiServerAvailable && (nowMs - m_lastAiServerCheckMs) < kAiUnavailableRetryMs));
    if (cacheActive) {
        return m_aiServerAvailable;
    }
    
    QNetworkAccessManager manager;
    QUrl url(m_aiServerUrl + "/health");
    QNetworkRequest request(url);
    request.setTransferTimeout(kAiHealthTimeoutMs);
    
    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    
    QTimer::singleShot(kAiHealthTimeoutMs, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    m_aiServerChecked = true;
    m_aiServerAvailable = false;
    if (reply->error() == QNetworkReply::NoError) {
        const QJsonObject health = QJsonDocument::fromJson(reply->readAll()).object();
        m_aiServerAvailable = health.value("model_loaded").toBool(false);
        if (!m_aiServerAvailable && health.value("model_loading").toBool(false)) {
            Logger::info("AI服务已启动，模型仍在加载: " + m_aiServerUrl);
        }
    }
    m_lastAiServerCheckMs = nowMs;
    
    if (m_aiServerAvailable) {
        Logger::info("AI服务可用: " + m_aiServerUrl);
    } else {
        Logger::warning("AI服务不可用，请启动本地大模型服务: " + reply->errorString());
    }
    
    reply->deleteLater();
    return m_aiServerAvailable;
}

QJsonObject AiService::callAiServer(const QString& endpoint, const QJsonObject& data) {
    QNetworkAccessManager manager;
    QUrl url(m_aiServerUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(kAiGenerationTimeoutMs);
    
    QJsonDocument doc(data);
    QEventLoop loop;
    QNetworkReply* reply = manager.post(request, doc.toJson());
    
    QTimer::singleShot(kAiGenerationTimeoutMs, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QJsonObject result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        result = QJsonDocument::fromJson(responseData).object();
        result["aiPowered"] = true;
    } else {
        result["error"] = reply->errorString();
        result["aiPowered"] = false;
    }
    
    reply->deleteLater();
    return result;
}

QJsonObject AiService::callBackendApi(const QString& endpoint, const QJsonObject& data) {
    QNetworkAccessManager manager;
    QUrl url(m_backendUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(kAiGenerationTimeoutMs);
    
    QEventLoop loop;
    QNetworkReply* reply;
    
    if (data.isEmpty()) {
        reply = manager.get(request);
    } else {
        QJsonDocument doc(data);
        reply = manager.post(request, doc.toJson());
    }
    
    QTimer::singleShot(kAiGenerationTimeoutMs, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QJsonObject result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonObject response = QJsonDocument::fromJson(responseData).object();
        if (response.contains("data")) {
            result = response["data"].toObject();
        } else {
            result = response;
        }
    } else {
        result["error"] = reply->errorString();
    }
    
    reply->deleteLater();
    return result;
}

QJsonObject AiService::analyze(const QJsonObject& data) {
    if (isAiServerAvailable()) {
        QJsonObject aiResult = analyzeWithAi(data);
        if (!hasAiError(aiResult)) {
            aiResult["aiPowered"] = true;
            return aiResult;
        }
        resetAiServerCheck();
    }

    // 外部AI不可用时，使用本地规则分析
    return analyzeLocal(data);
}

QJsonObject AiService::analyzeLocal(const QJsonObject& data) {
    QString analysisType = data["type"].toString("general");
    QJsonObject localResult = generateLocalAnalysis(analysisType);
    localResult["type"] = analysisType;
    localResult["aiPowered"] = false;
    return localResult;
}

QJsonObject AiService::generateLocalAnalysis(const QString& type) {
    if (type == "course") {
        return buildCourseAdvice();
    }
    if (type == "goal") {
        return buildGoalAdvice();
    }
    if (type == "experience" || type == "career") {
        return buildExperienceAdvice();
    }
    if (type == "resume") {
        return buildResumeAdvice();
    }
    if (type == "achievement") {
        return buildAchievementAdvice();
    }
    if (type == "comprehensive" || type == "general") {
        return buildComprehensiveAdvice();
    }
    return buildDefaultReply();
}

QJsonObject AiService::analyzeWithAi(const QJsonObject& data) {
    return callAiServer("/v1/analyze", data);
}

QJsonObject AiService::checkStatus() {
    QJsonObject status;

    // 检查外部 AI 服务器（使用缓存，避免阻塞）
    const bool aiAvailable = isAiServerAvailable(true);

    if (aiAvailable) {
        status["mode"] = "remote-model";
        status["model"] = "external-ai";
        status["available"] = true;
        status["aiServer"] = m_aiServerUrl;
    } else {
        // 外部 AI 不可用时，回退到后端本地规则模式
        status["mode"] = "rule-based";
        status["model"] = "local-model";
        status["available"] = true;
    }

    status["version"] = "2.0.0";
    return status;
}

QJsonObject AiService::chat(const QJsonObject& data) {
    if (isAiServerAvailable()) {
        QJsonObject aiResult = chatWithAi(data);
        if (!hasAiError(aiResult)) {
            aiResult["aiPowered"] = true;
            return aiResult;
        }
        resetAiServerCheck();
    }

    // 外部AI不可用时，使用本地规则分析
    return chatLocal(data);
}

QJsonObject AiService::chatLocal(const QJsonObject& data) {
    QString userMessage = data["message"].toString().trimmed();
    QJsonObject localResult = generateLocalReply(userMessage);
    localResult["type"] = "chat";
    localResult["aiPowered"] = false;
    return localResult;
}

QJsonObject AiService::generateLocalReply(const QString& message) {
    // 基于关键词的本地规则回复生成
    QString msg = message.toLower();

    if (msg.contains("课程") || msg.contains("gpa") || msg.contains("成绩") || msg.contains("学分")) {
        return buildCourseAdvice();
    }
    if (msg.contains("目标") || msg.contains("规划") || msg.contains("计划") || msg.contains("进度")) {
        return buildGoalAdvice();
    }
    if (msg.contains("经历") || msg.contains("实习") || msg.contains("项目") || msg.contains("经验")) {
        return buildExperienceAdvice();
    }
    if (msg.contains("简历") || msg.contains("求职") || msg.contains("岗位") || msg.contains("面试")) {
        return buildResumeAdvice();
    }
    if (msg.contains("成果") || msg.contains("竞赛") || msg.contains("奖项") || msg.contains("证书")) {
        return buildAchievementAdvice();
    }
    if (msg.contains("建议") || msg.contains("分析") || msg.contains("综合")) {
        return buildComprehensiveAdvice();
    }

    // 默认回复
    return buildDefaultReply();
}

QJsonObject AiService::buildCourseAdvice() {
    QJsonObject result;
    CourseService cs;
    QList<Course> courses = cs.getAll();
    int completed = 0;
    int inProgress = 0;
    double totalGpa = 0;
    int gpaCount = 0;

    for (const Course& c : courses) {
        if (c.status == "Completed") completed++;
        else if (c.status == "In Progress") inProgress++;
        if (c.gradePoint > 0) {
            totalGpa += c.gradePoint;
            gpaCount++;
        }
    }

    double avgGpa = gpaCount > 0 ? totalGpa / gpaCount : 0;
    QStringList tips;
    tips.append(QString("当前共 %1 门课程，其中已完成 %2 门，进行中 %3 门。").arg(courses.size()).arg(completed).arg(inProgress));
    if (avgGpa > 0) {
        tips.append(QString("平均 GPA 约为 %1。").arg(avgGpa, 0, 'f', 2));
    }
    if (avgGpa < 3.0) {
        tips.append("建议重点关注专业课学习，尝试优化学习方法或参加学习小组。");
    } else if (avgGpa >= 3.5) {
        tips.append("GPA 表现优秀，可考虑挑战更高难度课程或参与科研项目。");
    } else {
        tips.append("建议保持当前学习节奏，同时注重实践能力积累。");
    }

    result["reply"] = QString(
        "【课程分析】\n\n%1\n\n"
        "📚 学习建议：\n%2\n\n"
        "💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。")
        .arg(tips.join("\n"))
        .arg(tips.mid(2).join("\n"));
    return result;
}

QJsonObject AiService::buildGoalAdvice() {
    QJsonObject result;
    GoalService gs;
    QList<Goal> goals = gs.getAll();
    int completed = 0;
    int inProgress = 0;
    double totalProgress = 0;

    for (const Goal& g : goals) {
        if (g.status == "Completed") completed++;
        else if (g.status == "In Progress") inProgress++;
        totalProgress += g.progress();
    }

    double avgProgress = goals.size() > 0 ? totalProgress / goals.size() : 0;
    QStringList tips;
    tips.append(QString("当前共 %1 个目标，已完成 %2 个，进行中 %3 个。").arg(goals.size()).arg(completed).arg(inProgress));
    tips.append(QString("平均完成进度 %1%。").arg(avgProgress, 0, 'f', 1));
    if (avgProgress < 30) {
        tips.append("目标推进较慢，建议将大目标拆分为小步骤，每天完成一点点。");
    } else if (avgProgress >= 80) {
        tips.append("目标完成情况良好，注意保持节奏并及时调整计划。");
    }

    result["reply"] = QString(
        "【目标追踪】\n\n%1\n\n"
        "📋 规划建议：\n%2\n\n"
        "💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。")
        .arg(tips.join("\n"))
        .arg(tips.mid(2).join("\n"));
    return result;
}

QJsonObject AiService::buildExperienceAdvice() {
    QJsonObject result;
    ExperienceService es;
    QList<Experience> exps = es.getAll();
    int ongoing = 0;
    QStringList types;
    for (const Experience& e : exps) {
        if (e.isOngoing) ongoing++;
        if (!e.type.isEmpty() && !types.contains(e.type)) types.append(e.type);
    }

    QStringList tips;
    tips.append(QString("当前共 %1 条经历记录，进行中 %2 条，涵盖 %3 种类型。").arg(exps.size()).arg(ongoing).arg(types.size()));
    if (exps.size() < 3) {
        tips.append("经历积累较少，建议积极寻找实习和项目机会，丰富实践经验。");
    } else if (types.size() >= 3) {
        tips.append("经历类型丰富，建议梳理一条清晰的能力成长主线，便于简历展示。");
    }

    result["reply"] = QString(
        "【经历分析】\n\n%1\n\n"
        "💼 实践建议：\n%2\n\n"
        "💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。")
        .arg(tips.join("\n"))
        .arg(tips.mid(2).join("\n"));
    return result;
}

QJsonObject AiService::buildResumeAdvice() {
    QJsonObject result;
    CourseService cs;
    ExperienceService es;
    AchievementService as;
    QJsonObject advice;

    int courseCount = cs.getAll().size();
    int expCount = es.getAll().size();
    int achCount = as.getAll().size();

    QStringList tips;
    tips.append(QString("当前简历素材统计：课程 %1 门、经历 %2 条、成果 %3 项。").arg(courseCount).arg(expCount).arg(achCount));

    if (achCount < 5) {
        tips.append("成果积累偏少，建议积极参与学科竞赛或获取专业认证。");
    }
    if (expCount < 2) {
        tips.append("实践经历不足，建议尽早规划实习，积累职场经验。");
    }
    if (courseCount < 10) {
        tips.append("课程信息不够完整，建议完善课程库以便生成更准确的简历。");
    }

    result["reply"] = QString(
        "【简历建议】\n\n%1\n\n"
        "📝 简历优化建议：\n%2\n\n"
        "💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。")
        .arg(tips.join("\n"))
        .arg(tips.mid(2).join("\n"));
    return result;
}

QJsonObject AiService::buildAchievementAdvice() {
    QJsonObject result;
    AchievementService as;
    QList<Achievement> achs = as.getAll();
    int verified = 0;
    QStringList levels;
    for (const Achievement& a : achs) {
        if (a.verified) verified++;
        if (!a.level.isEmpty() && !levels.contains(a.level)) levels.append(a.level);
    }

    QStringList tips;
    tips.append(QString("当前共 %1 项成果，已验证 %2 项，覆盖 %3 个级别。").arg(achs.size()).arg(verified).arg(levels.size()));
    if (achs.size() < 5) {
        tips.append("成果记录较少，建议积极参加学科竞赛和获取专业证书。");
    }
    if (verified < achs.size() / 2) {
        tips.append("部分成果尚未验证，建议补充证书或证明材料以提升可信度。");
    }

    result["reply"] = QString(
        "【成果分析】\n\n%1\n\n"
        "🏆 成果积累建议：\n%2\n\n"
        "💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。")
        .arg(tips.join("\n"))
        .arg(tips.mid(2).join("\n"));
    return result;
}

QJsonObject AiService::buildComprehensiveAdvice() {
    QJsonObject result;
    CourseService cs;
    ExperienceService es;
    AchievementService as;
    GoalService gs;

    QList<Course> courses = cs.getAll();
    QList<Goal> goals = gs.getAll();
    int completedGoals = 0;
    double avgProgress = 0;
    for (const Goal& g : goals) {
        if (g.status == "Completed") completedGoals++;
        avgProgress += g.progress();
    }
    if (!goals.isEmpty()) avgProgress /= goals.size();

    int expCount = es.getAll().size();
    int achCount = as.getAll().size();
    int completedCourses = 0;
    for (const Course& c : courses) {
        if (c.status == "Completed") completedCourses++;
    }

    QStringList summary;
    summary.append(QString("📊 课程：%1/%2 门已完成").arg(completedCourses).arg(courses.size()));
    summary.append(QString("🎯 目标：%1/%2 个已完成，平均进度 %3%").arg(completedGoals).arg(goals.size()).arg(avgProgress, 0, 'f', 0));
    summary.append(QString("💼 经历：%1 条").arg(expCount));
    summary.append(QString("🏆 成果：%1 项").arg(achCount));

    QStringList suggestions;
    if (completedCourses < courses.size() * 0.5) {
        suggestions.append("• 建议优先完成必修课程，保障学业进度");
    }
    if (avgProgress < 50) {
        suggestions.append("• 目标进度偏慢，建议重新评估计划可行性并拆分步骤");
    }
    if (expCount < 3) {
        suggestions.append("• 建议利用寒暑假积累实习经验");
    }
    if (achCount < 5) {
        suggestions.append("• 建议积极参加学科竞赛或获取专业认证");
    }
    if (suggestions.isEmpty()) {
        suggestions.append("• 当前发展状态良好，建议继续保持并定期复盘");
    }

    result["reply"] = QString(
        "【综合学业分析】\n\n"
        "📋 当前发展概况：\n%1\n\n"
        "💡 发展建议：\n%2\n\n"
        "💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。")
        .arg(summary.join("\n"))
        .arg(suggestions.join("\n"));
    return result;
}

QJsonObject AiService::buildDefaultReply() {
    QJsonObject result;
    QStringList options;
    options.append("💬 你可以问我以下问题：");
    options.append("• 课程相关 — 分析我的课程成绩和学习建议");
    options.append("• 目标相关 — 分析目标完成进度和优化建议");
    options.append("• 经历相关 — 分析实践经历分布和建议");
    options.append("• 简历相关 — 给出简历优化方向");
    options.append("• 成果相关 — 分析成果积累情况");
    options.append("• 综合建议 — 给出整体发展规划建议");
    options.append("\n💡 提示：这是基于本地规则的简单分析。如需更深入的AI分析，可启动本地大模型服务（默认端口 8001）。");

    result["reply"] = options.join("\n");
    return result;
}

QJsonObject AiService::chatWithAi(const QJsonObject& data) {
    QString message = data["message"].toString();
    
    QJsonObject requestData;
    QJsonArray messages;
    messages.append(QJsonObject{
        {"role", "user"},
        {"content", message}
    });
    requestData["messages"] = messages;
    requestData["max_tokens"] = 256;
    requestData["temperature"] = 0.3;
    
    QJsonObject response = callAiServer("/v1/chat/completions", requestData);
    
    QJsonObject result;
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            result["reply"] = choices[0].toObject()["message"].toObject()["content"].toString();
            result["aiPowered"] = true;
            return result;
        }
    }
    
    result["error"] = response.contains("error")
        ? response["error"].toString()
        : QStringLiteral("未知错误");
    result["aiPowered"] = false;
    return result;
}

QJsonObject AiService::healthCheck()
{
    QJsonObject result;
    result["aiServerAvailable"] = isAiServerAvailable();
    result["aiServerUrl"] = m_aiServerUrl;
    result["backendUrl"] = m_backendUrl;
    result["mode"] = currentMode();
    return result;
}

QString AiService::currentMode()
{
    return isAiServerAvailable() ? "remote-model" : "rule-based";
}

bool AiService::isRuleMode()
{
    return false;
}

bool AiService::isRemoteMode()
{
    return isAiServerAvailable();
}
