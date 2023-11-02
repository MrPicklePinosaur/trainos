#include <stdint.h>
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

int kmain() {

    // initialize GPIO for both console and marklin uarts
    uart_init();
    uart_config_and_enable_marklin();
    marklin_init();

    // Clear any previous sensor detections
    marklin_dump_s88(5);
    for (int i = 0; i < BYTE_COUNT; ++i) {
        uart_getc(MARKLIN);
    }

    uint32_t BANKS[] = {3, 2, 4, 5, 5, 4, 5, 4, 2, 3, 1, 2};
    uint32_t SENSOR[] = {10, 1, 14, 14, 9, 5, 6, 4, 6, 12, 4, 16};
    size_t current_sensor = 0;

    uart_printf(CONSOLE, "expected group %d sensor %d next\r\n", BANKS[current_sensor], SENSOR[current_sensor]);
    while (current_sensor < lengthof(BANKS)) {

        uint32_t expected_group = BANKS[current_sensor];
        uint32_t expected_sensor = SENSOR[current_sensor];

        uint32_t triggered = query_sensor(expected_group);
        if (triggered == 0) continue;
        char sensor_group = (triggered / 16);
        uint8_t sensor_index = (triggered % 16)+1;
        if (sensor_group == expected_group && sensor_index == expected_sensor) {
            uart_printf(CONSOLE, "triggered sensor: %d %d\r\n", sensor_group, sensor_index);
            ++current_sensor;
            uart_printf(CONSOLE, "expected group %d sensor %d next\r\n", BANKS[current_sensor], SENSOR[current_sensor]);
        }

    }
    uart_printf(CONSOLE, "test complete\r\n");
}
