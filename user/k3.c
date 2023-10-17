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
    uint32_t delay;
    uint32_t num_delays;
} K3Resp;

void
K3Client()
{
    K3Resp resp_buf;
    Send(MyParentTid(), "a", sizeof(char), (char*)&resp_buf, sizeof(K3Resp));

    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    for (uint32_t i = 0; i < resp_buf.num_delays; ++i) {
        Delay(clock_server, resp_buf.delay);
        println("Tid: %u, delay interval: %u, completed delays: %u", MyTid(), resp_buf.delay, i+1);
    }

    Exit();
}

void
K3()
{
    Create(3, &K3Client);
    Create(4, &K3Client);
    Create(5, &K3Client);
    Create(6, &K3Client);

    int from_tid;
    char* msg = alloc(sizeof(char));

    uint32_t delay[4] = {10, 23, 33, 71};
    uint32_t num_delays[4] = {20, 9, 6, 3};

    for (uint32_t i = 0; i < 4; i++) {
        K3Resp resp = (K3Resp) {
            .delay = delay[i],
            .num_delays = num_delays[i]
        };
        Receive(&from_tid, msg, sizeof(char));
        Reply(from_tid, (char*)&resp, sizeof(K3Resp));
    }

    Exit();
}
