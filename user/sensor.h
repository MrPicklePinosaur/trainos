#ifndef __USER_SENSOR_H__
#define __USER_SENSOR_H__

#include <traindef.h>
#include <trainstd.h>
#include <trainsys.h>

#define SENSOR_ADDRESS "sensor"
#define MAX_TRIGGERED 16

void sensorServerTask();

int WaitForSensor(Tid sensor_server, isize sensor);
usize* WaitForAnySensor(Tid sensor_server, Arena* arena);

str8 sensor_id_to_name(u8 id, Arena* arena);

#endif // __USER_SENSOR_H__
