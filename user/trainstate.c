#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "user/sensor.h"
#include "user/switch.h"
#include "user/marklin.h"
#include "user/path/track_data.h"
#include "user/path/train_data.h"
#include "trainstate.h"

#define TRAIN_SPEED_MASK     0b001111
#define TRAIN_LIGHTS_MASK    0b010000

typedef enum {
    TRAINSTATE_GET_STATE,
    TRAINSTATE_SET_SPEED,
    TRAINSTATE_SET_LIGHTS,
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

usize trains[TRAIN_COUNT] = {2, 77};
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


void
reverseTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    TrainstateResp resp_buf;
    TrainstateMsg send_buf;

    // TODO potential optimization: adjust this delay time depending on the current speed of the train
    Delay(clock_server, 400); // TODO arbitrary delay
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
    Track* track = get_track_a();

    // can now wait for future sensor updates
    for (;;) {

        int sensor_id = WaitForSensor(sensor_server, -1);
        //ULOG_INFO("sensor id %d", sensor_id);

        // compute the next sensor each train is expecting
        for (usize i = 0; i < TRAIN_COUNT; ++i) {
            
            usize train = trains[i];
            TrackNode* node = track_node_by_sensor_id(track, train_state[train].pos);

            // walk node graph until next sensor
            TrackNode* next_sensor = track_next_sensor(switch_server, track, node); 
            if (next_sensor == NULL) {
                ULOG_WARN("couldn't find next sensor");
                continue;
            }
            //ULOG_INFO("next sensor for train %d is %s", train, next_sensor->name);

            // see if there is a train that is expecting this sensor
            if (sensor_id == next_sensor->num) {
                //ULOG_INFO("train %d moves to sensor %s", train, next_sensor->name);

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
            }

        }

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
        };
    }

    // calibrate trains to determine their intial positions
    for (usize i = 0; i < TRAIN_COUNT; ++i) {
        usize train = trains[i];
        // start the train and wait for it to hit the first sensor
        marklin_train_ctl(io_server, train, 3); // some decently slow speed
        int sensor = WaitForSensor(sensor_server, -1);

        train_state[train].pos = sensor;
        //ULOG_INFO("train %d starting at %d", train, sensor);

        // stop train for now
        marklin_train_ctl(io_server, train, 0);
        Delay(clock_server, 500);
    }

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

            bool was_already_reversing;
            if (reverse_tasks[train] != 0) {
                was_already_reversing = true;
            }
            else {
                was_already_reversing = false;
                TrainState temp_state = train_state[train];
                temp_state.speed = 0;
                marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));
                reverse_tasks[train] = Create(2, &reverseTask, "Trainstate Reverse Task");
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
            Track* track = get_track_a(); // TODO really ugly how this is here
            train_state[train].reversed = !train_state[train].reversed;
            // TODO this might be race condition with notifier server
            train_state[train].pos = track->nodes[train_state[train].pos].reverse - track->nodes; // TODO this is ugly calculation

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
            Track* track = get_track_a(); // TODO really ugly how this is here
            train_state[train].reversed = !train_state[train].reversed;
            // TODO this might be race condition with notifier server
            train_state[train].pos = track->nodes[train_state[train].pos].reverse - track->nodes; // TODO this is ugly calculation

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

        } else if (msg_buf.type == TRAINSTATE_POSITION_UPDATE) {

            train_state[msg_buf.data.position_update.train].pos = msg_buf.data.position_update.new_pos;

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

            TrainstateResp reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_POSITION_UPDATE
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

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
