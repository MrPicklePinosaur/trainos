#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "user/sensor.h"
#include "user/switch.h"
#include "user/marklin.h"
#include "user/path/reserve.h"
#include "user/path/dijkstra.h"
#include "user/path/track_data.h"
#include "user/path/train_data.h"
#include "user/path/dijkstra.h"
#include "trainstate.h"

#define TRAIN_SPEED_MASK     0b001111
#define TRAIN_LIGHTS_MASK    0b010000

typedef enum {
    TRAINSTATE_GET_STATE,
    TRAINSTATE_SET_SPEED,
    TRAINSTATE_SET_LIGHTS,
    TRAINSTATE_SET_OFFSET,
    TRAINSTATE_SET_DEST,  // Does not actually send the train to the destination. Simply a value used by the train state window
    TRAINSTATE_SET_POS,
    TRAINSTATE_REVERSE_STATIC, // special case of reverse, reverse from rest and right away
    TRAINSTATE_REVERSE,
    TRAINSTATE_REVERSE_REVERSE,
    TRAINSTATE_REVERSE_RESTART,
    TRAINSTATE_POSITION_WAIT,   // used to wait for position changes
    TRAINSTATE_POSITION_UPDATE, // used by notifier to update current position
} TrainstateMsgType;

typedef struct {
    TrainstateMsgType type;
    union {
        struct {
            usize train;
        } get;
        struct {
            usize train;
            usize speed;
        } set_speed;
        struct {
            usize train;
            bool lights;
        } set_lights;
        struct {
            usize train;
            isize offset;
        } set_offset;
        struct {
            usize train;
            usize dest;
        } set_dest;
        struct {
            usize train;
            usize pos;
        } set_pos;
        struct {
            usize train;
        } reverse;
        usize reverse_static;
        struct {
            usize train;
            usize new_pos;
        } position_update;
        isize position_wait; // the id of the train to wait for (or -1 for any train)
    } data;
} TrainstateMsg;

typedef struct {
    TrainstateMsgType type;
    union {
        struct {
            TrainState state;
        } get;
        struct {
            bool was_already_reversing;
        } reverse;
        struct {
            usize train;
            usize pos; // the location the train is currently at 
        } position_wait;
    } data;
} TrainstateResp;

usize trains[TRAIN_COUNT] = {2, 47};
TrainState train_state[NUMBER_OF_TRAINS] = {0};

// serialize trainstate o binary form for marklin
u8
trainstate_serialize(TrainState train_state)
{
    return ((train_state.lights << 4) & TRAIN_LIGHTS_MASK) | (train_state.speed & TRAIN_SPEED_MASK);
}

int
TrainstateSetSpeed(Tid trainstate_server, usize train, usize speed)
{

    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }
    if (!(0 <= speed && speed <= 14)) {
        ULOG_WARN("invalid train speed %d", speed);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_SET_SPEED,
        .data = {
            .set_speed = {
                .train = train,
                .speed = speed
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateSetSpeed errored");
        return -1;
    }
    return 0;
}

int
TrainstateReverseStatic(Tid trainstate_server, usize train)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }
    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_REVERSE_STATIC,
        .data = {
            .reverse_static = train
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateReverseStatic errored");
        return -1;
    }
    return 0;
}

