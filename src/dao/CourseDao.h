#ifndef COURSEDAO_H
#define COURSEDAO_H

#include <QList>
#include "DaoBase.h"
#include "model/Course.h"

class CourseDao : public DaoBase {
public:
    QList<Course> getAll() const {
        QList<Course> courses;
        if (!isOpen()) return courses;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM courses ORDER BY updated_at DESC, id DESC");
        while (query.next()) courses.append(mapFromQuery(query));
        return courses;
    }

    QList<Course> getScored() const {
        QList<Course> courses;
        if (!isOpen()) return courses;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM courses WHERE score IS NOT NULL ORDER BY updated_at DESC");
        while (query.next()) courses.append(mapFromQuery(query));
        return courses;
    }

    QList<Course> getCompleted() const {
        QList<Course> courses;
        if (!isOpen()) return courses;
        QSqlQuery query(m_db);
        query.exec("SELECT * FROM courses WHERE status = 'Completed'");
        while (query.next()) courses.append(mapFromQuery(query));
        return courses;
    }

    QList<Course> getBySemester(const QString& semester) const {
        QList<Course> courses;
        if (!isOpen()) return courses;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM courses WHERE semester = :semester");
        query.bindValue(":semester", semester);
        query.exec();
        while (query.next()) courses.append(mapFromQuery(query));
        return courses;
    }

    Course getById(int id) const {
        Course course;
        if (!isOpen()) return course;
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM courses WHERE id = :id");
        query.bindValue(":id", id);
        query.exec();
        if (query.next()) course = mapFromQuery(query);
        return course;
    }

    bool create(const Course& course) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "INSERT INTO courses (name, code, credits, semester, category, score, grade_point, status, "
            "teacher, location, description, tags) "
            "VALUES (:name, :code, :credits, :semester, :category, :score, :grade_point, :status, "
            ":teacher, :location, :description, :tags)"
        );
        query.bindValue(":name", course.name);
        query.bindValue(":code", course.code);
        query.bindValue(":credits", course.credits);
        query.bindValue(":semester", course.semester);
        query.bindValue(":category", course.category);
        query.bindValue(":score", course.score > 0 ? QVariant(course.score) : QVariant());
        query.bindValue(":grade_point", course.gradePoint > 0 ? QVariant(course.gradePoint) : QVariant());
        query.bindValue(":status", course.status);
        query.bindValue(":teacher", course.teacher);
        query.bindValue(":location", course.location);
        query.bindValue(":description", course.description);
        query.bindValue(":tags", course.tags);
        return query.exec();
    }

    bool update(const Course& course) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare(
            "UPDATE courses SET name = :name, code = :code, credits = :credits, semester = :semester, "
            "category = :category, score = :score, grade_point = :grade_point, status = :status, "
            "teacher = :teacher, location = :location, description = :description, tags = :tags, "
            "updated_at = CURRENT_TIMESTAMP WHERE id = :id"
        );
        query.bindValue(":id", course.id);
        query.bindValue(":name", course.name);
        query.bindValue(":code", course.code);
        query.bindValue(":credits", course.credits);
        query.bindValue(":semester", course.semester);
        query.bindValue(":category", course.category);
        query.bindValue(":score", course.score > 0 ? QVariant(course.score) : QVariant());
        query.bindValue(":grade_point", course.gradePoint > 0 ? QVariant(course.gradePoint) : QVariant());
        query.bindValue(":status", course.status);
        query.bindValue(":teacher", course.teacher);
        query.bindValue(":location", course.location);
        query.bindValue(":description", course.description);
        query.bindValue(":tags", course.tags);
        return query.exec();
    }

    bool remove(int id) {
        if (!isOpen()) return false;
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM courses WHERE id = :id");
        query.bindValue(":id", id);
        return query.exec();
    }

    int getTotalCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM courses");
        if (query.next()) return query.value(0).toInt();
        return 0;
    }

    int getCompletedCount() const {
        if (!isOpen()) return 0;
        QSqlQuery query(m_db);
        query.exec("SELECT COUNT(*) FROM courses WHERE status = 'Completed'");
        if (query.next()) return query.value(0).toInt();
        return 0;
    }

    int getLastInsertId() const {
        QSqlQuery query(m_db);
        query.exec("SELECT last_insert_rowid()");
        if (query.next()) return query.value(0).toInt();
        return -1;
    }

private:
    static Course mapFromQuery(const QSqlQuery& query) {
        Course c;
        c.id = query.value("id").toInt();
        c.name = query.value("name").toString();
        c.code = query.value("code").toString();
        c.credits = query.value("credits").toDouble();
        c.semester = query.value("semester").toString();
        c.category = query.value("category").toString();
        c.score = query.value("score").isNull() ? 0 : query.value("score").toDouble();
        c.gradePoint = query.value("grade_point").isNull() ? 0 : query.value("grade_point").toDouble();
        c.status = query.value("status").toString();
        c.teacher = query.value("teacher").toString();
        c.location = query.value("location").toString();
        c.description = query.value("description").toString();
        c.tags = query.value("tags").toString();
        c.createdAt = query.value("created_at").toDateTime();
        c.updatedAt = query.value("updated_at").toDateTime();
        return c;
    }
};

#endif // COURSEDAO_H