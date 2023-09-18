#include "task.h"
#include "addrspace.h"

static TaskTable tasktable;

void
tasktable_init(void)
{
    tasktable = (TaskTable) {
        .next_tid = 0
    };
}

// TODO should introduce error codes
void
tasktable_create_task(uint32_t priority)
{
    Addrspace addrspace = pagetable_createpage();

    Task new_task = (Task) {
        .tid = tasktable.next_tid,
        .priority = priority,
        .addrspace = addrspace,
    };
    tasktable.tasks[tasktable.next_tid] = new_task;

    ++(tasktable.next_tid);
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
