#ifndef __PERF_H__
#define __PERF_H__

#include <traindef.h>
#include <trainsys.h>

void perf_init(Tid idle_tid);
void start_idle(void);
void end_idle(void);
usize get_idle_time(void);

Tid idle_tid(void);

#endif // __PERF_H__
