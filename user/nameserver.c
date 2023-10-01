#include <trainsys.h>
#include <trainstd.h>
#include <stdbool.h>
#include <string.h>
#include "usertasks.h"

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
    println("starting nameserver task");

    NameserverMsg msg_buf;
    NameserverResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(NameserverMsg));
        if (msg_len < 0) {
            println("Error when receiving");
            continue;
        }

        if (msg_buf.type == NS_REGISTER_AS) {
            println("Got register as request from %d", from_tid);

            // insert namespace into list
            // TODO we don't handle duplicate names (the later one is ignored)
            // TODO this structure is leaked
            NsdbEntry* nsdb_entry = alloc(sizeof(nsdb_entry)); 
            *nsdb_entry = (NsdbEntry) {
                // TODO this might be dangerous, should we copy the string? Since it technically belongs to another task?
                .name = msg_buf.data.register_as.name,
                .tid = from_tid,
            };
            list_push_back(nsdb, nsdb_entry);
            println("Registered %d as '%s'", from_tid, msg_buf.data.register_as.name);

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

            // TODO this is sus closure stuff
            ListIter* it = list_iter(nsdb);
            Tid lookup_tid = 0; // error state is tid of zero
            for (;;) {
                NsdbEntry* entry = (NsdbEntry*)listiter_next(it);
                if (entry == 0) break;
                println("comparing %s and %s", entry->name, msg_buf.data.who_is.name);
                if (strcmp(entry->name, msg_buf.data.who_is.name) == 0) {
                    lookup_tid = from_tid;
                }
            }

            println("whois look up found %d", lookup_tid);

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

    int ret = Send(nameserver_tid, (const char*)&send_buf, sizeof(NameserverMsg), (char*)&resp_buf, sizeof(NameserverResp));
    if (ret < 0) return -1;

    if (resp_buf.type != NS_WHO_IS) return -1;

    println("who is result is %d", resp_buf.data.who_is.tid); 

    return resp_buf.data.who_is.tid;
}

void
initNameserverTask()
{
    if (nameserver_tid != 0) {
        println("Warning: nameserverTask has already been created before tid = %d", nameserver_tid);
    }

    // initalize namespace db
    nsdb = list_init();

    int ret = Create(2, &nameserverTask);

    if (ret < 0) {
        println("failed to initalized nameserver");
    }

    // set globally acessible nameserverTid
    nameserver_tid = ret;
}

