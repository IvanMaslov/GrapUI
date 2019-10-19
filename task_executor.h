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
    task_executor();
    ~task_executor();
    void schedule(abstract_task);
    void schedule(std::vector<abstract_task>);
    bool is_shutdown() const { return quit.load(); }
private:
    static const size_t thread_count = 3;

    std::unique_ptr<std::thread> executors[thread_count];

    mutable std::mutex pool;

    std::condition_variable cv;
    std::atomic_bool quit;
    std::queue<abstract_task> tasks;
};

#endif // BACKGROUND_JOB_H
