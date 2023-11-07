#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/path/path.h"

void
testDijkstra()
{
    println("Running test suite for dijkstra -----------------");

    Arena arena = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Arena tmp = arena_new(sizeof(TrackEdge*)*TRACK_MAX*2);

    Track track = track_a_init(&arena);

    usize start = (usize)map_get(&track.map, str8("C2"), &arena);
    usize dest = (usize)map_get(&track.map, str8("E2"), &arena);
    TrackEdge** path = dijkstra(&track, start, dest, &tmp);
    for (; *path != NULL; ++path) {
        print("%s,", (*path)->dest->name);
    }
    print("\n");

    Exit();
}
