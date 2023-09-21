#include "kern.h"
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"
#include "log.h"
#include "util.h"

void
kern_init(void)
{
    pagetable_init();
    tasktable_init();
    vector_table_init();
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

        Tid tid = tasktable_create_task(sf->x0, (void(*)())sf->x1);
        Task* new_task = tasktable_get_task(tid);

        // if first task, set parent to null
        if (tid == 1) new_task->parent_tid = 0;
        else new_task->parent_tid = current_tid;

        LOG_DEBUG("[SYSCALL] Created new task %d", tid);

        sf->x0 = tid;

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

        Tid from_tid = current_tid;
        Tid to_tid;

        // TODO: run scheduler to determine next task to run (this is just dumb schedule that toggles between two tasks)
        if (from_tid == 1) to_tid = 2; 
        else to_tid = 1; 

        LOG_DEBUG("context switch task_id from = %d to = %d", from_tid, to_tid);

        tasktable_set_current_task(to_tid);

        Task* from_task = tasktable_get_task(from_tid);
        Task* to_task = tasktable_get_task(to_tid);

        /* LOG_DEBUG("from_task: sp = %x, x30 = %x", from_task->saved_sp, from_task->saved_x30); */
        /* LOG_DEBUG("to_task: sp = %x, x30 = %x", to_task->saved_sp, to_task->saved_x30); */

        asm_enter_usermode(to_task->sf);

        // switchframe_switch(&to_task->saved_sp, &to_task->saved_sp);

    }
    else if (opcode == OPCODE_EXIT) {
        LOG_DEBUG("[SYSCALL] Exit");

    }

    LOG_WARN("Uncaught syscall with opcode %x", opcode);

}
