#include "usertasks.h"

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
        &(TaskMenuEntry){ "sendReceiveReplyTest", &sendReceiveReplyTestTask },
        &(TaskMenuEntry){ "graphics", &graphicsTask },
        &(TaskMenuEntry){ "test", &testHarness },
        0
    };

    // spawn init tasks
    initNameserverTask();


    for (;;) {
        println("================= SELECT TASK TO RUN =================");
        for (size_t i = 0; task_menu[i] != 0; ++i) {
            println("[%d] %s", i, task_menu[i]->name);
        }
        println("======================================================");
        int ch = getc() - '0';
        if (!(0 <= ch && ch < 9)) {
            println("invalid task");
            continue;
        }
        Create(5, task_menu[ch]->taskFn);
        Yield();
    }
    Exit();
}

