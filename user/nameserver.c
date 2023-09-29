#include <trainsys.h>
#include <trainstd.h>
#include <stdbool.h>
#include "usertasks.h"

static Tid nameserverTid = 0;

typedef enum {
    NS_REGISTER_AS, 
    NS_WHO_IS 
} NameserverMsgType;

typedef struct {
    NameserverMsgType type;

    union {
        struct {
            char* name;
        } register_as;

        struct {
            char* name;
        } who_is;
    } data;
} NameserverMsg;

typedef struct {
    NameserverMsgType type;

    union {
        struct { } register_as;

        struct {
            Tid tid;
        } who_is;
    } data;
} NameserverResp;

void nameserverTask();

void
initNameserverTask()
{
    if (nameserverTid != 0) {
        println("Warning: nameserverTask has already been created before");
    }

    int ret = Create(1, nameserverTask);

    if (ret < 0) {
        println("failed to initalized nameserver");
    }

    // set globally acessible nameserverTid
    nameserverTid = ret;
}

void
nameserverTask()
{

    NameserverMsg msg_buf;
    NameserverResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(NameserverMsg));

        if (msg_buf.type == NS_REGISTER_AS) {
            println("Got register as request from %d", from_tid);

            reply_buf = (NameserverResp) {
                .type = NS_REGISTER_AS,
                .data = {
                    .register_as = {}
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(NameserverResp));
        }
        else if (msg_buf.type == NS_WHO_IS) {
            println("Got whois request from %d", from_tid);

            reply_buf = (NameserverResp) {
                .type = NS_WHO_IS,
                .data = {
                    .who_is = {
                        .tid = 69
                    }
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(NameserverResp));
        }
    }
}

int
RegisterAs(const char *name)
{
    NameserverResp resp_buf;
    NameserverMsg send_buf = (NameserverMsg) {
        .type = NS_REGISTER_AS,
        .data = {
            .register_as = {
                .name = (char*)name
            }
        }
    };

    int ret = Send(nameserverTid, (const char*)&send_buf, sizeof(NameserverMsg), (char*)&resp_buf, sizeof(NameserverResp));
    if (ret < 0) return -1;

    if (resp_buf.type != NS_REGISTER_AS) return -1;

    return 0;
}

int 
WhoIs(const char *name)
{
    NameserverResp resp_buf;
    NameserverMsg send_buf = (NameserverMsg) {
        .type = NS_WHO_IS,
        .data = {
            .who_is = {
                .name = (char*)name
            }
        }
    };

    int ret = Send(nameserverTid, (const char*)&send_buf, sizeof(NameserverMsg), (char*)&resp_buf, sizeof(NameserverResp));
    if (ret < 0) return -1;

    if (resp_buf.type != NS_WHO_IS) return -1;

    println("who is result is %d", resp_buf.data.who_is.tid); 

    return resp_buf.data.who_is.tid;
}
