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
void calibTrainShortMove();
void calibTrainAcceleration();

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
        uart_printf(CONSOLE, "3: calibTrainShortMove\r\n");
        uart_printf(CONSOLE, "4: calibTrainAcceleration\r\n");
        uart_printf(CONSOLE, "========================\r\n");

        int ch = uart_getc(CONSOLE) - '0';
        switch (ch) {
            case 1:
                uart_printf(CONSOLE, "running train speed calib\r\n");
                calibTrainSpeed();
                break;
            case 2:
                uart_printf(CONSOLE, "running train stop calib\r\n");
                calibTrainStop();
                break;
            case 3:
                uart_printf(CONSOLE, "running train short move calib\r\n");
                calibTrainShortMove();
                break;
            case 4:
                uart_printf(CONSOLE, "running train acceleration calib\r\n");
                calibTrainAcceleration();
                break;
            default:
                uart_printf(CONSOLE, "invalid calib task");
        }
    }

}

void calibTrainSpeed() {
    marklin_switch_ctl(9, SWITCH_MODE_CURVED);
    marklin_switch_ctl(10, SWITCH_MODE_STRAIGHT);
    marklin_switch_ctl(15, SWITCH_MODE_CURVED);
    marklin_switch_ctl(16, SWITCH_MODE_STRAIGHT);

    const uint32_t train_number = 47;
    const uint32_t SPEEDS_TO_CALIB[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0 };
    int speed_index = 0;
    for (;;) {
        int train_speed = SPEEDS_TO_CALIB[speed_index];
        if (train_speed == 0) {
            uart_printf(CONSOLE, "All calibration completed\r\n");
            break;
        }

        uart_printf(CONSOLE, "=========== Calculating train velocity for train %d speed %d\r\n", train_number, train_speed);
        marklin_train_ctl(train_number, train_speed);

        // Clear any previous sensor detections
        marklin_dump_s88(5);
        for (int i = 0; i < BYTE_COUNT; ++i) {
            uart_getc(MARKLIN);
        }

        const uint32_t SAMPLES = 5;
        uint32_t BANKS[] = {3, 2, 4, 5, 5, 4, 5, 4, 2, 3, 1, 2};
        uint32_t SENSOR[] = {10, 1, 14, 14, 9, 5, 6, 4, 6, 12, 4, 16};
        uint64_t DISTANCES[] = {128+231, 404, 239+43, 376, 239+155+239, 376, 50+239, 404, 231+120, 333+43, 437, 50+326};
        size_t SENSOR_COUNT = 12;
        uint64_t SPEEDS[SENSOR_COUNT];
        for (int i = 0; i < SENSOR_COUNT; ++i) {
            SPEEDS[i] = 0;
        }

        uint64_t prev_time = 0;
        uint64_t current_time = 0;

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
                        uint64_t speed = DISTANCES[speed_index]*1000000000/(current_time-prev_time);
                        SPEEDS[speed_index] += speed;
                        //uart_printf(CONSOLE, "Set speed %d to %d\r\n", speed_index, speed);
                    }

                    if (sample == SAMPLES) goto end_test;
                    ++current_sensor;
                    // uart_printf(CONSOLE, "expected group %d sensor %d next\r\n", BANKS[current_sensor_index], SENSOR[current_sensor_index]);
                }
            }

            //uart_printf(CONSOLE, "sample %d complete\r\n", sample);
        } end_test:

        marklin_train_ctl(train_number, 0);
        uint32_t total = 0;
        for (int i = 0; i < SENSOR_COUNT; ++i) {
            uart_printf(CONSOLE, "Average speed (section %d): %d\r\n", i, SPEEDS[i]/SAMPLES);
            total += SPEEDS[i];
        }
        uart_printf(CONSOLE, "Average speed (overall): %d\r\n", total/(SENSOR_COUNT*SAMPLES));

        ++speed_index;
        delay(10000000);  // 10 second delay to let the train stop
    }
}

