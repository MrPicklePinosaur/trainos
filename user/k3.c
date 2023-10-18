#include "usertasks.h"
#include "nameserver.h"
#include "clock.h"
#include <trainsys.h>
#include <trainstd.h>

void
awaitEventTest()
{
    for (;;) {
        AwaitEvent(EVENT_CLOCK_TICK);
        println("got clock tick event");
    }
}

typedef struct {
    u32 delay;
    u32 num_delays;
} K3Resp;

typedef struct {

} K3Msg;

void
K3Client()
{
    K3Msg dummy;
    K3Resp resp_buf;
    Send(MyParentTid(), (char*)&dummy, sizeof(K3Msg), (char*)&resp_buf, sizeof(K3Resp));

    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    for (u32 i = 0; i < resp_buf.num_delays; ++i) {
        Delay(clock_server, resp_buf.delay);
        println("Tid: %u, delay interval: %u, completed delays: %u", MyTid(), resp_buf.delay, i+1);
    }

    Exit();
}

void
idleTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    uint32_t start_tick = Time(clock_server);
    uint32_t target_tick = start_tick + 1;
    uint32_t total_ticks = 0;

    for (;;) {
        uint32_t current_tick = DelayUntil(clock_server, target_tick);

        ++total_ticks;
        if (target_tick + 1 > current_tick) {
            ++target_tick;
        }
        else {
            target_tick = current_tick + 1;
        }

        println("Idle task percentage: %u", (total_ticks * 100) / (current_tick - start_tick));
    }
}

void
K3()
{
    Create(3, &K3Client);
    Create(4, &K3Client);
    Create(5, &K3Client);
    Create(6, &K3Client);

    Create(15, &idleTask);

    int from_tid;
    K3Msg dummy;

    u32 delay[4] = {10, 23, 33, 71};
    u32 num_delays[4] = {20, 9, 6, 3};

    for (u32 i = 0; i < 4; i++) {
        Receive(&from_tid, (char*)&dummy, sizeof(K3Msg));

        K3Resp resp = (K3Resp) {
            .delay = delay[i],
            .num_delays = num_delays[i]
        };
        Reply(from_tid, (char*)&resp, sizeof(K3Resp));
    }

    Exit();
}
