#ifndef __STD_H__
#define __STD_H__

/* Programmer interface */

int Create(int priority, void (*function)());
extern int MyTid(void);
int MyParentTid(void);
void Yield(void);
void Exit(void);

#endif // __STD_H__
