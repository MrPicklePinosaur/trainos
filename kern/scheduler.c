#include "scheduler.h"
#include "uart.h"
#include "log.h"
#include "alloc.h"

SchedulerNode* mlq[NUM_PRIORITY_LEVELS];
uint32_t task_count;

void
scheduler_init(void)
{
    task_count = 0;
    for (uint32_t i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        mlq[i] = nullptr;
    }
}

uint32_t
scheduler_count(void)
{
    return task_count;
}

// Return task coutn for a given level of the queue
uint32_t
scheduler_count_level(SchedulerNode* node)
{
    uint32_t count = 0;
    while (node != nullptr) {
        ++count;
        node = node->next;
    }
    return count;
}

uint32_t scheduler_valid_priority(uint32_t priority) {
    return priority < NUM_PRIORITY_LEVELS;
}

void
do_scheduler_insert(Tid tid, uint32_t priority)
{
    SchedulerNode* node = arena_alloc(sizeof(SchedulerNode));
    node->tid = tid;
    node-> priority = priority;
    node->next = nullptr;

    // Reach the end of the linked list and insert the task there
    if (mlq[priority] == nullptr) {
        mlq[priority] = node;
    }
    else {
        SchedulerNode* current = mlq[priority];
        for (;;) {
            if (current->next == nullptr) {
                current->next = node;
                break;
            }
            current = current->next;
        }
    }
}

void
scheduler_insert(Tid tid, uint32_t priority)
{
    LOG_DEBUG("inserting task id %d, with priority %d, previous task_count = %d", tid, priority, task_count);

    if (priority >= NUM_PRIORITY_LEVELS) {
        LOG_DEBUG("invalid priority");
        return;
    }

    do_scheduler_insert(tid, priority);

    task_count++;
}

Tid
scheduler_next(void)
{
    for (uint32_t i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        for (uint32_t j = 0; j < scheduler_count_level(mlq[i]); ++j) {
            SchedulerNode* queue_top = mlq[i];
            mlq[i] = mlq[i]->next;
            do_scheduler_insert(queue_top->tid, queue_top->priority);
            TaskState state = tasktable_get_task(queue_top->tid)->state;
            if (state == TASKSTATE_READY || state == TASKSTATE_ACTIVE) {
                return queue_top->tid;
            }
        }
    }
    LOG_DEBUG("no next task because the scheduler contains no ready/active tasks");
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
        SchedulerNode* previous = nullptr;
        SchedulerNode* current = mlq[i];
        for (; current != nullptr;) {
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
