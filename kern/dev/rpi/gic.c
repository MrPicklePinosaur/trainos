
#include "kern/dev/gic.h"

#if QEMU == 0

#include <traindef.h>

#define GIC_BASE 0xFF840000

static char* const GICD_BASE = (char*) (GIC_BASE + 0x1000);
static char* const GICC_BASE = (char*) (GIC_BASE + 0x2000);

static const u32 GICD_ISENABLE = 0x100;
static const u32 GICD_ITARGETS = 0x800;

static const u32 GICC_IAR = 0xC;
static const u32 GICC_EOIR = 0x10;

#define GICD_REG(offset) (*(volatile u32*)(GICD_BASE + offset))
#define GICC_REG(offset) (*(volatile u32*)(GICC_BASE + offset))

void
gic_target(u32 core, u32 id)
{
    char* target = GICD_BASE + GICD_ITARGETS + 4*(id/4);
    *(target + id%4) |= 0x1 << core;
}

void
gic_enable(u32 id)
{
    u32* enable = (u32*) (GICD_BASE + GICD_ISENABLE + 4*(id/32));
    *enable |= 0x1 << (id%32);
}

void
gic_init(void)
{
    gic_target(0, 97);
    gic_target(0, 153);
    // gic_target(0, 99);

    gic_enable(97);
    gic_enable(153);
    // gic_enable(99);
}

u32 gic_read_iar(void)
{
    u32* iar = (u32*) (GICC_BASE + GICC_IAR);
    return *iar;
}

void gic_write_eoir(u32 iar)
{
    u32* eoir = (u32*) (GICC_BASE + GICC_EOIR);
    (*eoir) = iar;
}
#endif
