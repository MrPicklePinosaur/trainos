#include <trainstd.h>
#include "perf.h"
#include "dev/timer.h"

Tid _idle_tid; // tid of the idle task

usize last_user_time; // timestamp when we last entered user mode
usize last_idle_time; // timestamp when we last entered idle mode
usize user_time; // total time spent in user mode
usize idle_time; // total time spent in idle

void
perf_init(Tid idle_tid)
{
    _idle_tid = idle_tid;

    idle_time = 0;
    last_idle_time = timer_get();

    user_time = 0;
    last_user_time = timer_get();
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

void
start_user(void)
{
    last_user_time = timer_get();
}

void
end_user(void)
{
    user_time += timer_get() - last_user_time;
}

usize
get_idle_time(void)
{
    return (idle_time*100)/(user_time);
}

Tid
idle_tid(void)
{
    return _idle_tid;
}
