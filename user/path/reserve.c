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

void
reservationTask()
{
    RegisterAs(RESERVE_ADDRESS);

    Track* track = get_track_a();

    reservations = alloc(track->zone_count*sizeof(usize));
    for (usize i = 0; i < track->zone_count; ++i) {
        reservations[i] = 0;
    }

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
        }
        else if (msg_buf.type == RESERVE_UNRESERVE_ALL) {
            for (usize i = 0; i < track->zone_count; ++i) {
                if (reservations[i] == msg_buf.data.unreserve_all) reservations[i] = 0;
            } 

            reply_buf = (ReserveResp) {
                .type = RESERVE_UNRESERVE_ALL,
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));

        }
        else if (msg_buf.type == RESERVE_IS_RESERVED) {
            usize train = msg_buf.data.is_reserved.train;
            ZoneId zone = msg_buf.data.is_reserved.zone;
            bool is_reserved = reservations[zone] != 0 && reservations[zone] != train;

            reply_buf = (ReserveResp) {
                .type = RESERVE_IS_RESERVED,
                .data = {
                    .is_reserved = is_reserved
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(ReserveResp));
        }

    }

    Exit();
}


usize*
zone_dump()
{
    return reservations;
}

