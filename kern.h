#ifndef __KERN_H__
#define __KERN_H__

// Initalize kernel data structurea
void kern_init(void);

extern void switchframe_switch();

// simple test for running assembly
extern int asm_adder(int, int);

#endif // __KERN_H__
