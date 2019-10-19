#ifndef BACKGROUND_JOB_H
#define BACKGROUND_JOB_H

#include <QObject>

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <queue>
#include <vector>

#include <abstract_task.h>

class task_executor : public QObject
{
    Q_OBJECT
public:
    task_executor() : working(false) { start(); }
    ~task_executor() { finish(); }
    void start();
    void finish();
    void schedule(std::shared_ptr<abstract_task>);
    void schedule(std::vector<std::shared_ptr<abstract_task>>);
    bool is_shutdown() const { return !working.load(); }
    bool is_working() const { return working.load(); }
    size_t sheduled_tasks() { return tasks.size(); }
private:
    static const size_t thread_count = 5;
    static const size_t task_limit = 500000;

    std::unique_ptr<std::thread> executors[thread_count];

    mutable std::mutex pool;
    mutable std::mutex reboot;

    std::condition_variable cv;
    std::atomic_bool working;
    std::queue<std::shared_ptr<abstract_task>> tasks;
};

#endif // BACKGROUND_JOB_H
