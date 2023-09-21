#include "switchframe.h"
#include "rpi.h"
#include "log.h"

SwitchFrame
switchframe_new(Address sp, void (*entrypoint)())
{
    return (SwitchFrame) {
        .x0 = 0,
        .x1 = 0,
        .x2 = 0,
        .x3 = 0,
        .x4 = 0,
        .x5 = 0,
        .x6 = 0,
        .x7 = 0,
        .x8 = 0,
        .x9 = 0,
        .x10 = 0,
        .x11 = 0,
        .x12 = 0,
        .x13 = 0,
        .x14 = 0,
        .x15 = 0,
        .x16 = 0,
        .x17 = 0,
        .x18 = 0,

        .x19 = 0,
        .x20 = 0,
        .x21 = 0,
        .x22 = 0,
        .x23 = 0,
        .x24 = 0,
        .x25 = 0,
        .x26 = 0,
        .x27 = 0,
        .x28 = 0,
        .x30 = (uint64_t)entrypoint,

        .sp_el0 = (uint64_t)sp,
        .elr_el1 = (uint64_t)entrypoint,
        .spsr_el1 = 0,
    };
}

void
switchframe_debug(SwitchFrame* sf)
{
    LOG_DEBUG("x0 = 0x%x\r\nx1 = 0x%x\r\nx2 = 0x%x\r\nx3 = 0x%x\r\nx4 = 0x%x\r\nx5 = 0x%x\r\nx6 = 0x%x\r\nx7 = 0x%x\r\nx8 = 0x%x\r\nx9 = 0x%x\r\nx10 = 0x%x\r\nx11 = 0x%x\r\nx12 = 0x%x\r\nx13 = 0x%x\r\nx14 = 0x%x\r\nx15 = 0x%x\r\nx16 = 0x%x\r\nx17 = 0x%x\r\nx18 = 0x%x\r\nx19 = 0x%x\r\nx20 = 0x%x\r\nx21 = 0x%x\r\nx22 = 0x%x\r\nx23 = 0x%x\r\nx24 = 0x%x\r\nx25 = 0x%x\r\nx26 = 0x%x\r\nx27 = 0x%x\r\nx28 = 0x%x\r\nx30 = 0x%x\r\nsp_el0 = 0x%x\r\nelr_el1 = 0x%x\r\nspsr_el1 = 0x%d",
        sf->x0,
        sf->x1,
        sf->x2,
        sf->x3,
        sf->x4,
        sf->x5,
        sf->x6,
        sf->x7,
        sf->x8,
        sf->x9,
        sf->x10,
        sf->x11,
        sf->x12,
        sf->x13,
        sf->x14,
        sf->x15,
        sf->x16,
        sf->x17,
        sf->x18,
        sf->x19,
        sf->x20,
        sf->x21,
        sf->x22,
        sf->x23,
        sf->x24,
        sf->x25,
        sf->x26,
        sf->x27,
        sf->x28,
        sf->x30,
        sf->sp_el0,
        sf->elr_el1,
        sf->spsr_el1
    );
}

