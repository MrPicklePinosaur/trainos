#include "task.h"
#include "addrspace.h"
#include "alloc.h"
#include "log.h"

typedef struct TaskTable TaskTable;

static TaskTable tasktable;
static Tid current_task;

struct TaskTable {
    uint32_t next_tid;
    Task* tasks[MAX_TASK_COUNT];
};

void
tasktable_init(void)
{
    current_task = 1; // uninitalized
    tasktable = (TaskTable) {
        .next_tid = 1,
        .tasks = {0}
    };
}

// TODO should introduce error codes
Tid
tasktable_create_task(uint32_t priority, void (*entrypoint)())
{
    Addrspace addrspace = pagetable_createpage();

    Tid new_task_id = (tasktable.next_tid)++;

    Task* new_task = arena_alloc(sizeof(Task));
    SwitchFrame* sf = arena_alloc(sizeof(SwitchFrame));
    *sf = switchframe_new(addrspace.stackbase, entrypoint);

    *new_task = (Task) {
        .tid = new_task_id,
        .priority = priority,
        .addrspace = addrspace,
        .sf = sf
    };

    tasktable.tasks[new_task_id] = new_task;

    return new_task_id;
}

Task*
tasktable_get_task(Tid tid)
{
    // TODO check if task actually exists
    /* if (tasktable.tasks[tid] == nullptr) LOG_WARN("getting invalid tid %d", tid); */

    return tasktable.tasks[tid];
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

void
tasktable_delete_task(Tid tid)
{
    Task* task = tasktable_get_task(tid);
    arena_free(task->sf);
    arena_free(task);
    tasktable.tasks[tid] = nullptr;
}
