#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "user/sensor.h"
#include "user/switch.h"
#include "user/marklin.h"
#include "user/cohort.h"
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
    TRAINSTATE_SET_COHORT,
    TRAINSTATE_REVERSE_STATIC, // special case of reverse, reverse from rest and right away
    TRAINSTATE_REVERSE,
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
            Cohort cohort;
        } set_cohort;
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

usize trains[TRAIN_COUNT] = {2, 47, 58, 77};
TrainState train_state[NUMBER_OF_TRAINS] = {0};
Tid reverse_tasks[NUMBER_OF_TRAINS] = {0};  // IMPORTANT: 0 means that the train is not currently reversing

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
        // TODO not sure if should break here if not allowed
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
        ULOG_WARN("TrainstateSetPos errored");
        return -1;
    }
    return 0;

}

int
TrainstateSetCohort(Tid trainstate_server, usize train, Cohort cohort)
{
    if (!(1 <= train && train <= 100)) {
        ULOG_WARN("invalid train number %d", train);
        return -1;
    }

    TrainstateResp resp_buf;
    TrainstateMsg send_buf = (TrainstateMsg) {
        .type = TRAINSTATE_SET_COHORT,
        .data = {
            .set_cohort = {
                .train = train,
                .cohort = cohort,
            }
        }
    };
    int ret = Send(trainstate_server, (const char*)&send_buf, sizeof(TrainstateMsg), (char*)&resp_buf, sizeof(TrainstateResp));
    if (ret < 0) {
        ULOG_WARN("TrainstateSetCohort errored");
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
train_join_cohort(usize train, Cohort cohort)
{
    if (train_state[train].cohort != cohort) {
        train_state[train].cohort = cohort;
        cbuf_push_back(train_state[cohort].followers, (void*)train);
    }
}

// make a train leave cohort
void
train_leave_cohort(usize train)
{
    Cohort cohort = train_state[train].cohort;

    if (cohort == train) {
        // already not in cohort
        return;
    }

    // remove train from previous cohort
    if (train_state[train].cohort_regulate_task != 0) {
        Kill(train_state[train].cohort_regulate_task);
        train_state[train].cohort_regulate_task = 0;
    }

    TrainState old_leader_state = train_state[cohort];
    if (cbuf_len(old_leader_state.followers) > 0 && cbuf_back(old_leader_state.followers) == (void*)train) {
        cbuf_pop_back(old_leader_state.followers);
    } else {
        ULOG_WARN("train %d not at back of cohort %d, can't remove", train, cohort);
    }
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
    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);

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

    usize train = msg_buf.train;
    usize speed = msg_buf.speed;

    Delay(clock_server, train_data_stop_time(train, speed) / 10 + 100);

    // reverse train
    TrainState temp_state = train_state[train];
    temp_state.speed = 15;
    marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));

    // set the train state to reversed
    Track* track = get_track(); // TODO really ugly how this is here
    train_state[train].reversed = !train_state[train].reversed;

    Delay(clock_server, 10); // TODO arbitrary delay

    // start train again
    marklin_train_ctl(marklin_server, train, trainstate_serialize(train_state[train]));
    reverse_tasks[train] = 0;

    Exit();
}

Tid
train_reverse(Tid marklin_server, usize train)
{
    usize speed = train_state[train].speed;

    if (speed == 0) {
        TrainState temp_state = train_state[train];
        temp_state.speed = 15;
        marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));
        return 0;
    } else {
        TrainState temp_state = train_state[train];
        temp_state.speed = 0;
        marklin_train_ctl(marklin_server, train, trainstate_serialize(temp_state));
        Tid reverse_task = Create(2, &reverseTask, "Trainstate Reverse Task");
        reverse_tasks[train] = reverse_task;

        ReverseResp resp_buf;
        ReverseMsg send_buf = (ReverseMsg) {
            .train = train,
            .speed = speed
        };
        int ret = Send(reverse_task, (const char*)&send_buf, sizeof(ReverseMsg), (char*)&resp_buf, sizeof(ReverseResp));
        return reverse_task;
    }
}

typedef struct {
    usize train; // leader of the cohort to disband and reverse
} CohortReverseMsg;

typedef struct {
} CohortReverseResp;

