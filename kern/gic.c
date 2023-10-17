#include "gic.h"

#define GIC_BASE 0xFF840000

static char* const GICD_BASE = (char*) (GIC_BASE + 0x1000);
static char* const GICC_BASE = (char*) (GIC_BASE + 0x2000);

static const uint32_t GICD_ISENABLE = 0x100;
static const uint32_t GICD_ITARGETS = 0x800;

static const uint32_t GICC_IAR = 0xC;
static const uint32_t GICC_EOIR = 0x10;

void
gic_target(uint32_t core, uint32_t id)
{
    char* target = GICD_BASE + GICD_ITARGETS + 4*(id/4);
    *(target + id%4) |= 0x1 << core;
}

void
gic_enable(uint32_t id)
{
    uint32_t* enable = (uint32_t*) (GICD_BASE + GICD_ISENABLE + 4*(id/32));
    *enable |= 0x1 << (id%32);
}

void
gic_init(void)
{
    gic_target(0, 97);
    // gic_target(0, 99);

    gic_enable(97);
    // gic_enable(99);
}

uint32_t gic_read_iar(void)
{
    uint32_t* iar = (uint32_t*) (GICC_BASE + GICC_IAR);
    return *iar;
}

void gic_write_eoir(uint32_t iar)
{
    uint32_t* eoir = (uint32_t*) (GICC_BASE + GICC_EOIR);
    (*eoir) = iar;
}
