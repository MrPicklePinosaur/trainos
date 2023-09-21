#ifndef __STD_H__
#define __STD_H__

/* Programmer interface */

extern int Create(int priority, void (*function)());
extern int MyTid(void);
extern int MyParentTid(void);
extern void Yield(void);
extern void Exit(void);

#endif // __STD_H__
