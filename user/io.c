#include <trainstd.h>
#include "io.h"
#include "clock.h"
#include "nameserver.h"
#include "kern/dev/uart.h"

typedef enum {
    IO_GETC = 1,
    IO_PUTC,
    IO_RX,
    IO_CTS,
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
        struct { } rx;
        struct { } cts;
    } data;
} IOMsg;

typedef struct {
    IOMsgType type;

    union {
        struct {
            unsigned char ch;
        } getc;
        struct { } putc;
        struct { } rx;
        struct { } cts;
    } data;
} IOResp;

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
SendRX(Tid io_server)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_RX,
        .data = {
            .rx = {}
        }
    };

    int ret = Send(io_server, (const char*)&send_buf, sizeof(IOMsg), (char*)&resp_buf, sizeof(IOResp));
    if (ret < 0) {
        ULOG_WARN("[TID %d] WARNING, SendRX()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_RX) {
        ULOG_WARN("[TID %d] WARNING, the reply to SendRX()'s Send() call is not the right type", MyTid());
        return -2;
    }

    return 0;
}

void
SendCTS(Tid io_server)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_CTS,
        .data = {
            .cts = {}
        }
    };

    int ret = Send(io_server, (const char*)&send_buf, sizeof(IOMsg), (char*)&resp_buf, sizeof(IOResp));
    if (ret < 0) {
        ULOG_WARN("[TID %d] WARNING, SendCTS()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_CTS) {
        ULOG_WARN("[TID %d] WARNING, the reply to SendCTS()'s Send() call is not the right type", MyTid());
        return -2;
    }

    return 0;
}

void
rxNotifierMarklin(void)
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    for (;;) {
        AwaitEvent(EVENT_MARKLIN_RX);
        SendRX(io_server);
    }
}

void
ctsNotifierMarklin(void)
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    for (;;) {
        AwaitEvent(EVENT_MARKLIN_CTS);
        SendCTS(io_server);
    }
}

void
putcTestTask(void)
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Putc(io_server, MARKLIN, 26);
    Putc(io_server, MARKLIN, 77);
    for (;;) {
        Putc(io_server, MARKLIN, 133);
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        println("DATA GOTTEN: %d", Getc(io_server, MARKLIN));
        Delay(clock_server, 1000);
    }
}

void
ioServer(size_t line)
{
    bool cts = true;

    IOMsg msg_buf;
    IOResp reply_buf;
    int from_tid;

    List* getc_tasks = list_init();  // all tasks waiting to get a character
    List* output_fifo = list_init();

    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(IOMsg));
        if (msg_len < 0) {
            ULOG_WARN("[LINE %d IO SERVER] Error when receiving", line);
            continue;
        }

        if (msg_buf.type == IO_GETC) {
            // Getc() implementation

            ULOG_INFO_M(LOG_MASK_IO, "Line %d Getc request from %d", line, from_tid);

            list_push_back(getc_tasks, (void*)from_tid);

        }
        else if (msg_buf.type == IO_PUTC) {
            // Putc() implementation

            ULOG_INFO_M(LOG_MASK_IO, "Line %d Putc request from %d", line, from_tid);

            if (cts) {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS on, sent immediately", line);
                uart_putc(line, msg_buf.data.putc.ch);
                cts = false;
            }
            else {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS off, queued", line);
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
        else if (msg_buf.type == IO_RX) {

            // Respond to notifier that you're done
            reply_buf = (IOResp) {
                .type = IO_RX,
                .data = {
                    .rx = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(IOResp));

            unsigned char ch = uart_getc_buffered(line);
            if (ch == 0) {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d RX interrupt received but no data in receive buffer", line);
                continue;
            }

            // Respond to all tasks waiting on Getc()
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
        else if (msg_buf.type == IO_CTS) {
            // SendCTS() implementation
            if (list_len(output_fifo) > 0) {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS signal received, there is a queued char, printing", line);
                uart_putc(line, list_pop_front(output_fifo));
            }
            else {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS signal received, there is no queued char", line);
                cts = true;
            }

            reply_buf = (IOResp) {
                .type = IO_CTS,
                .data = {
                    .cts = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(IOResp));
        }
        else {
            ULOG_WARN("[LINE %d IO SERVER] Invalid message type", line);
            continue;
        }
    }
}

void
consoleIO(void)
{
    RegisterAs(IO_ADDRESS_CONSOLE);
    ioServer(CONSOLE);
}

void
marklinIO(void)
{
    RegisterAs(IO_ADDRESS_MARKLIN);
    Create(5, &ctsNotifierMarklin, "Marklin IO Server CTS Notifier");
    Create(5, &rxNotifierMarklin, "Marklin IO Server RX Notifier");
    Create(4, &putcTestTask, "Marklin IO Server Putc Test");
    ioServer(MARKLIN);
}
