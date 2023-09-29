#include "kern.h"
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"
#include "log.h"
#include "util.h"
#include "scheduler.h"
#include "alloc.h"
#include "rpi.h"
#include "util.h"
#include "gacha.h"

void
kern_init(void)
{
    uart_init();
    arena_init();
    pagetable_init();
    tasktable_init();
    vector_table_init();
    scheduler_init();
    gacha_init();
}

Tid
handle_svc_create(uint32_t priority, void (*entrypoint)())
{
    Tid current_tid = tasktable_current_task();

    Tid new_tid = tasktable_create_task(priority, entrypoint);
    Task* new_task = tasktable_get_task(new_tid);

    // if first task, set parent to null
    if (new_tid == 1) new_task->parent_tid = 0;
    else new_task->parent_tid = current_tid;

    LOG_DEBUG("[SYSCALL] Created new task %d", new_tid);

    scheduler_insert(new_tid, priority);

    return new_tid;

}

int
handle_svc_send(int tid, const char* msg, int msglen, char* reply, int rplen)
{
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);

    // TODO: disallow sending to self?

    // Find the task in question
    Task* target_task = tasktable_get_task(tid);
    if (target_task == 0) {
        LOG_DEBUG("Invalid task priority %d", tid);
        return -1;
    }
    
    // Check task state
    if (target_task->state == TASKSTATE_RECEIVE_WAIT) {
        // RECEIVE_WAIT tasks exists, directly copy message
        current_task->state = TASKSTATE_REPLY_WAIT;

    }
    else if (target_task->state == TASKSTATE_READY) {
        // task is not in RECEIVE_WAIT, add sending task to recieving tasks' recieve queue 
        current_task->state = TASKSTATE_SEND_WAIT;
        /* target_task */
        cbuf_push_back(target_task->receive_queue, (uint8_t)current_tid);
    } else {
        LOG_WARN("Task %d is not available to receive since it is in state %d", tid, target_task->state);
    }

    return 0;
}

Tid
find_next_task(void)
{
    Tid next_tid = scheduler_next();
    if (next_tid == 0) {
        LOG_DEBUG("No more tasks: blocking..");
        for (;;) { }
    }
    return next_tid;
}

void
handle_svc(void)
{
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    SwitchFrame* sf = current_task->sf;

    Tid next_tid;

    /* switchframe_debug(sf); */
    /* LOG_DEBUG("current task tid = %d", current_tid); */

    uint32_t opcode = asm_esr_el1() & 0x1FFFFFF;
    LOG_DEBUG("jumped to vector table handler with opcode = %x", opcode);

    if (opcode == OPCODE_CREATE) {
        if (!scheduler_valid_priority(sf->x0)) {
            LOG_DEBUG("Invalid task priority %d", sf->x0);
            sf->x0 = -1;
        }
        else {
            sf->x0 = handle_svc_create(sf->x0, (void (*)()) sf->x1);
        }
        next_tid = current_tid;
    }
    else if (opcode == OPCODE_MY_TID) {
        LOG_DEBUG("[SYSCALL] MyTid");

        sf->x0 = tasktable_current_task();
        next_tid = current_tid;
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {
        LOG_DEBUG("[SYSCALL] MyParentTid");

        sf->x0 = current_task->parent_tid;
        next_tid = current_tid;
    }
    else if (opcode == OPCODE_YIELD) {
        LOG_DEBUG("[SYSCALL] Yield");

        next_tid = find_next_task();

        LOG_DEBUG("yield context switch task_id from = %d to = %d", current_tid, next_tid);
    }
    else if (opcode == OPCODE_EXIT) {
        LOG_DEBUG("[SYSCALL] Exit");

        // NOTE: maybe don't allow task 1 to be deleted?
        if (current_tid == 1) {
            LOG_WARN("Attempting to delete task 1");
        }

        scheduler_remove(current_tid);
        tasktable_delete_task(current_tid);

        next_tid = find_next_task();

        LOG_DEBUG("exit context switch task_id from = %d to = %d", current_tid, next_tid);

    } else if (opcode == OPCODE_SEND) {

        LOG_DEBUG("[SYSCALL] SEND");

        sf->x0 = handle_svc_send(sf->x0, (const char*)sf->x1, sf->x2, (char*)sf->x3, sf->x4);

        next_tid = find_next_task();

    } else {
        LOG_WARN("Uncaught syscall with opcode %x", opcode);
    }

    tasktable_set_current_task(next_tid);
    asm_enter_usermode(tasktable_get_task(next_tid)->sf);
}
