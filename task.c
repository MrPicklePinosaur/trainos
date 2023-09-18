#include "task.h"
#include "addrspace.h"

static TaskTable tasktable;
static Tid current_task;

void
tasktable_init(void)
{
    current_task = 1; // uninitalized
    tasktable = (TaskTable) {
        .next_tid = 1
    };
}

// TODO should introduce error codes
Tid
tasktable_create_task(uint32_t priority)
{
    Addrspace addrspace = pagetable_createpage();

    Tid new_task_id = (tasktable.next_tid)++;

    Task new_task = (Task) {
        .tid = new_task_id,
        .priority = priority,
        .addrspace = addrspace,
    };
    tasktable.tasks[new_task_id] = new_task;

    return new_task_id;
}

Task*
tasktable_get_task(Tid tid)
{
    // TODO check if task actually exists
    return &tasktable.tasks[tid];
}

void
tasktable_set_current_task(Tid task)
{
    current_task = task;
}

Tid
tasktable_current_task(void)
{
    return current_task;
}
