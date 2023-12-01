#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "cohort.h"
#include "user/path/train_data.h"
#include "user/path/track_data.h"
#include "user/switch.h"
#include "user/sensor.h"
#include "user/trainstate.h"

// if distance deviates this much from expected, then adjust speed up or down
#define ADJUST_TOLERANCE 0

void
cohort_follower_regulate()
{
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

    for (;;) {
        TrainState state = TrainstateGet(trainstate_server, ahead_train);

        // compute expected time until next trigger
        u32 dist_to_next_train = (FOLLOW_DISTANCE + TRAIN_LENGTH)*100; // multiply to add a bit more percision
        u32 ahead_train_vel = train_data_vel(ahead_train, state.speed);

        if (ahead_train_vel == 0) {
            ULOG_WARN("ahead train is at speed zero");
            Exit();
        }

        i32 expected_time = dist_to_next_train/ahead_train_vel;
        ULOG_INFO("Expect %d time between trains", expected_time);

        // wait for ahead train to get to projected next sensor
        TrackNode* ahead_current_sensor = track_node_by_sensor_id(track, state.pos);
        TrackNode* ahead_next_sensor = track_next_sensor(switch_server, track, ahead_current_sensor);
        ULOG_DEBUG("Expect sensor %s", ahead_next_sensor->name);

        // wait for current train to get to same snesor
        // TODO not sure if good to not use sensor attribution for this
        WaitForSensor(sensor_server, ahead_next_sensor->num);
        ULOG_DEBUG("hit sensor for ahead");
        i32 actual_time = Time(clock_server);

        WaitForSensor(sensor_server, ahead_next_sensor->num);
        ULOG_DEBUG("hit sensor for follower");
        actual_time = Time(clock_server) - actual_time;

        ULOG_DEBUG("Took %d time between sensors", actual_time);

        // compare time and see if we should speed up / slow down / maintain speed
        if (actual_time <= expected_time && expected_time-actual_time > ADJUST_TOLERANCE) {
            // too fast, bump down one speed
            TrainState follower_state = TrainstateGet(trainstate_server, follower_train);
            u8 new_speed = u8_max(u8_sub(follower_state.speed, 1), 1);
            ULOG_DEBUG("Bump down follower speed to %d", new_speed);
            TrainstateSetSpeed(trainstate_server, follower_train, new_speed);
        }
        else if (expected_time <= actual_time && actual_time-expected_time > ADJUST_TOLERANCE) {
            // too slow, bump up one speed
            TrainState follower_state = TrainstateGet(trainstate_server, follower_train);
            u8 new_speed = u8_min(follower_state.speed+1, 14);
            ULOG_DEBUG("Bump up follower speed to %d", new_speed);
            TrainstateSetSpeed(trainstate_server, follower_train, new_speed);
        }


    }
    Exit();

}
