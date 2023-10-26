#include <trainstd.h>
#include "task.h"
#include "addrspace.h"
#include "alloc.h"

#define RECEIVE_QUEUE_MAX_LEN 32

typedef struct TaskTable TaskTable;

static TaskTable tasktable;
static Tid current_task;

struct TaskTable {
    u32 next_tid;
    TaskNode* task_nodes[TASK_TABLE_SIZE];
};

void
tasktable_init(void)
{
    current_task = 1; // uninitalized
    tasktable = (TaskTable) {
        .next_tid = 1
    };
    for (u32 i = 0; i < TASK_TABLE_SIZE; i++) {
        tasktable.task_nodes[i] = nullptr;
    }
}

// TODO should introduce error codes
Tid
tasktable_create_task(u32 priority, void (*entrypoint)(), const char* name)
{
    Addrspace addrspace = pagetable_createpage();

    Tid new_task_id = (tasktable.next_tid)++;

    Task* new_task = kalloc(sizeof(Task));
    SwitchFrame* sf = kalloc(sizeof(SwitchFrame));
    *sf = switchframe_new(addrspace.stackbase, entrypoint);

    SendBuf* send_buf = kalloc(sizeof(SendBuf)); 
    *send_buf = (SendBuf) {
        .in_use = false
    };

    ReceiveBuf* receive_buf = kalloc(sizeof(ReceiveBuf)); 
    *receive_buf = (ReceiveBuf) {
        .in_use = false
    };

    *new_task = (Task) {
        .tid = new_task_id,
        .parent_tid = 0,
        .name = name,
        .state = TASKSTATE_READY,
        .priority = priority,
        .addrspace = addrspace,
        .sf = sf,
        .receive_queue = cbuf_new(RECEIVE_QUEUE_MAX_LEN),
        .send_buf = send_buf,
        .receive_buf = receive_buf,
        .blocking_event = EVENT_NONE
    };

    TaskNode* new_task_node = kalloc(sizeof(TaskNode));
    new_task_node->task = new_task;
    new_task_node->next = nullptr;

    u32 index = new_task_id % TASK_TABLE_SIZE;
    if (!tasktable.task_nodes[index]) {
        tasktable.task_nodes[index] = new_task_node;
    }
    else {
        TaskNode* current = tasktable.task_nodes[index];
        for (;;) {
            if (current->next == nullptr) {
                current->next = new_task_node;
                break;
            }
            current = current->next;
        }
    }

    return new_task_id;
}

Task*
tasktable_get_task(Tid tid)
{
    u32 index = tid % TASK_TABLE_SIZE;
    for (TaskNode* current = tasktable.task_nodes[index]; current != nullptr; current = current->next) {
        if (current->task->tid == tid) {
            return current->task;
        }
    }
    PANIC("getting invalid tid %d", tid);
    return nullptr;
}

void
tasktable_set_current_task(Tid tid)
{
    // make old current task ready
    /* tasktable_get_task(current_task)->state = TASKSTATE_READY; */

    tasktable_get_task(tid)->state = TASKSTATE_ACTIVE;
    current_task = tid;
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
    task->state = TASKSTATE_EXITED;
    // don't free it for now
    /* kfree(task->sf); */
    /* kfree(task); */
    // tasktable.tasks[tid] = nullptr;
}
