#ifndef ABSTRACT_JOB_H
#define ABSTRACT_JOB_H
#include <settings.h>

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
    void run_subtask(std::unique_ptr<abstract_task> task) { executor.schedule(std::move(task)); }
    void run_subtasks(std::vector<std::unique_ptr<abstract_task>> task) { executor.schedule(std::move(task)); }

    mutable std::mutex res;
    std::atomic_bool cancelled = false;

    QString errorlog;
private:
    task_executor& executor;
};

#endif // ABSTRACT_JOB_H
