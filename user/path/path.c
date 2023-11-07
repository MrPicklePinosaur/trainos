#include <trainstd.h>
#include <trainsys.h>
#include "path.h"
#include "track_data.h"
#include "../nameserver.h"
#include "../io.h"
#include "../ui/marklin.h"
#include "train_data.h"
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
calculatePath(Tid io_server, Track* track, usize src, usize dest, usize train_ind, usize train_speed, Arena tmp)
{

    TrackEdge** path_start = dijkstra(track, src, dest, &tmp); // -1 terminated array

    // compute which sensor to issue stop command from
    i32 stopping_distance = TRAIN_DATA_STOP_DIST[train_ind][train_speed];
    // TODO it is possible to run out of path
    TrackEdge** path = path_start;
    for (; *path != NULL; ++path) {
        stopping_distance -= (*path)->dist;
        if (stopping_distance <= 0 && (*path)->src->type == NODE_SENSOR)
            break;
    }

    TrackNode* waiting_sensor = (*path)->src; // sensor that we should wait to trip
    i32 distance_from_sensor = -stopping_distance; // distance after sensor in which to send stop command

    //ULOG_INFO_M(LOG_MASK_PATH, "sensor: %s, distance: %d", waiting_sensor->name, distance_from_sensor);

    // compute desired switch state
    path = path_start;
    for (; *path != NULL; ++path) {
        if ((*path)->src->type == NODE_BRANCH) {
            // compute id of the switch
            str8 switch_name = str8_from_cstr((*path)->src->name);
            switch_name = str8_substr(switch_name, 2, str8_len(switch_name));
            u32 switch_num = str8_to_u64(switch_name);

            if (&(*path)->src->edge[DIR_STRAIGHT] == *path) {
                //ULOG_INFO_M(LOG_MASK_PATH, "switch %d to straight", switch_num);
                marklin_switch_ctl(io_server, switch_num, SWITCH_MODE_STRAIGHT);
            }
            else if (&(*path)->src->edge[DIR_CURVED] == *path) {
                //ULOG_INFO_M(LOG_MASK_PATH, "switch %d to curved", switch_num);
                marklin_switch_ctl(io_server, switch_num, SWITCH_MODE_CURVED);
            }
            else {
                PANIC("invalid branch");
            }
        }
    }

}

typedef struct {
    u32 train;
    char* dest;
} PathMsg;

typedef struct {

} PathResp;

void
pathTask(void)
{
    RegisterAs(PATH_ADDRESS); 
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);

    Arena arena = arena_new(sizeof(TrackNode)*TRACK_MAX+sizeof(Map)*TRACK_MAX*4);
    Arena tmp = arena_new(sizeof(TrackEdge*)*TRACK_MAX*2);

    Track track = track_a_init(&arena);

    PathMsg msg_buf;
    PathResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(PathMsg));
        if (msg_len < 0) {
            ULOG_WARN("Error when receiving");
            continue;
        }

        char* start = "C10";

        // find where the train currently is

        usize src = (usize)map_get(&track.map, str8_from_cstr(start), &arena);
        usize dest = (usize)map_get(&track.map, str8_from_cstr(msg_buf.dest), &arena);
        calculatePath(io_server, &track, src, dest, TRAIN_2, TRAIN_SPEED_HIGH, tmp); 

        reply_buf = (PathResp){};        
        Reply(from_tid, (char*)&reply_buf, sizeof(PathResp));
    }

    Exit();
}

int
PlanPath(Tid path_tid, u32 train, char* dest)
{
    PathResp resp_buf;
    PathMsg send_buf = (PathMsg) {
        .train = train,
        .dest = dest
    };

    int ret = Send(path_tid, (const char*)&send_buf, sizeof(PathMsg), (char*)&resp_buf, sizeof(PathResp));

    return ret;
}

