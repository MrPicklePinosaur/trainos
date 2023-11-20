#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "path.h"
#include "track_data.h"
#include "train_data.h"
#include "user/marklin.h"
#include "user/sensor.h"
#include "user/switch.h"
#include "user/trainpos.h"
#include "user/path/reserve.h"
#include "kern/gacha.h"

#define INF 2147483647
#define NONE 2147483647

// Statically allocated arrays used in Dijkstra algorithm
uint32_t dist[TRACK_MAX];
uint32_t prev[TRACK_MAX];
TrackEdge* edges[TRACK_MAX];
uint32_t visited[TRACK_MAX];

CBuf*
dijkstra(Track* track, usize train, u32 src, u32 dest, bool allow_reversal, Arena* arena)
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

        // TODO what happens if the destination is reserved?
        if (curr == dest) break;

        // we don't care which direction we arrive at a sensor from
        uint32_t dest_rev = nodes[dest].reverse - nodes;
        if (curr == dest_rev) {
            dest = dest_rev;
            break;
        }

        // don't use this path if already reserved
        ZoneId curr_zone = nodes[curr].reverse->zone;
        if (curr_zone != -1) {
            if (zone_is_reserved(curr_zone)) {
                continue;
            }
        }

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

        // also add in the reverse edge
        if (allow_reversal) {
            uint32_t rev = nodes[curr].reverse - nodes;
            if (dist[curr] < dist[rev]) {
                dist[rev] = dist[curr];
                prev[rev] = curr;
                edges[rev] = &nodes[curr].edge[DIR_REVERSE];
            }
        }

    }

    // return edges the train will take
    CBuf* path = cbuf_new(128);

    usize iters = 0;

    uint32_t src_rev = nodes[src].reverse - nodes;
    for (uint32_t back = dest; back != src && back != src_rev; back = prev[back]) {
        TrackEdge* new_edge = arena_alloc(arena, TrackEdge);
        *new_edge = *edges[back]; 
        cbuf_push_front(path, new_edge);

        // reserve the path that we found
        // TODO technically there could be barging if another train reseves the task we found before we do (assuming not possible for another pathfind request to come this quickly for now)
        ZoneId zone = nodes[back].reverse->zone;
        if (zone != -1) {
            ULOG_INFO("train %d reserved zone %d", train, zone);
            if (!zone_reserve(train, zone)) {
                ULOG_WARN("failed reservation");
                zone_unreserve_all(train);
                return NULL;
            }
        }
        
        if (iters > 128) {
            ULOG_WARN("[dijkstra] unable to find src when constructing edge graph");
            return NULL;
        }
        ++iters;
    }

    // TODO should reserve src zone too?

    return path;
}

typedef PAIR(u32, SwitchMode) Pair_u32_SwitchMode;

typedef struct {
    usize src;
    usize dest;
    usize train;
    usize train_speed;
    i32 offset;
} PatherMsg;

typedef struct {

} PatherResp;

