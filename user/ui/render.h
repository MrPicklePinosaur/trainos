#ifndef __UI_RENDER_H__
#define __UI_RENDER_H__

#include <trainsys.h>

#define RENDERER_ADDRESS "renderer"

int renderer_append_console(Tid renderer_tid, char* line);
void renderTask();

#endif // __UI_RENDER_H__
