#include "task_executor.h"

task_executor::task_executor() :
    quit(false)
{
    for(size_t i = 0; i < thread_count; ++i){
        executors[i] = std::move(
                std::unique_ptr<std::thread>(
                        new std::thread([this] {
            while(true) {
                std::unique_lock<std::mutex> lg(pool);
                cv.wait(lg, [this] {
                    return !tasks.empty() || is_shutdown();
                });
                if (is_shutdown())
                    break;

                abstract_task argument = tasks.front();
                tasks.pop();
                lg.unlock();

                argument.execute();
            }
        })));
    }
}

task_executor::~task_executor() {
    quit.store(true);
    cv.notify_all();
    for(size_t i = 0; i < thread_count; ++i)
        executors[i]->join();
}

void task_executor::schedule(abstract_task task) {
    std::unique_lock<std::mutex> lg(pool);
    tasks.push(task);
    cv.notify_one();
}

void task_executor::schedule(std::vector<abstract_task> task) {
    std::unique_lock<std::mutex> lg(pool);
    for (abstract_task t : task)
        tasks.push(t);
    cv.notify_all();
}
