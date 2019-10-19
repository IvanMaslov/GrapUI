#include "task_executor.h"

void task_executor::start() {
    std::lock_guard<std::mutex> LG(reboot);
    if(is_working()) return;
    working.store(true);
    {
        std::lock_guard<std::mutex> lg(pool);
        while(!tasks.empty())
            tasks.pop();
    }
    for(size_t i = 0; i < thread_count; ++i){
        executors[i].reset( new std::thread([this] {
            while(true) {
                std::unique_lock<std::mutex> lg(pool);
                cv.wait(lg, [this] {
                    return !tasks.empty() || is_shutdown();
                });
                if (is_shutdown())
                    break;

                std::shared_ptr<abstract_task> argument = tasks.front();
                tasks.pop();
                lg.unlock();
                try {
                    argument->execute();
                } catch (const std::exception& e) {
                    ;// ignore
                }
            }
        }));
    }
}

void task_executor::finish() {
    std::lock_guard<std::mutex> LG(reboot);
    if(is_shutdown()) return;
    working.store(false);
    {
        std::lock_guard<std::mutex> lg(pool);
        while(!tasks.empty())
            tasks.pop();
    }
    cv.notify_all();
    for(size_t i = 0; i < thread_count; ++i) {
        executors[i]->join();
    }
}

void task_executor::schedule(std::shared_ptr<abstract_task> task) {
    std::unique_lock<std::mutex> lg(pool);
    tasks.push(task);
    cv.notify_one();
    if (tasks.size() > task_limit) {
        lg.unlock();
        finish();
        throw std::runtime_error("Too much tasks");
    }
}

void task_executor::schedule(std::vector<std::shared_ptr<abstract_task>> task) {
    std::unique_lock<std::mutex> lg(pool);
    for (std::shared_ptr t : task)
        tasks.push(t);
    cv.notify_all();
    if (tasks.size() > task_limit) {
        lg.unlock();
        finish();
        throw std::runtime_error("Too much tasks");
    }
}
