#ifndef ABSTRACT_JOB_H
#define ABSTRACT_JOB_H

#include <atomic>
#include <mutex>

#include <task_executor.h>

class abstract_job
{
public:
    abstract_job(task_executor&);
    virtual ~abstract_job();
    virtual void start();
    virtual void stop();
    bool shutdown() { return cancelled || executor.shutdown(); }
    void run_subtask(abstract_task task) { executor.schedule(task); }
    void run_subtasks(abstract_task task) { executor.schedule(task); }
private:
    mutable std::mutex res;
    std::atomic_bool started = false;
    std::atomic_bool cancelled = false;

    task_executor& executor;
};

#endif // ABSTRACT_JOB_H
