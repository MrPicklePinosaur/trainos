#include "io.h"
#include <trainsys.h>

typedef enum {
    IO_GETC = 1,
    IO_PUTC,
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
    } data;
} IOMsg;

typedef struct {
    IOMsgType type;

    union {
        struct {
            unsigned char ch;
        } getc;
        struct { } putc;
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
        println("[TID %d] WARNING, Getc()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_GETC) {
        println("[TID %d] WARNING, the reply to Getc()'s Send() call is not the right type", MyTid());
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
        println("[TID %d] WARNING, Putc()'s Send() call returned a negative value", MyTid());
        return -1;
    }
    if (resp_buf.type != IO_GETC) {
        println("[TID %d] WARNING, the reply to Putc()'s Send() call is not the right type", MyTid());
        return -2;  // -2 is not in the kernel description for Putc()
    }

    return 0;
}

void
marklinIO(void)
{
    IOMsg msg_buf;
    IOResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(IOMsg));
        if (msg_len < 0) {
            println("[IO SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == IO_GETC) {
            // Getc() implementation
        }
        else if (msg_buf.type == IO_PUTC) {
            // Putc() implementation
        }
        else {
            println("[IO SERVER] Invalid message type");
            continue;
        }
    }
}
