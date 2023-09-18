#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "addrspace.h"

#define MAX_TASK_COUNT 128

typedef uint32_t Tid;

typedef struct {
    Tid tid;
    uint32_t priority;
    Addrspace addrspace;
} Task;

typedef struct {
    uint32_t next_tid;
    Task tasks[MAX_TASK_COUNT];
} TaskTable;

void tasktable_init(void);
Tid tasktable_create_task(uint32_t priority);

#endif // __TASK_H__
