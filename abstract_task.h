#ifndef ABSTRACT_TASK_H
#define ABSTRACT_TASK_H

class abstract_task
{
public:
    abstract_task();
    virtual ~abstract_task();
    virtual void execute();
};

#endif // ABSTRACT_TASK_H
