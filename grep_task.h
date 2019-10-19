#ifndef GREP_TASK_H
#define GREP_TASK_H

#include <abstract_task.h>

class grep_task : public abstract_task
{
public:
    grep_task();
    ~grep_task();
    void execute();
private:

};

#endif // GREP_TASK_H
