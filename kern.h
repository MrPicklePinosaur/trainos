#ifndef __KERN_H__
#define __KERN_H__

#include <stdint.h>

// Initalize kernel data structurea
void kern_init(void);

// simple test for running assembly
extern int asm_adder(int, int);

#endif // __KERN_H__
