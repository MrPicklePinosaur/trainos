#include "usertasks.h"
#include <string.h>
#include <trainsys.h>


void
receiverTask()
{
    Exit();
}

void
K2()
{
    Tid receive_tid = Create(1, &receiverTask);
    const char* msg = "hello world";
    const char reply_buf[32];
    int res = Send((Tid)receive_tid, msg, strlen(msg), (char*)reply_buf, 32);
    Exit(); 
}

