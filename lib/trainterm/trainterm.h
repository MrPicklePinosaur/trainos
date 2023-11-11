#ifndef __TRAINTERM_H__
#define __TRAINTERM_H__

#include <traindef.h>
#include <trainsys.h>

/* Library for manipulating the terminal */

#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE "\033[%d;%dH"
#define ANSI_CLEAR_LINE "\033[K"

#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

#define ANSI_RESET "\033[0m"

#define WIN_BUF_SIZE 512

typedef struct {
    usize x;
    usize y;
    usize w;
    usize h;
    usize buf_ptr;
    Tid server;
    char write_buffer[WIN_BUF_SIZE];
} Window;

typedef enum {
    ATTR_BLACK,
    ATTR_RED,
    ATTR_GREEN,
    ATTR_YELLOW,
    ATTR_BLUE,
    ATTR_MAGENTA,
    ATTR_CYAN,
    ATTR_WHITE,
    ATTR_RESET,
} Attr;

void traintermTask();

// initialize screen for tui mode
void term_init(void);

// clear the screen
void term_clear(void);

// window commands
Window win_init(usize x, usize y, usize w, usize h);
void win_draw(Window* win);

void w_attr(Window* win, Attr attr);
void w_attr_reset(Window* win);

// window commands
void w_mv(Window* win, usize x, usize y);
void w_putc(Window* win, char ch);
void w_putc_mv(Window* win, char ch, usize x, usize y);
void w_puts(Window* win, char* s);
void w_puts_mv(Window* win, char* s, usize x, usize y);
void w_flush(Window* win);

#endif // __TRAINTERM_H__
