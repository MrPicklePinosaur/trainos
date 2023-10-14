#include "gic.h"

#define GIC_BASE 0xFF840000

static char* const GICD_BASE = (char*) (GIC_BASE + 0x1000);
static char* const GICC_BASE = (char*) (GIC_BASE + 0x2000);

static const uint32_t GICD_ISENABLE = 0x100;
static const uint32_t GICD_ITARGETS = 0x800;

void
gic_init(void)
{
    char* target = GICD_BASE + GICD_ITARGETS + 4*(96/4);
    *(target + 1) |= 0x1;  // ID 97 should target core 0
    *(target + 3) |= 0x1;  // ID 99 should target core 0

    char* enable = GICD_BASE + GICD_ISENABLE + 4*(96/32);
    *enable |= 0x2;  // Enable ID 97
    *enable |= 0x8;  // Enable ID 99
}
