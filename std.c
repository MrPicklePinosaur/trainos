#include "std.h"
#include "task.h"

int
Create(int priority, void (*function)())
{
    Tid tid = tasktable_create_task(priority);

    // context switch
}

int
MyTid(void)
{

}

int
MyParentTid(void)
{

}

void
Yield(void)
{

}

void
Exit(void)
{

}
