#include "rpi.h"
#include "kern.h"
#include "task.h"
#include "util.h"
#include "log.h"
#include "switchframe.h"
#include "task.h"
#include "alloc.h"
#include "gacha.h"

#include "user/usertasks.h"

int kmain() {
    
    kern_init();

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

    gacha_print_roll();

    // need to create first task using kernel primitives since we are in kernel mode right here
    Tid init_tid = handle_svc_create(1, &firstUserTask);
    /* Tid init_tid = handle_svc_create(4, &K2); */
    Task* init_task = tasktable_get_task(init_tid);
    asm_enter_usermode(init_task->sf);

    return 0;
}
