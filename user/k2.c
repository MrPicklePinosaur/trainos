#include "usertasks.h"
#include <string.h>
#include <trainsys.h>
#include <trainstd.h>

static Tid receive_tid;
static Tid sender_tid;

void
senderTask()
{
    const char* msg = "hello world"; // bug where null byte is not being copied
    const char reply_buf[32];
    int res = Send((Tid)receive_tid, msg, strlen(msg), (char*)reply_buf, 32);
    Exit();
}

void
receiverTask()
{
    Tid sender_tid;
    char receive_buf[32];
    int msglen = Receive((int*)&sender_tid, (char*)receive_buf, 32);
    receive_buf[msglen] = 0; // hack
    println("got data: %s, with len %d", (char*)receive_buf, msglen);
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

