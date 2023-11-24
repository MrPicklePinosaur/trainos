#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>

#include "reserve.h"

usize* reservations; // zero means no train has the zone reserved

typedef enum {
    RESERVE_RESERVE,
    RESERVE_UNRESERVE,
    RESERVE_UNRESERVE_ALL,
    RESERVE_IS_RESERVED,
    RESERVE_WAIT,
    RESERVE_WAIT_CHANGE, // wait for a change in zone reservation
} ReserveMsgType;

typedef struct {
    ReserveMsgType type;
    union {
        struct {
            usize train;
            ZoneId zone;
        } reserve;

        struct {
            usize train;
            ZoneId zone;
        } unreserve;

        usize unreserve_all; // train id

        struct {
            usize train;
            ZoneId zone;
        } is_reserved;

        struct {
            usize train;
            ZoneId zone;
        } wait;

    } data;
} ReserveMsg;

typedef struct {
    ReserveMsgType type;
    union {
        bool reserve; 
        bool unreserve;
        bool is_reserved;
    } data;
} ReserveResp;

bool
zone_reserve(Tid reserve_server, usize train, ZoneId zone)
{
    ReserveResp resp_buf;
    ReserveMsg send_buf = (ReserveMsg) {
        .type = RESERVE_RESERVE,
        .data = {
            .reserve = {
                .train = train,
                .zone = zone
            }
        }
    };
    int ret = Send(reserve_server, (const char*)&send_buf, sizeof(ReserveMsg), (char*)&resp_buf, sizeof(ReserveResp));
    return resp_buf.data.reserve;
}

bool
zone_unreserve(Tid reserve_server, usize train, ZoneId zone)
{
    ReserveResp resp_buf;
    ReserveMsg send_buf = (ReserveMsg) {
        .type = RESERVE_UNRESERVE,
        .data = {
            .unreserve = {
                .train = train,
                .zone = zone
            }
        }
    };
    int ret = Send(reserve_server, (const char*)&send_buf, sizeof(ReserveMsg), (char*)&resp_buf, sizeof(ReserveResp));
    return resp_buf.data.unreserve;
}

void
zone_unreserve_all(Tid reserve_server, usize train)
{
    ReserveResp resp_buf;
    ReserveMsg send_buf = (ReserveMsg) {
        .type = RESERVE_UNRESERVE_ALL,
        .data = {
            .unreserve_all = train
        }
    };
    Send(reserve_server, (const char*)&send_buf, sizeof(ReserveMsg), (char*)&resp_buf, sizeof(ReserveResp));
}

bool
zone_is_reserved(Tid reserve_server, ZoneId zone, usize train)
{
    ReserveResp resp_buf;
    ReserveMsg send_buf = (ReserveMsg) {
        .type = RESERVE_IS_RESERVED,
        .data = {
            .is_reserved = {
                .train = train,
                .zone = zone
            }
        }
    };
    int ret = Send(reserve_server, (const char*)&send_buf, sizeof(ReserveMsg), (char*)&resp_buf, sizeof(ReserveResp));
    return resp_buf.data.is_reserved;

}

void
zone_wait(Tid reserve_server, usize train, ZoneId zone)
{
    ReserveResp resp_buf;
    ReserveMsg send_buf = (ReserveMsg) {
        .type = RESERVE_WAIT,
        .data = {
            .wait = {
                .train = train,
                .zone = zone
            }
        }
    };
    int ret = Send(reserve_server, (const char*)&send_buf, sizeof(ReserveMsg), (char*)&resp_buf, sizeof(ReserveResp));
}

void
zone_wait_change(Tid reserve_server)
{
    ReserveResp resp_buf;
    ReserveMsg send_buf = (ReserveMsg) {
        .type = RESERVE_WAIT_CHANGE
    };
    int ret = Send(reserve_server, (const char*)&send_buf, sizeof(ReserveMsg), (char*)&resp_buf, sizeof(ReserveResp));
}

// returns if the zone was successfully reserved
bool
_zone_reserve(usize train, ZoneId zone)
{
    if (reservations[zone] == train) return true; // train is allowed to reserve a zone multiple times
    if (reservations[zone] != 0) {
        ULOG_WARN("unable for train %d to reserve zone %d, already reserved by train %d", train, zone, reservations[zone]);
        return false;
    }
    reservations[zone] = train;
    return true;
}

// returns false if zone was not able to be unreserved
bool
_zone_unreserve(usize train, ZoneId zone)
{
    if (reservations[zone] == 0) return true;
    if (reservations[zone] != train) {
        ULOG_WARN("failed to unreserve zone %d: currently reserved by train %d", zone, train);
        return false;
    }
    reservations[zone] = 0;

    return true;
}

