
#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"

#include "user/path/train_data.h"

void
testSpeedSearch()
{
    println("Running test suite for speed upstep and downstep search -----------------");

    const usize TRAIN1 = 2;
    const usize TRAIN2 = 47;
    const usize TRAIN3 = 58;

    const u32 SPEED_RANGE = 40;

    usize leader_vel = train_data_vel(TRAIN1, 8);

    u32 train2_downstep = get_speed_downstep(TRAIN2, leader_vel, SPEED_RANGE);
    u32 train2_upstep = get_speed_upstep(TRAIN2, leader_vel, SPEED_RANGE);
    println("train %d downstep = %d (%d) upstep = %d (%d)", TRAIN2, train2_downstep, train_data_vel(TRAIN2, train2_downstep), train2_upstep, train_data_vel(TRAIN2, train2_upstep));

    u32 train3_downstep = get_speed_downstep(TRAIN3, leader_vel, SPEED_RANGE);
    u32 train3_upstep = get_speed_upstep(TRAIN3, leader_vel, SPEED_RANGE);
    println("train %d downstep = %d (%d) upstep = %d (%d)", TRAIN3, train3_downstep, train_data_vel(TRAIN3, train3_downstep), train3_upstep, train_data_vel(TRAIN3, train3_upstep));

    Exit();
}