// moves the train to a specified destination without using any reversals
void
patherSimplePath(Track* track, CBuf* path, usize train, usize train_speed, isize offset, Arena* arena)
{
    ULOG_INFO("executing simple path");
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        print("%s->%s,", edge->src->name, edge->dest->name);
    }
    print("\r\n");

    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainpos_server = WhoIs(TRAINPOS_ADDRESS);

    // compute which sensor to issue stop command from
    i32 stopping_distance = train_data_stop_dist(train, train_speed)-offset;
    i32 train_vel = train_data_vel(train, train_speed);

    // TODO it is possible to run out of path
    TrackNode* waiting_sensor = 0;
    u32 distance_to_dest = 0;
    for (usize i = usize_sub(cbuf_len(path), 1); ; --i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        stopping_distance -= edge->dist;
        distance_to_dest += edge->dist;
        if (stopping_distance <= 0 && edge->src->type == NODE_SENSOR) {
            waiting_sensor = edge->src; // sensor that we should wait to trip
            break;
        }
        if (i == 0) break;
    }

    // compute desired switch state
    CBuf* desired_switch_modes = cbuf_new(24);
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        if (edge->src->type == NODE_BRANCH) {
            // compute id of the switch
            u32 switch_num = edge->src->num;

            if (track_edge_cmp(edge->src->edge[DIR_STRAIGHT], *edge)) {
                //ULOG_INFO_M(LOG_MASK_PATH, "switch %d to straight", switch_num);
                Pair_u32_SwitchMode* pair = arena_alloc(arena, Pair_u32_SwitchMode);
                pair->first = switch_num;
                pair->second = SWITCH_MODE_STRAIGHT;
                cbuf_push_back(desired_switch_modes, pair);
                ULOG_DEBUG("want switch %d as straight", pair->first);
            }
            else if (track_edge_cmp(edge->src->edge[DIR_CURVED], *edge)) {
                //ULOG_INFO_M(LOG_MASK_PATH, "switch %d to curved", switch_num);
                Pair_u32_SwitchMode* pair = arena_alloc(arena, Pair_u32_SwitchMode);
                pair->first = switch_num;
                pair->second = SWITCH_MODE_CURVED;
                cbuf_push_back(desired_switch_modes, pair);
                ULOG_DEBUG("want switch %d as curved", pair->first);
            }
            else {
                PANIC("invalid branch");
            }
        }
    }

    i32 distance_from_sensor = -stopping_distance; // distance after sensor in which to send stop command

    if (waiting_sensor == 0 || ((TrackEdge*)cbuf_front(path))->src == waiting_sensor) {
        ULOG_INFO("[PATHER] Short move");
        for (usize i = 0; i < cbuf_len(path); ++i) {
            // set the state of switches for all zones (short move claims all nodes)
            TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);

            ZoneId zone = edge->src->reverse->zone;
            if (edge->src->type == NODE_SENSOR && zone != -1) {
                ULOG_INFO("short move starts in zone %d", zone);

                // TODO duplicated code, would like some sort of 'set switch in zone' function
                TrackNode** zone_switches = (TrackNode**)track->zones[zone].switches;
                for (; *zone_switches != 0; ++zone_switches) {
                    for (usize j = 0; j < cbuf_len(desired_switch_modes); ++j) {
                        Pair_u32_SwitchMode* pair = cbuf_get(desired_switch_modes, j);
                        if ((*zone_switches)->num == pair->first) {
                            ULOG_INFO("setting switch %d to state %d for short move", pair->first, pair->second);
                            SwitchChange(switch_server, pair->first, pair->second);
                        }
                    }
                }
            }

        }
        marklin_train_ctl(io_server, train, TRAIN_DATA_SHORT_MOVE_SPEED);
        Delay(clock_server, train_data_short_move_time(train, distance_to_dest) / 10);
        marklin_train_ctl(io_server, train, 0);
        return;
    }
    /* ULOG_INFO("switching switches..."); */

    ULOG_INFO_M(LOG_MASK_PATH, "sensor: %s, %d, distance: %d", waiting_sensor->name, waiting_sensor->num, distance_from_sensor);

    /* CBuf* stops = cbuf_new(); */

    marklin_train_ctl(io_server, train, train_speed);

    /* ULOG_INFO("routing train..."); */
    // start at index one since we skip the starting node (assume no short move)
    for (usize i = 1; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        // wait for sensor
        if (edge->src->type == NODE_SENSOR) {
            // TODO what happens if we hit an unexpected sensor? (in the case that a sensor misses the trigger)
            // block until we hit desired sensor
            // ULOG_INFO("expecting sensor %s", edge->dest->name);
            /* WaitForSensor(sensor_server, edge->dest->num); */
            TrainPosWaitResult res = trainPosWait(trainpos_server, train);
            if (res.pos != edge->src->num) {
                ULOG_WARN("expect sensor %d, got sensor %d", edge->src->num, res.pos);
            }

            // check if we entered a new zone, if so, flip the switches that we need in the next zone
            TrackNode* node = track_node_by_sensor_id(track, res.pos);
            // NOTE: need reverse since zones are denoted by sensors that are leaving zone
            ULOG_INFO("in zone %d", node->reverse->zone);

            // find next sensor
            TrackNode* next_sensor_node = NULL;
            for (usize offset = i+1; offset < cbuf_len(path); ++offset) {
                TrackEdge* offset_edge = (TrackEdge*)cbuf_get(path, offset);
                if (offset_edge->src->type == NODE_SENSOR) {
                    next_sensor_node = offset_edge->src;
                    break;
                }
            }
            if (next_sensor_node != NULL) {
                ZoneId next_zone = next_sensor_node->reverse->zone;
                if (next_zone != -1) {
                    ULOG_INFO("next in zone %d", next_zone);
                    TrackNode** zone_switches = (TrackNode**)track->zones[next_zone].switches;
                    for (; *zone_switches != 0; ++zone_switches) {
                        for (usize j = 0; j < cbuf_len(desired_switch_modes); ++j) {
                            Pair_u32_SwitchMode* pair = cbuf_get(desired_switch_modes, j);
                            if ((*zone_switches)->num == pair->first) {
                                ULOG_INFO("setting switch %d to state %d", pair->first, pair->second);
                                SwitchChange(switch_server, pair->first, pair->second);
                            }
                        }
                    }
                }
            }


            if (edge->src->num == waiting_sensor->num) break;
            
        }
    }
    
    // ULOG_INFO_M(LOG_MASK_PATH, "hit target sensor");

    // now wait before sending stop command
    u64 delay_ticks = distance_from_sensor*100/train_vel;
    Delay(clock_server, delay_ticks);

    marklin_train_ctl(io_server, train, 0);

    // ULOG_INFO_M(LOG_MASK_PATH, "stopped train");

}

