#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/path/path.h"

void
runDijkstra(Track track, str8 start_str, str8 dest_str, Arena* arena, Arena* tmp)
{
    println("running dijkstra from %s to %s", str8_to_cstr(start_str), str8_to_cstr(dest_str));
#if 0
    usize start = (usize)map_get(&track.map, start_str, arena);
    usize dest = (usize)map_get(&track.map, dest_str, arena);
    TrackEdge** path = dijkstra(&track, start, dest, true, tmp);
    if (path == NULL) {
        return;
    }
    for (; *path != NULL; ++path) {
        print("%s,", (*path)->dest->name);
    }
    print("\r\n");
#endif
}

void
testDijkstra()
{
    println("Running test suite for dijkstra -----------------");

#if 0
    Arena arena = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Arena tmp = arena_new(sizeof(TrackEdge*)*TRACK_MAX*2);

    Track track = track_a_init(&arena);

    runDijkstra(track, str8("C10"), str8("A1"), &arena, &tmp);
    runDijkstra(track, str8("C10"), str8("D4"), &arena, &tmp);
    runDijkstra(track, str8("C10"), str8("D14"), &arena, &tmp);

#endif
    Exit();
}
