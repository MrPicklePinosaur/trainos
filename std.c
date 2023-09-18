#include "std.h"
#include "task.h"
#include "kern.h"
#include "switchframe.h"

int
Create(int priority, void (*function)())
{
    Tid tid = tasktable_create_task(priority);
    
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

    tasktable_set_current_task(to_tid);
    switchframe_switch(&tasktable_get_task(from_tid)->saved_sp, &tasktable_get_task(to_tid)->saved_sp);
}

void
Exit(void)
{

}
