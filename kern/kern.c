#include <trainstd.h>
#include "kern.h"
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"
#include "util.h"
#include "scheduler.h"
#include "alloc.h"
#include "kern/dev/uart.h"
#include "kern/dev/timer.h"
#include "kern/dev/gic.h"
#include "util.h"
#include "gacha.h"
#include "perf.h"

void
kern_init(void)
{
    arena_init();
    uart_init();
    log_init();
    pagetable_init();
    tasktable_init();
    vector_table_init();
    scheduler_init();
#if QEMU == 0
    gic_init();
#endif
    gacha_init();
}

void
set_task_state(Task* task, TaskState state)
{
    task->state = state;
}

// common code that should be ran when entering kernelmode
void
on_enter_kernelmode(Tid from_tid)
{
    if (from_tid == idle_tid()) end_idle();
}

// common code that should be ran when exiting kernelmode
void
on_exit_kernelmode(Tid to_tid)
{
    if (to_tid == idle_tid()) start_idle();
}

Tid
handle_svc_create(u32 priority, void (*entrypoint)(), const char* name)
{
    KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] Create task");
    Tid current_tid = tasktable_current_task();

    Tid new_tid = tasktable_create_task(priority, entrypoint, name);
    Task* new_task = tasktable_get_task(new_tid);

    // if first task, set parent to null
    if (new_tid == 1) new_task->parent_tid = 0;
    else new_task->parent_tid = current_tid;

    KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] Created new task %d", new_tid);

    scheduler_insert(new_tid, priority);
    set_task_state(new_task, TASKSTATE_READY);

    return new_tid;

}

int
handle_svc_send(int tid, const char* msg, int msglen, char* reply, int rplen)
{
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);

    // TODO: disallow sending to self?
    if (current_task->send_buf->in_use == true) {
        KLOG_WARN("Own send buffer hasn't been cleaned up");
    }

    *(current_task->send_buf) = (SendBuf) {
        .in_use = true,
        .reply_buf = reply,
        .reply_buf_len = rplen,
        .send_buf = 0,
        .send_buf_len = 0,
    };

    // Find the task in question
    Task* target_task = tasktable_get_task(tid);
    if (target_task == 0) {
        KLOG_ERROR("Invalid task %d", tid);
        return -1;
    }
    
    // Check task state
    if (target_task->state == TASKSTATE_RECEIVE_WAIT) {

        /* preconditions
         * - the target task is waiting for a message (in RECEIVE_WAIT state)
         * - the target task has an initalized receive buffer 
         */

        // RECEIVE_WAIT tasks exists, directly copy message
        set_task_state(current_task, TASKSTATE_REPLY_WAIT);

        KLOG_DEBUG_M(LOG_MASK_MSG, "Sending message to task %d, in RECEIVE_WAIT", tid);

        // copy message to buffer
        if (target_task->receive_buf->in_use == false) {
            PANIC("receiving task doesn't have initalized receive buffer");
        }

        if (msglen != target_task->receive_buf->buf_len) {
            KLOG_WARN("sender (msglen %d) and receiver (receive_buf len %d) don't have same receive buf len", msglen, target_task->receive_buf->buf_len);
        }
        int copylen = min(msglen, target_task->receive_buf->buf_len); 
        memcpy(target_task->receive_buf->buf, msg, copylen);
        
        // can unblock target task now
        set_task_state(target_task, TASKSTATE_READY);

        // set the return value for receive for the receive caller
        target_task->sf->x0 = copylen;

        // set sender tid for receive call
        /* KLOG_DEBUG("buffer for sender tid %x", target_task->receive_buf->sender_tid); */
        *(target_task->receive_buf->sender_tid) = current_tid;

        // delete receive_buf
        target_task->receive_buf->in_use = false;

    }
    else {

        /* preconditions
         * - the target task is not waiting to receive
         */

        // task is not in RECEIVE_WAIT, add sending task to recieving tasks' recieve queue 
        KLOG_DEBUG_M(LOG_MASK_MSG, "Sending message to task %d, not in RECEIVE_WAIT", tid);
        set_task_state(current_task, TASKSTATE_SEND_WAIT);
        if (cbuf_push_front(target_task->receive_queue, (u8)current_tid) == 1) {
            PANIC("receive queue is full");
        }

        // also buffer the data we are trying to send
        current_task->send_buf->send_buf = (char*)msg;
        current_task->send_buf->send_buf_len = msglen;
    }

    return 0;
}

