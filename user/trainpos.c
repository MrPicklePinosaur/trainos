#include <trainstd.h>
#include <traindef.h>
#include <traintasks.h>

#include "sensor.h"
#include "marklin.h"
#include "switch.h"
#include "user/path/track_data.h"

#include "trainpos.h"

#define TRAIN_COUNT 2

typedef enum {
    TRAINPOS_INIT,
} TrainposMsgType;

typedef struct {
    TrainposMsgType type;
    union {

    } data;
} TrainposMsg;

typedef struct {
    TrainposMsgType type;
    union {
        
    } data;
} TrainposResp;

usize trains[TRAIN_COUNT] = {2, 47};
usize train_pos[TRAIN_COUNT] = {0};

void
trainPosNotifierTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Track* track = track_a_init();

    // can now wait for future sensor updates
    Arena tmp_base = arena_new(256);
    for (;;) {
        Arena tmp = tmp_base;

        int sensor_id = WaitForSensor(sensor_server, -1);
        str8 sensor_name = sensor_id_to_name(sensor_id, &tmp);
        ULOG_DEBUG("triggered sensor name %s", str8_to_cstr(sensor_name));

        // compute the next sensor each train is expecting
        usize node_index = (usize)map_get(&track->map, sensor_name, &track->arena);
        ULOG_DEBUG("node index %d", node_index);
        TrackNode* node = &track->nodes[node_index];
        ULOG_DEBUG("node %x", node);

        // walk node graph until next sensor
        TrackNode* next_sensor = track_next_sensor(switch_server, track, node); 
        if (next_sensor == NULL) {
            ULOG_WARN("couldn't find next sensor");
            continue;
        }
        ULOG_DEBUG("next sensor %s", next_sensor->name);

        // decide which train is expecting this sensor

        // update that trains state

    }

    Exit();
}
void
trainPosTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);

    // calibrate trains to determine their intial positions
    for (usize i = 0; i < TRAIN_COUNT; ++i) {
        usize train = trains[i];
        // start the train and wait for it to hit the first sensor
        marklin_train_ctl(io_server, train, 5); // some decently slow speed
        int sensor = WaitForSensor(sensor_server, -1);

        train_pos[i] = sensor;

        // stop train for now
        marklin_train_ctl(io_server, train, 0);
    }

    ULOG_DEBUG("train positions finished calibration");

    Create(2, &trainPosNotifierTask, "train position notifier task");

    TrainposMsg msg_buf;
    TrainposResp reply_buf;
    int from_tid;
    for (;;) {
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(TrainposMsg));
        if (msg_len < 0) {
            ULOG_WARN("[TRAINPOS SERVER] Error when receiving");
            continue;
        }
    }

    Exit();
}
