#include <trainstd.h>
#include "./trainterm.h"
#include "window.h"
#include "render.h"

void
term_init(void)
{
    print("%s%s", ANSI_CLEAR, ANSI_HIDE);
}

void
term_clear(void)
{
    print("%s", ANSI_CLEAR);
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
c_attr(Attr attr)
{
    switch (attr) {
        case ATTR_BLACK:
            c_puts(ANSI_BLACK);
            break;
        case ATTR_RED:
            c_puts(ANSI_RED);
            break;
        case ATTR_GREEN:
            c_puts(ANSI_GREEN);
            break;
        case ATTR_YELLOW:
            c_puts(ANSI_YELLOW);
            break;
        case ATTR_BLUE:
            c_puts(ANSI_BLUE);
            break;
        case ATTR_MAGENTA:
            c_puts(ANSI_MAGENTA);
            break;
        case ATTR_CYAN:
            c_puts(ANSI_CYAN);
            break;
        case ATTR_WHITE:
            c_puts(ANSI_WHITE);
            break;
        case ATTR_RESET:
            c_puts(ANSI_RESET);
            break;
    }
}

void
c_attr_reset(void)
{
    c_attr(ATTR_RESET);
}

void
w_mv(Window* win, usize x, usize y)
{
    char ybuf[12] = {0};
    ui2a(win->y+y, 10, ybuf);
    char xbuf[12] = {0};
    ui2a(win->x+x, 10, xbuf);

    // TODO this code is ugly
    win_queue(win, "\033[", strlen("\033["));
    win_queue(win, ybuf, strlen(ybuf));
    win_queue(win, ";", strlen(";"));
    win_queue(win, xbuf, strlen(xbuf));
    win_queue(win, "H", strlen("H"));
}

void
w_putc(Window* win, char ch)
{
    win_queue(win, &ch, 1);
}

void
w_putc_mv(Window* win, char ch, usize x, usize y)
{
    w_mv(win, x, y);
    w_putc(win, ch);
}

void
w_puts(Window* win, char* s)
{
    win_queue(win, s, strlen(s));
}

void
w_puts_mv(Window* win, char* s, usize x, usize y)
{
    w_mv(win, x, y);
    w_puts(win, s);
}

void
w_flush(Window* win)
{
    // write all bytes and clear window
    print(win->write_buffer);
    win_flush(win);
}
