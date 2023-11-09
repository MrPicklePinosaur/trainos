#ifndef __UI_PROMPT_H__
#define __UI_PROMPT_H__

#include <traindef.h>
#include "user/trainstate.h"

#define CH_ENTER     0x0d
#define CH_BACKSPACE 0x08

TrainState get_train_state(usize train);
void promptTask();

#endif // __UI_PROMPT_H__
