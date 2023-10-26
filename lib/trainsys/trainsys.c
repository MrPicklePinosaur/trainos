#include "trainsys.h"

#include "kern/task.h"

// Block task until a specified task exits
void
WaitTid(Tid tid)
{

    Tid exit_tid;
    for (;;) {
        // TODO this is cheating
        if (tasktable_get_task(tid)->state == TASKSTATE_EXITED) {
            /* PRINT("[%d] %d already exited", MyTid(), tid); */
            return;
        }
        if ((exit_tid = AwaitEvent(EVENT_TASK_EXIT)) == tid) break;
        /* PRINT("[%d] task %d exited, we are waiting for %d", MyTid(), exit_tid, tid); */
    }
    /* PRINT("[%d] task %d exited, unblocking", MyTid(), tid); */

}
