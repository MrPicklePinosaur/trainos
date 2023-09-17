#ifndef __TASK_H__
#define __TASK_H__

int Create(int priority, void (*function)());
int MyTid(void);
int MyParentTid(void);
void Yield(void);
void Exit(void);

#endif // __TASK_H__
