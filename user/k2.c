#include "usertasks.h"
#include <string.h>
#include <trainsys.h>

static Tid receive_tid;
static Tid sender_tid;

void
senderTask()
{
    const char* msg = "hello world";
    const char reply_buf[32];
    int res = Send((Tid)receive_tid, msg, strlen(msg), (char*)reply_buf, 32);
    Exit();
}

void
receiverTask()
{
    Tid sender_tid;
    const char receive_buf[32];
    Receive((int*)&sender_tid, (char*)receive_buf, 32);
    Exit();
}

void
K2()
{
    receive_tid = Create(1, &receiverTask);
    sender_tid = Create(1, &senderTask);
    Yield();
    Exit(); 
}

