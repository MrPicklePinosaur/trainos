#include "std.h"
#include "rpi.h"
#include "task.h"
#include "kern.h"
#include "switchframe.h"

int
Create(int priority, void (*function)())
{
    Tid tid = tasktable_create_task(priority, function);

    uart_printf(CONSOLE, "created new task %d\r\n", tid);
    
    Task* new_task = tasktable_get_task(tid);

    return tid;
}

int
MyParentTid(void)
{

}

void
Exit(void)
{
}
