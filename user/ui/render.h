#ifndef __UI_RENDER_H__
#define __UI_RENDER_H__

#include <trainsys.h>
#include "../marklin.h"

#define RENDERER_ADDRESS "renderer"

int renderer_append_console(Tid renderer_tid, char* line);
int renderer_prompt(Tid renderer_tid, char ch);
int renderer_flip_switch(Tid renderer_tid, usize switch_id, SwitchMode mode);
int renderer_diagnostic(Tid renderer_tid, usize ticks, usize idle_percent);
void renderTask();

void uiTask();

#endif // __UI_RENDER_H__
