#include "timer.h"
#include "mem.h"
#include "kern/dev/timer.h"

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
        .sum_of_squares = 0,
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
    u64 end = timer_get();

    timer->previous = end - timer->start;
    timer->worst = timer->previous > timer->worst ? timer->previous : timer->worst;

    // Here we use Welford's online algorithm for incrementally computing variance
    // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    u64 prev_mean = timer->total / timer->samples;
    u64 curr_mean = (timer->total + timer->previous) / (timer->samples + 1);
    timer->sum_of_squares = timer->sum_of_squares + (timer->previous - prev_mean)*(timer->previous - curr_mean);

    timer->samples++;
    timer->total += timer->previous;
}

u64
timer_get_mean(Timer* timer)
{
    return timer->total / timer->samples;
}

u64
timer_get_variance(Timer* timer)
{
    return timer->sum_of_squares / timer->samples;
}
