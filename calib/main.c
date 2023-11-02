#include <stdint.h>
#include "rpi.h"
#include "marklin.h"

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
                uint8_t index = (7-j);
                return i*8+index;
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

    for (;;) {

        uint32_t triggered = query_sensor(1);
        if (triggered == 0) continue;
        char sensor_group = (triggered / 16);
        uint8_t sensor_index = (triggered % 16)+1;
        uart_printf(CONSOLE, "triggered sensor: %d %d\r\n", sensor_group, sensor_index);

#if 0
        // If A3 was triggered, record the time
        if ((triggered >> 5) & 0x1) {

            // Ignore first sensor trigger; we need the train to make a full loop first
            if (start_time == 0) {
                start_time = *(volatile uint32_t*)(TIMER_BASE + 0x04);
                prev_time = start_time;
            } else {
                uint32_t time = *(volatile uint32_t*)(TIMER_BASE + 0x04);
                samples++;
                uart_printf(CONSOLE, "This: %d   Avg: %d   Samples: %d\r\n", time - prev_time, (time - start_time) / samples, samples);
                prev_time = time;
            }
        }
#endif
    }
}
