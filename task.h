#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "addrspace.h"
#include "switchframe.h"

#define MAX_TASK_COUNT 128

typedef uint32_t Tid;

typedef struct {

    // switchframe is at beginning of struct for easy access
    SwitchFrame* sf;

    Tid tid;
    Tid parent_tid;
    uint32_t priority;
    Addrspace addrspace;

    // The value of sp before context switch

    Address saved_sp;
    Address saved_x30;
} Task;

void tasktable_init(void);
Tid tasktable_create_task(uint32_t priority, void (*entrypoint)());
Task* tasktable_get_task(Tid tid);

void tasktable_set_current_task(Tid task);
Tid tasktable_current_task(void);
void tasktable_delete_task(Tid tid);

#endif // __TASK_H__
