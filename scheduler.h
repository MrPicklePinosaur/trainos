#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task.h"

void scheduler_init();
void scheduler_insert(Task* task);
Task* scheduler_top();
Task* scheduler_pop();

#endif // __SCHEDULER_H__