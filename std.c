#include "std.h"
#include "rpi.h"
#include "task.h"
#include "kern.h"
#include "switchframe.h"

int
Create(int priority, void (*function)())
{
    Tid tid = tasktable_create_task(priority);

    uart_printf(CONSOLE, "created new task %d\r\n", tid);
    
    Task* new_task = tasktable_get_task(tid);

    // setup switchframe on the stack
    switchframe_init(new_task, function);

    return tid;
}

int
MyParentTid(void)
{

}

/* void */
/* Yield(void) */
/* { */

/* } */

void
Exit(void)
{
}
