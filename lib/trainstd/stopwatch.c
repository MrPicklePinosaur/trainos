#include "stopwatch.h"
#include "mem.h"
#include "kern/dev/timer.h"

Stopwatch*
stopwatch_new()
{
    Stopwatch* stopwatch = alloc(sizeof(Stopwatch));
    *stopwatch = (Stopwatch) {
        .start = 0,

        .previous = 0,
        .worst = 0,

        .samples = 0,
        .total = 0,
        .sum_of_squares = 0,
    };
    return stopwatch;
}

void
stopwatch_start(Stopwatch* stopwatch)
{
    stopwatch->start = timer_get();
}

void
stopwatch_end(Stopwatch* stopwatch)
{
    u64 end = timer_get();

    stopwatch->previous = end - stopwatch->start;
    stopwatch->worst = stopwatch->previous > stopwatch->worst ? stopwatch->previous : stopwatch->worst;

    // Here we use Welford's online algorithm for incrementally computing variance
    // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    u64 prev_mean = stopwatch->total / stopwatch->samples;
    u64 curr_mean = (stopwatch->total + stopwatch->previous) / (stopwatch->samples + 1);
    stopwatch->sum_of_squares = stopwatch->sum_of_squares + (stopwatch->previous - prev_mean)*(stopwatch->previous - curr_mean);

    stopwatch->samples++;
    stopwatch->total += stopwatch->previous;
}

u64
stopwatch_get_mean(Stopwatch* stopwatch)
{
    return stopwatch->total / stopwatch->samples;
}

u64
stopwatch_get_variance(Stopwatch* stopwatch)
{
    return stopwatch->sum_of_squares / stopwatch->samples;
}
