#include "usertasks.h"
#include <string.h>
#include <trainsys.h>
#include <trainstd.h>
#include <traintasks.h>

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
    receive_tid = Create(1, &receiverTask, "K2 Receive Tester");
    sender_tid = Create(1, &senderTask, "K2 Send Tester");
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
send_perf_test(u32 msglen)
{
    char* msg = alloc(sizeof(char)*msglen);
    char* reply_buf = alloc(sizeof(char)*msglen);

    Timer* timer = timer_new();

    for (u32 i = 0; i < PERF_TEST_REPITITIONS; i++) {
        timer_start(timer);
        Send((Tid)receive_tid, msg, msglen, reply_buf, msglen);
        timer_end(timer);
    }
    println("Samples: %u, Mean: %u, Variance: %u, Worst: %d", timer->samples, timer_get_mean(timer), timer_get_variance(timer), timer->worst);
    Exit();
}

void
receive_perf_test(u32 msglen)
{
    int from_tid;
    char* receive_buf = alloc(sizeof(char)*msglen);
    char* reply_msg = alloc(sizeof(char)*msglen);

    for (u32 i = 0; i < PERF_TEST_REPITITIONS; i++) {
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
    sender_tid = Create(1, &send4, "K2 Send-First Sender (4 Bytes)");
    receive_tid = Create(2, &receive4, "K2 Send-First Receiver (4 Bytes)");
    Yield();

    println("Send-first, 64 bytes");
    sender_tid = Create(1, &send64, "K2 Send-First Sender (64 Bytes)");
    receive_tid = Create(2, &receive64, "K2 Send-First Receiver (64 Bytes)");
    Yield();

    println("Send-first, 256 bytes");
    sender_tid = Create(1, &send256, "K2 Send-First Sender (256 Bytes)");
    receive_tid = Create(2, &receive256, "K2 Send-First Receiver (256 Bytes)");
    Yield();

    println("Receive-first, 4 bytes");
    sender_tid = Create(2, &send4, "K2 Receive-First Sender (4 Bytes)");
    receive_tid = Create(1, &receive4, "K2 Receive-First Receiver (4 Bytes)");
    Yield();

    println("Receive-first, 64 bytes");
    sender_tid = Create(2, &send64, "K2 Receive-First Sender (64 Bytes)");
    receive_tid = Create(1, &receive64, "K2 Receive-First Receiver (64 Bytes)");
    Yield();

    println("Receive-first, 256 bytes");
    sender_tid = Create(2, &send256, "K2 Receive-First Sender (256 Bytes)");
    receive_tid = Create(1, &receive256, "K2 Receive-First Receiver (256 Bytes)");
    Yield();

    Exit();
}