void
patherTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainpos_server = WhoIs(TRAINPOS_ADDRESS);

    Track* track = get_track_a();

    int from_tid;
    PatherMsg msg_buf;
    PatherResp reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(PatherMsg));
    if (msg_len < 0) {
        ULOG_WARN("[PATHER] Error when receiving");
        Exit();
    }
    reply_buf = (PatherResp){};
    Reply(from_tid, (char*)&reply_buf, sizeof(PatherResp));

    usize src = msg_buf.src;
    usize dest = msg_buf.dest;
    usize train = msg_buf.train;
    usize train_speed = msg_buf.train_speed;
    i32 offset = msg_buf.offset; // TODO ignored for now

    if (src == dest || src == track->nodes[dest].reverse - track->nodes) {
        ULOG_INFO("[PATHER] Source equals destination");
        Exit();
    }

    Arena arena = arena_new(sizeof(TrackEdge*)*TRACK_MAX*2);
    zone_init(); // TODO wont need this once zone is an actual task

    ULOG_INFO("computing path...");
    CBuf* path = dijkstra(track, train, src, dest, true, &arena);
    if (path == NULL) {
        ULOG_WARN("[PATHER] dijkstra can't find path");
        arena_release(&arena);
        Exit();
    }

    // check if offset is valid
    TrackNode dest_node = track->nodes[dest];

    // TODO currently not allowed to use offsets too large or too small (greater than next node, less that prev node), and also can't offset off of nodes other than sensors
    if (offset != 0 && dest_node.type != NODE_SENSOR) {
        ULOG_WARN("[PATHER] can't use offset from node other than sensor");
        arena_release(&arena);
        Exit();
    }
    i32 max_fwd_dist = dest_node.edge[DIR_AHEAD].dist;
    if (offset > 0 && offset > max_fwd_dist) {
        ULOG_WARN("[PATHER] forward offset too large (max value for node %s is %d)", dest_node.name, max_fwd_dist);
        arena_release(&arena);
        Exit();
    }
    i32 max_bck_dist = dest_node.reverse->edge[DIR_AHEAD].dist;
    if (offset < 0 && -offset > max_bck_dist) {
        ULOG_WARN("[PATHER] backward offset too large (max value for node %s is %d)", dest_node.name, max_bck_dist);
        arena_release(&arena);
        Exit();
    }

    // break path into simple paths (no reversals)
    CBuf* simple_path = cbuf_new(128);
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* cur_edge = cbuf_get(path, i);
        cbuf_push_back(simple_path, cur_edge);
        // check for reversal
        if (cur_edge->type == EDGE_REVERSE) {
            ULOG_INFO("found reversal");
            // insert an extra edge to give some buffer if there are switches
            TrackEdge* extra_edge = arena_alloc(&arena, TrackEdge);
            *extra_edge = *track_next_edge(switch_server, track, cur_edge->src);
            cbuf_push_back(simple_path, extra_edge);

            ULOG_INFO("inserting extra edge %s->%s", extra_edge->src->name, extra_edge->dest->name);
            patherSimplePath(track, simple_path, train, train_speed, offset+TRAIN_LENGTH, &arena);

            Delay(clock_server, 400); // TODO arbritatary and questionably necessary delay
            marklin_train_ctl(io_server, train, SPEED_REVERSE);
            cbuf_clear(simple_path);

            cbuf_push_back(simple_path, extra_edge->reverse); // also need to add the extra edge in next path
            ULOG_INFO("sending reverse to train");
        }
    }
    if (cbuf_len(simple_path) > 0) {
        patherSimplePath(track, simple_path, train, train_speed, offset, &arena);
    }

    arena_release(&arena);
    Exit();
}

