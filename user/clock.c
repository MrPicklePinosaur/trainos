#include "clock.h"
#include "trainstd.h"
#include "nameserver.h"

#include "kern/dev/timer.h" // not very nice to straight up include kernel code like this

typedef enum {
    CLOCK_TIME = 1,
    CLOCK_DELAY,
    CLOCK_DELAY_UNTIL,
    CLOCK_TICK,
} ClockMsgType;

typedef struct {
    ClockMsgType type;

    union {
        struct { } time;
        struct {
            int ticks;
        } delay;
        struct {
            int ticks;
        } delay_until;
        struct {} tick;
    } data;
} ClockMsg;

typedef struct {
    ClockMsgType type;

    union {
        struct {
            int ticks;
        } time;
        struct {
            int ticks;
        } delay;
        struct {
            int ticks;
        } delay_until;
        struct {} tick;
    } data;
} ClockResp;

void Tick(Tid clock_server);

void notifierTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    for (;;) {
        AwaitEvent(EVENT_CLOCK_TICK); 
        Tick(clock_server);
    }
}

typedef struct {
    Tid tid;
    int target_delay;
    ClockMsgType type;
} ClockRequest;

void
clockTask()
{
    RegisterAs(CLOCK_ADDRESS);

    timer_init_c1();

    List* clock_requests = list_init(); // TODO could make this sorted list
    u32 ticks = 0;

    ClockMsg msg_buf;
    ClockResp reply_buf;
    int from_tid;

    // TODO need to ensure clock is registered with nameserver
    Create(1, &notifierTask);
    Yield();

    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(ClockMsg));
        if (msg_len < 0) {
            println("[CLOCK SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == CLOCK_TIME) {

            // println("[CLOCK SERVER] TIME request from %d", from_tid);

            // Time() implementation

            reply_buf = (ClockResp) {
                .type = CLOCK_TIME,
                .data = {
                    .time = {
                        .ticks = ticks
                    }
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ClockResp));

        }
        else if (msg_buf.type == CLOCK_DELAY) {

            // println("[CLOCK SERVER] DELAY request from %d, trigger at %d", from_tid, msg_buf.data.delay.ticks + ticks);

            // Delay() implementation
            ClockRequest* request = alloc(sizeof(ClockRequest));
            *request = (ClockRequest) {
                .tid = from_tid,
                .target_delay = msg_buf.data.delay.ticks + ticks,
                .type = CLOCK_DELAY
            };
            list_push_back(clock_requests, request);

        }
        else if (msg_buf.type == CLOCK_DELAY_UNTIL) {

            // println("[CLOCK SERVER] DELAY_UNTIL request from %d, trigger at %d", from_tid, msg_buf.data.delay_until.ticks);

            // DelayUntil() implementation

            ClockRequest* request = alloc(sizeof(ClockRequest));
            *request = (ClockRequest) {
                .tid = from_tid,
                .target_delay = msg_buf.data.delay_until.ticks,
                .type = CLOCK_DELAY_UNTIL
            };
            list_push_back(clock_requests, request);

        }
        else if (msg_buf.type == CLOCK_TICK) {

            ++ticks;

            // println("[CLOCK SERVER] tick %d", ticks);

            reply_buf = (ClockResp) {
                .type = CLOCK_TICK,
                .data = {
                    .tick = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ClockResp));

            // check if any delays requests should be replied to
            ListIter it = list_iter(clock_requests); 
            ClockRequest* clock_request;
            while (listiter_next(&it, (void**)&clock_request)) {

                if (clock_request->target_delay <= ticks) {

                    if (clock_request->type == CLOCK_DELAY) {
                        reply_buf = (ClockResp) {
                            .type = CLOCK_DELAY,
                            .data = {
                                .delay = {
                                    .ticks = ticks
                                }
                            }
                        };
                    } else if (clock_request->type == CLOCK_DELAY_UNTIL) {
                        reply_buf = (ClockResp) {
                            .type = CLOCK_DELAY_UNTIL,
                            .data = {
                                .delay_until = {
                                    .ticks = ticks,
                                }
                            }
                        };
                    }
                    // println("delay request done for %d", clock_request->tid);
                    Reply(clock_request->tid, (char*)&reply_buf, sizeof(ClockResp));

                    list_remove(clock_requests, clock_request);
                }
            }
        } else {
            println("[CLOCK SERVER] Invalid message type");
            continue;
        }
    }

}

int Time(Tid clock_server) {
    ClockResp resp_buf;
    ClockMsg send_buf = (ClockMsg) {
        .type = CLOCK_TIME,
        .data = {
            .time = {}
        }
    };

    int ret = Send(clock_server, (const char*)&send_buf, sizeof(ClockMsg), (char*)&resp_buf, sizeof(ClockResp));
    if (ret < 0) {
        println("[TID %d] WARNING, Time()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != CLOCK_TIME) {
        println("[TID %d] WARNING, the reply to Time()'s Send() call is not the right type", MyTid());
        return -2;  // -2 is not in the kernel description for Time()
    }

    return resp_buf.data.time.ticks;
}

int Delay(Tid clock_server, int ticks) {
    if (ticks < 0) {
        println("[TID %d] WARNING, tried to pass negative ticks to Delay()", MyTid());
        return -2;
    }

    ClockResp resp_buf;
    ClockMsg send_buf = (ClockMsg) {
        .type = CLOCK_DELAY,
        .data = {
            .delay = {
                .ticks = ticks
            }
        }
    };

    int ret = Send(clock_server, (const char*)&send_buf, sizeof(ClockMsg), (char*)&resp_buf, sizeof(ClockResp));
    if (ret < 0) {
        println("[TID %d] WARNING, Delay()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != CLOCK_DELAY) {
        println("[TID %d] WARNING, the reply to Delay()'s Send() call is not the right type", MyTid());
        return -1;
    }

    return resp_buf.data.delay.ticks;
}

int DelayUntil(Tid clock_server, int ticks) {
    ClockResp resp_buf;
    ClockMsg send_buf = (ClockMsg) {
        .type = CLOCK_DELAY_UNTIL,
        .data = {
            .delay_until = {
                .ticks = ticks
            }
        }
    };

    int ret = Send(clock_server, (const char*)&send_buf, sizeof(ClockMsg), (char*)&resp_buf, sizeof(ClockResp));
    if (ret < 0) {
        println("[TID %d] WARNING, DelayUntil()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != CLOCK_DELAY_UNTIL) {
        println("[TID %d] WARNING, the reply to DelayUntil()'s Send() call is not the right type", MyTid());
        return -1;
    }

    return resp_buf.data.delay_until.ticks;
}

void Tick(Tid clock_server) {
    ClockResp resp_buf;
    ClockMsg send_buf = (ClockMsg) {
        .type = CLOCK_TICK,
        .data = {
            .tick = {}
        }
    };

    int ret = Send(clock_server, (const char*)&send_buf, sizeof(ClockMsg), (char*)&resp_buf, sizeof(ClockResp));
    if (ret < 0) {
        println("Something went wrong when calling Tick()");
    }
}
