#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void timer_set_c1_next_tick(void);
void timer_init_c1(void);

u64 timer_get(void);

#endif // __TIMER_H__
