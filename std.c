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

}

int
MyTid(void)
{
    return (int)tasktable_current_task();
}

int
MyParentTid(void)
{

}

void
Yield(void)
{
    Tid from_tid = MyTid();
    Tid to_tid;

    // TODO: run scheduler to determine next task to run (this is just dumb schedule that toggles between two tasks)
    if (from_tid == 1) to_tid = 2; 
    else to_tid = 1; 

    uart_printf(CONSOLE, "context switch task_id from = %d to = %d\r\n", from_tid, to_tid);

    tasktable_set_current_task(to_tid);

    Task* from_task= tasktable_get_task(from_tid);
    Task* to_task = tasktable_get_task(to_tid);

    uart_printf(CONSOLE, "context switch stack ptr from = %x to = %x\r\n", from_task->saved_sp, to_task->saved_sp);

    switchframe_switch(&to_task->saved_sp, &to_task->saved_sp);

}

void
Exit(void)
{

}
