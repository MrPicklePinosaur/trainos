#include <trainstd.h>
#include "task.h"
#include "addrspace.h"
#include "alloc.h"
#include "kern/dev/uart.h"
#include "kern/dev/timer.h"

#define RECEIVE_QUEUE_MAX_LEN 32

typedef struct TaskTable TaskTable;

static TaskTable tasktable;
static Tid current_task;

struct TaskTable {
    u32 next_tid;
    TaskNode* task_nodes[TASK_TABLE_SIZE];
};

static u64 last_time;
static u64 start_time;

void
tasktable_init(void)
{
    last_time = 0;
    start_time = timer_get();

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
        .blocking_event = EVENT_NONE,
        .enter_time = 0,
        .total_time = 0
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
    Tid current_task = tasktable_current_task();
    PANIC("getting invalid tid %d, current tid %u '%s'", tid, current_task, tasktable_get_task(current_task)->name);
    return nullptr;
}

void
tasktable_print_task_info(void)
{
    uart_printf(CONSOLE, "\033[36;1H\033[0J");
    for (u32 i = 0; i < TASK_TABLE_SIZE; i++) {
        if (tasktable.task_nodes[i]) {
            for (TaskNode* current = tasktable.task_nodes[i]; current != nullptr; current = current->next) {
                uart_printf(CONSOLE, "%d %d | ", current->task->total_time*100/(timer_get()-start_time), current->task->total_time);
                if (current->task->state == TASKSTATE_ACTIVE) {
                    uart_printf(CONSOLE, "ACTIVE");
                }
                else if (current->task->state == TASKSTATE_READY) {
                    uart_printf(CONSOLE, "READY");
                }
                else if (current->task->state == TASKSTATE_EXITED) {
                    uart_printf(CONSOLE, "EXITED");
                }
                else if (current->task->state == TASKSTATE_SEND_WAIT) {
                    uart_printf(CONSOLE, "SEND WAIT");
                }
                else if (current->task->state == TASKSTATE_RECEIVE_WAIT) {
                    uart_printf(CONSOLE, "RECEIVE WAIT");
                }
                else if (current->task->state == TASKSTATE_REPLY_WAIT) {
                    uart_printf(CONSOLE, "REPLY WAIT");
                }
                else if (current->task->state == TASKSTATE_AWAIT_EVENT_WAIT) {
                    uart_printf(CONSOLE, "AWAIT EVENT WAIT");
                }
                uart_printf(CONSOLE, " | %d %s\r\n", current->task->tid, current->task->name);
            }
        }
    }
}

void
tasktable_set_current_task(Tid tid)
{
    // make old current task ready
    /* tasktable_get_task(current_task)->state = TASKSTATE_READY; */

    Task* current = tasktable_get_task(current_task);
    current->total_time += timer_get() - current->enter_time;

    if (timer_get() - last_time > 1000000) {
        // TODO suppressing print, make this configurable
        /* tasktable_print_task_info(); */
        last_time = timer_get();
    }

    Task* next = tasktable_get_task(tid);
    next->state = TASKSTATE_ACTIVE;
    next->enter_time = timer_get();
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
