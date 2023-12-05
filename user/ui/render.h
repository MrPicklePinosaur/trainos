#ifndef __UI_RENDER_H__
#define __UI_RENDER_H__

#include <trainsys.h>
#include "../marklin.h"

#define PROMPT_ADDRESS "prompt"
#define CONSOLE_ADDRESS "console"

#define PROMPT_MAX_LEN 56

int renderer_append_console(Tid console_renderer_server, char* line);
int renderer_prompt(Tid prompt_renderer_server, char ch);
int renderer_diagnostic(Tid renderer_tid, usize ticks, usize idle_percent);

void uiTask();

#endif // __UI_RENDER_H__
