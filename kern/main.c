#include "rpi.h"
#include "kern.h"
#include "task.h"
#include "util.h"
#include "log.h"
#include "switchframe.h"
#include "task.h"
#include "alloc.h"

#include "user/k1.h"

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