int
TrainstateReverse(Tid trainstate_server, usize train)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_REVERSE,
        .data = {
            .reverse = {
                .train = train,
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (resp_buf.data.reverse.was_already_reversing) {
        ULOG_WARN("Train was already reversing");
        return -1;
    }
    if (ret < 0) {
        ULOG_WARN("TrainstateReverse errored");
        return -1;
    }
    return 0;
}

int
TrainstateSetLights(Tid trainstate_server, usize train, bool lights)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_SET_LIGHTS,
        .data = {
            .set_lights = {
                .train = train,
                .lights = lights,
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateSetLights errored");
        return -1;
    }
    return 0;
}

int
TrainstateSetOffset(Tid trainstate_server, usize train, isize offset)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_SET_OFFSET,
        .data = {
            .set_offset = {
                .train = train,
                .offset = offset,
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateSetOffset errored");
        return -1;
    }
    return 0;
}

int
TrainstateSetDest(Tid trainstate_server, usize train, usize dest)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_SET_DEST,
        .data = {
            .set_dest = {
                .train = train,
                .dest = dest,
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateSetDest errored");
        return -1;
    }
    return 0;
}

int
TrainstateSetPos(Tid trainstate_server, Tid reserve_server, usize train, TrackNode* node)
{
    Track* track = get_track();

    if (node->type != NODE_SENSOR) {
        UNIMPLEMENTED("TrainstateSetPos doesn't support nodes besides sensors");
    }

    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }

    // TODO not sure if reserve server should be coupled here
    if (node->zone != -1 && !zone_reserve(reserve_server, train, node->zone)) {
        ULOG_WARN("Failed to reserve zone %d when setting position", node->zone);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_SET_POS,
        .data = {
            .set_pos = {
                .train = train,
                .pos = track_node_index(track, node),
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateSetOffset errored");
        return -1;
    }
    return 0;

}

TrainState
TrainstateGet(Tid trainstate_server, usize train)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return (TrainState){0};
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_GET_STATE,
        .data = {
            .get = {
                .train = train,
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateGet errored");
        return (TrainState){0};
    }
    return resp_buf.data.get.state;
}


Pair_usize_usize
TrainstateWaitForSensor(Tid trainstate_server, isize train)
{
    if (!(1 <= train && train <= 100 || train == -1)) {
        ULOG_WARN("invalid train number %d", train);
        return (Pair_usize_usize){0};
    }
    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_POSITION_WAIT,
        .data = {
            .position_wait = train,
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateGet errored");
        return (Pair_usize_usize){0};
    }
    return (Pair_usize_usize){resp_buf.data.position_wait.train, resp_buf.data.position_wait.pos};
}


typedef struct {
    usize train;
    usize speed;
} ReverseMsg;

typedef struct {

} ReverseResp;


void
reverseTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    int from_tid;
    ReverseMsg msg_buf;
    ReverseResp reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(ReverseMsg));
    if (msg_len < 0) {
        ULOG_WARN("[REVERSE] Error when receiving");
        Exit();
    }
    reply_buf = (ReverseResp){};
    Reply(from_tid, (char*)&reply_buf, sizeof(ReverseResp));

    TrainstateResp resp_buf;
    TrainstateMsg send_buf;

    Delay(clock_server, train_data_stop_time(msg_buf.train, msg_buf.speed) / 10 + 100);
    send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_REVERSE_REVERSE,
    };
    Send(MyParentTid(), (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));

    Delay(clock_server, 10); // TODO arbitrary delay
    send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_REVERSE_RESTART,
    };
    Send(MyParentTid(), (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));

    Exit();
}

void
trainstateCallibration()
{

}

void
trainPosNotifierTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = MyParentTid();
    Track* track = get_track();

    Arena tmp_base = arena_new(512);
    // can now wait for future sensor updates
    for (;;) {
        Arena tmp = tmp_base;

        int sensor_id = WaitForSensor(sensor_server, -1);
        //ULOG_INFO("sensor id %d", sensor_id);

        // ====== zone impl (checks to see if sensor is in zone of train)
        for (usize i = 0; i < TRAIN_COUNT; ++i) {

            usize train = trains[i];
            TrackNode* node = track_node_by_sensor_id(track, train_state[train].pos);

            ZoneId train_zones[2] = {node->reverse->zone, node->zone};
            for (usize zone_i = 0; zone_i < 2; ++zone_i) {
                ZoneId train_zone = train_zones[zone_i];

                if (train_zone == -1) continue;

                for (usize j = 0; ; ++j) {
                    // TODO train positions MUST be sensors
                    TrackNode* zone_sensor = track->zones[train_zone].sensors[j];
                    if (zone_sensor == 0) break;
                    if (sensor_id == zone_sensor->num) {

                        ULOG_INFO("train %d moves to sensor %s", train, track->nodes[sensor_id].name);

                        // notify server that train position changed
                        TrainstateMsg send_buf = (TrainstateMsg) {
                            .type = TRAINSTATE_POSITION_UPDATE,
                            .data = {
                                .position_update = {
                                    .train = train,
                                    .new_pos = sensor_id
                                }
                            }
                        };
                        TrainstateResp resp_buf;
                        Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
                        goto loop_break;

                    }
                }
            }

        }

        ULOG_WARN("[TRAINSTATE NOTIF] superious sensor %s", track_node_by_sensor_id(track, sensor_id)->name);
        loop_break: {}

#if 0
        // ====== dijkstra impl (should work well if dijkstra was more efficient)
        // TODO currently a bit buggy since there are reversals

        // compute the next sensor each train is expecting
        // for each train, find the distance of the new sensor trigger from the previous observed position of the train, and take the most likely to be attributed to that specific train
        usize min_dist = 200000; //arbritrary large number
        usize min_train = 0;
        usize dest = sensor_id;
        for (usize i = 0; i < TRAIN_COUNT; ++i) {
            
            usize train = trains[i];
            usize src = train_state[train].pos; // TODO assumed that start node should always be sensor
            
            CBuf* path = dijkstra(track, train, src, dest, true, false, &tmp);

            if (cbuf_len(path) < min_dist) {
                min_dist = cbuf_len(path);
                min_train = train;
            }

        }
        
        if (min_train == 0) {
            PANIC("at least one train should have been found");
        }

        ULOG_INFO("train %d moves to sensor %s", min_train, track->nodes[sensor_id].name);

        // notify server that train position changed
        TrainstateMsg send_buf = (TrainstateMsg) {
            .type = TRAINSTATE_POSITION_UPDATE,
            .data = {
                .position_update = {
                    .train = min_train,
                    .new_pos = sensor_id
                }
            }
        };
        TrainstateResp resp_buf;
        Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
#endif
    }

    Exit();
}

