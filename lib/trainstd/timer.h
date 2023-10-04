#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

typedef struct Timer Timer;

struct Timer {
    uint64_t start;

    uint64_t previous;
    uint64_t worst;

    uint32_t samples;
    uint64_t total;
};

Timer* timer_new();
void timer_start(Timer* timer);
void timer_end(Timer* timer);
uint64_t timer_get_mean();

#endif // __TIMER_H__