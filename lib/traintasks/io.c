#include <trainstd.h>
#include "io.h"
#include "clock.h"
#include "nameserver.h"
#include "kern/dev/uart.h"

typedef enum {
    IO_GETC = 1,
    IO_PUTC,
    IO_PUTS,
    IO_RX,
    IO_CTS,
} IOMsgType;

typedef struct {
    IOMsgType type;
    union {
        struct {
        } getc;
        struct {
            unsigned char ch;
        } putc;
        struct {
            unsigned char s[PUTS_BLOCK_SIZE];
            usize data_length;
        } puts;
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
        struct { } puts;
        struct { } rx;
        struct { } cts;
    } data;
} IOResp;

int
Getc(Tid io_server)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_GETC,
        .data = {
            .getc = {
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
Putc(Tid io_server, unsigned char ch)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_PUTC,
        .data = {
            .putc = {
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

// Currently has max buffer length of 8
int
Puts(Tid io_server, unsigned char* s, usize data_length)
{
    IOResp resp_buf;
    IOMsg send_buf = (IOMsg) {
        .type = IO_PUTS,
        .data = {
            .puts = {
                .s = {0},
                .data_length = data_length
            }
        }
    };

    memcpy(send_buf.data.puts.s, s, min(data_length, PUTS_BLOCK_SIZE*sizeof(unsigned char)));

    int ret = Send(io_server, (const char*)&send_buf, sizeof(IOMsg), (char*)&resp_buf, sizeof(IOResp));
    if (ret < 0) {
        ULOG_WARN("[TID %d] WARNING, Puts()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_PUTS) {
        ULOG_WARN("[TID %d] WARNING, the reply to Puts()'s Send() call is not the right type", MyTid());
        return -2;  // -2 is not in the kernel description for Putc()
    }

    return 0;
}

int
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

int
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
rxNotifierConsole(void)
{
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);
    for (;;) {
        AwaitEvent(EVENT_CONSOLE_RX);
        SendRX(io_server);
    }
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
ioServer(size_t line)
{
    bool cts = true;

    IOMsg msg_buf;
    IOResp reply_buf;
    int from_tid;

    CBuf* getc_tasks = cbuf_new(64);  // all tasks waiting to get a character
    CBuf* output_fifo = cbuf_new(256);

    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(IOMsg));
        if (msg_len < 0) {
            ULOG_WARN("[LINE %d IO SERVER] Error when receiving", line);
            continue;
        }

        if (msg_buf.type == IO_GETC) {
            // Getc() implementation
            bool is_buffer_empty;
            unsigned char ch = uart_getc_buffered(line, &is_buffer_empty);

            // If there are characters in the buffer, reply to request immediately
            if (!is_buffer_empty) {
                reply_buf = (IOResp) {
                    .type = IO_GETC,
                    .data = {
                        .getc = {
                            .ch = ch
                        }
                    }
                };
                Reply(from_tid, (char*)&reply_buf, sizeof(IOResp));
            }

            // Otherwise, wait until RX interrupt happens
            else {
                cbuf_push_back(getc_tasks, (void*)from_tid);
            }

        }
        else if (msg_buf.type == IO_PUTC) {
            // Putc() implementation

            ULOG_INFO_M(LOG_MASK_IO, "Line %d Putc request from %u '%s'", line, from_tid, TaskName(from_tid));

            if (cts) {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS on, writing %d immediately", line, msg_buf.data.putc.ch);
                uart_putc(line, msg_buf.data.putc.ch);
                cts = false;
            }
            else {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS off, queued %d", line, msg_buf.data.putc.ch);
                cbuf_push_back(output_fifo, (void*)msg_buf.data.putc.ch);
            }

            reply_buf = (IOResp) {
                .type = IO_PUTC,
                .data = {
                    .putc = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(IOResp));
        }
        else if (msg_buf.type == IO_PUTS) {
            // Puts() implementation

            ULOG_INFO_M(LOG_MASK_IO, "Line %d Puts request from %u '%s'", line, from_tid, TaskName(from_tid));

            unsigned char* s = msg_buf.data.puts.s;

            for (usize i = 0; i < min(PUTS_BLOCK_SIZE, msg_buf.data.puts.data_length); ++i) {
                
                if (i == 0 && cts) {
                    ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS on, writing %d immediately", line, s[i]);
                    uart_putc(line, s[i]);
                    cts = false;
                    continue;
                }

                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS off, queued %d", line, s[i]);
                cbuf_push_back(output_fifo, (void*) s[i]);

            }

            reply_buf = (IOResp) {
                .type = IO_PUTS,
                .data = {
                    .puts = {}
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

            // If no tasks are waiting on Getc(), wait until the next Getc()
            if (cbuf_len(getc_tasks) == 0) {
                continue;
            }

            // Respond to all tasks waiting on Getc()
            bool is_buffer_empty;
            unsigned char ch = uart_getc_buffered(line, &is_buffer_empty);
            while (cbuf_len(getc_tasks) > 0) {
                Tid tid = (Tid)cbuf_pop_front(getc_tasks);

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
            reply_buf = (IOResp) {
                .type = IO_CTS,
                .data = {
                    .cts = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(IOResp));

            if (cbuf_len(output_fifo) > 0) {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS received, there is a queued char %d, printing", line, cbuf_front(output_fifo));
                uart_putc(line, (char)cbuf_pop_front(output_fifo));
            }
            else {
                ULOG_INFO_M(LOG_MASK_IO, "Line %d CTS received, there is no queued char", line);
                cts = true;
            }
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
    Create(2, &rxNotifierConsole, "Console IO Server RX Notifier");
    ioServer(CONSOLE);
}

void
marklinIO(void)
{
    RegisterAs(IO_ADDRESS_MARKLIN);
    Create(2, &ctsNotifierMarklin, "Marklin IO Server CTS Notifier");
    Create(2, &rxNotifierMarklin, "Marklin IO Server RX Notifier");
    ioServer(MARKLIN);
}
