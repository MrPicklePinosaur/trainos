#ifndef __KERN_H__
#define __KERN_H__

#include <stdint.h>

typedef struct {
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
    uint64_t x29;
    uint64_t x30;
} SwitchFrame;

// Initalize kernel data structurea
void kern_init(void);

extern void switchframe_switch(char* from_sp, char* to_sp);

// simple test for running assembly
extern int asm_adder(int, int);

#endif // __KERN_H__
