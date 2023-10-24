#include "usertasks.h"
#include "nameserver.h"
#include "clock.h"
#include "io.h"

#include <trainstd.h>
#include <trainsys.h>
#include <stddef.h>

#include "kern/perf.h"
#include "kern/dev/uart.h"

typedef struct {
    char* name;
    void (*taskFn)(void);
} TaskMenuEntry;

void
idleTask()
{
    for (;;) {
        asm_wfi();
    }

    Exit();
}

// periodically prints out performance metrics
void
perfTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    for (;;) {
        Delay(clock_server, 100); 
        println("Idle time %d percent", get_idle_time());
    }

    Exit();
}

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
    Create(1, &clockTask, "Clock Server");
    Yield();  // Yield to let the clock server run at least once before the SELECT TASK loop
    //Create(5, &perfTask, "Idle Percentage Printer");
    //Yield();
    Tid io_server = Create(5, &marklinIO, "Marklin IO Server");
    Yield();

    /*
    for (;;) {
        println("================= SELECT TASK TO RUN =================");
        for (size_t i = 0; task_menu[i] != 0; ++i) {
            println("[%d] %s", i, task_menu[i]->name);
        }
        println("======================================================");

        int ch = getc() - '0';

        // unsigned char ch;
        // while ((ch = getc_poll()) == 0) Yield();
        // ch = ch - '0';

        if (!(0 <= ch && ch < 9)) {
            println("invalid task");
            continue;
        }
        Create(5, task_menu[ch]->taskFn, "Menu-Started Task");
        Yield();
    }
    */

    /* Create(2, &K3, "K3"); */
    /* Yield(); */

    char ch = Getc(io_server, CONSOLE);
    println("got ch %d", ch);

    // Block init by receiving from no-one
    char dummy;
    Tid from_tid;
    Receive(&from_tid, &dummy, sizeof(char));

    println("attempting to exit init task");
    for (;;) {}

    Exit();
}

