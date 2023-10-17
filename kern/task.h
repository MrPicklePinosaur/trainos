#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <trainstd.h>
#include <trainsys.h>
#include "addrspace.h"
#include "switchframe.h"

#define TASK_TABLE_SIZE 128

typedef u32 Tid;

typedef enum {
    TASKSTATE_ACTIVE,
    TASKSTATE_READY,
    TASKSTATE_EXITED,
    TASKSTATE_SEND_WAIT,
    TASKSTATE_RECEIVE_WAIT,
    TASKSTATE_REPLY_WAIT,
    TASKSTATE_AWAIT_EVENT_WAIT,
} TaskState;

// Store data for message senders
typedef struct {
    // used by receive to copy data to sender
    char* reply_buf;
    size_t reply_buf_len;

    // may be used by sender to hold data before receiver calls receive
    char* send_buf;
    size_t send_buf_len;
} SendBuf;

// Store data for message receivers
typedef struct {
    char* buf; // receive buffer
    size_t buf_len;

    int* sender_tid; // the memory location to copy the sender's tid into
} ReceiveBuf;

typedef struct {

    // switchframe is at beginning of struct for easy access
    SwitchFrame* sf;

    Tid tid;
    Tid parent_tid;
    TaskState state;
    u32 priority;
    Addrspace addrspace;

    CBuf* receive_queue;
    SendBuf* send_buf;
    ReceiveBuf* receive_buf;

    EventId blocking_event;
} Task;

// Nodes for a linked list
typedef struct TaskNode TaskNode;
struct TaskNode {
    Task* task;
    TaskNode* next;
};

void tasktable_init(void);
Tid tasktable_create_task(u32 priority, void (*entrypoint)());
Task* tasktable_get_task(Tid tid);

// Update the current active tasks. If a task was active before, it will be made into ready
void tasktable_set_current_task(Tid tid);
Tid tasktable_current_task(void);
void tasktable_delete_task(Tid tid);

#endif // __TASK_H__
