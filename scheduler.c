#include "scheduler.h"
#include "rpi.h"
#include "log.h"
#include "alloc.h"

SchedulerNode* mlq[16];
uint32_t task_count;

void
scheduler_init(void)
{
    task_count = 0;
    for (uint32_t i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        mlq[i] = 0;
    }
}

uint32_t
scheduler_count(void)
{
    return task_count;
}

void
do_scheduler_insert(Tid tid, uint32_t priority)
{
    SchedulerNode* node = arena_alloc(sizeof(SchedulerNode));
    node->tid = tid;
    node-> priority = priority;
    node->next = 0;

    // Reach the end of the linked list and insert the task there
    if (mlq[priority] == 0) {
        mlq[priority] = node;
    }
    else {
        SchedulerNode* current = mlq[priority];
        for (;;) {
            if (current->next == 0) {
                current->next = node;
                break;
            }
            current = current->next;
        }
    }
}

int
scheduler_insert(Tid tid, uint32_t priority)
{
    LOG_DEBUG("inserting task id %d, with priority %d, previous task_count = %d", tid, priority, task_count);

    if (priority >= NUM_PRIORITY_LEVELS) {
        LOG_DEBUG("invalid priority");
        return -1;
    }

    do_scheduler_insert(tid, priority);

    task_count++;

    return 0;
}

Tid
scheduler_next(void)
{
    for (uint32_t i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        if (mlq[i] != 0) {
            SchedulerNode* queue_top = mlq[i];
            mlq[i] = mlq[i]->next;
            do_scheduler_insert(queue_top->tid, queue_top->priority);
            return queue_top->tid;
        }
    }
    LOG_DEBUG("no next task because scheduler is empty");
    return 0;
}

void
scheduler_remove(Tid tid)
{
    LOG_DEBUG("removing task id %d, previous task_count = %d", tid, task_count);
    if (task_count == 0) {
        LOG_DEBUG("cannot remove task id %d because scheduler is empty", tid);
        return;
    }

    for (uint32_t i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        SchedulerNode* previous = 0;
        SchedulerNode* current = mlq[i];
        for (; current != 0;) {
            if (current->tid == tid) {
                if (previous) {
                    previous->next = current->next;
                }
                else {
                    mlq[i] = current->next;
                }
                arena_free(current);
                task_count--;
                return;
            }
            previous = current;
            current = current->next;
        }
    }

    LOG_DEBUG("could not find task id %d in scheduler", tid);
}
