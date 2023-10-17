#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <traindef.h>

typedef struct Timer Timer;

struct Timer {
    u64 start;

    u64 previous;
    u64 worst;

    u32 samples;
    u64 total;
    u64 sum_of_squares;
};

Timer* timer_new();
void timer_start(Timer* timer);
void timer_end(Timer* timer);
u64 timer_get_mean(Timer* timer);
u64 timer_get_variance(Timer* timer);

#endif // __TIMER_H__
