#include "task.h"

#include <stdint.h>

#define MAX_TASK_COUNT 128

typedef uint32_t TaskId;

typedef struct {
    TaskId task_id;
    uint32_t priority;
} Task;

typedef struct {
    uint32_t next_task_id;
    Task tasks[MAX_TASK_COUNT];
} TaskTable;

TaskTable tasktable_new(void);
void tasktable_create_task(TaskTable* this, uint32_t priority);


TaskTable
tasktable_new(void)
{
    return (TaskTable) {
        .next_task_id = 0
    };
}

// TODO should introduce error codes
void
tasktable_create_task(TaskTable* this, uint32_t priority)
{
    Task new_task = (Task) {
        .task_id = this->next_task_id,
        .priority = priority
    };
    this->tasks[this->next_task_id] = new_task;

    ++(this->next_task_id);
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