// false means that the zone is free to be claimed by train
bool
_zone_is_reserved(ZoneId zone, usize train)
{
    return reservations[zone] != 0 && reservations[zone] != train;
}

typedef struct {
    Tid tid;
    usize train;
    ZoneId zone;
} ZoneRequest;

void
reservationWaitUnblock(List* zone_requests, ZoneId updated_zone)
{
    ListIter it = list_iter(zone_requests);
    ZoneRequest* request;
    while (listiter_next(&it, (void**)&request)) {
        if (request->zone != updated_zone) continue;

        if (!_zone_is_reserved(updated_zone, request->train)) {

            ULOG_INFO_M(LOG_MASK_RESERVE, "[RESERVE SERVER] unblocking task %d", request->tid);
            ReserveResp reply_buf = (ReserveResp) {
                .type = RESERVE_WAIT,
            };
            Reply(request->tid, (char*)&reply_buf, sizeof(ReserveResp));
            list_remove(zone_requests, request);
            return; // return since we only grant one wait request at a time
            // TODO maybe we will unblock all and let everyone race? or if we unblock one then we have first come first serve
        }
    }
}

void
unblockAllWaiting(CBuf* wait_change_requests)
{
    ReserveResp reply_buf = (ReserveResp) {
        .type = RESERVE_WAIT_CHANGE
    };
    while (cbuf_len(wait_change_requests) > 0) {
        Tid from_tid = (Tid)cbuf_pop_front(wait_change_requests);
        Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));
    }
}

void
reservationTask()
{
    RegisterAs(RESERVE_ADDRESS);

    Track* track = get_track();

    reservations = alloc(track->zone_count*sizeof(usize));
    for (usize i = 0; i < track->zone_count; ++i) {
        reservations[i] = 0;
    }

    List* zone_requests = list_init();
    CBuf* wait_change_requests = cbuf_new(16);

    ReserveMsg msg_buf;
    ReserveResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(ReserveMsg));
        if (msg_len < 0) {
            ULOG_WARN("[RESERVE SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == RESERVE_RESERVE) {
            bool ret = _zone_reserve(msg_buf.data.reserve.train, msg_buf.data.reserve.zone);

            reply_buf = (ReserveResp) {
                .type = RESERVE_RESERVE,
                .data = {
                    .reserve = ret
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));

            unblockAllWaiting(wait_change_requests);

        }
        else if (msg_buf.type == RESERVE_UNRESERVE) {
            bool ret = _zone_unreserve(msg_buf.data.reserve.train, msg_buf.data.reserve.zone);

            reply_buf = (ReserveResp) {
                .type = RESERVE_UNRESERVE,
                .data = {
                    .unreserve = ret
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));

            reservationWaitUnblock(zone_requests, msg_buf.data.reserve.zone);

            unblockAllWaiting(wait_change_requests);
        }
        else if (msg_buf.type == RESERVE_UNRESERVE_ALL) {
            for (usize i = 0; i < track->zone_count; ++i) {
                if (reservations[i] == msg_buf.data.unreserve_all) {
                    reservations[i] = 0;
                    reservationWaitUnblock(zone_requests, i);
                }
            } 

            reply_buf = (ReserveResp) {
                .type = RESERVE_UNRESERVE_ALL,
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));

            unblockAllWaiting(wait_change_requests);

        }
        else if (msg_buf.type == RESERVE_IS_RESERVED) {
            usize train = msg_buf.data.is_reserved.train;
            ZoneId zone = msg_buf.data.is_reserved.zone;
            bool is_reserved = _zone_is_reserved(zone, train);

            reply_buf = (ReserveResp) {
                .type = RESERVE_IS_RESERVED,
                .data = {
                    .is_reserved = is_reserved
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));
        }
        else if (msg_buf.type == RESERVE_WAIT) {

            usize train = msg_buf.data.wait.train;
            ZoneId zone = msg_buf.data.wait.zone;

            // return right away if the train is already holding the zone
            if (!_zone_is_reserved(zone, train)) {
                ULOG_INFO_M(LOG_MASK_RESERVE, "[RESERVE SERVER] train %d is alreading holding zone %d, unblocking", train, zone);
                ReserveResp reply_buf = (ReserveResp) {
                    .type = RESERVE_WAIT,
                };
                Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));
                continue;
            }

            ZoneRequest* request = alloc(sizeof(ZoneRequest));
            *request = (ZoneRequest) {
                .tid = from_tid,
                .train = train,
                .zone = zone,
            };
            list_push_back(zone_requests, request);
        }
        else if (msg_buf.type == RESERVE_WAIT_CHANGE) {

            cbuf_push_back(wait_change_requests, (void*)from_tid);

        }

    }

    Exit();
}


usize*
zone_dump()
{
    return reservations;
}

