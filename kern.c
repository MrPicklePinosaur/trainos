#include "kern.h"
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"
#include "log.h"
#include "util.h"
#include "scheduler.h"
#include "alloc.h"
#include "rpi.h"

void
kern_init(void)
{
    uart_init();
    arena_init();
    pagetable_init();
    tasktable_init();
    vector_table_init();
    scheduler_init();
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

void
handle_svc(void)
{
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    SwitchFrame* sf = current_task->sf;

    /* switchframe_debug(sf); */

    uint32_t opcode = asm_esr_el1() & 0x1FFFFFF;
    LOG_DEBUG("jumped to vector table handler with opcode = %x", opcode);

    /* LOG_DEBUG("current task = %x", tasktable_current_task()); */

    if (opcode == OPCODE_CREATE) {
        sf->x0 = handle_svc_create(sf->x0, (void (*)()) sf->x1);
        asm_enter_usermode(current_task->sf);
    }
    else if (opcode == OPCODE_MY_TID) {
        LOG_DEBUG("[SYSCALL] MyTid");

        sf->x0 = tasktable_current_task();
        asm_enter_usermode(current_task->sf);
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {
        LOG_DEBUG("[SYSCALL] MyParentTid");

        sf->x0 = current_task->parent_tid;
        asm_enter_usermode(current_task->sf);

    }
    else if (opcode == OPCODE_YIELD) {
        LOG_DEBUG("[SYSCALL] Yield");

        Tid next_tid = scheduler_next();

        if (next_tid == 0) {
            LOG_ERROR("No tasks left");
            return; // TODO have better error state
        }

        // TODO: run scheduler to determine next task to run (this is just dumb schedule that toggles between two tasks)

        LOG_DEBUG("context switch task_id from = %d to = %d", current_tid, to_task->tid);

        tasktable_set_current_task(next_tid);

        /* LOG_DEBUG("from_task: sp = %x, x30 = %x", from_task->saved_sp, from_task->saved_x30); */
        /* LOG_DEBUG("to_task: sp = %x, x30 = %x", to_task->saved_sp, to_task->saved_x30); */

        asm_enter_usermode(tasktable_get_task(next_tid)->sf);

        // switchframe_switch(&to_task->saved_sp, &to_task->saved_sp);

    }
    else if (opcode == OPCODE_EXIT) {
        LOG_DEBUG("[SYSCALL] Exit");

        // NOTE: maybe don't allow task 1 to be deleted?

        tasktable_delete_task(current_tid);

        // find next task

    }

    LOG_WARN("Uncaught syscall with opcode %x", opcode);

    asm_enter_usermode(current_task->sf);

}
