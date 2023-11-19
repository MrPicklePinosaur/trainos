#include <trainstd.h>
#include <traindef.h>
#include <traintasks.h>

#include "sensor.h"
#include "marklin.h"
#include "switch.h"
#include "user/path/track_data.h"
#include "user/path/train_data.h"

#include "trainpos.h"

#define TRAIN_COUNT 2

typedef enum {
    TRAINPOS_TRIGGERED,
    TRAINPOS_WAIT,
    TRAINPOS_QUERY,
} TrainposMsgType;

typedef struct {
    TrainposMsgType type;
    union {
        isize wait;
        isize query;
        struct {
            usize train;
            usize pos;
        } triggered;
    } data;
} TrainposMsg;

typedef struct {
    TrainposMsgType type;
    union {
        struct {
            usize train;
            usize pos; // the location the train is currently at 
        } wait;
        usize query;
    } data;
} TrainposResp;

usize trains[TRAIN_COUNT] = {2, 47};
usize train_pos[TRAIN_COUNT] = {0};

TrainPosWaitResult
trainPosWait(Tid trainpos_server, isize train)
{
    TrainposResp resp_buf;
    TrainposMsg send_buf = (TrainposMsg) {
        .type = TRAINPOS_WAIT,
        .data = {
            .wait = train
        }
    };
    int ret = Send(trainpos_server, (const char*)&send_buf, sizeof(TrainposMsg), (char*)&resp_buf, sizeof(TrainposResp));
    if (ret < 0) {
        ULOG_WARN("trainPosWait errored");
        return (TrainPosWaitResult){0};
    }
    return (TrainPosWaitResult){ resp_buf.data.wait.train, resp_buf.data.wait.pos };
}

isize
trainPosQuery(Tid trainpos_server, isize train)
{
    TrainposResp resp_buf;
    TrainposMsg send_buf = (TrainposMsg) {
        .type = TRAINPOS_QUERY,
        .data = {
            .query = train
        }
    };
    int ret = Send(trainpos_server, (const char*)&send_buf, sizeof(TrainposMsg), (char*)&resp_buf, sizeof(TrainposResp));
    if (ret < 0) {
        ULOG_WARN("trainPosQuery errored");
        return -1;
    }
    return resp_buf.data.query;
}

void
trainPosNotifierTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainpos_server = MyParentTid();
    Track* track = get_track_a();

    // can now wait for future sensor updates
    for (;;) {

        int sensor_id = WaitForSensor(sensor_server, -1);
        //ULOG_INFO("sensor id %d", sensor_id);

        // compute the next sensor each train is expecting
        for (usize i = 0; i < TRAIN_COUNT; ++i) {
            
            usize train = trains[i];
            TrackNode* node = track_node_by_sensor_id(track, train_pos[i]);

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
                train_pos[i] = sensor_id;

                // notify server that train position changed
                TrainposMsg send_buf = (TrainposMsg) {
                    .type = TRAINPOS_TRIGGERED,
                    .data = {
                        .triggered = {
                            .train = train,
                            .pos = train_pos[i]
                        }
                    }
                };
                TrainposResp resp_buf;
                Send(trainpos_server, (const char*)&send_buf, sizeof(TrainposMsg), (char*)&resp_buf, sizeof(TrainposResp));
            }

        }

    }

    Exit();
}

typedef struct {
    Tid tid;
    isize train;
} TrainposRequest;

void
trainPosTask()
{
    RegisterAs(TRAINPOS_ADDRESS);

    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    // calibrate trains to determine their intial positions
    for (usize i = 0; i < TRAIN_COUNT; ++i) {
        usize train = trains[i];
        // start the train and wait for it to hit the first sensor
        marklin_train_ctl(io_server, train, 3); // some decently slow speed
        int sensor = WaitForSensor(sensor_server, -1);

        train_pos[i] = sensor;
        //ULOG_INFO("train %d starting at %d", train, sensor);

        // stop train for now
        marklin_train_ctl(io_server, train, 0);
        Delay(clock_server, 500);
    }

    //ULOG_INFO("train positions finished calibration");

    Create(2, &trainPosNotifierTask, "train position notifier task");

    List* trainpos_requests = list_init();

    TrainposMsg msg_buf;
    TrainposResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(TrainposMsg));
        if (msg_len < 0) {
            ULOG_WARN("[TRAINPOS SERVER] Error when receiving");
            continue;
        }

        if (msg_buf.type == TRAINPOS_TRIGGERED) {
            
            ListIter it = list_iter(trainpos_requests); 
            TrainposRequest* request;
            while (listiter_next(&it, (void**)&request)) {
                if (request->train == msg_buf.data.triggered.train || request->train == -1) {
                    TrainposResp reply_buf = (TrainposResp) {
                        .type = TRAINPOS_WAIT,
                        .data = {
                            .wait = {
                                .train = msg_buf.data.triggered.train,
                                .pos =  msg_buf.data.triggered.pos
                            }
                        }
                    };
                    Reply(request->tid, (char*)&reply_buf, sizeof(TrainposResp));
                    list_remove(trainpos_requests, request);
                }
            }

            TrainposResp reply_buf = (TrainposResp) {
                .type = TRAINPOS_TRIGGERED
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainposResp));

        }
        else if (msg_buf.type == TRAINPOS_WAIT) {
            TrainposRequest* request = alloc(sizeof(TrainposRequest));
            *request = (TrainposRequest) {
                .tid = from_tid,
                .train = msg_buf.data.wait
            };
            list_push_back(trainpos_requests, request);

            // Don't reply just yet
        }
        else if (msg_buf.type == TRAINPOS_QUERY) {
            TrainposResp reply_buf = (TrainposResp) {
                .type = TRAINPOS_QUERY,
                .data = {
                    .query = train_pos[get_train_index(msg_buf.data.query)]
                }
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainposResp));
        }
        else {
            ULOG_WARN("[TRAINPOS SERVER] Invalid message type");
            continue;
        }
    }

    Exit();
}
