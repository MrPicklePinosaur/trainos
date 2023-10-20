#include <trainstd.h>
#include "perf.h"
#include "dev/timer.h"

Tid _idle_tid; // tid of the idle task

usize init_time; // timestamp when perf counter is started
usize last_idle_time; // timestamp when we last entered idle mod
usize idle_time; // total time spent in idle

void
perf_init(Tid idle_tid)
{
    _idle_tid = idle_tid;

    init_time = timer_get();
    idle_time = 0;
    last_idle_time = 0;
}

void
start_idle(void)
{
    last_idle_time = timer_get();
}

void
end_idle(void)
{
    idle_time += timer_get() - last_idle_time;
}

usize
get_idle_time(void)
{
    return (idle_time*100)/(timer_get()-init_time);
}

Tid
idle_tid(void)
{
    return _idle_tid;
}
