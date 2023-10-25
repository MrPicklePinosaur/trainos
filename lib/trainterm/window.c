#include <trainstd.h>
#include "./trainterm.h"

Window
win_init(usize x, usize y, usize w, usize h)
{
    return (Window) {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}


void
win_draw(Window* win)
{
    // TURBO naive window drawing

    w_puts_mv(win, "╭", 0, 0);
    for (unsigned int i = 0; i < (win->w)-2; ++i) w_puts(win, "─"); // TODO make this more efficient by building the string ahead of time
    w_puts(win, "╮");

    for (unsigned int i = 0; i < (win->h)-2; ++i) {
        w_puts_mv(win, "│", 0, i+1);
        w_puts_mv(win, "│", win->w-1, i+1);
    }

    w_puts_mv(win, "╰", 0, win->h-1);

    for (unsigned int i = 0; i < (win->w)-2; ++i) w_puts(win, "─");
    w_puts(win, "╯");
}