typedef PAIR(Tid, isize) Pair_Tid_isize;

void
trainStateServer()
{
    RegisterAs(TRAINSTATE_ADDRESS); 

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    for (usize i = 0; i < NUMBER_OF_TRAINS; ++i) {
        train_state[i] = (TrainState) {
            .speed = 0,
            .lights = 0,
            .reversed = false,
            .pos = 0,
            .offset = 0,
        };
    }

    // temp disabling calibration
#if 0
    // calibrate trains to determine their intial positions
    for (usize i = 0; i < TRAIN_COUNT; ++i) {
        usize train = trains[i];
        // start the train and wait for it to hit the first sensor
        marklin_train_ctl(io_server, train, TRAIN_SPEED_ROCK); // some decently slow speed
        int sensor = WaitForSensor(sensor_server, -1);

        train_state[train].pos = sensor;
        /* train_state[train].offset = train_data_stop_dist(train, TRAIN_SPEED_ROCK); */
        /* ULOG_INFO("train %d starting at %d with offset %d", train, sensor, train_state[train].offset); */

        // stop train for now
        marklin_train_ctl(io_server, train, 0);
        Delay(clock_server, 500);
    }
#endif

    ULOG_INFO("completed train calibration");

    Tid reverse_tasks[NUMBER_OF_TRAINS] = {0};  // IMPORTANT: 0 means that the train is not currently reversing
    List* trainpos_requests = list_init(); // list of tasks waiting for train position to update

    Create(2, &trainPosNotifierTask, "train position notifier task");

    TrainstateMsg msg_buf;
    TrainstateResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(TrainstateMsg));
        if (msg_len < 0) {
            ULOG_WARN("[TRAINSTATE SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == TRAINSTATE_GET_STATE) {

            ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Querying train state");

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_GET_STATE,
                .data = {
                    .get = {
                        .state = train_state[msg_buf.data.get.train]
                    }
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_SET_SPEED) {

            usize train = msg_buf.data.set_speed.train;
            usize speed = msg_buf.data.set_speed.speed;
            train_state[train].speed = speed;

            ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting speed for train %d: %d", train, train_state[train].speed);
            marklin_train_ctl(marklin_server, train, trainstate_serialize(train_state[train]));

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_SPEED,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_REVERSE) {

            usize train = msg_buf.data.reverse.train;
            usize speed = train_state[train].speed;

            bool was_already_reversing;
            if (reverse_tasks[train] != 0) {
                was_already_reversing = true;
            }
            else {
                was_already_reversing = false;
                if (speed == 0) {
                    TrainState temp_state = train_state[train];
                    temp_state.speed = 15;
                    marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));
                } else {
                    TrainState temp_state = train_state[train];
                    temp_state.speed = 0;
                    marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));
                    reverse_tasks[train] = Create(2, &reverseTask, "Trainstate Reverse Task");

                    ReverseResp resp_buf;
                    ReverseMsg send_buf = (ReverseMsg) {
                        .train = train,
                        .speed = speed
                    };
                    int ret = Send(reverse_tasks[train], (const char*)&send_buf, sizeof(ReverseMsg), (char*)&resp_buf, sizeof(ReverseResp));
                }
            }

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_REVERSE,
                .data = {
                    .reverse = {
                        .was_already_reversing = was_already_reversing
                    }
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_REVERSE_STATIC) {

            usize train = msg_buf.data.reverse_static;

            TrainState temp_state = train_state[train];
            temp_state.speed = 15;
            marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));

            train_state[train].speed = 0; // also force speed to be zero

            // set the train state to reversed
            Track* track = get_track(); // TODO really ugly how this is here
            train_state[train].reversed = !train_state[train].reversed;
            //train_state[train].offset = -train_state[train].offset; // flip offset if reversing
            // TODO this might be race condition with notifier server
            // TODO this is commented out since we will explicitly set the position of the train to be facing the right way.
            //      this makes use of the fact that paths with a reverse will end with a reverse node.
            /* ULOG_INFO("train %d position was %s, will update to", train, track->nodes[train_state[train].pos].name, track->nodes[train_state[train].pos].reverse - track->nodes); */
            /* train_state[train].pos = track->nodes[train_state[train].pos].reverse - track->nodes; // TODO this is ugly calculation */

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_REVERSE_STATIC,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_REVERSE_REVERSE) {

            usize train = NUMBER_OF_TRAINS;
            for (usize i = 0; i < NUMBER_OF_TRAINS; i++) {
                if (reverse_tasks[i] == from_tid) {
                    train = i;
                    break;
                }
            }
            if (train == NUMBER_OF_TRAINS) {
                PANIC("Couldn't find train associated with reverse task");
            }


            TrainState temp_state = train_state[train];
            temp_state.speed = 15;
            marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));

            // set the train state to reversed
            Track* track = get_track(); // TODO really ugly how this is here
            train_state[train].reversed = !train_state[train].reversed;
            //train_state[train].offset = -train_state[train].offset; // flip offset if reversing
            // TODO this might be race condition with notifier server
            /* train_state[train].pos = track->nodes[train_state[train].pos].reverse - track->nodes; // TODO this is ugly calculation */

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_REVERSE_REVERSE,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_REVERSE_RESTART) {

            usize train = NUMBER_OF_TRAINS;
            for (usize i = 0; i < NUMBER_OF_TRAINS; i++) {
                if (reverse_tasks[i] == from_tid) {
                    train = i;
                    break;
                }
            }
            if (train == NUMBER_OF_TRAINS) {
                PANIC("Couldn't find train associated with reverse task");
            }

            marklin_train_ctl(marklin_server, train, trainstate_serialize(train_state[train]));
            reverse_tasks[train] = 0;

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_REVERSE_RESTART,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_SET_LIGHTS) {

            usize train = msg_buf.data.set_lights.train;

            train_state[train].lights = msg_buf.data.set_lights.lights;

            ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting lights for train %d: %d", train, train_state[train].lights);
            marklin_train_ctl(marklin_server, train, trainstate_serialize(train_state[train]));

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_LIGHTS,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_SET_OFFSET) {

            usize train = msg_buf.data.set_offset.train;

            train_state[train].offset = msg_buf.data.set_offset.offset;

            ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting offset for train %d: %d", train, train_state[train].offset);

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_OFFSET,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_SET_DEST) {

            usize train = msg_buf.data.set_dest.train;
            usize dest = msg_buf.data.set_dest.dest;

            train_state[train].dest = dest;

            ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting dest for train %d: %d", train, train_state[train].dest);

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_DEST,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_SET_POS) {

            usize train = msg_buf.data.set_pos.train;

            train_state[train].pos = msg_buf.data.set_pos.pos;

            ULOG_INFO("Explicitly setting pos for train %d: %s", train, get_track()->nodes[train_state[train].pos].name);

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_POS,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_POSITION_UPDATE) {

            train_state[msg_buf.data.position_update.train].pos = msg_buf.data.position_update.new_pos;

            TrainstateResp reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_POSITION_UPDATE
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

            ListIter it = list_iter(trainpos_requests); 
            Pair_Tid_isize* request;
            // pair is (Tid of sending task , train that updated)
            while (listiter_next(&it, (void**)&request)) {
                if (request->second == msg_buf.data.position_update.train || request->second == -1) {
                    // reply to task that was blocking
                    TrainstateResp reply_buf = (TrainstateResp) {
                        .type = TRAINSTATE_POSITION_WAIT,
                        .data = {
                            .position_wait = {
                                .train = msg_buf.data.position_update.train,
                                .pos =  msg_buf.data.position_update.new_pos
                            }
                        }
                    };
                    Reply(request->first, (char*)&reply_buf, sizeof(TrainstateResp));
                    list_remove(trainpos_requests, request);
                }
            }


        } else if (msg_buf.type == TRAINSTATE_POSITION_WAIT) {

            Pair_Tid_isize* request = alloc(sizeof(Pair_Tid_isize));
            *request = (Pair_Tid_isize) {
                from_tid,
                msg_buf.data.position_wait
            };
            list_push_back(trainpos_requests, request);
            // dont reply 

        } else {
            ULOG_WARN("[TRAINSTATE SERVER] Invalid message type");
            continue;
        }


    }

    Exit();
}
