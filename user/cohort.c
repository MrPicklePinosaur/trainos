#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "cohort.h"
#include "user/path/train_data.h"
#include "user/path/track_data.h"
#include "user/switch.h"
#include "user/sensor.h"
#include "user/trainstate.h"

void
cohort_follower_regulate()
{

    // PARAMETERS =======
    // if distance deviates this much from expected, then adjust speed up or down
    const u32 ACCEL_ADJUST_TOLERANCE  = 10;
    const u32 DECCEL_ADJUST_TOLERANCE = 10;
    const u32 SPEED_RANGE_UP = 25; // +- this value of velocities to use
    const u32 SPEED_RANGE_DOWN = 50;

    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    Track* track = get_track();

    int from_tid;
    CohortFollowerRegulate msg_buf;
    struct {} reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(CohortFollowerRegulate));
    if (msg_len < 0) {
        ULOG_WARN("[FOLLOWER REGULATE] Error when receiving");
        Exit();
    }
    Reply(from_tid, (char*)&reply_buf, 0);

    usize ahead_train = msg_buf.ahead_train; 
    usize follower_train = msg_buf.follower_train; 

    Delay(clock_server, 300); // wait a bit for acceleration

    // speed bound is based off of the cohort leader
    TrainState follower_state = TrainstateGet(trainstate_server, follower_train);

    usize leader_train = follower_state.cohort;
    TrainState leader_state = TrainstateGet(trainstate_server, leader_train);

    usize leader_vel  = train_data_vel(leader_train, leader_state.speed);

    u8 follower_min_speed = get_speed_downstep(follower_train, leader_vel, SPEED_RANGE_DOWN);
    if (follower_min_speed == 0) {
        ULOG_WARN("invalid downstep speed for train %d", follower_train);
        Exit();
    }
    u8 follower_max_speed = get_speed_upstep(follower_train, leader_vel, SPEED_RANGE_UP);
    if (follower_max_speed == 0) {
        ULOG_WARN("invalid upstep speed for train %d", follower_train);
        Exit();
    }

    ULOG_INFO("train %d downstep = %d (%d) upstep = %d (%d)", follower_train, follower_min_speed, train_data_vel(follower_train, follower_min_speed), follower_max_speed, train_data_vel(follower_train, follower_max_speed));

    for (;;) {

        TrainState state = TrainstateGet(trainstate_server, ahead_train);

        // compute expected time until next trigger
        u32 ahead_train_vel = train_data_vel(ahead_train, state.speed);
        u32 follow_distance = ahead_train_vel / 2; // maintain half a second of distance between trains
        u32 dist_to_next_train = (follow_distance + TRAIN_LENGTH);

        if (ahead_train_vel == 0) {
            ULOG_WARN("ahead train is at speed zero");
            Exit();
        }

        i32 expected_time = dist_to_next_train*100/ahead_train_vel; // multiply to add a bit more percision
        //ULOG_INFO("Expect %d time between trains", expected_time);

        // wait for ahead train to get to projected next sensor
        TrackNode* ahead_current_sensor = track_node_by_sensor_id(track, state.pos);
        TrackNode* ahead_next_sensor = track_next_sensor(switch_server, track, ahead_current_sensor);
        //ULOG_DEBUG("Expect sensor %s", ahead_next_sensor->name);

        // wait for current train to get to same snesor
        // TODO not sure if good to not use sensor attribution for this
        WaitForSensor(sensor_server, ahead_next_sensor->num);
        //ULOG_DEBUG("hit sensor for ahead");
        i32 actual_time = Time(clock_server);

        // TODO we can drop sensors in this time

        WaitForSensor(sensor_server, ahead_next_sensor->num);
        //ULOG_DEBUG("hit sensor for follower");
        actual_time = Time(clock_server) - actual_time;

        ULOG_DEBUG("Took %d time between sensors, expected %d", actual_time, expected_time);

        // TODO scale speed based on deviation between real and expected?

        // compare time and see if we should speed up / slow down / maintain speed
        if (actual_time <= expected_time && expected_time-actual_time > DECCEL_ADJUST_TOLERANCE) {
            // too fast, bump down one speed
            TrainState follower_state = TrainstateGet(trainstate_server, follower_train);
            u8 new_speed = u8_max(u8_sub(follower_state.speed, 1), follower_min_speed);
            ULOG_DEBUG("Bump down follower train %d speed to %d", follower_train, new_speed);
            TrainstateSetSpeed(trainstate_server, follower_train, new_speed);
        }
        else if (expected_time <= actual_time && actual_time-expected_time > ACCEL_ADJUST_TOLERANCE) {
            // too slow, bump up one speed
            TrainState follower_state = TrainstateGet(trainstate_server, follower_train);
            u8 new_speed = u8_min(follower_state.speed+1, follower_max_speed);
            ULOG_DEBUG("Bump up follower train %d speed to %d", follower_train, new_speed);
            TrainstateSetSpeed(trainstate_server, follower_train, new_speed);
        }


    }
    Exit();

}
