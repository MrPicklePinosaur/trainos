#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/sensor.h"
#include "user/nameserver.h"

void
testSensor()
{
    println("Running test suite for sensor server -----------------");

    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    println("waiting for sensor A1...");
    WaitForSensor(sensor_server, 1);
    println("waiting for sensor B1...");
    WaitForSensor(sensor_server, 17);
    println("complete");



    Exit();
}
