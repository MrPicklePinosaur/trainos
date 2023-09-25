#ifndef __TRAINSYS_H__
#define __TRAINSYS_H__

/* System call interface */

extern int Create(int priority, void (*function)());
extern int MyTid(void);
extern int MyParentTid(void);
extern void Yield(void);
extern void Exit(void);


#endif // __TRAINSYS_H__
