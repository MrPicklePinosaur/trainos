#include "trainterm.h"
#include "usertasks.h"

// task that tests the trainterm library
void
graphicsTask()
{
    term_init();
    Window win = win_init(2, 2, 20, 10);
    win_draw(&win);

    for (;;) {} // TODO block for now (due to kernel limitations)
}
