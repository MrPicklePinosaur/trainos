#ifndef __USER_SENSOR_H__
#define __USER_SENSOR_H__

#include <traindef.h>
#include <trainsys.h>

#define SENSOR_ADDRESS "sensor"

typedef u8 TrainState;

void sensorServerTask();

// passing sensor = 0 means we don't care which sensor
int WaitForSensor(Tid sensor_server, usize sensor);

#endif // __USER_SENSOR_H__
