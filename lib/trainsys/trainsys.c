#include "trainsys.h"

#include <trainstd.h>

// Block task until a specified task exits
void
WaitTid(Tid tid)
{
    Tid exit_tid;
    while ((exit_tid = AwaitEvent(EVENT_TASK_EXIT)) != tid) {
        PRINT("task %d just exited", exit_tid);
    }
}
