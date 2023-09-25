#include "./trainterm.h"
#include "trainstd.h"

Window
win_init(size_t x, size_t y, size_t w, size_t h)
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

    print("\033[%d;%dH", win->y, win->x);
    print("╭");
    // TODO bounds check
    for (unsigned int i = 0; i < (win->w)-2; ++i) print("─"); // TODO make this more efficient by building the string ahead of time
    print("╮");

    for (unsigned int i = 0; i < (win->h)-2; ++i) {
        print("\033[%d;%dH│", win->y+1+i, win->x);
        print("\033[%d;%dH│", win->y+1+i, (win->x)+(win->w)-1);
    }

    print("\033[%d;%dH", (win->y)+(win->h)-1, win->x);

    print("╰");
    for (unsigned int i = 0; i < (win->w)-2; ++i) print("─");
    print("╯");
}
