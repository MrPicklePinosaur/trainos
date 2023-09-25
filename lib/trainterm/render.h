#ifndef __TRAINTERM_RENDER_H__
#define __TRAINTERM_RENDER_H__

#include "./trainterm.h"

#define MAX_WINDOWS 32

// TODO WISHLIST: want a dynamic array collection in std
typedef struct {
    Window* windows[MAX_WINDOWS];
    size_t window_count;
} RenderState;

extern RenderState render_state;

void renderer_init();

#endif // __TRAINTERM_RENDER_H__
