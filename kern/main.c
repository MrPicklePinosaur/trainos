#include <trainstd.h>
#include "kern/dev/uart.h"
#include "kern/dev/timer.h"
#include "kern.h"
#include "task.h"
#include "switchframe.h"
#include "task.h"
#include "alloc.h"
#include "gacha.h"
#include "perf.h"
#include "user/usertasks.h"

int kmain() {

    kern_init();

#if defined ( NATIVE )

    set_log_level(LOG_LEVEL_DEBUG);
    /* set_log_mask(LOG_MASK_KERN|LOG_MASK_USER|LOG_MASK_COHORT); */
    set_log_mask(LOG_MASK_KERN);

    // print the banner
    PRINT("");
    PRINT(".___________..______          ___       __  .__   __.   ______        _______.");
    PRINT("|           ||   _  \\        /   \\     |  | |  \\ |  |  /  __  \\      /       |");
    PRINT("`---|  |----`|  |_)  |      /  ^  \\    |  | |   \\|  | |  |  |  |    |   (----`");
    PRINT("    |  |     |      /      /  /_\\  \\   |  | |  . `  | |  |  |  |     \\   \\    ");
    PRINT("    |  |     |  |\\  \\----./  _____  \\  |  | |  |\\   | |  `--'  | .----)   |   ");
    PRINT("    |__|     | _| `._____/__/     \\__\\ |__| |__| \\__|  \\______/  |_______/    ");
    PRINT("                                                                              ");

    // gacha current does not work on simulator (no timer)
    gacha_print_roll();

    // PRINT("kernel stack in kmain %x", asm_sp_el1());
    PRINT("Running at debug level %d with log mask 0x%x", get_log_level(), get_log_mask());
#else
    set_log_level(LOG_LEVEL_DEBUG);
    set_log_mask(0); // disable all logging
#endif

    // need to create first task using kernel primitives since we are in kernel mode right here
    Tid init_tid = handle_svc_create(14, &initTask, "Init Task"); // temp making starting task very low priority
    Tid idle_tid = handle_svc_create(15, &idleTask, "Idle Task");
    perf_init(idle_tid);

    /* Tid init_tid = handle_svc_create(4, &K2, "K2"); */
    Task* init_task = tasktable_get_task(init_tid);
    init_task->enter_time = timer_get();
    asm_enter_usermode(init_task->sf);

    return 0;
}
