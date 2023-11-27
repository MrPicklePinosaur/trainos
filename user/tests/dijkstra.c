#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "tester.h"
#include "user/path/path.h"
#include "user/path/reserve.h"
#include "user/path/dijkstra.h"

void
runDijkstra(Tid reserve_server, Track* track, usize train, char* start_str, char* dest_str, bool allow_reversal, bool check_reserve, Arena tmp)
{
    /* println("running dijkstra from %s to %s", start_str, dest_str); */
    usize start = track_node_by_name(track, start_str) - track->nodes;
    usize dest = track_node_by_name(track, dest_str) - track->nodes;

    CBuf* path = dijkstra(track, train, start, dest, allow_reversal, check_reserve, &tmp);
    if (path == NULL) {
        ULOG_WARN("couldn't find path");
        return;
    }
    if (check_reserve) {
        for (usize i = 0; i < cbuf_len(path); ++i) {
            TrackEdge* edge = cbuf_get(path, i);
            ZoneId zone = edge->dest->reverse->zone; // TODO should also reserve start?
            if (zone != -1) {
                if (!zone_reserve(reserve_server, train, zone)) {
                    ULOG_WARN("failed reservation waiting for zone %d", zone);
                    zone_unreserve_all(reserve_server, train);
                    return;
                }
                ULOG_INFO_M(LOG_MASK_PATH, "train %d reserved zone %d", train, zone);
            }
        }
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
    Tid reserver_server = WhoIs(RESERVE_ADDRESS);

    Arena tmp = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Track* track = get_track();

    ULOG_INFO("test basic pathfinding");
    runDijkstra(reserver_server, track, 2, "C10", "A1", false, false, tmp);
    zone_unreserve_all(reserver_server, 2);

    runDijkstra(reserver_server, track, 2, "C10", "D4", false, false, tmp);
    zone_unreserve_all(reserver_server, 2);

    runDijkstra(reserver_server, track, 2, "C10", "D14", false, false, tmp);
    zone_unreserve_all(reserver_server, 2);

    ULOG_INFO("test pathfinding with reverse");
    runDijkstra(reserver_server, track, 2, "C10", "A1", true, false, tmp);
    zone_unreserve_all(reserver_server, 2);

    runDijkstra(reserver_server, track, 2, "C12", "A5", true, false, tmp);
    zone_unreserve_all(reserver_server, 2);

    // test pathfinding with multiple trains
    ULOG_INFO("test multiple trains pathfinding");
    runDijkstra(reserver_server, track, 2, "B1", "D14", true, true, tmp);
    runDijkstra(reserver_server, track, 3, "B16", "E9", true, true, tmp);
    zone_unreserve_all(reserver_server, 2);
    zone_unreserve_all(reserver_server, 3);

    // test an impossible path
    ULOG_INFO("test impossible path");
    runDijkstra(reserver_server, track, 2, "B1", "A3", true, true, tmp);
    runDijkstra(reserver_server, track, 3, "D9", "D5", true, true, tmp);
    runDijkstra(reserver_server, track, 4, "C15", "B5", true, true, tmp);
    zone_unreserve_all(reserver_server, 2);
    zone_unreserve_all(reserver_server, 3);
    zone_unreserve_all(reserver_server, 4);

    Exit();
}
