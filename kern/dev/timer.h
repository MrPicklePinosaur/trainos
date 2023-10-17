#ifndef __DEV_TIMER_H__
#define __DEV_TIMER_H__

#include <stdint.h>

void timer_set_c1_next_tick(void);
void timer_init_c1(void);

uint64_t timer_get(void);

#endif // __DEV_TIMER_H__
