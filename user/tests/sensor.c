#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "tester.h"
#include "user/sensor.h"

void
testSensor()
{
    println("Running test suite for sensor server helpers -----------------");

    Arena arena = arena_new(8*80);
    for (usize i = 0; i < 80; ++i) {
        str8 sensor_name = sensor_id_to_name(i, &arena);
        println("%d %s", i, str8_to_cstr(sensor_name));
    }

    println("Running test suite for sensor server -----------------");

    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    println("waiting for sensor A1...");
    WaitForSensor(sensor_server, 0);
    println("waiting for sensor B1...");
    WaitForSensor(sensor_server, 16);
    println("complete");



    Exit();
}
