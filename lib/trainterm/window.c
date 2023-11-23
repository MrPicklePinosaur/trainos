#include <trainstd.h>
#include <traintasks.h>
#include "./trainterm.h"
#include "render.h"

Window
win_init(usize x, usize y, usize w, usize h)
{
    return (Window) {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .server = WhoIs(TRAINTERM_ADDRESS),
        .buf_ptr = 0,
        .write_buffer = {0},
    };
}

// add bytes to the window write queue
void
win_queue(Window* win, char* data, usize len)
{
    if (win->buf_ptr+len >= WIN_BUF_SIZE) {
        PANIC("window queue out of memory");
    }

    memcpy(win->write_buffer+win->buf_ptr, data, len);
    win->buf_ptr += len;
}

void
win_flush(Window* win)
{
    win->buf_ptr = 0;
    memset(win->write_buffer, 0, WIN_BUF_SIZE);
}

void
win_draw_border(Window* win)
{
    // TURBO naive window drawing
    w_puts_mv(win, "╭", 0, 0);
    for (unsigned int i = 0; i < (win->w)-2; ++i) w_puts(win, "─"); // TODO make this more efficient by building the string ahead of time
    w_puts(win, "╮");
    w_flush(win);

    for (unsigned int i = 0; i < (win->h)-2; ++i) {
        w_puts_mv(win, "│", 0, i+1);
        w_puts_mv(win, "│", win->w-1, i+1);
        w_flush(win);
    }

    w_puts_mv(win, "╰", 0, win->h-1);

    for (unsigned int i = 0; i < (win->w)-2; ++i) w_puts(win, "─");
    w_puts(win, "╯");
    w_flush(win);

}