void
calibTrainStop()
{
    marklin_switch_ctl(9, SWITCH_MODE_CURVED);
    marklin_switch_ctl(10, SWITCH_MODE_STRAIGHT);
    marklin_switch_ctl(15, SWITCH_MODE_CURVED);
    marklin_switch_ctl(16, SWITCH_MODE_STRAIGHT);

    const uint32_t granularity = 15; // how many binary search cycles to perform
    const uint32_t timeout = 6000000; // timeout before we asssume that train did not make it to second sensor
    const uint32_t train_number = 2;

    uart_printf(CONSOLE, "Calculating stop time for train %d\r\n", train_number);

    const int SENSOR_1_GROUP = 3;
    const int SENSOR_1_INDEX = 10;

    const int SENSOR_2_GROUP = 5;
    const int SENSOR_2_INDEX = 14;

    const int SPEEDUP_END_GROUP = 3;
    const int SPEEDUP_END_INDEX = 12;

    uint32_t wait_time = 0;

    const uint32_t SPEEDS_TO_CALIB[] = { 10, 12, 13, 0 };
    int speed_index = 0;
    for (;;) {
        
        uint32_t lower_bound_time = 0;
        uint32_t upper_bound_time = 5000000;  // MAY CAUSE ISSUES: If upper bound is larger than twice the time the train takes to travel from sensor 1 to sensor 2, it will miss the sensor

        uint32_t train_speed = SPEEDS_TO_CALIB[speed_index];
        if (train_speed == 0) {
            break;
        }

        for (int i = 0; i < granularity; ++i) {

            // set train to max speed
            marklin_train_ctl(train_number, 14);

            // wait for sensor C12, then slow down to desired speed
            for (;;) {
                uint32_t triggered = query_sensor(SPEEDUP_END_GROUP);
                if (triggered == 0) continue;
                char sensor_group = (triggered / 16);
                uint8_t sensor_index = (triggered % 16)+1;
                if (sensor_group == SPEEDUP_END_GROUP && sensor_index == SPEEDUP_END_INDEX) {
                    break;
                }
            }
            marklin_train_ctl(train_number, train_speed);

            // how long to wait after first sensor is triggered before issuing stop commanad
            wait_time = (upper_bound_time+lower_bound_time)/2;

            // wait for sensor C10
            for (;;) {
                uint32_t triggered = query_sensor(SENSOR_1_GROUP);
                if (triggered == 0) continue;
                char sensor_group = (triggered / 16);
                uint8_t sensor_index = (triggered % 16)+1;
                if (sensor_group == SENSOR_1_GROUP && sensor_index == SENSOR_1_INDEX) {
                    break;
                }
            }

            // delay for wait time
            uint32_t trigger_time = get_time();
            while (get_time()-trigger_time < wait_time) { }

            // issue stop
            marklin_train_ctl(train_number, 0);

            trigger_time = get_time();
            // wait for sensor E14 (or timeout)
            for (;;) {

                if (get_time()-trigger_time > timeout) {
                    // if we did not reach the second sensor before timeout, then time was too short
                    lower_bound_time = wait_time;
                    uart_printf(CONSOLE, "adjusting time bound to [%d, %d]\r\n", lower_bound_time, upper_bound_time);
                    break;
                }
                // TODO lot of duplicate code
                uint32_t triggered = query_sensor(SENSOR_2_GROUP);
                if (triggered == 0) continue;
                char sensor_group = (triggered / 16);
                uint8_t sensor_index = (triggered % 16)+1;
                if (sensor_group == SENSOR_2_GROUP && sensor_index == SENSOR_2_INDEX) {
                    // if triggered, then time was too long
                    upper_bound_time = wait_time;
                    uart_printf(CONSOLE, "adjusting time bound to [%d, %d]\r\n", lower_bound_time, upper_bound_time);
                    break;
                }
            }
        }
        uart_printf(CONSOLE, "Speed %d wait time %d\r\n", train_speed, wait_time);

        ++speed_index;
    }
}

