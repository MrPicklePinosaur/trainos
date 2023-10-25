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

void
c_mv(usize x, usize y)
{
    print(ANSI_MOVE, y, x);
}

void
c_putc(char ch)
{
    print("%c", ch);
}

void
c_putc_mv(char ch, usize x, usize y)
{
    c_mv(x, y);
    c_putc(ch);
}

void
w_putc_mv(Window* win, char ch, usize x, usize y)
{
    c_putc_mv(ch, win->x+x, win->y+y);
}
