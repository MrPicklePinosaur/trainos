#include <trainstd.h>
#include "io.h"
#include "clock.h"
#include "nameserver.h"
#include "kern/dev/uart.h"

typedef enum {
    IO_GETC = 1,
    IO_PUTC,
    IO_UPDATE_CTS
} IOMsgType;

typedef struct {
    IOMsgType type;
    union {
        struct {
            int channel;
        } getc;
        struct {
            int channel;
            unsigned char ch;
        } putc;
        struct { } update_cts;
    } data;
} IOMsg;

typedef struct {
    IOMsgType type;

    union {
        struct {
            unsigned char ch;
        } getc;
        struct { } putc;
        struct { } update_cts;
    } data;
} IOResp;

void inFifoTask(void);
void outFifoTask(void);

int
Getc(Tid io_server, int channel)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_GETC,
        .data = {
            .getc = {
                .channel = channel
            }
        }
    };

    int ret = Send(io_server, (const char*)&send_buf, sizeof(IOMsg), (char*)&resp_buf, sizeof(IOResp));
    if (ret < 0) {
        ULOG_WARN("[TID %d] WARNING, Getc()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_GETC) {
        ULOG_WARN("[TID %d] WARNING, the reply to Getc()'s Send() call is not the right type", MyTid());
        return -2;  // -2 is not in the kernel description for Getc()
    }

    return resp_buf.data.getc.ch;
}

int
Putc(Tid io_server, int channel, unsigned char ch)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_PUTC,
        .data = {
            .putc = {
                .channel = channel,
                .ch = ch
            }
        }
    };

    int ret = Send(io_server, (const char*)&send_buf, sizeof(IOMsg), (char*)&resp_buf, sizeof(IOResp));
    if (ret < 0) {
        ULOG_WARN("[TID %d] WARNING, Putc()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_PUTC) {
        ULOG_WARN("[TID %d] WARNING, the reply to Putc()'s Send() call is not the right type", MyTid());
        return -2;  // -2 is not in the kernel description for Putc()
    }

    return 0;
}

void
UpdateCTS(Tid io_server)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_UPDATE_CTS,
        .data = {
            .update_cts = {}
        }
    };

    int ret = Send(io_server, (const char*)&send_buf, sizeof(IOMsg), (char*)&resp_buf, sizeof(IOResp));
    if (ret < 0) {
        ULOG_WARN("[TID %d] WARNING, UpdateCTS()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_UPDATE_CTS) {
        ULOG_WARN("[TID %d] WARNING, the reply to UpdateCTS()'s Send() call is not the right type", MyTid());
        return -2;
    }

    return 0;
}

void
ctsNotifier(void)
{
    Tid io_server = WhoIs(IO_ADDRESS);
    for (;;) {
        AwaitEvent(EVENT_MARKLIN_CTS);
        UpdateCTS(io_server);
    }
}

void
putcTestTask(void)
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid io_server = WhoIs(IO_ADDRESS);
    for (;;) {
        Delay(clock_server, 500);
        Putc(io_server, MARKLIN, 26);
        Delay(clock_server, 500);
        Putc(io_server, MARKLIN, 77);
        Delay(clock_server, 500);
        Putc(io_server, MARKLIN, 0);
        Delay(clock_server, 500);
        Putc(io_server, MARKLIN, 77);
    }
}

List* getc_tasks; // all tasks waiting to get a character

void
marklinIO(void)
{
    bool cts = true;

    getc_tasks = list_init();

    RegisterAs(IO_ADDRESS);

    IOMsg msg_buf;
    IOResp reply_buf;
    int from_tid;

    List* output_fifo = list_init();

    Create(5, &ctsNotifier);  // Not sure about these priorities at the moment
    Create(4, &putcTestTask);
    Create(5, &inFifoTask);
    Yield();

    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(IOMsg));
        if (msg_len < 0) {
            ULOG_WARN("[IO SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == IO_GETC) {
            // Getc() implementation

            ULOG_INFO_M(LOG_MASK_IO, "Getc request from %d", from_tid);

            list_push_back(getc_tasks, (void*)from_tid);

        }
        else if (msg_buf.type == IO_PUTC) {
            // Putc() implementation

            ULOG_INFO_M(LOG_MASK_IO, "Putc request from %d", from_tid);

            if (cts) {
                ULOG_INFO_M(LOG_MASK_IO, "CTS on, sent immediately");
                uart_putc(MARKLIN, msg_buf.data.putc.ch);
                cts = false;
            }
            else {
                ULOG_INFO_M(LOG_MASK_IO, "CTS off, queued");
                list_push_back(output_fifo, (void*)msg_buf.data.putc.ch);
            }

            reply_buf = (IOResp) {
                .type = IO_PUTC,
                .data = {
                    .putc = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(IOResp));
        }
        else if (msg_buf.type == IO_UPDATE_CTS) {
            // UpdateCTS() implementation
            if (list_len(output_fifo) > 0) {
                ULOG_INFO_M(LOG_MASK_IO, "CTS signal received, there is a queued char, printing");
                uart_putc(MARKLIN, list_pop_front(output_fifo));
            }
            else {
                ULOG_INFO_M(LOG_MASK_IO, "CTS signal received, there is no queued char");
                cts = true;
            }
        }
        else {
            ULOG_WARN("[IO SERVER] Invalid message type");
            continue;
        }
    }
}


// task that writes to outbuf if it is avaliable
void
outFifoTask(void)
{
    for (;;) {

    }
}

// task that queries for new input if it is avaliable
void
inFifoTask(void)
{
    IOResp reply_buf;

    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    u32 delay_time = Time(clock_server);
    for (;;) {
        delay_time += 10; // arbritry choose read poll for every 10 ticks
        DelayUntil(clock_server, delay_time);

        unsigned char ch = uart_getc_buffered(CONSOLE);
        if (ch == 0) continue;

        ULOG_DEBUG_M(LOG_MASK_IO, "got buffered character %d", ch);

        // reply to all tasks that are waiting for character
        ListIter it = list_iter(getc_tasks);
        Tid tid;
        while (list_len(getc_tasks) > 0) {
            Tid tid = list_pop_front(getc_tasks);

            reply_buf = (IOResp) {
                .type = IO_GETC,
                .data = {
                    .getc = {
                        .ch = ch
                    }
                }
            };
            Reply(tid, (char*)&reply_buf, sizeof(IOResp));
        }



    }
}