void
calibTrainShortMove()
{
    marklin_switch_ctl(9, SWITCH_MODE_CURVED);
    marklin_switch_ctl(10, SWITCH_MODE_STRAIGHT);
    marklin_switch_ctl(15, SWITCH_MODE_CURVED);
    marklin_switch_ctl(16, SWITCH_MODE_STRAIGHT);

    const uint32_t time_increment = 250000;  // 250 milliseconds
    const uint32_t start_wait = time_increment;
    const uint32_t train_number = 2;
    const uint32_t train_speed = 8;

    const uint32_t SENSOR_GROUP = 2;
    const uint32_t SENSOR_INDEX = 16;

    marklin_train_ctl(train_number, 0);
    uart_printf(CONSOLE, "Running short move measurement for train %d speed %d\r\n", train_number, train_speed);

    for (uint32_t wait = start_wait;;) {
        for (;;) {
            uart_printf(CONSOLE, "Current wait time %d, next wait time %d. Press 1 to measure current wait time, 2 to measure next wait time, 3 to send train around to B16.\r\n", wait, wait+time_increment);
            unsigned char ch = uart_getc(CONSOLE);
            if (ch == '1') {
                break;
            }
            if (ch == '2') {
                wait += time_increment;
                break;
            }
            if (ch == '3') {
                marklin_train_ctl(train_number, 11);
                for (;;) {
                    uint32_t triggered = query_sensor(SENSOR_GROUP);
                    if (triggered == 0) continue;
                    char sensor_group = (triggered / 16);
                    uint8_t sensor_index = (triggered % 16)+1;
                    if (sensor_group == SENSOR_GROUP && sensor_index == SENSOR_INDEX) {
                        break;
                    }
                }
                marklin_train_ctl(train_number, 0);
            }
        }
        uart_printf(CONSOLE, "Measuring wait time of %d\r\n", wait);

        marklin_train_ctl(train_number, train_speed);
        uint32_t start_time = get_time();
        while (get_time() - start_time <= wait) { }
        marklin_train_ctl(train_number, 0);
    }
}

void
calibTrainAcceleration()
{
    marklin_switch_ctl(9, SWITCH_MODE_CURVED);
    marklin_switch_ctl(10, SWITCH_MODE_STRAIGHT);
    marklin_switch_ctl(15, SWITCH_MODE_CURVED);
    marklin_switch_ctl(16, SWITCH_MODE_STRAIGHT);

    const uint32_t train_number = 2;

    const uint32_t SENSOR_GROUP = 5;
    const uint32_t SENSOR_INDEX = 14;

    const uint32_t SPEEDUP_END_GROUP = 3;
    const uint32_t SPEEDUP_END_INDEX = 12;

    const uint32_t SPEED_COUNT = 5;
    const uint32_t SPEEDS[] = {2, 5, 8, 11, 14};

    marklin_train_ctl(train_number, 0);
    uart_printf(CONSOLE, "Running acceleration for train %d\r\n", train_number);

    for (;;) {
        uint32_t train_speed;
        for (;;) {
            uart_printf(CONSOLE, "Once you've placed the train on C10:\r\n");
            for (uint32_t i = 0; i < SPEED_COUNT; i++) {
                uart_printf(CONSOLE, "[%d] for speed %d\r\n", i+1, SPEEDS[i]);
            }
            unsigned char ch = uart_getc(CONSOLE);
            if ('1' <= ch && ch < '1'+SPEED_COUNT) {
                train_speed = SPEEDS[ch-'1'];
                break;
            }
        }

        marklin_train_ctl(train_number, train_speed);
        uint32_t start_time = get_time();
        for (;;) {
            uint32_t triggered = query_sensor(SENSOR_GROUP);
            if (triggered == 0) continue;
            char sensor_group = (triggered / 16);
            uint8_t sensor_index = (triggered % 16)+1;
            if (sensor_group == SENSOR_GROUP && sensor_index == SENSOR_INDEX) {
                break;
            }
        }
        uint32_t end_time = get_time();
        uart_printf(CONSOLE, "Time: %d\r\n", end_time - start_time);

        // Loop back around to C10
        for (;;) {
            uint32_t triggered = query_sensor(SPEEDUP_END_GROUP);
            if (triggered == 0) continue;
            char sensor_group = (triggered / 16);
            uint8_t sensor_index = (triggered % 16)+1;
            if (sensor_group == SPEEDUP_END_GROUP && sensor_index == SPEEDUP_END_INDEX) {
                break;
            }
        }
        marklin_train_ctl(train_number, 0);
    }
}
