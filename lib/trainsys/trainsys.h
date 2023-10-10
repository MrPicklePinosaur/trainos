#ifndef __TRAINSYS_H__
#define __TRAINSYS_H__

/* System call interface */

#include <stdint.h>

typedef uint32_t Tid;

extern int Create(int priority, void (*function)());
extern int MyTid(void);
extern int MyParentTid(void);
extern void Yield(void);
extern void Exit(void);
extern int Send(int tid, const char* msg, int msglen, char* reply, int rplen);
extern int Receive(int* tid, char* msg, int msglen);
extern int Reply(int tid, const char* reply, int rplen);
extern int AwaitEvent(int eventid);

#endif // __TRAINSYS_H__
