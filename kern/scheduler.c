#include <trainstd.h>
#include "scheduler.h"
#include "kern/dev/uart.h"
#include "alloc.h"

SchedulerNode* mlq[NUM_PRIORITY_LEVELS];
u32 task_count;

void
scheduler_init(void)
{
    task_count = 0;
    for (u32 i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        mlq[i] = nullptr;
    }
}

u32
scheduler_count(void)
{
    return task_count;
}

// Return task coutn for a given level of the queue
u32
scheduler_count_level(SchedulerNode* node)
{
    u32 count = 0;
    while (node != nullptr) {
        ++count;
        node = node->next;
    }
    return count;
}

u32 scheduler_valid_priority(u32 priority) {
    return priority < NUM_PRIORITY_LEVELS;
}

void
do_scheduler_insert(SchedulerNode* node)
{
    // Reach the end of the linked list and insert the task there
    if (mlq[node->priority] == nullptr) {
        mlq[node->priority] = node;
    }
    else {
        SchedulerNode* current = mlq[node->priority];
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
scheduler_insert(Tid tid, u32 priority)
{
    KLOG_DEBUG_M(LOG_MASK_SCHED, "inserting task id %d, with priority %d, previous task_count = %d", tid, priority, task_count);

    if (priority >= NUM_PRIORITY_LEVELS) {
        KLOG_ERROR("invalid priority");
        return;
    }

    SchedulerNode* node = arena_alloc(sizeof(SchedulerNode));
    node->tid = tid;
    node->priority = priority;
    node->next = nullptr;
    do_scheduler_insert(node);

    task_count++;
}

Tid
scheduler_next(void)
{
    for (u32 i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        for (u32 j = 0; j < scheduler_count_level(mlq[i]); ++j) {
            SchedulerNode* queue_top = mlq[i];
            mlq[i] = mlq[i]->next;
            queue_top->next = nullptr;
            do_scheduler_insert(queue_top);
            TaskState state = tasktable_get_task(queue_top->tid)->state;
            if (state == TASKSTATE_READY || state == TASKSTATE_ACTIVE) {
                return queue_top->tid;
            }
        }
    }
    PANIC("no next task because the scheduler contains no ready/active tasks");
    return 0;
}

void
scheduler_remove(Tid tid)
{
    KLOG_DEBUG_M(LOG_MASK_SCHED, "removing task id %d, previous task_count = %d", tid, task_count);
    if (task_count == 0) {
        KLOG_ERROR("cannot remove task id %d because scheduler is empty", tid);
        return;
    }

    for (u32 i = 0; i < NUM_PRIORITY_LEVELS; i++) {
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

    KLOG_ERROR("could not find task id %d in scheduler", tid);
}

void scheduler_unblock_event(int eventid, int event_data) {
    for (u32 i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        SchedulerNode* current = mlq[i];
        for (; current != nullptr;) {
            Task* task = tasktable_get_task(current->tid);
            if (task->state == TASKSTATE_AWAIT_EVENT_WAIT && task->blocking_event == eventid) {
                task->state = TASKSTATE_READY;
                task->blocking_event = EVENT_NONE;
                task->sf->x0 = event_data; // set return value to the event data that was passed
            }
            current = current->next;
        }
    }
}
