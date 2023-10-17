#ifndef __TRAINSYS_H__
#define __TRAINSYS_H__

/* System call interface */

#include <traindef.h>

typedef u32 Tid;

typedef enum {
    EVENT_NONE = 0,
    EVENT_CLOCK_TICK,
} EventId;

extern int Create(int priority, void (*function)());
extern int MyTid(void);
extern int MyParentTid(void);
extern void Yield(void);
extern void Exit(void);
extern int Send(int tid, const char* msg, int msglen, char* reply, int rplen);
extern int Receive(int* tid, char* msg, int msglen);
extern int Reply(int tid, const char* reply, int rplen);
extern int AwaitEvent(EventId eventid);

#endif // __TRAINSYS_H__
