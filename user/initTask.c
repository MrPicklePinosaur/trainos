#include "usertasks.h"
#include "sensor.h"
#include "switch.h"
#include "trainstate.h"
#include "trainpos.h"

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

    // dummy vars for synchronization using Send/Recv
    char dummy;
    Tid from_tid;

    TaskMenuEntry* task_menu[] = {
        // &(TaskMenuEntry){ "K1", &firstUserTask },
        // &(TaskMenuEntry){ "K2", &RPSTask },
        // &(TaskMenuEntry){ "K2Perf", &K2Perf },
        // &(TaskMenuEntry){ "K3", &K3 },
        // &(TaskMenuEntry){ "K4", &uiTask },
        &(TaskMenuEntry){ "MarklinCTL", &uiTask },
        &(TaskMenuEntry){ "test", &testHarness },
        0
    };

    // spawn init tasks
    initNameserverTask();
    Tid clock_server = Create(1, &clockTask, "Clock Server");

    println("Initializing IO servers...");
    Tid io_server_marklin = Create(5, &marklinIO, "Marklin IO Server");
    Tid io_server_console = Create(5, &consoleIO, "Console IO Server");

    println("Initalizing track...");
    track_init();

    println("Initializing sensors and switches...");
    Tid sensor_server = Create(2, &sensorServerTask, "Sensor Server");
    Tid switch_server = Create(2, &switchServerTask, "Switch Server");
    marklin_go(io_server_marklin); // marklin needs to be on to change switches
    SwitchInit(switch_server);

    println("Initializing trains...");
    Tid trainstate_server = Create(2, &trainStateServer, "Train State Server");
    Tid train_pos = Create(2, &trainPosTask, "Train position task");
    Tid path_tid = Create(3, &pathTask, "Path Task");

    println("Initalizing UI...");
    Tid trainterm_server = Create(3, &traintermTask, "Train Term Server");

    Delay(clock_server, 100);

    Tid marklinctl_task = Create(5, &uiTask, "MarklinCTL");
    WaitTid(marklinctl_task);

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

        Tid task_menu_task = Create(5, task_menu[ch]->taskFn, task_menu[ch]->name);
        WaitTid(task_menu_task);
        set_log_mode(LOG_MODE_STANDARD);
    }


    Receive(&from_tid, &dummy, sizeof(char));

    println("attempting to exit init task");

    Exit();
}

