#include <traintasks.h>
#include "switch.h"
#include "user/marklin.h"

typedef enum {
    SWITCH_CHANGE,
    SWITCH_QUERY,
    SWITCH_QUERY_ALL,
    SWITCH_WAIT,
    SWITCH_INIT
} SwitchMsgType;

typedef struct {
    SwitchMsgType type;
    union {
        struct {
            usize switch_id;
            SwitchMode mode;
        } change;
        isize query;
        isize wait;
    } data;
} SwitchMsg;

typedef struct {
    SwitchMsgType type;
    union {
        SwitchMode query;
        SwitchMode* query_all;
        struct {
            isize switch_id;
            SwitchMode mode;
        } wait;
    } data;
} SwitchResp;

typedef struct {
    Tid tid;
    isize switch_id;
} SwitchRequest;

const PAIR(u32, SwitchMode) track_a_init_switch_state[SWITCH_COUNT] = {
    { 1, SWITCH_MODE_STRAIGHT },
    { 2, SWITCH_MODE_STRAIGHT },
    { 3, SWITCH_MODE_STRAIGHT },
    { 4, SWITCH_MODE_STRAIGHT },
    { 5, SWITCH_MODE_CURVED },
    { 6, SWITCH_MODE_STRAIGHT },
    { 7, SWITCH_MODE_STRAIGHT },
    { 8, SWITCH_MODE_CURVED },
    { 9, SWITCH_MODE_CURVED },
    { 10, SWITCH_MODE_STRAIGHT },
    { 11, SWITCH_MODE_CURVED },
    { 12, SWITCH_MODE_CURVED },
    { 13, SWITCH_MODE_STRAIGHT },
    { 14, SWITCH_MODE_CURVED },
    { 15, SWITCH_MODE_STRAIGHT },
    { 16, SWITCH_MODE_STRAIGHT },
    { 17, SWITCH_MODE_STRAIGHT },
    { 18, SWITCH_MODE_CURVED },
    { 0x99, SWITCH_MODE_STRAIGHT },
    { 0x9A, SWITCH_MODE_CURVED },
    { 0x9B, SWITCH_MODE_STRAIGHT },
    { 0x9C, SWITCH_MODE_CURVED },
};

int
SwitchChange(Tid switch_server, isize switch_id, SwitchMode mode)
{
    SwitchResp resp_buf;
    SwitchMsg send_buf = (SwitchMsg) {
        .type = SWITCH_CHANGE,
        .data = {
            .change = {
                .switch_id = switch_id,
                .mode = mode
            }
        }
    };
    int ret = Send(switch_server, (const char*)&send_buf, sizeof(SwitchMsg), (char*)&resp_buf, sizeof(SwitchResp));
    if (ret < 0) {
        ULOG_WARN("SwitchChange errored");
        return -1;
    }
    return 0;
}

SwitchMode
SwitchQuery(Tid switch_server, isize switch_id)
{
    SwitchResp resp_buf;
    SwitchMsg send_buf = (SwitchMsg) {
        .type = SWITCH_QUERY,
        .data = {
            .query = switch_id
        }
    };
    int ret = Send(switch_server, (const char*)&send_buf, sizeof(SwitchMsg), (char*)&resp_buf, sizeof(SwitchResp));
    if (ret < 0) {
        ULOG_WARN("SwitchQuery errored");
        return -1;
    }
    return resp_buf.data.query;
}

const SwitchMode*
SwitchQueryAll(Tid switch_server)
{
    SwitchResp resp_buf;
    SwitchMsg send_buf = (SwitchMsg) {
        .type = SWITCH_QUERY_ALL,
    };
    int ret = Send(switch_server, (const char*)&send_buf, sizeof(SwitchMsg), (char*)&resp_buf, sizeof(SwitchResp));
    if (ret < 0) {
        ULOG_WARN("SwitchQueryAll errored");
        return -1;
    }
    return resp_buf.data.query_all;
}

int
SwitchInit(Tid switch_server)
{
    SwitchResp resp_buf;
    SwitchMsg send_buf = (SwitchMsg) {
        .type = SWITCH_INIT,
    };
    int ret = Send(switch_server, (const char*)&send_buf, sizeof(SwitchMsg), (char*)&resp_buf, sizeof(SwitchResp));
    if (ret < 0) {
        ULOG_WARN("SwitchInit errored");
        return -1;
    }
    return 0;
}

WaitForSwitchResult
WaitForSwitch(Tid switch_server, isize switch_id)
{
    SwitchResp resp_buf;
    SwitchMsg send_buf = (SwitchMsg) {
        .type = SWITCH_WAIT,
        .data = {
            .wait = switch_id
        }
    };
    int ret = Send(switch_server, (const char*)&send_buf, sizeof(SwitchMsg), (char*)&resp_buf, sizeof(SwitchResp));
    if (ret < 0) {
        ULOG_WARN("WaitForSwitch errored");
        return (WaitForSwitchResult) {
            .first = -1,
            .second = SWITCH_MODE_UNKNOWN
        };
    }
    return (WaitForSwitchResult) {
        .first = resp_buf.data.wait.switch_id,
        .second = resp_buf.data.wait.mode
    };
}

