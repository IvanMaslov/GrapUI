#ifndef ABSTRACT_TASK_H
#define ABSTRACT_TASK_H
#include <settings.h>
#include <stdexcept>

class abstract_task
{
public:
    abstract_task() {}
    virtual ~abstract_task() {}
    virtual void execute() = 0;
};

class task_error : public std::runtime_error {
public:
    task_error(const std::string s) : runtime_error(s) {}
    ~task_error() {}
};

#endif // ABSTRACT_TASK_H
