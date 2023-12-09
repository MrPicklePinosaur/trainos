#include <trainstd.h>

#include "train_data.h"

u32
get_train_index(u32 train)
{
    for (u32 i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++) {
        if (TRAIN_DATA_TRAINS[i] == train) {
            return i;
        }
    }
    PANIC("Invalid train for train data %u", train);
}

bool
train_is_supported(usize train)
{
    for (u32 i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++) {
        if (TRAIN_DATA_TRAINS[i] == train) {
            return true;
        }
    }
    return false;
}

void
_speed_bounds_check(u32 speed)
{
    if (speed >= 15) { PANIC("Invalid speed for train data %u", speed); }
}

u32
train_data_vel(u32 train, u32 speed)
{
    _speed_bounds_check(speed);
    u32 train_index = get_train_index(train);
    return TRAIN_DATA_VEL[train_index][speed];
}

u32
train_data_stop_dist(u32 train, u32 speed)
{
    _speed_bounds_check(speed);
    u32 train_index = get_train_index(train);
    return TRAIN_DATA_STOP_DIST[train_index][speed];
}

u32
train_data_stop_time(u32 train, u32 speed)
{
    _speed_bounds_check(speed);
    u32 train_index = get_train_index(train);
    if (TRAIN_DATA_VEL[train_index][speed] == 0) {
        return 0;
    }
    return TRAIN_DATA_STOP_DIST[train_index][speed]*2000/TRAIN_DATA_VEL[train_index][speed];
}

u32
train_data_short_move_time(u32 train, u32 dist)
{
    u32 train_index = get_train_index(train);
    for (u32 i = 0; i < TRAIN_DATA_SHORT_MOVE_DIST_COUNT; i++) {
        // For exact matches, return the exact time
        if (TRAIN_DATA_SHORT_MOVE_DIST[train_index][i] == dist) {
            return i*TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
        }
        // For non matches, linearly interpolate the time from the two surrounding measurements
        if (TRAIN_DATA_SHORT_MOVE_DIST[train_index][i] > dist) {
            u32 prev_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][i-1];
            u32 next_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][i];
            u32 prev_time = (i-1)*TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
            u32 next_time = i*TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
            return prev_time + (next_time-prev_time)*(dist-prev_dist)/(next_dist-prev_dist);
        }
    }

    // For distances above the highest measured distance, linearly extrapolate based on distance between last two recorded distances
    u32 last_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][TRAIN_DATA_SHORT_MOVE_DIST_COUNT-1];
    u32 penultimate_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][TRAIN_DATA_SHORT_MOVE_DIST_COUNT-2];
    u32 last_time = TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT*(TRAIN_DATA_SHORT_MOVE_DIST_COUNT-1);

    return last_time + TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT*(dist-last_dist)/(last_dist-penultimate_dist);

}

u32
train_data_acceleration_dist(u32 train, u32 speed)
{
    _speed_bounds_check(speed);
    u32 train_index = get_train_index(train);
    return TRAIN_DATA_ACCELERATION_DIST[train_index][speed];
}

u32
get_safe_speed(u32 train, u32 velocity)
{
    for (usize speed = 14; speed >= 2; --speed) {
        usize follower_vel = train_data_vel(train, speed); 
        if (follower_vel <= velocity) {
            return speed;
        }
    }
    return 0;
}

u32
get_speed_upstep(u32 train, u32 velocity, i32 bound)
{
    i32 target_vel = velocity+bound;
    for (usize speed = 14; speed >= 2; --speed) {
        i32 follower_vel = train_data_vel(train, speed); 
        if (follower_vel <= target_vel) {
            // now check if we are closer to next speed or current speed
            u32 down_diff = i32_abs(target_vel-follower_vel);
            u8 next_speed = u8_min(speed+1, 14);
            u32 up_diff = i32_abs(target_vel-train_data_vel(train, next_speed));
            return (down_diff <= up_diff) ? speed : next_speed;
        }
    }
    return 0;
}

u32
get_speed_downstep(u32 train, u32 velocity, i32 bound)
{
    i32 target_vel = i32_max(velocity-bound, 0);

    for (usize speed = 2; speed < 15; ++speed) {
        i32 follower_vel = train_data_vel(train, speed); 
        if (follower_vel >= target_vel) {
            // now check if we are closer to lower speed or current speed
            u32 up_diff = i32_abs(target_vel-follower_vel);
            u8 prev_speed = u8_max(speed-1, 5);
            u32 down_diff = i32_abs(target_vel-train_data_vel(train, prev_speed));
            return (down_diff <= up_diff) ? prev_speed : speed;
        }
    }
    return 0;
}
