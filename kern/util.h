#ifndef __UTIL_H__
#define __UTIL_H__

#include <traindef.h>
#include <stddef.h>
#include <stdint.h>
#include "log.h"

// ansi codes
#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE(r, c) "\033[" r ";" c "H"
#define ANSI_CLEAR_LINE "\033[K"

#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

#define ANSI_RESET "\033[0m"

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