void
cohortReverseTask()
{
    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);

    int from_tid;
    CohortReverseMsg msg_buf;
    CohortReverseResp reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(CohortReverseMsg));
    if (msg_len < 0) {
        ULOG_WARN("[COHORT REVERSE] Error when receiving");
        Exit();
    }
    reply_buf = (CohortReverseResp){};
    Reply(from_tid, (char*)&reply_buf, sizeof(CohortReverseResp));

    usize train = msg_buf.train;

    CBuf* reverse_tasks = cbuf_new(12);

    Tid leader_reverse_task = train_reverse(marklin_server, train);
    if (leader_reverse_task != 0) {
        cbuf_push_back(reverse_tasks, (void*)leader_reverse_task);
    }

    // TODO this should be safe since cohort leave only removes from back of cbuf
    usize follower_len = cbuf_len(train_state[train].followers);
    if (follower_len == 0) {
        PANIC("followers len is zero");
    }

    for (usize i = 0; i < follower_len; ++i) {
        usize follower_train = (usize)cbuf_get(train_state[train].followers, i);
        Tid reverse_task = train_reverse(marklin_server, follower_train);
        if (reverse_task != 0) {
            cbuf_push_back(reverse_tasks, (void*)reverse_task);
        }
    }

    // spawn task that blocks until all trains are done reversing, then disbands and reforms cohort in reverse

    // block until all reverse tasks complete
    for (usize i = 0; i < cbuf_len(reverse_tasks); ++i) {
        Tid reverse_task = (Tid)cbuf_get(reverse_tasks, i);
        WaitTid(reverse_task);
    }
    ULOG_DEBUG("All reverse tasks finished");

    // disband and reform the original cohort 
    usize old_leader = train;
    usize new_leader = (usize)cbuf_back(train_state[train].followers); // NOTE safe since we asserted that follower_len > 0

    for (usize i = 0; i < follower_len; ++i) {
        usize follower_train = (usize)cbuf_get(train_state[train].followers, follower_len-1-i);
        
        // leave old cohort
        train_leave_cohort(follower_train);

        // join new cohort
        train_join_cohort(follower_train, new_leader);
    }

    train_join_cohort(old_leader, new_leader);

    Exit();
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

        usize* sensor_ids = WaitForAnySensor(sensor_server, &tmp);
        for (; *sensor_ids != -1; ++sensor_ids) {
            usize sensor_id = *sensor_ids;

            //ULOG_DEBUG("got sensor id %d", sensor_id);

            // ====== zone impl (checks to see if sensor is in zone of train)
            for (usize i = 0; i < TRAIN_COUNT; ++i) {

                usize train = trains[i];

                // train position not calibrated, skip checking
                if (train_state[train].pos == TRAIN_POS_NULL) {
                    /* ULOG_WARN("train %d has null position, could not attribute sensor", train); */
                    continue;
                }

                TrackNode* node = track_node_by_sensor_id(track, train_state[train].pos);

                /* ZoneId train_zones[2] = {node->reverse->zone, node->zone}; */
                ZoneId train_zones[1] = {node->reverse->zone};
                for (usize zone_i = 0; zone_i < 1; ++zone_i) {
                    ZoneId train_zone = train_zones[zone_i];

                    if (train_zone == -1) continue;

                    for (usize j = 0; ; ++j) {
                        // TODO train positions MUST be sensors
                        TrackNode* zone_sensor = track->zones[train_zone].sensors[j];
                        if (zone_sensor == 0) break;
                        if (sensor_id == zone_sensor->num) {

                            //ULOG_INFO("[ATTRIBUTION] train %d @ sensor %s, current zone %d", train, track->nodes[sensor_id].name, train_zone);
                            train_state[train].pos = sensor_id; // update train position right away here

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
        usize train_id = trains[i];

        train_state[i] = (TrainState) {
            .speed = 0,
            .lights = 0,
            .reversed = false,
            .pos = TRAIN_POS_NULL,
            .offset = 0,
            .cohort = i,
            .followers = cbuf_new(12),
            .cohort_regulate_task = 0
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

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_SPEED,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

            if (train_state[train].cohort == train) {
                // if leader, also modify speeds of followers

                ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting speed for cohort leader train %d: %d", train, speed);
                train_state[train].speed = speed;
                marklin_train_ctl(marklin_server, train, trainstate_serialize(train_state[train]));

                // if speed zero just set all followers to speed zero
                if (speed == 0) {
                    for (usize i = 0; i < cbuf_len(train_state[train].followers); ++i) {
                        usize follower_train = (usize)cbuf_get(train_state[train].followers, i);

                        // also kill cohort task
                        if (train_state[follower_train].cohort_regulate_task != 0) {
                            Kill(train_state[follower_train].cohort_regulate_task);
                            train_state[follower_train].cohort_regulate_task = 0;
                        }

                        train_state[follower_train].speed = 0;
                        marklin_train_ctl(marklin_server, follower_train, trainstate_serialize(train_state[follower_train]));
                    }
                    continue;
                }

                usize next_train_vel = train_data_vel(train, speed); // speed of the train thats ahead

                Cohort cohort = train;
                usize next_train = train;

                // pair is (Tid of sending task , train that updated)
                for (usize i = 0; i < cbuf_len(train_state[train].followers); ++i) {
                    usize follower_train = (usize)cbuf_get(train_state[train].followers, i);

                    Delay(clock_server, 30); // TODO arbritrary propogation delay

                    // search for the speed that is lower than the train ahead
                    u32 follower_speed = get_safe_speed(follower_train, next_train_vel);
                    if (follower_speed == 0) {
                        ULOG_WARN("NO VALID SPEED FOUND");
                    }

                    ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting speed for train %d in cohort %d: %d", follower_train, cohort, follower_speed);

                    next_train_vel = train_data_vel(follower_train, follower_speed);

                    train_state[follower_train].speed = follower_speed;
                    marklin_train_ctl(marklin_server, follower_train, trainstate_serialize(train_state[follower_train]));

                    // kill old regulate task
                    if (train_state[follower_train].cohort_regulate_task != 0) {
                        Kill(train_state[follower_train].cohort_regulate_task);
                        train_state[follower_train].cohort_regulate_task = 0;
                    }

                    // now spawn a task that will monitor train to adjust speed of train based on train in front
                    CohortFollowerRegulate send_buf = (CohortFollowerRegulate) {
                        .ahead_train = next_train,
                        .follower_train = follower_train,
                    };
                    struct {} resp_buf;

                    Tid follower_regulate_task = Create(2, &cohort_follower_regulate, "Cohort follower regulate");
                    train_state[follower_train].cohort_regulate_task = follower_regulate_task;
                    
                    Send(follower_regulate_task, (const char*)&send_buf, sizeof(CohortFollowerRegulate), (char*)&resp_buf, 0);

                    next_train = follower_train;

                }
            } else {

                // if not leader, just set the speed explicitly

                ULOG_INFO_M(LOG_MASK_TRAINSTATE, "[TRAINSTATE SERVER] Setting speed explicitly for follower train %d: %d", train, speed);
                train_state[train].speed = speed;
                marklin_train_ctl(marklin_server, train, trainstate_serialize(train_state[train]));
            }


        } else if (msg_buf.type == TRAINSTATE_REVERSE) {

            usize train = msg_buf.data.reverse.train;
            usize speed = train_state[train].speed;

            if (reverse_tasks[train] != 0) {
                reply_buf = (TrainstateResp) {
                    .type = TRAINSTATE_REVERSE,
                    .data = {
                        .reverse = {
                            .was_already_reversing = true
                        }
                    }
                };
                Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));
                continue;
            }

            // TODO reverse all trains in cohort

            // disband cohort and reverse trains
            usize follower_len = cbuf_len(train_state[train].followers);
            if (follower_len == 0) {
                train_reverse(marklin_server, train);
            } else {
                Tid cohort_reverse_task = Create(5, &cohortReverseTask, "Cohort reverse task"); 

                CohortReverseResp resp_buf;
                CohortReverseMsg send_buf = (CohortReverseMsg) {
                    .train = train,
                };
                int ret = Send(cohort_reverse_task, (const char*)&send_buf, sizeof(CohortReverseMsg), (char*)&resp_buf, sizeof(CohortReverseResp));

            }
            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_REVERSE,
                .data = {
                    .reverse = {
                        .was_already_reversing = false
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

        } else if (msg_buf.type == TRAINSTATE_SET_COHORT) {

            usize train = msg_buf.data.set_cohort.train;
            Cohort cohort = msg_buf.data.set_cohort.cohort;

            Cohort old_cohort = train_state[train].cohort;
            if (old_cohort != cohort) {

                train_leave_cohort(train);

                train_join_cohort(train, cohort);

                ULOG_INFO("Setting cohort for train %d: %d", train, cohort);
            }

            reply_buf = (TrainstateResp) {
                .type = TRAINSTATE_SET_COHORT,
                .data = {}
            };
            Reply(from_tid, (char*)&reply_buf, sizeof(TrainstateResp));

        } else if (msg_buf.type == TRAINSTATE_POSITION_UPDATE) {

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