void
handle_svc_receive(int *tid, char *msg, int msglen)
{
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);

    // check in receive queue for any pending messages
    if (cbuf_len(current_task->receive_queue) == 0) {

        /* preconditions
         * none
         */

        // if no pending messages, go into RECEIVE_WAIT
        set_task_state(current_task, TASKSTATE_RECEIVE_WAIT);
        *(current_task->receive_buf) = (ReceiveBuf) {
            .in_use = true,
            .buf = msg,
            .buf_len = msglen,
            .sender_tid = tid 
        };

        KLOG_DEBUG_M(LOG_MASK_MSG, "empty receive queue, blocking, in tid %d state %d", current_task->tid, current_task->state);

    } else {

        /* preconditions
         * - sender task is in SEND_WAIT
         * - sender task has an initalized send_buf with the message inside
         */

        // if there is pending messages, take the first and copy it
        Tid sender_tid = (Tid)cbuf_pop_back(current_task->receive_queue);
        Task* sender_task = tasktable_get_task(sender_tid);
        KLOG_DEBUG_M(LOG_MASK_MSG, "got message from tid = %d", sender_tid);

        // copy sender's message
        if (sender_task->send_buf->in_use == false) {
            // this should not happen (hopefully)
            PANIC("send buf is not initalized");
        }

        if (msglen != sender_task->send_buf->send_buf_len) {
            KLOG_WARN("sender (send_buf %d) and receiver (msglen %d) don't have same send buf len", sender_task->send_buf->send_buf_len, msglen);
        }
        int copylen = min(sender_task->send_buf->send_buf_len, msglen); 
        memcpy(msg, sender_task->send_buf->send_buf, copylen);

        // set sender to REPLY_WAIT
        set_task_state(sender_task, TASKSTATE_REPLY_WAIT);

        // set message length for receieve
        current_task->sf->x0 = copylen;

        // set the sender tid
        *tid = sender_tid;
    }
}

int
handle_svc_reply(int tid, const char *reply, int rplen)
{
    Task* target_task = tasktable_get_task(tid);
    if (target_task == 0) {
        return -1;
    }

    if (target_task->state != TASKSTATE_REPLY_WAIT) {
        return -2;
    }

    if (target_task->send_buf->in_use == false) {
        PANIC("sender's reply buf is not initalized");
    }

    int copylen = min(rplen, target_task->send_buf->reply_buf_len); 
    memcpy(target_task->send_buf->reply_buf, reply, copylen);

    // free the sender's buf
    // free(target_task->send_buf);
    target_task->send_buf->in_use = false;

    // make sender ready again
    set_task_state(target_task, TASKSTATE_READY);

    // set the return value for the sender's Send() call
    target_task->sf->x0 = copylen;

    return copylen;
}

void
handle_svc_await_event(EventId event_id)
{
    // TODO check if event_id is valid
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    current_task->state = TASKSTATE_AWAIT_EVENT_WAIT;
    current_task->blocking_event = event_id;
}

Tid
find_next_task(void)
{
    Tid next_tid = scheduler_next();
    if (next_tid == 0) {
        PANIC("No more tasks to run");
    }
    return next_tid;
}

