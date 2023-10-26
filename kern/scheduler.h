#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stddef.h>
#include "task.h"

#define NUM_PRIORITY_LEVELS 16

typedef struct SchedulerNode SchedulerNode;
struct SchedulerNode {
    Tid tid;
    u32 priority;
    SchedulerNode* next;
};

void scheduler_init(void);
u32 scheduler_count(void);
u32 scheduler_count_level(SchedulerNode* node);
u32 scheduler_valid_priority(u32 priority);  // Returns 1 if valid priority, 0 otherwise
void scheduler_insert(Tid tid, u32 priority);
Tid scheduler_next(void);
void scheduler_remove(Tid tid);
void scheduler_unblock_event(int eventid, int event_data);  // Unblock all tasks waiting for eventid

#endif // __SCHEDULER_H__
