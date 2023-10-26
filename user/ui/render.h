#ifndef __UI_RENDER_H__
#define __UI_RENDER_H__

#include <trainsys.h>

#define RENDERER_ADDRESS "renderer"

int renderer_append_console(Tid renderer_tid, char* line);
int renderer_prompt(Tid renderer_tid, char ch);
int renderer_sensor_triggered(Tid renderer_tid, usize sensor_id);
void renderTask();

#endif // __UI_RENDER_H__
