#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task.h"

void scheduler_init(void);
uint32_t scheduler_count(void);
void scheduler_insert(Task* task);
Task* scheduler_top(void);
Task* scheduler_pop(void);

#endif // __SCHEDULER_H__