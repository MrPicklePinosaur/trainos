#include "rpi.h"
#include "kern.h"
#include "task.h"
#include "std.h"
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

void mytask1() {

    LOG_DEBUG("entered task tid = %d, parent_tid = %d", MyTid(), MyParentTid());

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


int kmain() {
    
    kern_init();
    arena_init();

    // initialize both console and marklin uarts
    uart_init();

    // not strictly necessary, since line 1 is configured during boot
    // but we'll configure the line anyways, so we know what state it is in
    uart_config_and_enable(CONSOLE, 115200, 0x70);

    set_log_level(LOG_LEVEL_DEBUG);

    // print the banner
    PRINT("");
    PRINT(".___________..______          ___       __  .__   __.   ______        _______.");
    PRINT("|           ||   _  \\        /   \\     |  | |  \\ |  |  /  __  \\      /       |");
    PRINT("`---|  |----`|  |_)  |      /  ^  \\    |  | |   \\|  | |  |  |  |    |   (----`");
    PRINT("    |  |     |      /      /  /_\\  \\   |  | |  . `  | |  |  |  |     \\   \\    ");
    PRINT("    |  |     |  |\\  \\----./  _____  \\  |  | |  |\\   | |  `--'  | .----)   |   ");
    PRINT("    |__|     | _| `._____/__/     \\__\\ |__| |__| \\__|  \\______/  |_______/    ");
    PRINT("                                                                              ");

    Tid tid1 = handle_svc_create(0, &mytask1);
    Tid tid2 = handle_svc_create(0, &mytask2);

    LOG_DEBUG("task1 = %x, task2 = %x", tid1, tid2);

    LOG_DEBUG("privledge level %d", priviledge_level());

    LOG_DEBUG("vbar value %x", vbar_value());

    Task* task1 = tasktable_get_task(tid1);
    asm_enter_usermode(task1->sf);

    /* mytask1(); */

    char c = ' ';
    while (c != 'q') {
    c = uart_getc(CONSOLE);
        if (c == '\r') {
            /* uart_printf(CONSOLE, "task0 %d\r\n", task0); */
            /* uart_printf(CONSOLE, "task1 %d\r\n", task1); */
        } else {
            uart_putc(CONSOLE, c);
        }
    }

    // U-Boot displays the return value from main - might be handy for debugging
    return 0;
}
