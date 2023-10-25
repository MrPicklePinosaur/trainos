#ifndef __UTIL_H__
#define __UTIL_H__

#include <traindef.h>
#include <trainstd.h>

// memory
void *memset(void *s, int c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);

int min(int a, int b);

// Get the current privledge level fro the cpu
extern int priviledge_level(void);
// Get the value in ELR register
extern int reg_elr(void);

int vbar_value(void);

extern u32 asm_esr_el1(void);
extern u32 asm_elr_el1(void);
extern u32 asm_sp_el0(void);
extern u32 asm_spsr_el1(void);
extern void asm_wfi(void);

#endif // __UTIL_H__
