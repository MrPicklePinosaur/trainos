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
    // TODO handle exited case
    else {
        // RECEIVE_WAIT task does not exist, add sending task to recieving tasks' recieve queue 
        current_task->state = TASKSTATE_SEND_WAIT;

    }

    return 0;
}

void
handle_svc(void)
{
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    SwitchFrame* sf = current_task->sf;

    Task* next_task;

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
        next_task = current_task;
    }
    else if (opcode == OPCODE_MY_TID) {
        LOG_DEBUG("[SYSCALL] MyTid");

        sf->x0 = tasktable_current_task();
        next_task = current_task;
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {
        LOG_DEBUG("[SYSCALL] MyParentTid");

        sf->x0 = current_task->parent_tid;
        next_task = current_task;
    }
    else if (opcode == OPCODE_YIELD) {
        LOG_DEBUG("[SYSCALL] Yield");

        Tid next_tid = scheduler_next();

        if (next_tid == 0) {
            LOG_ERROR("No tasks left");
            return; // TODO have better error state
        }

        LOG_DEBUG("yield context switch task_id from = %d to = %d", current_tid, next_tid);

        next_task = tasktable_get_task(next_tid);
    }
    else if (opcode == OPCODE_EXIT) {
        LOG_DEBUG("[SYSCALL] Exit");

        // NOTE: maybe don't allow task 1 to be deleted?

        scheduler_remove(current_tid);
        tasktable_delete_task(current_tid);

        Tid next_tid = scheduler_next();
        if (next_tid == 0) {
            for (;;) {
                LOG_DEBUG("No more tasks");
            }
            // Somehow return to kmain
        }

        LOG_DEBUG("exit context switch task_id from = %d to = %d", current_tid, next_tid);

        next_task = tasktable_get_task(next_tid);
    } else if (opcode == OPCODE_SEND) {

        LOG_DEBUG("[SYSCALL] SEND");

        sf->x0 = handle_svc_send(sf->x0, (const char*)sf->x1, sf->x2, (char*)sf->x3, sf->x4);

        Tid next_tid = scheduler_next();
        // TODO error state
        next_task = tasktable_get_task(next_tid);
    } else {
        LOG_WARN("Uncaught syscall with opcode %x", opcode);
    }

    asm_enter_usermode(next_task->sf);
}
