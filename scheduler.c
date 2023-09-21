#include "scheduler.h"
#include "rpi.h"
#include "log.h"

Task* pqueue[MAX_TASK_COUNT];
uint32_t pqueue_size;

void
scheduler_init(void)
{
    pqueue_size = 0;
}

uint32_t
scheduler_count(void)
{
    return pqueue_size;
}

void
scheduler_insert(Task* task)
{

    LOG_DEBUG("inserting task %x, with pqueue_size = %d", task, pqueue_size);
    if (pqueue_size >= MAX_TASK_COUNT) {
        // TODO error message
        return;
    }

    pqueue[pqueue_size] = task;

    for (uint32_t current = pqueue_size; current != 0; current = (current - 1) / 2) {
        // Lower number is a higher priority. So if pqueue[current] has a lower number
        // than its parent, it must swap up.
        uint32_t parent = (current - 1) / 2;
        if (pqueue[current]->priority >= pqueue[parent]->priority) {
            break;
        }
        Task* temp = pqueue[parent];
        pqueue[parent] = pqueue[current];
        pqueue[current] = temp;
    }
    pqueue_size++;
}

Task*
scheduler_top(void)
{
    if (pqueue_size == 0) {
        // TODO error message
        return 0;
    }
    return pqueue[0];
}

Task* scheduler_pop(void)
{
    LOG_DEBUG("popping task %x, with pqueue_size = %d", scheduler_top(), pqueue_size);
    if (pqueue_size == 0) {
        // TODO error message
        return 0;
    }

    Task* result = pqueue[0];
    pqueue_size--;
    pqueue[0] = pqueue[pqueue_size];
    
    for (uint32_t current = 0;;) {
        uint32_t left = 2 * current + 1;
        uint32_t right = 2 * current + 2;
        uint32_t max_child;

        if (right >= pqueue_size) {
            if (left >= pqueue_size) {
                break;  // Current element has no children
            }
            max_child = left;  // Current element has one child
        }
        else if (pqueue[left]->priority <= pqueue[right]->priority) {
            max_child = left;  // Left element is the higher priority child
        }
        else {
            max_child = right;  // Right element is the higher priority child
        }

        // Lower number is a higher priority. So if pqueue[current] has a higher number
        // than either of its children, it must swap down.
        if (pqueue[current]->priority < pqueue[max_child]->priority) {
            break;
        }
        Task* temp = pqueue[max_child];
        pqueue[max_child] = pqueue[current];
        pqueue[current] = temp;
    }

    return result;
}
