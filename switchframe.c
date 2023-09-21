#include "switchframe.h"
#include "rpi.h"

SwitchFrame
switchframe_new(void)
{
    return (SwitchFrame) {
        .x19 = 0,
        .x20 = 0,
        .x21 = 0,
        .x22 = 0,
        .x23 = 0,
        .x24 = 0,
        .x25 = 0,
        .x26 = 0,
        .x27 = 0,
        .x28 = 0,
        .x30 = 0,
    };
}

void
switchframe_init(Task* task, void (*entrypoint)())
{
    Address base = task->addrspace.stackbase;

    SwitchFrame* switchframe = ((SwitchFrame*)base)-1;

    // align to word

    // zero out the data
    *switchframe = switchframe_new();

    // set the ra register so when we return from switchframe we go to entrypoint
    task->saved_x30 = (Address)entrypoint;

    // Also save the new value of stack pointer to the task
    task->saved_sp = (Address)switchframe;
}
