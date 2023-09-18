#include "task.h"
#include "addrspace.h"

static TaskTable tasktable;
static Tid current_task;

void
tasktable_init(void)
{
    current_task = 0; // uninitalized
    tasktable = (TaskTable) {
        .next_tid = 0
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
