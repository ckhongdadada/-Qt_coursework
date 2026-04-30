#ifndef COURSESERVICE_H
#define COURSESERVICE_H

#include <QList>
#include "model/Course.h"
#include "dao/CourseDao.h"

class CourseService {
public:
    static QList<Course> getAll() {
        CourseDao dao;
        return dao.getAll();
    }

    static Course getById(int id) {
        CourseDao dao;
        return dao.getById(id);
    }

    static Course create(Course& course) {
        if (course.score > 0) {
            course.gradePoint = Course::calculateGradePoint(course.score);
        }
        CourseDao dao;
        dao.create(course);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) course = dao.getById(lastId);
        return course;
    }

    static Course update(int id, Course& course) {
        CourseDao dao;
        Course existing = dao.getById(id);
        if (existing.id == 0) return existing;
        course.id = id;
        if (course.score > 0) {
            course.gradePoint = Course::calculateGradePoint(course.score);
        } else {
            course.gradePoint = 0;
        }
        dao.update(course);
        return dao.getById(id);
    }

    static bool remove(int id) {
        CourseDao dao;
        return dao.remove(id);
    }

    static QJsonObject getStatistics(const QString& scale = "standard", double creditTarget = 120) {
        CourseDao dao;
        QList<Course> scoredCourses = dao.getScored();
        QList<Course> completedCourses = dao.getCompleted();
        double totalCompletedCredits = 0;
        for (const auto& c : completedCourses) totalCompletedCredits += c.credits;
        double target = creditTarget > 0 ? creditTarget : 120;

        double gpa = calculateGPA(scoredCourses, scale);
        double weightedAvg = calculateWeightedAverage(scoredCourses);
        double totalCredits = 0;
        for (const auto& c : scoredCourses) totalCredits += c.credits;

        QJsonObject stats;
        stats["totalCourses"] = dao.getTotalCount();
        stats["completedCourses"] = dao.getCompletedCount();
        stats["gpa"] = gpa;
        stats["weightedAverage"] = weightedAvg;
        stats["totalCredits"] = qRound(totalCredits * 100) / 100.0;
        stats["completedCredits"] = qRound(totalCompletedCredits * 100) / 100.0;
        stats["creditTarget"] = target;
        stats["creditProgress"] = target > 0 ? qRound(std::min(totalCompletedCredits / target * 100.0, 100.0) * 100) / 100.0 : 0;
        stats["gpaScale"] = scale;
        stats["categoryBreakdown"] = getCategoryStatistics(scoredCourses);
        stats["semesterStats"] = getSemesterStatistics(scale);
        return stats;
    }

    static QJsonArray getSemesterStatistics(const QString& scale = "standard") {
        CourseDao dao;
        QList<Course> courses = dao.getScored();
        QMap<QString, QJsonObject> semesterMap;

        for (const auto& course : courses) {
            if (course.semester.isEmpty()) continue;
            QString sem = course.semester;
            if (!semesterMap.contains(sem)) {
                QJsonObject obj;
                obj["semester"] = sem;
                obj["count"] = 0;
                obj["credits"] = 0.0;
                obj["scoreTotal"] = 0.0;
                obj["scoreCount"] = 0;
                obj["gpaWeightedTotal"] = 0.0;
                obj["gpaCredits"] = 0.0;
                semesterMap[sem] = obj;
            }
            QJsonObject obj = semesterMap[sem];
            double credits = course.credits;
            obj["count"] = obj["count"].toInt() + 1;
            obj["credits"] = qRound((obj["credits"].toDouble() + credits) * 100) / 100.0;
            obj["scoreTotal"] = obj["scoreTotal"].toDouble() + course.score;
            obj["scoreCount"] = obj["scoreCount"].toInt() + 1;
            double gp = Course::calculateGradePoint(course.score, scale);
            if (credits > 0 && gp > 0) {
                obj["gpaWeightedTotal"] = obj["gpaWeightedTotal"].toDouble() + gp * credits;
                obj["gpaCredits"] = obj["gpaCredits"].toDouble() + credits;
            }
            semesterMap[sem] = obj;
        }

        QJsonArray result;
        for (auto it = semesterMap.begin(); it != semesterMap.end(); ++it) {
            QJsonObject obj = it.value();
            int scoreCount = obj["scoreCount"].toInt();
            double gpaCredits = obj["gpaCredits"].toDouble();
            obj["avgScore"] = scoreCount > 0 ? qRound(obj["scoreTotal"].toDouble() / scoreCount * 100) / 100.0 : 0;
            obj["gpa"] = gpaCredits > 0 ? qRound(obj["gpaWeightedTotal"].toDouble() / gpaCredits * 100) / 100.0 : 0;
            obj.remove("scoreTotal");
            obj.remove("scoreCount");
            obj.remove("gpaWeightedTotal");
            obj.remove("gpaCredits");
            result.append(obj);
        }
        return result;
    }

private:
    static double calculateGPA(const QList<Course>& courses, const QString& scale = "standard") {
        double totalCredits = 0, totalGP = 0;
        for (const auto& c : courses) {
            if (c.score > 0 && c.credits > 0) {
                double gp = Course::calculateGradePoint(c.score, scale);
                if (gp > 0) {
                    totalCredits += c.credits;
                    totalGP += gp * c.credits;
                }
            }
        }
        return totalCredits > 0 ? qRound(totalGP / totalCredits * 100) / 100.0 : 0;
    }

    static double calculateWeightedAverage(const QList<Course>& courses) {
        double totalCredits = 0, totalScore = 0;
        for (const auto& c : courses) {
            if (c.score > 0 && c.credits > 0) {
                totalCredits += c.credits;
                totalScore += c.score * c.credits;
            }
        }
        return totalCredits > 0 ? qRound(totalScore / totalCredits * 100) / 100.0 : 0;
    }

    static QJsonArray getCategoryStatistics(const QList<Course>& courses) {
        QMap<QString, QJsonObject> catMap;
        for (const auto& c : courses) {
            QString cat = c.category.isEmpty() ? "Other" : c.category;
            if (!catMap.contains(cat)) {
                QJsonObject obj;
                obj["category"] = cat;
                obj["count"] = 0;
                obj["credits"] = 0.0;
                obj["scoreTotal"] = 0.0;
                obj["scoreCount"] = 0;
                catMap[cat] = obj;
            }
            QJsonObject obj = catMap[cat];
            obj["count"] = obj["count"].toInt() + 1;
            obj["credits"] = qRound((obj["credits"].toDouble() + c.credits) * 100) / 100.0;
            if (c.score > 0) {
                obj["scoreTotal"] = obj["scoreTotal"].toDouble() + c.score;
                obj["scoreCount"] = obj["scoreCount"].toInt() + 1;
            }
            catMap[cat] = obj;
        }
        QJsonArray result;
        for (auto it = catMap.begin(); it != catMap.end(); ++it) {
            QJsonObject obj = it.value();
            int sc = obj["scoreCount"].toInt();
            obj["avgScore"] = sc > 0 ? qRound(obj["scoreTotal"].toDouble() / sc * 100) / 100.0 : 0;
            obj.remove("scoreTotal");
            obj.remove("scoreCount");
            result.append(obj);
        }
        return result;
    }
};

#endif // COURSESERVICE_H