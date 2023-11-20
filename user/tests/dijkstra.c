#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/path/path.h"
#include "user/path/reserve.h"

void
runDijkstra(Track* track, usize train, char* start_str, char* dest_str, bool allow_reversal, Arena tmp)
{
    println("running dijkstra from %s to %s", start_str, dest_str);
    usize start = track_node_by_name(track, start_str) - track->nodes;
    usize dest = track_node_by_name(track, dest_str) - track->nodes;

    CBuf* path = dijkstra(track, train, start, dest, allow_reversal, &tmp);
    if (path == NULL) {
        return;
    }
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        print("%s->%s [type %d]\r\n", edge->src->name, edge->dest->name, edge->type);
    }
}

void
testDijkstra()
{
    println("Running test suite for dijkstra -----------------");

    Arena tmp = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Track* track = get_track_a();

    zone_init(track);

    runDijkstra(track, 2, "C10", "A1", false, tmp);
    zone_unreserve_all(track, 2);

    runDijkstra(track, 2, "C10", "D4", false, tmp);
    zone_unreserve_all(track, 2);

    runDijkstra(track, 2, "C10", "D14", false, tmp);
    zone_unreserve_all(track, 2);

    println("test pathfinding with reverse");
    runDijkstra(track, 2, "C10", "A1", true, tmp);
    zone_unreserve_all(track, 2);

    // test pathfinding with multiple trains
    println("test multiple trains pathfinding");
    runDijkstra(track, 2, "B1", "D14", true, tmp);
    runDijkstra(track, 3, "B16", "E9", true, tmp);
    zone_unreserve_all(track, 2);
    zone_unreserve_all(track, 3);

    Exit();
}
