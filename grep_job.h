#ifndef GREP_JOB_H
#define GREP_JOB_H

#include <abstract_job.h>

class grep_job : public abstract_job
{
public:
    grep_job(task_executor&);
    grep_job(task_executor&, QString path, QString occurency);
    ~grep_job() override;
    void start() override;
    void stop() override;
    QString getOccurency() { return occurency; }
private:
    QString occurency;
};

#endif // GREP_JOB_H
