#include "kern.h"
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"
#include "log.h"
#include "util.h"
#include "scheduler.h"
#include "alloc.h"
#include "kern/dev/uart.h"
#include "kern/dev/timer.h"
#include "util.h"
#include "gic.h"
#include "gacha.h"

void
kern_init(void)
{
    uart_init();
    arena_init();
    pagetable_init();
    tasktable_init();
    vector_table_init();
    scheduler_init();
    gic_init();
    gacha_init();
}

void
set_task_state(Task* task, TaskState state)
{
    task->state = state;
}

Tid
handle_svc_create(uint32_t priority, void (*entrypoint)())
{
    Tid current_tid = tasktable_current_task();

    Tid new_tid = tasktable_create_task(priority, entrypoint);
    Task* new_task = tasktable_get_task(new_tid);

    // if first task, set parent to null
    if (new_tid == 1) new_task->parent_tid = 0;
    else new_task->parent_tid = current_tid;

    LOG_INFO("[SYSCALL] Created new task %d", new_tid);

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
    if (current_task->send_buf != 0) {
        LOG_WARN("Own send buffer hasn't been cleaned up");
    }

    SendBuf* send_buf = arena_alloc(sizeof(send_buf));
    *send_buf = (SendBuf) {
        .reply_buf = reply,
        .reply_buf_len = rplen,
        .send_buf = 0,
        .send_buf_len = 0,
    };
    current_task->send_buf = send_buf;

    // Find the task in question
    Task* target_task = tasktable_get_task(tid);
    if (target_task == 0) {
        LOG_DEBUG("Invalid task %d", tid);
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

        LOG_DEBUG("Sending message to task %d, in RECEIVE_WAIT", tid);

        // copy message to buffer
        if (target_task->receive_buf == NULL) {
            PANIC("receiving task doesn't have initalized receive buffer");
        }

        int copylen = min(msglen, target_task->receive_buf->buf_len); 
        memcpy(target_task->receive_buf->buf, msg, copylen);
        
        // can unblock target task now
        set_task_state(target_task, TASKSTATE_READY);

        // set the return value for receive for the receive caller
        target_task->sf->x0 = copylen;

        // set sender tid for receive call
        /* LOG_DEBUG("buffer for sender tid %x", target_task->receive_buf->sender_tid); */
        *(target_task->receive_buf->sender_tid) = current_tid;

        // delete receive_buf
        arena_free(target_task->receive_buf);
        target_task->receive_buf = 0;

    }
    else {

        /* preconditions
         * - the target task is not waiting to receive
         */

        // task is not in RECEIVE_WAIT, add sending task to recieving tasks' recieve queue 
        LOG_DEBUG("Sending message to task %d, not in RECEIVE_WAIT", tid);
        set_task_state(current_task, TASKSTATE_SEND_WAIT);
        cbuf_push_front(target_task->receive_queue, (uint8_t)current_tid);

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
        ReceiveBuf* receive_buf = arena_alloc(sizeof(ReceiveBuf));
        *receive_buf = (ReceiveBuf) {
            .buf = msg,
            .buf_len = msglen,
            .sender_tid = tid 
        };
        current_task->receive_buf = receive_buf;
        LOG_DEBUG("empty receive queue, blocking, in tid %d state %d", current_task->tid, current_task->state);

    } else {

        /* preconditions
         * - sender task is in SEND_WAIT
         * - sender task has an initalized send_buf with the message inside
         */

        // if there is pending messages, take the first and copy it
        Tid sender_tid = (Tid)cbuf_pop_back(current_task->receive_queue);
        Task* sender_task = tasktable_get_task(sender_tid);
        LOG_DEBUG("got message from tid = %d", sender_tid);

        // copy sender's message
        if (sender_task->send_buf == 0) {
            // this should not happen (hopefully)
            PANIC("send buf is not initalized");
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

    if (target_task->send_buf == 0) {
        PANIC("sender's reply buf is not initalized");
    }

    int copylen = min(rplen, target_task->send_buf->reply_buf_len); 
    memcpy(target_task->send_buf->reply_buf, reply, copylen);

    // free the sender's buf
    free(target_task->send_buf);
    target_task->send_buf = 0;

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
    Tid current_tid = tasktable_current_task();
    Task* current_task = tasktable_get_task(current_tid);
    SwitchFrame* sf = current_task->sf;

    Tid next_tid;

    /* switchframe_debug(sf); */
    /* LOG_DEBUG("current task tid = %d", current_tid); */

    uint32_t opcode = asm_esr_el1() & 0x1FFFFFF;
    /* LOG_DEBUG("jumped to vector table handler with opcode = %x", opcode); */
    LOG_INFO("[SYSCALL] In task %d", current_tid);

    if (opcode == OPCODE_CREATE) {
        if (!scheduler_valid_priority(sf->x0)) {
            LOG_DEBUG("Invalid task priority %d", sf->x0);
            sf->x0 = -1;
        }
        else {
            sf->x0 = handle_svc_create(sf->x0, (void (*)()) sf->x1);
        }
        next_tid = current_tid;
    }
    else if (opcode == OPCODE_MY_TID) {
        LOG_INFO("[SYSCALL] MyTid");

        sf->x0 = tasktable_current_task();
        next_tid = current_tid;
    }
    else if (opcode == OPCODE_MY_PARENT_TID) {
        LOG_INFO("[SYSCALL] MyParentTid");

        sf->x0 = current_task->parent_tid;
        next_tid = current_tid;
    }
    else if (opcode == OPCODE_YIELD) {
        LOG_INFO("[SYSCALL] Yield");

        next_tid = find_next_task();

        set_task_state(current_task, TASKSTATE_READY);
        LOG_DEBUG("yield context switch task_id from = %d to = %d", current_tid, next_tid);
    }
    else if (opcode == OPCODE_EXIT) {
        LOG_INFO("[SYSCALL] Exit");

        // NOTE: maybe don't allow task 1 to be deleted?
        if (current_tid == 1) {
            LOG_WARN("Attempting to delete task 1");
        }

        scheduler_remove(current_tid);
        set_task_state(current_task, TASKSTATE_EXITED);
        tasktable_delete_task(current_tid);

        next_tid = find_next_task();

        LOG_DEBUG("exit context switch task_id from = %d to = %d", current_tid, next_tid);

    } else if (opcode == OPCODE_SEND) {

        LOG_INFO("[SYSCALL] SEND");

        int ret = handle_svc_send(sf->x0, (const char*)sf->x1, sf->x2, (char*)sf->x3, sf->x4);
        if (ret < 0) {
            // if error return to caller immediately (TODO is this a good design idea?)
            sf->x0 = ret;
            next_tid = current_tid;
        } else {
            next_tid = find_next_task();
        }


    } else if (opcode == OPCODE_RECEIVE) {

        LOG_INFO("[SYSCALL] RECEIVE");

        handle_svc_receive((int*)sf->x0, (char*)sf->x1, sf->x2);

        next_tid = find_next_task();

    } else if (opcode == OPCODE_REPLY) {

        LOG_INFO("[SYSCALL] REPLY");

        sf->x0 = handle_svc_reply((int)sf->x0, (const char*)sf->x1, sf->x2);

        next_tid = find_next_task();

    } else if (opcode == OPCODE_AWAITEVENT) {

        LOG_INFO("[SYSCALL] AWAIT EVENT");

        handle_svc_await_event((EventId)sf->x0);
        sf->x0 = 0; // TODO set proper return value

        next_tid = find_next_task();

    } else {
        LOG_WARN("Uncaught syscall with opcode %x", opcode);
        next_tid = current_tid;
    }

    LOG_DEBUG("returning to task %d", next_tid);
    tasktable_set_current_task(next_tid);
    asm_enter_usermode(tasktable_get_task(next_tid)->sf);
}

void
handle_interrupt(void)
{

    uint32_t iar = gic_read_iar();
    uint32_t interrupt_id = iar & 0x3FF;  // Get last 10 bits

    // LOG_DEBUG("[INTERRUPT] ID: %d from task %d", interrupt_id, tasktable_current_task());

    // Timer task
    if (interrupt_id == 97) {
        scheduler_unblock_event(EVENT_CLOCK_TICK);
        timer_set_c1_next_tick();
    }

    gic_write_eoir(iar); // TODO should this be iar or interrupt_id?

    // Find next task to go to
#if 0
    // TODO commenting for now since scheduler allocates memory and doesn't reclaim
    Tid next_tid = find_next_task();
    Task* next_task = tasktable_get_task(next_tid);
    tasktable_set_current_task(next_tid);
    asm_enter_usermode(next_task->sf);
#endif
    asm_enter_usermode(tasktable_get_task(tasktable_current_task())->sf); // TODO we might want to use the scheduler to find next task
}

void
unhandled_vector_table()
{
    PANIC("unhandled vector table entry");
}

