#ifndef __SWITCHFRAME_H__
#define __SWITCHFRAME_H__

#include "addrspace.h"

typedef struct {
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x30;

    // Stack pointer
    uint64_t sp_el0;
    // Return address
    uint64_t elr_el1;
    // PState
    uint64_t spsr_el1;
} SwitchFrame;

SwitchFrame switchframe_new(Address sp, void (*entrypoint)());
void switchframe_debug(SwitchFrame* sf);

/* extern void switchframe_switch(Address* from_sp, Address* to_sp); */

extern void asm_enter_usermode(SwitchFrame* sf);

#endif // __SWITCHFRAME_H__
