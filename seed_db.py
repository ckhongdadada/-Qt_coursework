import sqlite3
import random
from datetime import datetime

def seed_db():
    conn = sqlite3.connect('pdp.db')
    c = conn.cursor()

    # Seed Courses
    courses = [
        ("深入理解计算机系统", "CS201", 4.0, "2025-秋季", "Required", 92, 4.0, "Completed", "Dr. Smith", "A-101", "系统编程神课", "核心,底层"),
        ("高级算法设计", "CS302", 3.5, "2026-春季", "Required", 88, 3.7, "Completed", "Dr. Wang", "B-202", "动态规划与图论", "算法"),
        ("人工智能导论", "CS401", 3.0, "2026-秋季", "Elective", 95, 4.0, "Completed", "Dr. Li", "C-303", "机器学习基础", "AI,ML"),
        ("数据科学实践", "DS101", 3.0, "2027-春季", "Elective", 0, 0, "In Progress", "Dr. Zhao", "D-404", "数据清洗与可视化", "数据分析"),
        ("软件工程实习", "SE500", 2.0, "2027-夏季", "Required", 0, 0, "Planned", "-", "-", "企业实习准备", "实践")
    ]
    c.execute("DELETE FROM courses")
    for cr in courses:
        c.execute("""INSERT INTO courses (name, code, credits, semester, category, score, grade_point, status, teacher, location, description, tags)
                     VALUES (?,?,?,?,?,?,?,?,?,?,?,?)""", cr)

    # Seed Roles
    roles = [
        ("软件开发实习生", "Intern", "腾讯科技", "负责后台管理系统迭代", "2025-06-01", "2025-09-01", 0, "重构了支付模块", "hr@tencent.com", "张三"),
        ("算法学长助手", "TA", "计算机学院", "协助批改作业并组织答疑", "2026-03-01", "", 1, "提升了代码评审能力", "teacher@edu", "李四")
    ]
    c.execute("DELETE FROM roles")
    for r in roles:
        c.execute("""INSERT INTO roles (title, type, organization, description, start_date, end_date, is_active, achievements, contact, supervisor)
                     VALUES (?,?,?,?,?,?,?,?,?,?)""", r)

    # Seed Achievements
    achievements = [
        ("全国大学生数学建模大赛", "竞赛", "国家级", "教育部", "获国家二等奖", "2025-10-15", "链接1", "", "王五, 赵六", 2, "国二", 1),
        ("ACM-ICPC 区域赛", "竞赛", "省级", "ACM协理会", "获得铜牌", "2025-11-20", "", "高级算法设计", "队伍A", 3, "铜牌", 1),
        ("校级优秀奖学金", "荣誉", "校级", "学校教务处", "学业排名前5%", "2026-04-10", "", "", "", 1, "一等奖", 1)
    ]
    c.execute("DELETE FROM achievements")
    for a in achievements:
        c.execute("""INSERT INTO achievements (title, type, level, organization, description, date, certificate, related_course, team_members, ranking, prize, verified)
                     VALUES (?,?,?,?,?,?,?,?,?,?,?,?)""", a)

    # Seed Experiences
    experiences = [
        ("开源社区贡献者", "项目", "GitHub", "Core Contributor", "提交并合并了超过10个核心PR", "2025-01-01", "2026-01-01", 0, "C++, Qt", "优化性能", "", "", "Remote", "github.com"),
        ("字节跳动青训营", "培训", "字节跳动", "学员", "完成全栈微服务项目大作业", "2026-07-01", "2026-08-30", 0, "Golang, Redis", "获优秀团队", "", "", "Beijing", "")
    ]
    c.execute("DELETE FROM experiences")
    for e in experiences:
        c.execute("""INSERT INTO experiences (title, type, organization, role, description, start_date, end_date, is_ongoing, technologies, achievements, supervisor, contact, location, url)
                     VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)""", e)

    # Seed Goals
    goals = [
        ("斩获大厂秋招 Offer", "职业", "进入一线互联网公司核心业务线", 100, 40, "进度", "2027-09-30", "High", "In Progress", "准备简历, 刷题300道"),
        ("雅思达到 7.5 分", "学术", "出国交换筹备", 7.5, 6.0, "分", "2026-12-31", "Medium", "In Progress", "听说读写特训"),
        ("完成开源操作系统开发", "技术", "毕业设计核心要求", 100, 100, "%", "2025-12-01", "High", "Completed", "内核启动, 内存管理")
    ]
    c.execute("DELETE FROM goals")
    for g in goals:
        c.execute("""INSERT INTO goals (title, category, description, target_value, current_value, unit, deadline, priority, status, milestones)
                     VALUES (?,?,?,?,?,?,?,?,?,?)""", g)

    conn.commit()
    conn.close()
    print("Database seeded successfully with rich mock data!")

if __name__ == '__main__':
    seed_db()
