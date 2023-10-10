#ifndef __USER_CLOCK_H__
#define __USER_CLOCK_H__

#include <trainsys.h>

void clockTask();

int Time(Tid server, Tid tid);
int Delay(Tid server, Tid tid, int ticks);
int DelayUntil(Tid server, Tid tid, int ticks);

#endif // __USER_CLOCK_H__
