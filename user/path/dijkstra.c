#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include <traintasks.h>
#include "user/path/reserve.h"
#include "dijkstra.h"

#define INF 2147483647
#define NONE 2147483647

CBuf*
dijkstra(Track* track, usize train, u32 src, u32 dest, bool allow_reversal, bool check_reserve, Arena* arena)
{
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);

    uint32_t dist[TRACK_MAX];
    uint32_t prev[TRACK_MAX];
    TrackEdge* edges[TRACK_MAX];
    uint32_t visited[TRACK_MAX];

    ULOG_INFO_M(LOG_MASK_PATH, "RUNNING dijkstra from %s to %s", track->nodes[src].name, track->nodes[dest].name);
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
        if (curr == NONE) {
            return NULL;
        }

        visited[curr] = 1;

        // TODO what happens if the destination is reserved?
        if (curr == dest) break;

        // we don't care which direction we arrive at a sensor from
        uint32_t dest_rev = nodes[dest].reverse - nodes;
        if (curr == dest_rev) {
            dest = dest_rev;
            break;
        }

        // TODO ideally should make dijkstra not have any concept of reservations at all
        if (check_reserve) {
            // don't use this path if already reserved
            ZoneId curr_zone = nodes[curr].reverse->zone;
            if (curr_zone != -1) {
                // returns true if zone is NOT reserved by train
                if (zone_is_reserved(reserve_server, curr_zone, train)) {
                    continue;
                }
            }
        }

        if (nodes[curr].type == NODE_SENSOR || nodes[curr].type == NODE_MERGE) {
            TrackEdge* edge_ahead = &nodes[curr].edge[DIR_AHEAD];
            uint32_t ahead = edge_ahead->dest - nodes;  // Use array math to get the index of the neighbor node
            if (dist[curr] + edge_ahead->dist < dist[ahead]) {
                dist[ahead] = dist[curr] + edge_ahead->dist + edge_ahead->bias;
                prev[ahead] = curr;
                edges[ahead] = edge_ahead;
            }
        }
        else if (nodes[curr].type == NODE_BRANCH) {
            TrackEdge* edge_straight = &nodes[curr].edge[DIR_STRAIGHT];
            uint32_t straight = edge_straight->dest - nodes;
            if (dist[curr] + edge_straight->dist < dist[straight]) {
                dist[straight] = dist[curr] + edge_straight->dist + edge_straight->bias;
                prev[straight] = curr;
                edges[straight] = edge_straight;
            }

            TrackEdge* edge_curved = &nodes[curr].edge[DIR_CURVED];
            uint32_t curved = edge_curved->dest - nodes;
            if (dist[curr] + edge_curved->dist < dist[curved]) {
                dist[curved] = dist[curr] + edge_curved->dist + edge_curved->bias;
                prev[curved] = curr;
                edges[curved] = edge_curved;
            }
        }

        // also add in the reverse edge
        // reversals are only allowed on sensor nodes
        if (allow_reversal && nodes[curr].type == NODE_SENSOR) {
            TrackEdge* edge_rev = &nodes[curr].edge[DIR_REVERSE];
            uint32_t rev = nodes[curr].reverse - nodes;
            if (dist[curr] + edge_rev->dist < dist[rev]) {
                dist[rev] = dist[curr] + edge_rev->dist + edge_rev->bias;
                prev[rev] = curr;
                edges[rev] = edge_rev;
            }
        }

    }

    // return edges the train will take
    CBuf* path = cbuf_new(128);

    usize iters = 0;

    uint32_t src_rev = nodes[src].reverse - nodes;
    uint32_t back = dest;
    for (; back != src && back != src_rev; back = prev[back]) {

        TrackEdge* new_edge = arena_alloc(arena, TrackEdge);
        *new_edge = *edges[back]; 
        cbuf_push_front(path, new_edge);
        
        if (iters > 128) {
            ULOG_WARN("[dijkstra] unable to find src when constructing edge graph");
            return NULL;
        }
        ++iters;
    }

    // add an extra reverse to beginning if needed
    if (back == src_rev) {
        cbuf_push_front(path, &nodes[src].edge[DIR_REVERSE]);
    }

    return path;
}
