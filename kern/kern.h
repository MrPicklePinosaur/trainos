#ifndef __KERN_H__
#define __KERN_H__

#include <stdint.h>
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"

typedef enum {
    OPCODE_CREATE = 0,
    OPCODE_MY_TID = 1,
    OPCODE_MY_PARENT_TID = 2,
    OPCODE_YIELD = 3,
    OPCODE_EXIT = 4,
    OPCODE_SEND = 5,
    OPCODE_RECEIVE = 6,
    OPCODE_REPLY = 7,
    OPCODE_AWAITEVENT = 8,
} OpCode;

// Initalize kernel data structurea
void kern_init(void);

// Handle a call from svc
extern void vector_table_init(void);
void handle_svc(void);
void handle_interrupt(void);
Tid handle_svc_create(uint32_t priority, void (*entrypoint)());

void unhandled_vector_table(void);

#endif // __KERN_H__
