#include "clock.h"
#include "trainstd.h"
#include "nameserver.h"

#define CLOCK_ADDRESS "Clock"

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
            int tick;
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
            bool valid_delay;
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

void
clockTask()
{
    RegisterAs(CLOCK_ADDRESS);

    Create(0, &notifierTask);
    Yield();
    
    timer_init_c1();

    u32 ticks = 0;

    ClockMsg msg_buf;
    ClockResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(ClockMsg));
        if (msg_len < 0) {
            println("[CLOCK SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == CLOCK_TIME) {
            // Time() implementation
        }
        else if (msg_buf.type == CLOCK_DELAY) {
            // Delay() implementation
        }
        else if (msg_buf.type == CLOCK_DELAY_UNTIL) {
            // DelayUntil() implementation
        }
        else if (msg_buf.type == CLOCK_TICK) {
            println("[CLOCK SERVER] tick");
            ++ticks;

            reply_buf = (ClockResp) {
                .type = CLOCK_TICK,
                .data = {
                    .tick = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ClockResp));
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
    if (resp_buf.type != CLOCK_TIME) {
        println("[TID %d] WARNING, the reply to Delay()'s Send() call is not the right type", MyTid());
        return -1;
    }

    return resp_buf.data.time.ticks;
}

int DelayUntil(Tid clock_server, int ticks) {
    ClockResp resp_buf;
    ClockMsg send_buf = (ClockMsg) {
        .type = CLOCK_DELAY_UNTIL,
        .data = {
            .delay = {
                .ticks = ticks
            }
        }
    };

    int ret = Send(clock_server, (const char*)&send_buf, sizeof(ClockMsg), (char*)&resp_buf, sizeof(ClockResp));
    if (ret < 0) {
        println("[TID %d] WARNING, DelayUntil()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != CLOCK_TIME) {
        println("[TID %d] WARNING, the reply to DelayUntil()'s Send() call is not the right type", MyTid());
        return -1;
    }
    if (!resp_buf.data.delay_until.valid_delay) {
        println("[TID %d] WARNING, tried to pass a past time to DelayUntil()", MyTid());
        return -2;
    }

    return resp_buf.data.time.ticks;
}

void Tick(Tid clock_server) {
    ClockResp resp_buf;
    ClockMsg send_buf = {
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
