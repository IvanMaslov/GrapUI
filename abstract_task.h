#ifndef ABSTRACT_TASK_H
#define ABSTRACT_TASK_H
#include <settings.h>

class abstract_task
{
public:
    abstract_task() {}
    virtual ~abstract_task() {}
    virtual void execute() = 0;
};

#endif // ABSTRACT_TASK_H
