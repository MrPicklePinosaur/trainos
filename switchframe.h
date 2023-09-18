#ifndef __SWITCHFRAME_H__
#define __SWITCHFRAME_H__

#include "addrspace.h"
#include "task.h"

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
    uint64_t x30;
} SwitchFrame;

SwitchFrame switchframe_new(void);

extern void switchframe_switch(uint64_t* from_sp, uint64_t* to_sp);

// Initalize the switchframe for a task
void switchframe_init(Task* task, void (*entrypoint)());

#endif // __SWITCHFRAME_H__
