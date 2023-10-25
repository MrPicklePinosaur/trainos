#include "trainterm.h"
#include "usertasks.h"

// task that tests the trainterm library
void
graphicsTask()
{
    term_init();
    Window win = win_init(2, 2, 20, 10);
    win_draw(&win);

    c_putc_mv('A', 30, 30);
    c_putc('B');

    c_putc_mv('C', 30, 32);
    c_putc('D');

    for (;;) {} // TODO block for now (due to kernel limitations)
}
