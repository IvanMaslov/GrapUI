#include "task_executor.h"

#include <iostream>

void task_executor::start() {
    std::lock_guard<std::mutex> LG(reboot);
    if(is_working()) return;
    std::cerr << "START EXECUTOR: " << std::endl;
    {
        std::lock_guard<std::mutex> lg(pool);
        while(!tasks.empty())
            tasks.pop();
    }
    working.store(true);
    for(size_t i = 0; i < thread_count; ++i){
        executors[i].reset(new std::thread([this] {
            std::cerr << "START THREAD: " << std::endl;
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
                    std::cerr << "ERROR: *" << std::endl;
                    ;// ignore
                }
            }
        }));
    }
}

void task_executor::finish() {
    std::lock_guard<std::mutex> LG(reboot);
    if(is_shutdown()) return;
    std::cerr << "FINISH EXECUTOR: " << std::endl;
    working.store(false);
    {
        std::lock_guard<std::mutex> lg(pool);
        while(!tasks.empty())
            tasks.pop();
    }
    cv.notify_all();
    for(size_t i = 0; i < thread_count; ++i) {
        executors[i]->join();
        executors[i].reset();
        std::cerr << "FINISH THREAD: " << std::endl;
    }
}

void task_executor::schedule(std::shared_ptr<abstract_task> task) {
    std::unique_lock<std::mutex> lg(pool);
    tasks.push(task);
    cv.notify_one();
    if (tasks.size() > task_limit) {
        std::cerr << "ERROR: shedule1" << std::endl;
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
        std::cerr << "ERROR: shedule2" << std::endl;
        lg.unlock();
        finish();
        throw std::runtime_error("Too much tasks");
    }
}
