#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"

#include "user/path/track_data.h"

void
testTraindata()
{
    println("Running test suite for train data -----------------");

    Track* track = get_track_a();

    TEST(strcmp(track_node_by_name(track, "A1")->name, "A1") == 0);
    TEST(strcmp(track_node_by_name(track, "A16")->name, "A16") == 0);
    TEST(strcmp(track_node_by_name(track, "B1")->name, "B1") == 0);

    TEST(strcmp(track_node_by_sensor_id(track, 0)->name, "A1") == 0);
    TEST(strcmp(track_node_by_sensor_id(track, 15)->name, "A16") == 0);
    TEST(strcmp(track_node_by_sensor_id(track, 16)->name, "B1") == 0);

    TEST(strcmp(track_node_by_branch_id(track, 1)->name, "BR1") == 0);
    TEST(strcmp(track_node_by_branch_id(track, 18)->name, "BR18") == 0);
    TEST(strcmp(track_node_by_branch_id(track, 153)->name, "BR153") == 0);

    for (usize i = 0; i < 20; ++i) {
        println("the zone of node %s is %d", track->nodes[i].name, track->nodes[i].zone);
        
    }

    Exit();
}
