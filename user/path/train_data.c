#include <trainstd.h>

#include "train_data.h"

uint32_t
get_train_index(uint32_t train)
{
    for (uint32_t i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++) {
        if (TRAIN_DATA_TRAINS[i] == train) {
            return i;
        }
    }
    PANIC("Invalid train for train data %u", train);
}

uint32_t
get_speed_index(uint32_t speed)
{
    for (uint32_t i = 0; i < TRAIN_DATA_SPEED_COUNT; i++) {
        if (TRAIN_DATA_SPEEDS[i] == speed) {
            return i;
        }
    }
    PANIC("Invalid speed for train data %u", speed);
}

uint32_t
train_data_vel(uint32_t train, uint32_t speed)
{
    uint32_t train_index = get_train_index(train);
    uint32_t speed_index = get_speed_index(speed);
    return TRAIN_DATA_VEL[train_index][speed_index];
}

uint32_t
train_data_stop_dist(uint32_t train, uint32_t speed)
{
    uint32_t train_index = get_train_index(train);
    uint32_t speed_index = get_speed_index(speed);
    return TRAIN_DATA_STOP_DIST[train_index][speed_index];
}

uint32_t
train_data_stop_time(uint32_t train, uint32_t speed)
{
    uint32_t train_index = get_train_index(train);
    uint32_t speed_index = get_speed_index(speed);
    return TRAIN_DATA_STOP_TIME[train_index][speed_index];
}

uint32_t
train_data_short_move_time(uint32_t train, uint32_t dist)
{
    uint32_t train_index = get_train_index(train);
    for (uint32_t i = 0; i < TRAIN_DATA_SHORT_MOVE_DIST_COUNT; i++) {
        // For exact matches, return the exact time
        if (TRAIN_DATA_SHORT_MOVE_DIST[train_index][i] == dist) {
            return i*TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
        }
        // For non matches, linearly interpolate the time from the two surrounding measurements
        if (TRAIN_DATA_SHORT_MOVE_DIST[train_index][i] > dist) {
            uint32_t prev_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][i-1];
            uint32_t next_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][i];
            uint32_t prev_time = (i-1)*TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
            uint32_t next_time = i*TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
            return prev_time + (next_time-prev_time)*(dist-prev_dist)/(next_dist-prev_dist);
        }
    }

    // For distances above the highest measured distance, linearly extrapolate based on distance between last two recorded distances
    uint32_t last_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][TRAIN_DATA_SHORT_MOVE_DIST_COUNT-1];
    uint32_t penultimate_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][TRAIN_DATA_SHORT_MOVE_DIST_COUNT-2];
    uint32_t last_time = TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT*(TRAIN_DATA_SHORT_MOVE_DIST_COUNT-1);

    return last_time + TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT*(dist-last_dist)/(last_dist-penultimate_dist);
}
