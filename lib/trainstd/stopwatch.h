#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <stdint.h>
#include <traindef.h>

typedef struct Stopwatch Stopwatch;

struct Stopwatch {
    u64 start;

    u64 previous;
    u64 worst;

    u32 samples;
    u64 total;
    u64 sum_of_squares;
};

Stopwatch* stopwatch_new();
void stopwatch_start(Stopwatch* stopwatch);
void stopwatch_end(Stopwatch* stopwatch);
u64 stopwatch_get_mean(Stopwatch* stopwatch);
u64 stopwatch_get_variance(Stopwatch* stopwatch);

#endif // __STOPWATCH_H__
