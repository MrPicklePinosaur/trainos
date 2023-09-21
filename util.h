#ifndef __UTIL_H__
#define __UTIL_H__

#include <stddef.h>
#include <stdint.h>

// conversions
int a2d(char ch);
char a2i( char ch, char **src, int base, int *nump );
void ui2a( unsigned int num, unsigned int base, char *bf );
void i2a( int num, char *bf );

// memory
void *memset(void *s, int c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);

// Get the current privledge level fro the cpu
extern int priviledge_level(void);
// Get the value in ELR register
extern int reg_elr(void);

void panic(void);
int vbar_value(void);

extern uint32_t asm_esr_el1(void);
extern uint32_t asm_elr_el1(void);
extern uint32_t asm_sp_el0(void);

#endif // __UTIL_H__