usize
switch_index(usize switch_id)
{
    if (SWITCH_RANGE_1_LOW <= switch_id && switch_id <= SWITCH_RANGE_1_HIGH) {
        return switch_id-SWITCH_RANGE_1_LOW;
    }
    if (SWITCH_RANGE_2_LOW <= switch_id && switch_id <= SWITCH_RANGE_2_HIGH) {
        return switch_id-SWITCH_RANGE_2_LOW+SWITCH_RANGE_1_HIGH;
    }
    PANIC("Invalid switch id");
}

void
try_unblock(SwitchMode* states, List* switch_requests, isize switch_id)
{
    // Unblock tasks waiting for switches
    // TODO Sometimes the renderer doesn't update switches
    ListIter it = list_iter(switch_requests);
    SwitchRequest* request;
    while (listiter_next(&it, (void**)&request)) {
        if (request->switch_id == switch_id || request->switch_id == -1) {
            ULOG_INFO_M(LOG_MASK_SWITCH, "[SWITCH SERVER] unblocking task %d", request->tid);
            SwitchResp reply_buf = (SwitchResp) {
                .type = SWITCH_WAIT,
                .data = {
                    .wait = {
                        .switch_id = switch_id,
                        .mode = states[switch_index(switch_id)]
                    }
                }
            };
            Reply(request->tid, (char*)&reply_buf, sizeof(SwitchResp));
            list_remove(switch_requests, request);
        }
    }
}

void
switchServerTask()
{
    RegisterAs(SWITCH_ADDRESS);

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);

    List* switch_requests = list_init();

    SwitchMode states[SWITCH_COUNT];
    for (uint32_t i = 0; i < SWITCH_COUNT; i++) {
        states[i] = SWITCH_MODE_UNKNOWN;
    }

    SwitchMsg msg_buf;
    SwitchResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(SwitchMsg));
        if (msg_len < 0) {
            ULOG_WARN("[SWITCH SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == SWITCH_CHANGE) {
            reply_buf = (SwitchResp){};
            Reply(from_tid, (char*)&reply_buf, sizeof(SwitchResp));

            if (msg_buf.data.change.mode == SWITCH_MODE_UNKNOWN) {
                ULOG_WARN("[SWITCH SERVER] Tried to change switch to SWITCH_MODE_UNKNOWN");
                continue;
            }

            // Switch requested switch
            uint32_t switch_id = msg_buf.data.change.switch_id;
            SwitchMode mode = msg_buf.data.change.mode;
            marklin_switch_ctl(marklin_server, switch_id, mode);
            states[switch_index(switch_id)] = mode;
            try_unblock(states, switch_requests, switch_id);

            // Switch special center switches
            if (switch_id == 153 || switch_id == 154) {
                u32 second_switch = switch_id == 153 ? 154 : 153;
                SwitchMode second_mode = mode == SWITCH_MODE_STRAIGHT ? SWITCH_MODE_CURVED : SWITCH_MODE_STRAIGHT;
                marklin_switch_ctl(marklin_server, second_switch, second_mode);
                states[switch_index(second_switch)] = second_mode;
                try_unblock(states, switch_requests, second_switch);
            }
            else if (switch_id == 155 || switch_id == 156) {
                u32 second_switch = switch_id == 155 ? 156 : 155;
                SwitchMode second_mode = mode == SWITCH_MODE_STRAIGHT ? SWITCH_MODE_CURVED : SWITCH_MODE_STRAIGHT;
                marklin_switch_ctl(marklin_server, second_switch, second_mode);
                states[switch_index(second_switch)] = second_mode;
                try_unblock(states, switch_requests, second_switch);
            }
        }
        else if (msg_buf.type == SWITCH_QUERY) {
            reply_buf = (SwitchResp) {
                .type = SWITCH_QUERY,
                .data = {
                    .query = states[switch_index(msg_buf.data.query)]
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(SwitchResp));
        }
        else if (msg_buf.type == SWITCH_QUERY_ALL) {
            reply_buf = (SwitchResp) {
                .type = SWITCH_QUERY_ALL,
                .data = {
                    .query_all = states
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(SwitchResp));
        }
        else if (msg_buf.type == SWITCH_WAIT) {
            SwitchRequest* request = alloc(sizeof(SwitchRequest));
            *request = (SwitchRequest) {
                .tid = from_tid,
                .switch_id = msg_buf.data.wait
            };
            list_push_back(switch_requests, request);
        }
        else if (msg_buf.type == SWITCH_INIT) {

            for (usize i = 0; i < SWITCH_COUNT; ++i) {
                u32 switch_id = track_a_init_switch_state[i].first;
                SwitchMode mode = track_a_init_switch_state[i].second;
                marklin_switch_ctl(marklin_server, switch_id, mode);
                states[switch_index(switch_id)] = mode;
            }

            reply_buf = (SwitchResp) {
                .type = SWITCH_INIT,
                .data = {
                    .query = states
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(SwitchResp));
        }
    }
}
