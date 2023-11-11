#ifndef __TRAINTERM_WINDOW_H__
#define __TRAINTERM_WINDOW_H__

#include <traindef.h>
#include "./trainterm.h"

void win_queue(Window* win, char* data, usize len);
void win_flush(Window* win);

#endif // __TRAINTERM_WINDOW_H__
