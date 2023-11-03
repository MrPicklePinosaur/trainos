#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"
#include "marklin.h"
#include "util.h"

#define BYTE_COUNT 10

static uint32_t sensor_state[BYTE_COUNT] = {0};
static uint32_t prev_sensor_state[BYTE_COUNT] = {0};

// assumption: only one sensor will be activated per poll, so we only return the first sensor that matches
uint32_t query_sensor(size_t sensor_group) {

    marklin_pick_s88(sensor_group);

    // seperate loop to ensure that we update prev state of other byte as well
    for (int offset = 0; offset < 2; ++offset) {
        size_t i = sensor_group*2+offset;

        uint8_t sensor_byte = uart_getc(MARKLIN);

        prev_sensor_state[i] = sensor_state[i];
        sensor_state[i] = sensor_byte;
    }

    for (int offset = 0; offset < 2; ++offset) {
        size_t i = sensor_group*2+offset;

        uint8_t triggered = ~(prev_sensor_state[i]) & sensor_state[i];
        for (uint32_t j = 0; j < 8; ++j) {
            if (((triggered >> j) & 0x1) == 1) {
                uint8_t sensor_index = (7-j);
                return i*8+sensor_index;
            }
        }
    }
    return 0;

}

void calibTrainSpeed();
void calibTrainStop();

int kmain() {

    // initialize GPIO for both console and marklin uarts
    uart_init();
    uart_config_and_enable_marklin();
    marklin_init();

    for (;;) {
        uart_printf(CONSOLE, "========================\r\n");
        uart_printf(CONSOLE, "SELECT CALIB TO RUN\r\n");
        uart_printf(CONSOLE, "1: calibTrainSpeed\r\n");
        uart_printf(CONSOLE, "2: calibTrainStop\r\n");
        uart_printf(CONSOLE, "========================\r\n");

        int ch = uart_getc(CONSOLE);
        switch (ch) {
            case 1:
                uart_printf(CONSOLE, "running train speed calib\r\n");
                calibTrainSpeed();
                break;
            case 2:
                uart_printf(CONSOLE, "running train stop calib\r\n");
                calibTrainStop();
                break;
            default:
                uart_printf(CONSOLE, "invalid calib task");
        }
    }

}

void calibTrainSpeed() {

    // Clear any previous sensor detections
    marklin_dump_s88(5);
    for (int i = 0; i < BYTE_COUNT; ++i) {
        uart_getc(MARKLIN);
    }

    const uint32_t SAMPLES = 3;
    uint32_t BANKS[] = {3, 2, 4, 5, 5, 4, 5, 4, 2, 3, 1, 2};
    uint32_t SENSOR[] = {10, 1, 14, 14, 9, 5, 6, 4, 6, 12, 4, 16};
    uint32_t DISTANCES[] = {128+231, 404, 239+43, 376, 239+155+239, 376, 50+239, 404, 231+120, 333+43, 437, 50+326};
    size_t SENSOR_COUNT = 12;
    uint32_t SPEEDS[SENSOR_COUNT];
    for (int i = 0; i < SENSOR_COUNT; ++i) {
        SPEEDS[i] = 0;
    }

    uint32_t prev_time = 0;
    uint32_t current_time = 0;

    for (int sample = 0; sample < SAMPLES+1; ++sample) {

        size_t current_sensor = 0;

        // uart_printf(CONSOLE, "expected group %d sensor %d next\r\n", BANKS[current_sensor], SENSOR[current_sensor]);
        while (current_sensor < SENSOR_COUNT) {
            size_t current_sensor_index = current_sensor % SENSOR_COUNT;

            uint32_t expected_group = BANKS[current_sensor_index];
            uint32_t expected_sensor = SENSOR[current_sensor_index];

            uint32_t triggered = query_sensor(expected_group);
            if (triggered == 0) continue;
            char sensor_group = (triggered / 16);
            uint8_t sensor_index = (triggered % 16)+1;
            if (sensor_group == expected_group && sensor_index == expected_sensor) {

                // uart_printf(CONSOLE, "triggered sensor: %d %d\r\n", sensor_group, sensor_index);

                prev_time = current_time;
                current_time = get_time();
                if (!(current_sensor == 0 && sample == 0)) {
                    uint32_t speed_index = (current_sensor - 1 + SENSOR_COUNT) % SENSOR_COUNT;
                    uint32_t speed = DISTANCES[speed_index]*1000000/(current_time-prev_time);
                    SPEEDS[speed_index] += speed;
                    uart_printf(CONSOLE, "Set speed %d to %d\r\n", speed_index, speed);
                }

                if (sample == SAMPLES) goto end_test;
                ++current_sensor;
                // uart_printf(CONSOLE, "expected group %d sensor %d next\r\n", BANKS[current_sensor_index], SENSOR[current_sensor_index]);
            }
        }

        uart_printf(CONSOLE, "sample %d complete\r\n", sample);
    } end_test:

    for (int i = 0; i < SENSOR_COUNT; ++i) {
        uart_printf(CONSOLE, "Speed: %d\r\n", SPEEDS[i]/SAMPLES);
    }

}

void
calibTrainStop()
{

}
