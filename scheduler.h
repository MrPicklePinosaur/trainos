#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task.h"

typedef struct SchedulerNode SchedulerNode;
struct SchedulerNode {
    Tid tid;
    uint32_t priority;
    SchedulerNode* next;
};

static const uint32_t NUM_PRIORITY_LEVELS = 16;

void scheduler_init(void);
uint32_t scheduler_count(void);
int scheduler_insert(Tid tid, uint32_t priority);  // Returns -1 if the priority is invalid
Tid scheduler_next(void);
void scheduler_remove(Tid tid);

#endif // __SCHEDULER_H__