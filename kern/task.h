#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "addrspace.h"
#include "switchframe.h"

#define TASK_TABLE_SIZE 128

typedef uint32_t Tid;

typedef enum {
    TASKSTATE_ACTIVE,
    TASKSTATE_READY,
    TASKSTATE_EXITED
} TaskState;

typedef struct {

    // switchframe is at beginning of struct for easy access
    SwitchFrame* sf;

    Tid tid;
    Tid parent_tid;
    TaskState state;
    uint32_t priority;
    Addrspace addrspace;

    // The value of sp before context switch

    Address saved_sp;
    Address saved_x30;
} Task;

// Nodes for a linked list
typedef struct TaskNode TaskNode;
struct TaskNode {
    Task* task;
    TaskNode* next;
};

void tasktable_init(void);
Tid tasktable_create_task(uint32_t priority, void (*entrypoint)());
Task* tasktable_get_task(Tid tid);

// Update the current active tasks. If a task was active before, it will be made into ready
void tasktable_set_current_task(Tid tid);
Tid tasktable_current_task(void);
void tasktable_delete_task(Tid tid);

#endif // __TASK_H__
