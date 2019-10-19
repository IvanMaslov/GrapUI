#ifndef ABSTRACT_JOB_H
#define ABSTRACT_JOB_H

#include <atomic>
#include <mutex>

#include <task_executor.h>

class abstract_job
{
public:
    abstract_job(task_executor& te) : executor(te) {}
    virtual ~abstract_job() { stop(); }
    virtual void start() = 0;
    void stop() { cancelled.store(true); }
    bool is_shutdown() { return cancelled || executor.is_shutdown(); }
protected:
    void run_subtask(std::shared_ptr<abstract_task> task) { executor.schedule(task); }
    void run_subtasks(std::vector<std::shared_ptr<abstract_task>> task) { executor.schedule(task); }

    mutable std::mutex res;
    std::atomic_bool started = false;
    std::atomic_bool cancelled = false;
private:
    task_executor& executor;
};

#endif // ABSTRACT_JOB_H
