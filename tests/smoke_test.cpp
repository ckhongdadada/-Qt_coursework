#include <QCoreApplication>
#include <QTest>
#include <QJsonObject>
#include "service/CourseService.h"
#include "service/GoalService.h"
#include "service/AchievementService.h"
#include "service/ExperienceService.h"
#include "service/RoleService.h"
#include "service/ActivityService.h"
#include "service/PeerBenchmarkService.h"
#include "service/AiService.h"
#include "model/ImportResult.h"

class SmokeTest : public QObject {
    Q_OBJECT

private slots:
    void testCourseCRUD();
    void testGoalCRUD();
    void testAchievementCRUD();
    void testExperienceCRUD();
    void testRoleCRUD();
    void testActivityCRUD();
    void testPeerBenchmarkCRUD();
    void testAiServiceHealthCheck();
    void testImportResultModel();
};

void SmokeTest::testCourseCRUD()
{
    Course c;
    c.name = "SmokeTest课程";
    c.code = "ST101";
    c.credits = 3.0;
    c.semester = "2026春";
    c.status = "Planned";
    CourseService::create(c);

    QList<Course> all = CourseService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& course : all) {
        if (course.code == "ST101") {
            found = true;
            foundId = course.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        Course retrieved = CourseService::getById(foundId);
        QCOMPARE(retrieved.code, QString("ST101"));
        CourseService::remove(foundId);
    }
}

void SmokeTest::testGoalCRUD()
{
    Goal g;
    g.title = "SmokeTest目标";
    g.description = "测试目标创建";
    g.progress = 50;
    GoalService::create(g);

    QList<Goal> all = GoalService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& goal : all) {
        if (goal.title == "SmokeTest目标") {
            found = true;
            foundId = goal.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        GoalService::remove(foundId);
    }
}

void SmokeTest::testAchievementCRUD()
{
    Achievement a;
    a.title = "SmokeTest成果";
    a.level = "校级";
    a.date = "2026-04-29";
    AchievementService::create(a);

    QList<Achievement> all = AchievementService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& ach : all) {
        if (ach.title == "SmokeTest成果") {
            found = true;
            foundId = ach.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        AchievementService::remove(foundId);
    }
}

void SmokeTest::testExperienceCRUD()
{
    Experience e;
    e.title = "SmokeTest经历";
    e.organization = "测试组织";
    ExperienceService::create(e);

    QList<Experience> all = ExperienceService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& exp : all) {
        if (exp.title == "SmokeTest经历") {
            found = true;
            foundId = exp.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        ExperienceService::remove(foundId);
    }
}

void SmokeTest::testRoleCRUD()
{
    Role r;
    r.title = "SmokeTest角色";
    r.organization = "测试组织";
    RoleService::create(r);

    QList<Role> all = RoleService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& role : all) {
        if (role.title == "SmokeTest角色") {
            found = true;
            foundId = role.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        RoleService::remove(foundId);
    }
}

void SmokeTest::testActivityCRUD()
{
    Activity a;
    a.name = "SmokeTest活动";
    a.category = "学术";
    ActivityService::create(a);

    QList<Activity> all = ActivityService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& act : all) {
        if (act.name == "SmokeTest活动") {
            found = true;
            foundId = act.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        ActivityService::remove(foundId);
    }
}

void SmokeTest::testPeerBenchmarkCRUD()
{
    PeerBenchmark p;
    p.name = "SmokeTest同学";
    p.major = "计算机科学";
    p.gpa = 3.5;
    PeerBenchmarkService::create(p);

    QList<PeerBenchmark> all = PeerBenchmarkService::getAll();
    bool found = false;
    int foundId = 0;
    for (const auto& peer : all) {
        if (peer.name == "SmokeTest同学") {
            found = true;
            foundId = peer.id;
            break;
        }
    }
    QVERIFY(found);

    if (foundId > 0) {
        PeerBenchmarkService::remove(foundId);
    }
}

void SmokeTest::testAiServiceHealthCheck()
{
    QJsonObject health = AiService::healthCheck();
    QVERIFY(health.contains("aiServerAvailable"));
    QVERIFY(health.contains("mode"));
    QString mode = AiService::currentMode();
    QVERIFY(mode == "remote" || mode == "rule");
    QVERIFY(AiService::isRuleMode() != AiService::isRemoteMode());
}

void SmokeTest::testImportResultModel()
{
    ImportResult result;
    result.totalRows = 10;
    result.successCount = 8;
    result.entityType = "course";

    result.addError(3, "name", "名称不能为空");
    result.addWarning(5, "credits", "学分超出范围");

    QVERIFY(result.hasErrors());
    QVERIFY(result.hasWarnings());
    QCOMPARE(result.failCount, 1);
    QCOMPARE(result.skipCount, 1);
    QVERIFY(!result.isSuccess());

    QJsonObject json = result.toJson();
    ImportResult restored = ImportResult::fromJson(json);
    QCOMPARE(restored.totalRows, 10);
    QCOMPARE(restored.successCount, 8);
    QCOMPARE(restored.errors.size(), 2);

    QString summary = result.summary();
    QVERIFY(summary.contains("10"));
    QVERIFY(summary.contains("8"));
}

QTEST_MAIN(SmokeTest)
#include "smoke_test.moc"
