#include "task.h"

TaskTable
tasktable_new(void)
{
    return (TaskTable) {
        .next_tid = 0
    };
}

// TODO should introduce error codes
void
tasktable_create_task(TaskTable* this, uint32_t priority)
{
    Task new_task = (Task) {
        .tid = this->next_tid,
        .priority = priority
    };
    this->tasks[this->next_tid] = new_task;

    ++(this->next_tid);
}

int
Create(int priority, void (*function)())
{

}

int
MyTid(void)
{

}

int
MyParentTid(void)
{

}

void
Yield(void)
{

}

void
Exit(void)
{

}
