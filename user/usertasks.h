#ifndef __USERTASKS_H__
#define __USERTASKS_H__

// Ths first user task to be run
void initTask();
void idleTask();

// K1
void firstUserTask();
void secondUserTask();

// K2
void K2();
void sendReceiveReplyTestTask();

void RPSTask(void);

void K2Perf();

// K3
void K3();

// K4
#include "ui/ui.h"

// Other
void graphicsTask();
void testHarness();

#endif // __USERTASKS_H__
