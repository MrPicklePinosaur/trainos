#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "user/marklin.h"
#include "trainstate.h"

#define TRAIN_SPEED_MASK     0b001111
#define TRAIN_LIGHTS_MASK    0b010000

typedef enum {
    TRAINSTATE_GET_STATE,
    TRAINSTATE_SET_SPEED,
    TRAINSTATE_SET_LIGHTS,
    TRAINSTATE_REVERSE,
    TRAINSTATE_REVERSE_REVERSE,
    TRAINSTATE_REVERSE_RESTART,
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
    } data;
} TrainstateResp;

// serialize trainstate o binary form for marklin
u8
trainstate_serialize(TrainState train_state)
{
    return ((train_state.reversed << 5) & TRAIN_LIGHTS_MASK) | (train_state.speed & TRAIN_SPEED_MASK);
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
trainStateServer()
{
    RegisterAs(TRAINSTATE_ADDRESS); 

    Tid marklin_server = WhoIs(IO_ADDRESS_MARKLIN);

    TrainState train_state[NUMBER_OF_TRAINS] = {0};
    Tid reverse_tasks[NUMBER_OF_TRAINS] = {0};  // IMPORTANT: 0 means that the train is not currently reversing

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

        } else {
            ULOG_WARN("[TRAINSTATE SERVER] Invalid message type");
            continue;
        }


    }

    Exit();
}
