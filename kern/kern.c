#include "kern.h"
#include "addrspace.h"
#include "switchframe.h"
#include "task.h"
#include "log.h"
#include "util.h"
#include "scheduler.h"
#include "alloc.h"
#include "rpi.h"
#include "util.h"
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
    gacha_init();
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

        // RECEIVE_WAIT tasks exists, directly copy message
        current_task->state = TASKSTATE_REPLY_WAIT;

        LOG_DEBUG("Sending message to task %d, in RECEIVE_WAIT", tid);

        // copy message to buffer
        if (target_task->receive_buf == NULL) {
            LOG_ERROR("receiving task doesn't have initalized receive buffer");
            return -1; // TODO better error code
        }

        int copylen = min(msglen, target_task->receive_buf->buf_len); 
        memcpy(target_task->receive_buf->buf, msg, copylen);
        
        // delete receive_buf
        arena_free(target_task->receive_buf);
        target_task->receive_buf = 0;

        // can unblock target task now
        target_task->state = TASKSTATE_READY;

        // set the return value for receive for the receive caller
        target_task->sf->x0 = copylen;
    }
    else if (target_task->state == TASKSTATE_READY) {

        // task is not in RECEIVE_WAIT, add sending task to recieving tasks' recieve queue 
        LOG_DEBUG("Sending message to task %d, not in RECEIVE_WAIT", tid);
        current_task->state = TASKSTATE_SEND_WAIT;
        cbuf_push_front(target_task->receive_queue, (uint8_t)current_tid);

        // also buffer the data we are trying to send
        current_task->send_buf->send_buf = (char*)msg;
        current_task->send_buf->send_buf_len = msglen;

    } else {
        LOG_WARN("Task %d is not available to receive since it is in state %d", tid, target_task->state);
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

        // if no pending messages, go into RECEIVE_WAIT
        current_task->state = TASKSTATE_RECEIVE_WAIT;        
        ReceiveBuf* receive_buf = arena_alloc(sizeof(ReceiveBuf));
        *receive_buf = (ReceiveBuf) {
            .buf = msg,
            .buf_len = msglen
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
            LOG_ERROR("send buf is not initalized: panicking kernel");
            for (;;) {}
        }

        int copylen = min(sender_task->send_buf->send_buf_len, msglen); 
        memcpy(msg, sender_task->send_buf->send_buf, copylen);

        // set sender to REPLY_WAIT
        sender_task->state = TASKSTATE_REPLY_WAIT;

        // set message length for receieve
        current_task->sf->x0 = copylen;
    }
}

Tid
find_next_task(void)
{
    Tid next_tid = scheduler_next();
    if (next_tid == 0) {
        LOG_DEBUG("No more tasks: blocking..");
        for (;;) { }
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
    LOG_DEBUG("[SYSCALL] In task %d", current_tid);

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

        current_task->state = TASKSTATE_READY;
        LOG_DEBUG("yield context switch task_id from = %d to = %d", current_tid, next_tid);
    }
    else if (opcode == OPCODE_EXIT) {
        LOG_INFO("[SYSCALL] Exit");

        // NOTE: maybe don't allow task 1 to be deleted?
        if (current_tid == 1) {
            LOG_WARN("Attempting to delete task 1");
        }

        scheduler_remove(current_tid);
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

    } else {
        LOG_WARN("Uncaught syscall with opcode %x", opcode);
    }

    LOG_DEBUG("returning to task %d", next_tid);
    tasktable_set_current_task(next_tid);
    asm_enter_usermode(tasktable_get_task(next_tid)->sf);
}
