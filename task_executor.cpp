#include "task_executor.h"

void task_executor::start() {
    {
        qDebug() << "START EXECUTOR: ";
        std::lock_guard<std::mutex> lg(pool);
        working.store(true);
        while(!tasks.empty())
            tasks.pop();
    }
    for(size_t i = 0; i < thread_count; ++i){
        executors[i].reset(new std::thread([this] {
            qDebug() << "START THREAD: ";
            while(true) {
                std::unique_lock<std::mutex> lg(pool);
                cv.wait(lg, [this] {
                    return !tasks.empty() || is_shutdown();
                });
                if (is_shutdown())
                    break;

                std::unique_ptr<abstract_task> argument = std::move(tasks.front());
                tasks.pop();
                lg.unlock();
                try {
                    argument->execute();
                } catch (const std::exception& e) {
                    qDebug() << "ERROR: *";
                    // ignore
                }
            }
        }));
    }
}

void task_executor::finish() {
    {
        qDebug() << "FINISH EXECUTOR: ";
        std::lock_guard<std::mutex> lg(pool);
        working.store(false);
        while(!tasks.empty())
            tasks.pop();
    }
    cv.notify_all();
    for(size_t i = 0; i < thread_count; ++i) {
        executors[i]->join();
        executors[i].reset();
        qDebug() << "FINISH THREAD: ";
    }
}

void task_executor::schedule(std::unique_ptr<abstract_task> task) {
    std::vector<std::unique_ptr<abstract_task>> t;
    t.push_back(std::move(task));
    schedule(std::move(t));
}

void task_executor::schedule(std::vector<std::unique_ptr<abstract_task>> task) {
    if(is_shutdown()) throw task_error("Too much tasks");
    std::unique_lock<std::mutex> lg(pool);
    for (size_t i = 0; i < task.size(); ++i)
        tasks.push(std::move(task[i]));
    cv.notify_all();
    if (tasks.size() > task_limit) {
        working.store(false);
        qDebug() << "ERROR: shedule";
        throw task_error("Too much tasks");
    }
}
