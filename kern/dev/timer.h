#ifndef __DEV_TIMER_H__
#define __DEV_TIMER_H__

#include <stdint.h>

void timer_init(void);
uint64_t timer_get(void);

#endif // __DEV_TIMER_H__
