#include <trainsys.h>
#include <trainstd.h>
#include <stdbool.h>
#include <string.h>
#include "usertasks.h"
#include "nameserver.h"

typedef struct NsdbEntry NsdbEntry;

static Tid nameserver_tid;
static List* nsdb;

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

struct NsdbEntry {
    char* name;
    Tid tid;
};

void
nameserverTask()
{
    // println("starting nameserver task");

    NameserverMsg msg_buf;
    NameserverResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(NameserverMsg));
        if (msg_len < 0) {
            ULOG_WARN("Error when receiving");
            continue;
        }

        if (msg_buf.type == NS_REGISTER_AS) {
            // ULOG_DEBUG("Got register as request from %d for %s", from_tid, msg_buf.data.register_as.name);

            // insert namespace into list
            // TODO we don't handle duplicate names (the later one is ignored)
            // TODO this structure is leaked
            NsdbEntry* nsdb_entry = alloc(sizeof(NsdbEntry)); 
            *nsdb_entry = (NsdbEntry) {
                // TODO this might be dangerous, should we copy the string? Since it technically belongs to another task?
                .name = msg_buf.data.register_as.name,
                .tid = from_tid,
            };
            list_push_back(nsdb, nsdb_entry);
            ULOG_INFO_M(LOG_MASK_NS, "Registered %d as '%s'", from_tid, msg_buf.data.register_as.name);

            reply_buf = (NameserverResp) {
                .type = NS_REGISTER_AS,
                .data = {
                    .register_as = {}
                }
            };

            Reply(from_tid, (char*)&reply_buf, sizeof(NameserverResp));
        }
        else if (msg_buf.type == NS_WHO_IS) {
            // println("Got whois request from %d", from_tid);

            // TODO this is sus closure stuff
            ListIter it = list_iter(nsdb);
            Tid lookup_tid = 0; // error state is tid of zero
            NsdbEntry* entry;
            while (listiter_next(&it, (void**)&entry)) {
                // println("comparing %s and %s", entry->name, msg_buf.data.who_is.name);
                if (strcmp(entry->name, msg_buf.data.who_is.name) == 0) {
                    lookup_tid = entry->tid;
                }
            }

            ULOG_INFO_M(LOG_MASK_NS, "whois look up found tid %d for %s", lookup_tid, msg_buf.data.who_is.name);

            reply_buf = (NameserverResp) {
                .type = NS_WHO_IS,
                .data = {
                    .who_is = {
                        .tid = lookup_tid
                    }
                }
            };

            // TODO implement nameserver database lookup

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

    int ret = Send(nameserver_tid, (const char*)&send_buf, sizeof(NameserverMsg), (char*)&resp_buf, sizeof(NameserverResp));
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

    // retry a couple of times
    const u8 WHOIS_RETRY = 5;
    Tid tid = 0;
    for (int i = 0; i < WHOIS_RETRY; ++i) {
        int ret = Send(nameserver_tid, (const char*)&send_buf, sizeof(NameserverMsg), (char*)&resp_buf, sizeof(NameserverResp));
        if (ret < 0) return -1;
        if (resp_buf.type != NS_WHO_IS) return -1;
        if (resp_buf.data.who_is.tid > 0) {
            tid = resp_buf.data.who_is.tid;
            break;
        }
    }

    if (tid == 0) {
        PANIC("Couldn't find tid for name %s", name);
    }

    return tid;
}

void
initNameserverTask()  // Please do not call this more than once
{
    // initalize namespace db
    nsdb = list_init();

    int ret = Create(2, &nameserverTask, "Name Server");

    if (ret < 0) {
        PANIC("failed to initalized nameserver");
    }

    // set globally acessible nameserverTid
    nameserver_tid = ret;
}

