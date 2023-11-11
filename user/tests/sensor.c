#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "tester.h"
#include "user/sensor.h"

void
testSensor()
{
    println("Running test suite for sensor server -----------------");

    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    println("waiting for sensor A1...");
    WaitForSensor(sensor_server, 0);
    println("waiting for sensor B1...");
    WaitForSensor(sensor_server, 16);
    println("complete");



    Exit();
}
