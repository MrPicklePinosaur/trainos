#include "usertasks.h"
#include <string.h>
#include <trainsys.h>
#include <trainstd.h>

static Tid receive_tid;
static Tid sender_tid;

void
senderTask()
{
    const char* msg = "hello world";
    const char reply_buf[32];
    int replylen = Send((Tid)receive_tid, msg, strlen(msg)+1, (char*)reply_buf, 32);
    println("got reply: %s, with len %d", (char*)reply_buf, replylen);
    Exit();
}

void
receiverTask()
{
    char receive_buf[32];
    int msglen = Receive((int*)&sender_tid, (char*)receive_buf, 32);
    println("got data: %s, with len %d", (char*)receive_buf, msglen);

    const char* reply_msg = "goodbye world";
    Reply(sender_tid, reply_msg, strlen(reply_msg)+1);

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

