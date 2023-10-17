#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stddef.h>
#include "task.h"

#define NUM_PRIORITY_LEVELS 16

typedef struct SchedulerNode SchedulerNode;
struct SchedulerNode {
    Tid tid;
    uint32_t priority;
    SchedulerNode* next;
};

void scheduler_init(void);
uint32_t scheduler_count(void);
uint32_t scheduler_count_level(SchedulerNode* node);
uint32_t scheduler_valid_priority(uint32_t priority);  // Returns 1 if valid priority, 0 otherwise
void scheduler_insert(Tid tid, uint32_t priority);
Tid scheduler_next(void);
void scheduler_remove(Tid tid);
void scheduler_unblock_event(int eventid);  // Unblock all tasks waiting for eventid

#endif // __SCHEDULER_H__
