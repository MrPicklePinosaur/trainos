#ifndef __USERTASKS_H__
#define __USERTASKS_H__

// Ths first user task to be run
void initTask();

// K1
void firstUserTask();
void secondUserTask();

// K2
void K2();
void sendReceiveReplyTestTask();

// Nameserver
void initNameserverTask();

int RegisterAs(const char *name);
int WhoIs(const char *name);

// Other
void graphicsTask();
void testHarness();

#endif // __USERTASKS_H__
