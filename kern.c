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
    LOG_DEBUG("jumped to vector table handler");

    LOG_DEBUG("current task = %x", tasktable_current_task());

    // save special registers for the current task
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    current_task->saved_x30 = (Address)asm_elr_el1();
    current_task->saved_sp = (Address)asm_sp_el0();
    
    
    uint32_t opcode = asm_esr_el1() & 0x1FFFFFF;

    /* uint32_t sp_el0 = pop_stack(); */
    /* uint32_t spsr_el1 = pop_stack(); */
    /* uint32_t elr_el1 = pop_stack(); */
    /* uint32_t esr_el1 = pop_stack(); */
    /* uint32_t opcode = esr_el1 & 0x1FFFFFF;  // Last 25 bits of ESR store the opcode */
    if (opcode == OPCODE_CREATE) {

    }
    else if (opcode == OPCODE_MY_TID) {
        LOG_DEBUG("MyTid %u", tasktable_current_task());

        asm_enter_usermode(current_task->saved_sp, current_task->saved_x30);
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {

    }
    else if (opcode == OPCODE_YIELD) {

        Tid from_tid = current_tid;
        Tid to_tid;

        // TODO: run scheduler to determine next task to run (this is just dumb schedule that toggles between two tasks)
        if (from_tid == 1) to_tid = 2; 
        else to_tid = 1; 

        LOG_DEBUG("context switch task_id from = %d to = %d", from_tid, to_tid);

        tasktable_set_current_task(to_tid);

        Task* from_task = tasktable_get_task(from_tid);
        Task* to_task = tasktable_get_task(to_tid);

        LOG_DEBUG("from_task: sp = %x, x30 = %x", from_task->saved_sp, from_task->saved_x30);
        LOG_DEBUG("to_task: sp = %x, x30 = %x", to_task->saved_sp, to_task->saved_x30);

        asm_enter_usermode(to_task->saved_sp, to_task->saved_x30);

        // switchframe_switch(&to_task->saved_sp, &to_task->saved_sp);

    }
    else if (opcode == OPCODE_EXIT) {

    }

}
