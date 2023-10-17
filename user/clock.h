#ifndef __USER_CLOCK_H__
#define __USER_CLOCK_H__

#include <trainsys.h>

#define CLOCK_ADDRESS "Clock"

void clockTask();

int Time(Tid clock_server);
int Delay(Tid clock_server, int ticks);
int DelayUntil(Tid clock_server, int ticks);

#endif // __USER_CLOCK_H__
