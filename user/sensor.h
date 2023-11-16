#ifndef __USER_SENSOR_H__
#define __USER_SENSOR_H__

#include <traindef.h>
#include <trainstd.h>
#include <trainsys.h>

#define SENSOR_ADDRESS "sensor"

void sensorServerTask();

// passing sensor = -1 means we don't care which sensor
int WaitForSensor(Tid sensor_server, isize sensor);

str8 sensor_id_to_name(u8 id, Arena* arena);

#endif // __USER_SENSOR_H__