void
handle_svc(void)
{
    //PRINT("kernel stack %x", asm_sp_el1());
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    SwitchFrame* sf = current_task->sf;

    on_enter_kernelmode(current_tid);

    /* switchframe_debug(sf); */
    /* KLOG_DEBUG("current task tid = %d", current_tid); */

    u32 opcode = asm_esr_el1() & 0x1FFFFFF;
    /* KLOG_DEBUG("jumped to vector table handler with opcode = %x", opcode); */
    KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] In task %d with name %s", current_tid, current_task->name);

    if (opcode == OPCODE_CREATE) {
        if (!scheduler_valid_priority(sf->x0)) {
            KLOG_WARN("Invalid task priority %d", sf->x0);
            sf->x0 = -1;
        }
        else {
            sf->x0 = handle_svc_create(sf->x0, (void (*)()) sf->x1, sf->x2);
        }
    }
    else if (opcode == OPCODE_MY_TID) {
        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] MyTid");

        sf->x0 = tasktable_current_task();
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {
        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] MyParentTid");

        sf->x0 = current_task->parent_tid;
    }
    else if (opcode == OPCODE_YIELD) {
        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] Yield");

        set_task_state(current_task, TASKSTATE_READY);
    }
    else if (opcode == OPCODE_EXIT) {
        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] Exit");

        // NOTE: maybe don't allow task 1 to be deleted?
        if (current_tid == 1) {
            KLOG_WARN("Attempting to delete task 1");
        }

        scheduler_remove(current_tid);
        set_task_state(current_task, TASKSTATE_EXITED);
        tasktable_delete_task(current_tid);

    } else if (opcode == OPCODE_SEND) {

        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] SEND");

        int ret = handle_svc_send(sf->x0, (const char*)sf->x1, sf->x2, (char*)sf->x3, sf->x4);
        if (ret < 0) {
            // set error code
            sf->x0 = ret;
        }

    } else if (opcode == OPCODE_RECEIVE) {

        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] RECEIVE");

        handle_svc_receive((int*)sf->x0, (char*)sf->x1, sf->x2);

    } else if (opcode == OPCODE_REPLY) {

        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] REPLY");

        sf->x0 = handle_svc_reply((int)sf->x0, (const char*)sf->x1, sf->x2);

    } else if (opcode == OPCODE_AWAITEVENT) {

        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] AWAIT EVENT");

        handle_svc_await_event((EventId)sf->x0);
        sf->x0 = 0; // TODO set proper return value

    } else if (opcode == OPCODE_TASK_NAME) {

        KLOG_INFO_M(LOG_MASK_SYSCALL, "[SYSCALL] TASK NAME");

        sf->x0 = tasktable_get_task(sf->x0)->name;

    } else {
        KLOG_WARN("Uncaught syscall with opcode %x", opcode);
    }

    Tid next_tid = find_next_task();

    KLOG_DEBUG_M(LOG_MASK_SYSCALL, "returning to task %d with name %s", next_tid, tasktable_get_task(next_tid)->name);
    tasktable_set_current_task(next_tid);

    on_exit_kernelmode(next_tid);
    asm_enter_usermode(tasktable_get_task(next_tid)->sf);
}

void
handle_interrupt(void)
{
    on_enter_kernelmode(tasktable_current_task());

    u32 iar = gic_read_iar();
    u32 interrupt_id = iar & 0x3FF;  // Get last 10 bits

    KLOG_INFO_M(LOG_MASK_ISR, "[INTERRUPT] ID: %d, IAR: %d from task %d", interrupt_id, iar, tasktable_current_task());

    // Timer task
    if (interrupt_id == 97) {
        scheduler_unblock_event(EVENT_CLOCK_TICK);
        timer_set_c1_next_tick();
    } else if (interrupt_id == 153) {
        bool marklin_interrupted = false;
        bool console_interrupted = false;

        // Handle whatever interrupts occurred
        if (uart_is_cts_interrupt(MARKLIN)) {
            KLOG_INFO_M(LOG_MASK_IO, "[INTERRUPT] Marklin CTS interrupt");
            if (uart_get_cts(MARKLIN)) {  // If CTS is high
                KLOG_INFO_M(LOG_MASK_IO, "[INTERRUPT] Unblocking CTS");
                scheduler_unblock_event(EVENT_MARKLIN_CTS);
            }
            marklin_interrupted = true;
        }
        if (uart_is_rx_interrupt(MARKLIN)) {
            KLOG_INFO_M(LOG_MASK_IO, "[INTERRUPT] Marklin RX interrupt");
            scheduler_unblock_event(EVENT_MARKLIN_RX);
            marklin_interrupted = true;
        }

        // Clear interrupts
        if (marklin_interrupted) {
            uart_clear_interrupts(MARKLIN);
        }
        if (console_interrupted) {
            uart_clear_interrupts(CONSOLE);
        }
    } else {
        PANIC("invalid interrupt id");
    }

    gic_write_eoir(iar); // TODO should this be iar or interrupt_id?

    // Find next task to go to
    Tid next_tid = find_next_task();
    Task* next_task = tasktable_get_task(next_tid);
    tasktable_set_current_task(next_tid);
    /* PRINT("[INTERRUPT] returning to task %d", next_task->tid); */

    on_exit_kernelmode(next_tid);
    asm_enter_usermode(next_task->sf);
}

void
unhandled_vector_table(int number)
{
    PANIC("unhandled vector table entry %d", number);
}

