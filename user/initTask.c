#include "usertasks.h"
#include "sensor.h"
#include "switch.h"
#include "trainstate.h"

#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include <trainterm.h>
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
        &(TaskMenuEntry){ "K4", &uiTask },
        &(TaskMenuEntry){ "sendReceiveReplyTest", &sendReceiveReplyTestTask },
        &(TaskMenuEntry){ "graphics", &graphicsTask },
        &(TaskMenuEntry){ "test", &testHarness },
        0
    };

    // spawn init tasks
    initNameserverTask();
    Create(1, &clockTask, "Clock Server");
    //Create(5, &perfTask, "Idle Percentage Printer");

    Tid io_server_marklin = Create(5, &marklinIO, "Marklin IO Server");
    Tid io_server_console = Create(5, &consoleIO, "Console IO Server");

    Tid sensor_server = Create(2, &sensorServerTask, "Sensor Server");
    Tid switch_server = Create(2, &switchServerTask, "Switch Server");

    Tid trainstate_server = Create(2, &trainStateServer, "Train State Server");
    Tid trainterm_server = Create(3, &traintermTask, "Train Term Server");

    Tid path_tid = Create(3, &pathTask, "Path Task");

    for (;;) {
        println("================= SELECT TASK TO RUN =================");
        for (size_t i = 0; task_menu[i] != 0; ++i) {
            println("[%d] %s", i, task_menu[i]->name);
        }
        println("======================================================");

        int ch = Getc(io_server_console) - '0';

        if (!(0 <= ch && ch < 9)) {
            println("invalid task");
            continue;
        }

        // If train term task was selected, switch to a special logging mode
        if (ch == 4) {
            set_log_mode(LOG_MODE_TRAIN_TERM);
        }

        Tid task_menu_task = Create(5, task_menu[ch]->taskFn, task_menu[ch]->name);
        WaitTid(task_menu_task);
        set_log_mode(LOG_MODE_STANDARD);
    }


    char dummy;
    Tid from_tid;
    Receive(&from_tid, &dummy, sizeof(char));

    println("attempting to exit init task");

    Exit();
}

