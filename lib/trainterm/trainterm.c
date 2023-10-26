#include "trainstd.h"
#include "./trainterm.h"
#include "render.h"

void
term_init(void)
{
    renderer_init();
    print("%s%s", ANSI_CLEAR, ANSI_HIDE);
}

void
term_clear(void)
{
    print("%s", ANSI_CLEAR);
}

void
term_render(void)
{

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
c_puts(char* s)
{
    print("%s", s);
}

void
c_puts_mv(char* s, usize x, usize y)
{
    c_mv(x, y);
    c_puts(s);
}

void
w_mv(Window* win, usize x, usize y)
{
    // TODO currently windows don't have cursor of their own
    c_mv(win->x+x, win->y+y);
}

void
w_putc(Window* win, char ch)
{
    // TODO cursor local to each window
    c_putc(ch);
}

void
w_putc_mv(Window* win, char ch, usize x, usize y)
{
    c_putc_mv(ch, win->x+x, win->y+y);
}

void
w_puts(Window* win, char* s)
{
    // TODO cursor local to each window
    c_puts(s);
}

void
w_puts_mv(Window* win, char* s, usize x, usize y)
{
    c_puts_mv(s, win->x+x, win->y+y);
}
