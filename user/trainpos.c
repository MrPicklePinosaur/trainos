#include <trainstd.h>
#include <traindef.h>
#include <traintasks.h>

#include "sensor.h"
#include "marklin.h"
#include "switch.h"
#include "user/path/track_data.h"

#include "trainpos.h"

#define TRAIN_COUNT 2

void
trainPosTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Track* track = track_a_init();

    // calibrate trains to determine their intial positions
    usize trains[TRAIN_COUNT] = {2, 47};
    usize train_pos[TRAIN_COUNT] = {0};

    for (usize i = 0; i < TRAIN_COUNT; ++i) {
        usize train = trains[i];
        // start the train and wait for it to hit the first sensor
        marklin_train_ctl(io_server, train, 5); // some decently slow speed
        int sensor = WaitForSensor(sensor_server, -1);

        train_pos[i] = sensor;

        // stop train for now
        marklin_train_ctl(io_server, train, 0);
    }

    // can now wait for future sensor updates
    Arena tmp_base = arena_new(256);
    for (;;) {
        Arena tmp = tmp_base;

        int sensor_id = WaitForSensor(sensor_server, -1);
        str8 sensor_name = sensor_id_to_name(sensor_id, &tmp);

        // compute the next sensor each train is expecting
        usize node_index = (usize)map_get(&track->map, sensor_name, &track->arena);
        TrackNode* node = &track->nodes[node_index];

        // walk node graph until next sensor
        const SwitchMode* all_switch_modes = SwitchQueryAll(switch_server);

        for (;;) {
            if (node->type == NODE_SENSOR) break;

            if (node->type == NODE_BRANCH) {
                node = node->edge[DIR_AHEAD].dest;    
            } else if (node->type == NODE_BRANCH) {
                // query switch state
                SwitchMode switch_mode = all_switch_modes[node->num-1];
                if (switch_mode == SWITCH_MODE_UNKNOWN) {
                    PANIC("unknown switch state");
                }

                if (switch_mode == SWITCH_MODE_STRAIGHT) {
                    node = node->edge[DIR_STRAIGHT].dest;
                } else {
                    node = node->edge[DIR_CURVED].dest;
                }

            } else {
                PANIC("unhandled case")
            }
        }

        // decide which train is expecting this sensor

        // update that trains state

    }

    Exit();
}
