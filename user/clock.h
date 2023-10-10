#ifndef __USER_CLOCK_H__
#define __USER_CLOCK_H__

#include <trainsys.h>

void clockTask();

int Time(Tid tid);
int Delay(Tid tid, int ticks);
int DelayUntil(Tid tid, int ticks);

#endif // __USER_CLOCK_H__
