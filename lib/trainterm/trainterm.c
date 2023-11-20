#include <trainstd.h>
#include <traintasks.h>
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
w_attr(Window* win, Attr attr)
{
    char* attr_str;
    switch (attr) {
        case ATTR_BLACK:
            attr_str = ANSI_BLACK;
            break;
        case ATTR_RED:
            attr_str = ANSI_RED;
            break;
        case ATTR_GREEN:
            attr_str = ANSI_GREEN;
            break;
        case ATTR_YELLOW:
            attr_str = ANSI_YELLOW;
            break;
        case ATTR_BLUE:
            attr_str = ANSI_BLUE;
            break;
        case ATTR_MAGENTA:
            attr_str = ANSI_MAGENTA;
            break;
        case ATTR_CYAN:
            attr_str = ANSI_CYAN;
            break;
        case ATTR_WHITE:
            attr_str = ANSI_WHITE;
            break;
        case ATTR_RESET:
            attr_str = ANSI_RESET;
            break;
        case ATTR_BLINK:
            attr_str = ANSI_BLINK;
            break;
    }
    win_queue(win, attr_str, strlen(attr_str));
}

void
w_attr_reset(Window* win)
{
    w_attr(win, ATTR_RESET);
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
    win_queue(win, s, cstr_len(s));
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
    //print(win->write_buffer);
    // TODO this is kind lazy
    FlushWin(win->server, win);
    win_flush(win);
}
