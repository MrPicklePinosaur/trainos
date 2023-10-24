
#include <traindef.h>
#include "kern/dev/dev.h"

static char* const TIMER_BASE = (char*)(MMIO_BASE + 0x00003000);

// Timer offsets
static const u32 TIMER_CS  =   0x00; // status
static const u32 TIMER_CLO =   0x04; // counter low
static const u32 TIMER_CHI =   0x08; // counter high
static const u32 TIMER_C0  =   0x0c; // timer compare 0
static const u32 TIMER_C1  =   0x10; // timer compare 1
static const u32 TIMER_C2  =   0x14; // timer compare 2
static const u32 TIMER_C3  =   0x18; // timer compare 3

#define TIMER_REG(offset) (*(volatile u32*)(TIMER_BASE + offset))

const u32 TICK_TIME = 10000;  // 10000 normally

u64 timer_get(void) {
    u64 time;
    time = TIMER_REG(TIMER_CLO);
    time |= (u64)TIMER_REG(TIMER_CHI) << 32;
    return(time);
}

void
timer_set_c1_next_tick()
{
    TIMER_REG(TIMER_C1) = TIMER_REG(TIMER_C1) + ((timer_get() - TIMER_REG(TIMER_C1))/TICK_TIME + 1) * TICK_TIME;
    TIMER_REG(TIMER_CS) |= (0x1 << 1);
}

void
timer_init_c1(void)
{
    TIMER_REG(TIMER_C1) = timer_get() + TICK_TIME;
}
