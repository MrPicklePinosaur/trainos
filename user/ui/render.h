#ifndef __UI_RENDER_H__
#define __UI_RENDER_H__

#include <trainsys.h>
#include "marklin.h"

#define RENDERER_ADDRESS "renderer"

int renderer_append_console(Tid renderer_tid, char* line);
int renderer_prompt(Tid renderer_tid, char ch);
int renderer_sensor_triggered(Tid renderer_tid, usize sensor_id);
int renderer_flip_switch(Tid renderer_tid, usize switch_id, SwitchMode mode);
void renderTask();

#endif // __UI_RENDER_H__
