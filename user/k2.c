#include "usertasks.h"
#include "nameserver.h"
#include <string.h>
#include <trainsys.h>
#include <trainstd.h>

static Tid receive_tid;
static Tid sender_tid;

#define PERF_TEST_REPITITIONS 20

void
senderTask()
{
    println("Entered senderTask");

    const char* msg = "hello world";
    const char reply_buf[32];
    int replylen = Send((Tid)receive_tid, msg, strlen(msg)+1, (char*)reply_buf, 32);
    println("got reply: %s, with len %d", (char*)reply_buf, replylen);
    Exit();
}

void
receiverTask()
{
    println("Entered receiverTask");

    int from_tid;
    char receive_buf[32];
    int msglen = Receive((int*)&from_tid, (char*)receive_buf, 32);
    println("got message from %d: from  %s, with len %d", from_tid, (char*)receive_buf, msglen);

    const char* reply_msg = "goodbye world";
    Reply(sender_tid, reply_msg, strlen(reply_msg)+1);

    Exit();
}

void
sendReceiveReplyTestTask()
{
    receive_tid = Create(1, &receiverTask);
    sender_tid = Create(1, &senderTask);
    Yield();
    Exit(); 
}

void
K2()
{
    println("In k2 task");
    RegisterAs("firstTask");
    WhoIs("firstTask");

    for (;;) {}
    Exit();
}

void
send_perf_test(uint32_t msglen)
{
    char* msg = alloc(sizeof(char)*msglen);
    char* reply_buf = alloc(sizeof(char)*msglen);

    Timer* timer = timer_new();

    for (uint32_t i = 0; i < PERF_TEST_REPITITIONS; i++) {
        timer_start(timer);
        Send((Tid)receive_tid, msg, msglen, reply_buf, msglen);
        timer_end(timer);
    }
    println("Samples: %u, Mean: %u, Variance: %u, Worst: %d", timer->samples, timer_get_mean(timer), timer_get_variance(timer), timer->worst);
    Exit();
}

void
receive_perf_test(uint32_t msglen)
{
    int from_tid;
    char* receive_buf = alloc(sizeof(char)*msglen);
    char* reply_msg = alloc(sizeof(char)*msglen);

    for (uint32_t i = 0; i < PERF_TEST_REPITITIONS; i++) {
        Receive((int*)&from_tid, receive_buf, msglen);
        Reply(sender_tid, reply_msg, msglen);
    }

    Exit();
}

void send4() { send_perf_test(4); }
void send64() { send_perf_test(64); }
void send256() { send_perf_test(256); }

void receive4() { receive_perf_test(4); }
void receive64() { receive_perf_test(64); }
void receive256() { receive_perf_test(256); }

void
K2Perf()
{
    println("Send-first, 4 bytes");
    sender_tid = Create(1, &send4);
    receive_tid = Create(2, &receive4);
    Yield();

    println("Send-first, 64 bytes");
    sender_tid = Create(1, &send64);
    receive_tid = Create(2, &receive64);
    Yield();

    println("Send-first, 256 bytes");
    sender_tid = Create(1, &send256);
    receive_tid = Create(2, &receive256);
    Yield();

    println("Receive-first, 4 bytes");
    sender_tid = Create(2, &send4);
    receive_tid = Create(1, &receive4);
    Yield();

    println("Receive-first, 64 bytes");
    sender_tid = Create(2, &send64);
    receive_tid = Create(1, &receive64);
    Yield();

    println("Receive-first, 256 bytes");
    sender_tid = Create(2, &send256);
    receive_tid = Create(1, &receive256);
    Yield();

    Exit();
}
