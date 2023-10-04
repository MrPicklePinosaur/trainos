#include "timer.h"
#include "mem.h"
#include "kern/rpi.h"

Timer*
timer_new()
{
    Timer* timer = alloc(sizeof(Timer));
    *timer = (Timer) {
        .start = 0,

        .previous = 0,
        .worst = 0,

        .samples = 0,
        .total = 0,
    };
    return timer;
}

void
timer_start(Timer* timer)
{
    timer->start = timer_get();
}

void
timer_end(Timer* timer)
{
    uint64_t end = timer_get();

    timer->previous = end - timer->start;
    timer->worst = timer->previous > timer->worst ? timer->previous : timer->worst;

    timer->samples++;
    timer->total += timer->previous;
}

uint64_t
timer_get_mean(Timer* timer)
{
    return timer->total / timer->samples;
}
