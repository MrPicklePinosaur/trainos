
#include <stdint.h>
#include "kern/dev/dev.h"

static char* const TIMER_BASE = (char*)(MMIO_BASE + 0x00003000);

// Timer offsets
static const uint32_t TIMER_CS  =   0x00; // status
static const uint32_t TIMER_CLO =   0x04; // counter low
static const uint32_t TIMER_CHI =   0x08; // counter high
static const uint32_t TIMER_C0  =   0x0c; // timer compare 0
static const uint32_t TIMER_C1  =   0x10; // timer compare 1
static const uint32_t TIMER_C2  =   0x14; // timer compare 2
static const uint32_t TIMER_C3  =   0x18; // timer compare 3

#define TIMER_REG(offset) (*(volatile uint32_t*)(TIMER_BASE + offset))

const uint32_t TICK_TIME = 1000000;  // 10000 normally

uint64_t timer_get(void) {
    uint64_t time;
    time = TIMER_REG(TIMER_CLO);
    time |= (uint64_t)TIMER_REG(TIMER_CHI) << 32;
    return(time);
}

void
timer_set_c1_next_tick()
{
    TIMER_REG(TIMER_C1) = TIMER_REG(TIMER_C1) + TICK_TIME;
}

void
timer_init_c1(void)
{
    TIMER_REG(TIMER_C1) = timer_get() + TICK_TIME;
}
