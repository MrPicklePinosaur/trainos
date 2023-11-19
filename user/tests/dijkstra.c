#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/path/path.h"

void
runDijkstra(Track* track, char* start_str, char* dest_str, Arena tmp)
{
    println("running dijkstra from %s to %s", start_str, dest_str);
    usize start = track_node_by_name(track, start_str) - track->nodes;
    usize dest = track_node_by_name(track, dest_str) - track->nodes;

    CBuf* path = dijkstra(track, start, dest, false, &tmp);
    if (path == NULL) {
        return;
    }
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        print("%s->%s,", edge->src->name, edge->dest->name);
    }
    print("\r\n");
}

void
testDijkstra()
{
    println("Running test suite for dijkstra -----------------");

    Arena tmp = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Track* track = get_track_a();

    runDijkstra(track, "C10", "A1", tmp);
    runDijkstra(track, "C10", "D4", tmp);
    runDijkstra(track, "C10", "D14", tmp);

    Exit();
}
