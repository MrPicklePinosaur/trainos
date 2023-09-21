#include "kern.h"
#include "addrspace.h"
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
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {

    }
    else if (opcode == OPCODE_YIELD) {

    }
    else if (opcode == OPCODE_EXIT) {

    }
}
