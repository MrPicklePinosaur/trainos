
#include "kern/dev/timer.h"

#if QEMU == 0

#include "rpi.h"

/*********** TIMER CONFIGURATION ********************************/

static char* const TIMER_BASE = (char*)(MMIO_BASE + 0x00003000);

// Timer offsets
static const uint32_t TIMER_CS  =   0x00; // status
static const uint32_t TIMER_CLO =   0x04; // counter low
static const uint32_t TIMER_CHI =   0x08; // counter high

#define TIMER_REG(offset) (*(volatile uint32_t*)(TIMER_BASE + offset))

void
timer_init(void)
{

}

uint64_t
timer_get(void) {
    uint64_t time;
    time = TIMER_REG(TIMER_CLO);
    time |= (uint64_t)TIMER_REG(TIMER_CHI) << 32;
    return(time);
}

#endif
