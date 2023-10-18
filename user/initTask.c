#include "usertasks.h"
#include "nameserver.h"
#include "clock.h"

#include <trainstd.h>
#include <trainsys.h>
#include <stddef.h>

typedef struct {
    char* name;
    void (*taskFn)(void);
} TaskMenuEntry;


// task selection menu
void
initTask()
{
    // max 10 tasks for now
    TaskMenuEntry* task_menu[11] = {
        &(TaskMenuEntry){ "K1", &firstUserTask },
        &(TaskMenuEntry){ "K2", &RPSTask },
        &(TaskMenuEntry){ "K2Perf", &K2Perf },
        &(TaskMenuEntry){ "K3", &K3 },
        &(TaskMenuEntry){ "sendReceiveReplyTest", &sendReceiveReplyTestTask },
        &(TaskMenuEntry){ "graphics", &graphicsTask },
        &(TaskMenuEntry){ "test", &testHarness },
        0
    };

    // spawn init tasks
    initNameserverTask();
    Create(1, &clockTask);
    Yield();  // Yield to let the clock server run at least once before the SELECT TASK loop

    for (;;) {
        println("================= SELECT TASK TO RUN =================");
        for (size_t i = 0; task_menu[i] != 0; ++i) {
            println("[%d] %s", i, task_menu[i]->name);
        }
        println("======================================================");

#if 0
        int ch = getc() - '0';

        /* int ch; */
        /* while ((ch = (int)getc_poll()) == 0) Yield(); */
        /* ch = ch - '0'; */

        if (!(0 <= ch && ch < 9)) {
            println("invalid task");
            continue;
        }
        Create(5, task_menu[ch]->taskFn);
        Yield();
#endif
        Create(5, &K3);
        Yield();
    }
    Exit();
}

