#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "cohort.h"
#include "user/path/train_data.h"
#include "user/path/track_data.h"
#include "user/switch.h"
#include "user/trainstate.h"

void
cohort_follower_regulate()
{
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
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

    TrainState state = TrainstateGet(trainstate_server, ahead_train);

    // compute expected time until next trigger
    u32 dist_to_next_train = FOLLOW_DISTANCE + TRAIN_LENGTH;
    u32 ahead_train_vel = train_data_vel(ahead_train, state.speed);

    if (ahead_train_vel == 0) {
        ULOG_WARN("ahead train is at speed zero");
        Exit();
    }

    u32 expected_time = dist_to_next_train/ahead_train_vel;
    ULOG_DEBUG("Expect %d time between trains", expected_time);

    // wait for ahead train to get to projected next sensor
    TrackNode* ahead_current_sensor = track_node_by_sensor_id(track, state.pos);
    TrackNode* ahead_next_sensor = track_next_sensor(switch_server, track, ahead_current_sensor);

    // wait for current train to get to same snesor
    u32 time_between_sensor = Time(clock_server);
    Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, ahead_train);
    if (res.second != ahead_next_sensor->num) {
        ULOG_WARN("Unexpected sensor for ahead train, wanted %s, got %s", ahead_next_sensor->name, track_node_by_sensor_id(track, res.second)->name);
        Exit();
    }

    res = TrainstateWaitForSensor(trainstate_server, follower_train);
    if (res.second != ahead_next_sensor->num) {
        ULOG_WARN("Unexpected sensor for follower train, wanted %s, got %s", ahead_next_sensor->name, track_node_by_sensor_id(track, res.second)->name);
        Exit();
    }
    time_between_sensor = Time(clock_server) - time_between_sensor;

    ULOG_DEBUG("Took %d time between sensors", time_between_sensor);
     
    // compare time and see if we should speed up / slow down / maintain speed
    Exit();

}
