#include "rpi.h"
#include "kern.h"
#include "task.h"
#include "trainsys.h"
#include "util.h"
#include "log.h"
#include "switchframe.h"
#include "task.h"
#include "alloc.h"

// Serial line 1 on the RPi hat is used for the console
void myprinttask() {
    LOG_DEBUG("hello i am in new task");

    MyTid();

    LOG_DEBUG("i am back from the kernel");
}

void mytask1();
void mytask2();

void mytask1() {

    LOG_DEBUG("entered task tid = %d, parent_tid = %d", MyTid(), MyParentTid());

    Tid tid2 = Create(0, &mytask2);

    uint64_t timer_value = 0;
    uint64_t print_timer = 0;
    uint64_t yield_timer = 0;
    for (;;) {
        timer_value = timer_get();

        if (timer_value - print_timer > 1000000) {
            print_timer = timer_value;
            PRINT("%s[Task 1] Timer%s", ANSI_RED, ANSI_RESET);
        }

        if (timer_value - yield_timer > 3000000) {
            PRINT("%s[Task 1] Yielding%s", ANSI_RED, ANSI_RESET);
            yield_timer = timer_value;
            Yield();
        }
    }
}

void mytask2() {

    LOG_DEBUG("entered task tid = %d, parent_tid = %d", MyTid(), MyParentTid());

    uint64_t timer_value = 0;
    uint64_t print_timer = 0;
    uint64_t yield_timer = 0;
    for (;;) {
        timer_value = timer_get();

        if (timer_value - print_timer > 1000000) {
            print_timer = timer_value;
            PRINT("%s[Task 2] Timer%s", ANSI_CYAN, ANSI_RESET);
        }

        if (timer_value - yield_timer > 3000000) {
            PRINT("%s[Task 2] Yielding%s", ANSI_CYAN, ANSI_RESET);
            yield_timer = timer_value;
            Yield();
        }
    }
}

void firstUserTask();
void secondUserTask();

void
firstUserTask()
{
    Tid t1 = Create(5, &secondUserTask);
    PRINT("Created: %d", t1);
    Tid t2 = Create(5, &secondUserTask);
    PRINT("Created: %d", t2);

    Tid t3 = Create(3, &secondUserTask);
    PRINT("Created: %d", t3);
    Tid t4 = Create(3, &secondUserTask);
    PRINT("Created: %d", t4);

    PRINT("FirstUserTask: exiting");
    Exit();
}

void
secondUserTask()
{
    PRINT("MyTid = %d, MyParentTid = %d", MyTid(), MyParentTid());
    Yield();
    PRINT("MyTid = %d, MyParentTid = %d", MyTid(), MyParentTid());
    Exit();
}

int kmain() {
    
    kern_init();

    set_log_level(LOG_LEVEL_WARN);

    // print the banner
    PRINT("");
    PRINT(".___________..______          ___       __  .__   __.   ______        _______.");
    PRINT("|           ||   _  \\        /   \\     |  | |  \\ |  |  /  __  \\      /       |");
    PRINT("`---|  |----`|  |_)  |      /  ^  \\    |  | |   \\|  | |  |  |  |    |   (----`");
    PRINT("    |  |     |      /      /  /_\\  \\   |  | |  . `  | |  |  |  |     \\   \\    ");
    PRINT("    |  |     |  |\\  \\----./  _____  \\  |  | |  |\\   | |  `--'  | .----)   |   ");
    PRINT("    |__|     | _| `._____/__/     \\__\\ |__| |__| \\__|  \\______/  |_______/    ");
    PRINT("                                                                              ");

    // need to create first task using kernel primitives since we are in kernel mode right here
    Tid tid1 = handle_svc_create(4, &firstUserTask);
    Task* task1 = tasktable_get_task(tid1);
    asm_enter_usermode(task1->sf);

    return 0;
}