typedef struct {
    u32 train;
    u32 speed;
    i32 offset;
    char* dest;
} PathMsg;

typedef struct {
    Tid pather;
} PathResp;

void
pathRandomizer(void)
{
    usize train_num = 2;
    usize train_speed = 8;

    Tid path_task = WhoIs(PATH_ADDRESS);
    Track* track = get_track_a();

    for (;;) {

        usize dest = randint() % 80;
        // There's a deadspot in this corner that will trap trains in it
        if (dest == 0 || dest == 1 || dest == 12 || dest == 13 || dest == 14 || dest == 15) {
            continue;
        }

        PathResp resp_buf;
        PathMsg send_buf = (PathMsg) {
            .train = train_num,
            .speed = train_speed,
            .offset = 0,
            .dest = track->nodes[dest].name,
        };

        int ret = Send(path_task, (const char*)&send_buf, sizeof(PathMsg), (char*)&resp_buf, sizeof(PathResp));
        if (ret < 0) {
            ULOG_WARN("pathRandomizer's Send() errored");
            return -1;
        }
        WaitTid(resp_buf.pather);
    }
}

void
pathTask(void)
{
    RegisterAs(PATH_ADDRESS); 

    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid trainpos_server = WhoIs(TRAINPOS_ADDRESS);

    Track* track = get_track_a();

    PathMsg msg_buf;
    PathResp reply_buf;

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(PathMsg));
        if (msg_len < 0) {
            ULOG_WARN("Error when receiving");
            continue;
        }

        isize start_sensor = trainPosQuery(trainpos_server, msg_buf.train).pos;
        TrackNode* dest = track_node_by_name(track, msg_buf.dest);
        if (dest == NULL) {
            // TODO send back error?
            ULOG_WARN("invalid destination");
            continue;
        }
        usize dest_sensor = dest - track->nodes;
        ULOG_INFO("routing train %d from %d to %d", msg_buf.train, start_sensor, dest_sensor);

        PatherResp resp_buf;
        PatherMsg send_buf = (PatherMsg) {
            .src = start_sensor,
            .dest = dest_sensor,
            .train = msg_buf.train,
            .train_speed = msg_buf.speed,
            .offset = msg_buf.offset
        };
        int pather_task = Create(2, &patherTask, "Pather Task");
        reply_buf = (PathResp){
            .pather = pather_task
        };
        Reply(from_tid, (char*)&reply_buf, sizeof(PathResp));
        Send(pather_task, (const char*)&send_buf, sizeof(PatherMsg), (char*)&resp_buf, sizeof(PatherResp));
    }

    Exit();
}

int
PlanPath(Tid path_tid, u32 train, u32 speed, i32 offset, char* dest)
{
    PathResp resp_buf;
    PathMsg send_buf = (PathMsg) {
        .train = train,
        .speed = speed,
        .offset = offset,
        .dest = dest
    };

    int ret = Send(path_tid, (const char*)&send_buf, sizeof(PathMsg), (char*)&resp_buf, sizeof(PathResp));

    return ret;
}

