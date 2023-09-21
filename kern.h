#ifndef __KERN_H__
#define __KERN_H__

#include <stdint.h>
#include "addrspace.h"

static const uint32_t OPCODE_CREATE = 0;
static const uint32_t OPCODE_MY_TID = 1;
static const uint32_t OPCODE_MY_PARENT_TID = 2;
static const uint32_t OPCODE_YIELD = 3;
static const uint32_t OPCODE_EXIT = 4;

// Initalize kernel data structurea
void kern_init(void);

// simple test for running assembly
extern int asm_adder(int, int);

// Handle a call from svc
extern void vector_table_init(void);
void handle_svc(void);
extern int return_from_svc(int);
extern int pop_stack(void);

#endif // __KERN_H__
