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
#include "user/path/train_data.h"
#include "user/path/reserve.h"

#include "user/client/io.h"


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

    // spawn init tasks
    initNameserverTask();
    Tid clock_server = Create(1, &clockTask, "Clock Server");

#if defined( NATIVE )
    println("Initializing IO servers...");
#endif
    Tid io_server_marklin = Create(2, &marklinIO, "Marklin IO Server");
    Tid io_server_console = Create(2, &consoleIO, "Console IO Server");

#if defined( NATIVE )
    println("Initalizing track...");
#endif
    marklin_init(io_server_marklin);
    track_init();

#if defined( NATIVE )
    println("Initializing sensors and switches...");
#endif
    Tid sensor_server = Create(2, &sensorServerTask, "Sensor Server");
    Tid switch_server = Create(2, &switchServerTask, "Switch Server");

    marklin_stop(io_server_marklin);
    for (usize i = 0; i < TRAIN_DATA_TRAIN_COUNT; ++i) {
        marklin_train_ctl(io_server_marklin, TRAIN_DATA_TRAINS[i], 0);
    }
    marklin_go(io_server_marklin);

    SwitchInit(switch_server);

#if defined( NATIVE )
    println("Initializing trains...");
#endif
    Tid trainstate_server = Create(5, &trainStateServer, "Train State Server");
    Tid reserve_server = Create(5, &reservationTask, "Reservation Server");

#if defined( NATIVE )
    println("Initalizing UI...");
#endif
    Tid trainterm_server = Create(3, &traintermTask, "Train Term Server");

    Delay(clock_server, 100);

    Tid client_task = Create(5, &clientIoTask, "Client IO Task");


    Track* track = get_track();
    TrainstateSetPos(trainstate_server, reserve_server, 2, track_node_by_name(track, "A1"));

    /* Tid marklinctl_task = Create(5, &uiTask, "MarklinCTL"); */
    /* WaitTid(marklinctl_task); */

    /* Tid tester = Create(5, &testHarness, "test harness"); */
    /* WaitTid(tester); */

#if 0
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
#endif


    // dummy vars for synchronization using Send/Recv
    char dummy;
    Tid from_tid;
    Receive(&from_tid, &dummy, sizeof(char));

    println("attempting to exit init task");

    Exit();
}

