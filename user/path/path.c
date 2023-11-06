#include <trainstd.h>
#include "path.h"
#include "track_data.h"
#include "../include/trainstd.h"
#include <stdint.h>

#define INF 2147483647
#define NONE 2147483647

// Statically allocated arrays used in Dijkstra algorithm
uint32_t dist[TRACK_MAX];
uint32_t prev[TRACK_MAX];
TrackEdge* edges[TRACK_MAX];
uint32_t visited[TRACK_MAX];

TrackEdge**
dijkstra(Track* track, uint32_t src, uint32_t dest, Arena* arena)
{
    TrackNode* nodes = track->nodes;

    for (uint32_t i = 0; i < TRACK_MAX; i++) {
        dist[i] = INF;
        prev[i] = NONE;
        edges[i] = 0;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (;;) {
        // Find the unvisited node with the least distance to it
        // O(n), can be improved to O(log(n)) if we use a priority queue
        // We assume that any part of the track can be reached from any other part of the track
        uint32_t curr = NONE;
        for (uint32_t i = 0; i < TRACK_MAX; i++) {
            if (dist[i] != INF && visited[i] == 0) {
                if (curr == NONE) {
                    curr = i;
                }
                else {
                    curr = dist[curr] < dist[i] ? curr : i;
                }
            }
        }

        visited[curr] = 1;

        if (curr == dest) break;

        // we don't care which direction we arrive at a sensor from
        uint32_t rev = nodes[curr].reverse - nodes;
        if (rev == dest) break;

        if (nodes[curr].type == NODE_SENSOR || nodes[curr].type == NODE_MERGE) {
            TrackEdge* edge_ahead = &nodes[curr].edge[DIR_AHEAD];
            uint32_t ahead = edge_ahead->dest - nodes;  // Use array math to get the index of the neighbor node
            if (dist[curr] + edge_ahead->dist < dist[ahead]) {
                dist[ahead] = dist[curr] + edge_ahead->dist;
                prev[ahead] = curr;
                edges[ahead] = edge_ahead;
            }
        }
        else if (nodes[curr].type == NODE_BRANCH) {
            TrackEdge* edge_straight = &nodes[curr].edge[DIR_STRAIGHT];
            uint32_t straight = edge_straight->dest - nodes;
            if (dist[curr] + edge_straight->dist < dist[straight]) {
                dist[straight] = dist[curr] + edge_straight->dist;
                prev[straight] = curr;
                edges[straight] = edge_straight;
            }

            TrackEdge* edge_curved = &nodes[curr].edge[DIR_CURVED];
            uint32_t curved = edge_curved->dest - nodes;
            if (dist[curr] + edge_curved->dist < dist[curved]) {
                dist[curved] = dist[curr] + edge_curved->dist;
                prev[curved] = curr;
                edges[curved] = edge_curved;
            }
        }
    }

    // return edges the train will take
    TrackEdge** path_start = arena_alloc(arena, TrackEdge*);
    TrackEdge** path = path_start;
    for (uint32_t back = dest; back != src; back = prev[back]) {
        *path = edges[back]; 
        path = arena_alloc(arena, TrackEdge*);
    }
    *path = NULL;


    return path_start;
}

void
Pathfind(const char* start, const char* end)
{

}

void
pathTask(void)
{
    Arena arena = arena_new(sizeof(TrackNode)*TRACK_MAX*8);
    Arena path_arena = arena_new(sizeof(usize)*TRACK_MAX);

    Track track_a = track_a_init(&arena);

    usize src = (usize)map_get(&track_a.map, str8("C10"), &arena);
    usize dest = (usize)map_get(&track_a.map, str8("D4"), &arena);

    TrackEdge** path_start = dijkstra(&track_a, src, dest, &path_arena); // -1 terminated array

    // compute which sensor to issue stop command from
    i32 stopping_distance = 500;
    // TODO it is possible to run out of path
    TrackEdge** path = path_start;
    for (; *path != NULL; ++path) {
        stopping_distance -= (*path)->dist;
        if (stopping_distance <= 0 && (*path)->src->type == NODE_SENSOR)
            break;
    }

    TrackNode* waiting_sensor = (*path)->src; // sensor that we should wait to trip
    u32 distance_from_sensor = -stopping_distance; // distance after sensor in which to send stop command

    PRINT("sensor: %s, disance: %d", waiting_sensor->name, distance_from_sensor);

    // compute desired switch state
    path = path_start;
    for (; *path != NULL; ++path) {
        if ((*path)->src->type == NODE_BRANCH) {
            // compute id of the switch
            str8 switch_name = str8_from_cstr((*path)->src->name);
            PRINT("switch name %s", switch_name);
            switch_name = str8_substr(switch_name, 2, str8_len(switch_name));
            u32 switch_num = str8_to_u64(switch_name);

            if (&(*path)->src->edge[DIR_STRAIGHT] == *path) {
                PRINT("switch %d to straight", switch_num);
            }
            else if (&(*path)->src->edge[DIR_CURVED] == *path) {
                PRINT("switch %d to curved", switch_num);

            }
            else {
                UNREACHABLE("invalid branch");
            }
        }
    }



    // should be receiver

    // find path (sequence of switches) for train to take given current train state

    // using calibrated train data, calculate stopping time for train (measured in a delay after a sensor being triggered)



    Exit();
}
