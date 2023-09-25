#ifndef __TRAINTERM_H__
#define __TRAINTERM_H__

#include <stddef.h>

/* Library for manipulating the terminal */

#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE(r, c) "\033[" r ";" c "H"
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

typedef struct {
    size_t x;
    size_t y;
    size_t w;
    size_t h;
} Window;

// initialize screen for tui mode
void term_init(void);

// clear the screen
void term_clear(void);

// render the screen
void term_render(void);

// window commands
Window win_init(size_t x, size_t y, size_t w, size_t h);
void win_draw(Window* win);

#endif // __TRAINTERM_H__
