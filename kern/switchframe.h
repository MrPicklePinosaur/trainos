#ifndef __SWITCHFRAME_H__
#define __SWITCHFRAME_H__

#include "addrspace.h"

typedef struct {
    u64 x0;
    u64 x1;
    u64 x2;
    u64 x3;
    u64 x4;
    u64 x5;
    u64 x6;
    u64 x7;
    u64 x8;
    u64 x9;
    u64 x10;
    u64 x11;
    u64 x12;
    u64 x13;
    u64 x14;
    u64 x15;
    u64 x16;
    u64 x17;
    u64 x18;
    u64 x19;
    u64 x20;
    u64 x21;
    u64 x22;
    u64 x23;
    u64 x24;
    u64 x25;
    u64 x26;
    u64 x27;
    u64 x28;
    u64 x30;

    // Stack pointer
    u64 sp_el0;
    // Return address
    u64 elr_el1;
    // PState
    u64 spsr_el1;
} SwitchFrame;

SwitchFrame switchframe_new(Address sp, void (*entrypoint)());
void switchframe_debug(SwitchFrame* sf);

/* extern void switchframe_switch(Address* from_sp, Address* to_sp); */

extern void asm_enter_usermode(SwitchFrame* sf);

#endif // __SWITCHFRAME_H__
