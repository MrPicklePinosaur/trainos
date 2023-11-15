#include <trainstd.h>
#include <traintasks.h>

#include "sensor.h"
#include "marklin.h"
#include "trainpos.h"

#define TRAIN_COUNT 2

void
trainPosTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);

    // calibrate trains to determine their intial positions
    usize trains[TRAIN_COUNT] = {2, 77};
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
    for (;;) {
        int sensor = WaitForSensor(sensor_server, -1);

        // compute the next sensor each train is expecting

        // decide which train is expecting this sensor

        // update that trains state

    }

    Exit();
}
