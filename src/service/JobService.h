#ifndef JOBSERVICE_H
#define JOBSERVICE_H

#include <QList>
#include "model/Job.h"
#include "dao/JobDao.h"

class JobService {
public:
    static QList<Job> getAll() {
        JobDao dao;
        return dao.getAll();
    }

    static Job getById(int id) {
        JobDao dao;
        return dao.getById(id);
    }

    static Job create(Job& j) {
        JobDao dao;
        dao.create(j);
        int lastId = dao.getLastInsertId();
        if (lastId > 0) j = dao.getById(lastId);
        return j;
    }

    static Job update(int id, Job& j) {
        JobDao dao;
        Job existing = dao.getById(id);
        if (existing.id == 0) return existing;
        j.id = id;
        dao.update(j);
        return dao.getById(id);
    }

    static bool remove(int id) {
        JobDao dao;
        return dao.remove(id);
    }

    static Job importJobs(const QJsonArray& jobsData) {
        JobDao dao;
        Job lastCreated;
        for (const auto& item : jobsData) {
            Job j = Job::fromDict(item.toObject());
            dao.create(j);
            int lastId = dao.getLastInsertId();
            if (lastId > 0) lastCreated = dao.getById(lastId);
        }
        return lastCreated;
    }

    static Job toggleRequirement(int jobId, int reqIndex) {
        JobDao dao;
        Job j = dao.getById(jobId);
        if (j.id == 0) return j;
        if (reqIndex >= 0 && reqIndex < j.requirements.size()) {
            j.requirements[reqIndex].met = !j.requirements[reqIndex].met;
            dao.update(j);
            return dao.getById(jobId);
        }
        return j;
    }
};

#endif // JOBSERVICE_H