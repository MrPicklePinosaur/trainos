#include "render.h"

RenderState render_state;

void
renderer_init()
{
    render_state = (RenderState) {
        .windows = {0},
        .window_count = 0,
    };
}

