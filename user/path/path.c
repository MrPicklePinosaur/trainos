#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>
#include "path.h"
#include "track_data.h"
#include "train_data.h"
#include "user/marklin.h"
#include "user/sensor.h"
#include "user/switch.h"
#include "user/trainstate.h"
#include "user/path/reserve.h"

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
            ULOG_INFO_M(LOG_MASK_PATH, "Dijkstra could not find path");
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

        // don't use this path if already reserved
        ZoneId curr_zone = nodes[curr].reverse->zone;
        if (curr_zone != -1) {
            if (zone_is_reserved(curr_zone, train)) {
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
    uint32_t back = dest;
    for (; back != src && back != src_rev; back = prev[back]) {

        // reserve the path that we found
        // TODO technically there could be barging if another train reseves the task we found before we do (assuming not possible for another pathfind request to come this quickly for now)
        ZoneId zone = nodes[back].reverse->zone;
        if (zone != -1) {
            ULOG_INFO_M(LOG_MASK_PATH, "train %d reserved zone %d", train, zone);
            if (!zone_reserve(train, zone)) {
                ULOG_WARN("failed reservation");
                zone_unreserve_all(track, train);
                return NULL;
            }
        }

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

void
setSwitchesInZone(Tid switch_server, Track* track, ZoneId zone, CBuf* desired_switches)
{
    if (zone == -1) {
        ULOG_WARN("zone was -1");
        return;
    }

    TrackNode** zone_switches = (TrackNode**)track->zones[zone].switches;
    for (; *zone_switches != 0; ++zone_switches) {
        for (usize j = 0; j < cbuf_len(desired_switches); ++j) {
            Pair_u32_SwitchMode* pair = cbuf_get(desired_switches, j);
            if ((*zone_switches)->num == pair->first) {
                ULOG_INFO_M(LOG_MASK_PATH, "setting switch %d to state %d", pair->first, pair->second);
                SwitchChange(switch_server, pair->first, pair->second);
            }
        }
    }
}

// moves the train to a specified destination without using any reversals
void
patherSimplePath(Track* track, CBuf* path, usize train, usize train_speed, isize offset, Arena* arena)
{
    ULOG_INFO_M(LOG_MASK_PATH, "Executing simple path");
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        print("%s->%s,", edge->src->name, edge->dest->name);
    }
    print("\r\n");

    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    // compute which sensor to issue stop command from
    TrainState state = TrainstateGet(trainstate_server, train);
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
    ULOG_INFO_M(LOG_MASK_PATH, "Computing switch states...");
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
                ULOG_INFO("want switch %d as straight", pair->first);
            }
            else if (track_edge_cmp(edge->src->edge[DIR_CURVED], *edge)) {
                //ULOG_INFO_M(LOG_MASK_PATH, "switch %d to curved", switch_num);
                Pair_u32_SwitchMode* pair = arena_alloc(arena, Pair_u32_SwitchMode);
                pair->first = switch_num;
                pair->second = SWITCH_MODE_CURVED;
                cbuf_push_back(desired_switch_modes, pair);
                ULOG_INFO("want switch %d as curved", pair->first);
            }
            else {
                PANIC("invalid branch");
            }
        }
    }

    i32 distance_from_sensor = -stopping_distance; // distance after sensor in which to send stop command

    // Set switches in immediate zone
    ULOG_INFO_M(LOG_MASK_PATH, "Setting immediate switches...");
    TrackEdge* first_edge = ((TrackEdge*)cbuf_front(path));
    ZoneId cur_zone = first_edge->src->reverse->zone;
    ZoneId next_zone = track_next_sensor(switch_server, track, first_edge->src)->reverse->zone;
    ZoneId immediate_zones[2] = {cur_zone, next_zone};
    // switch switches in current zone and next zone
    for (usize i = 0; i < 2; ++i) {
        // set the state of switches for all zones (short move claims all nodes)
        ZoneId zone = immediate_zones[i];
        if (zone == -1) {
            ULOG_WARN("zone index %d was -1", i);
            continue;
        }

        setSwitchesInZone(switch_server, track, zone, desired_switch_modes);
    }


    if (waiting_sensor == 0 || ((TrackEdge*)cbuf_front(path))->src == waiting_sensor) {
        ULOG_INFO_M(LOG_MASK_PATH, "Executing short move...");

        TrainstateSetSpeed(trainstate_server, train, TRAIN_DATA_SHORT_MOVE_SPEED);
        Delay(clock_server, train_data_short_move_time(train, distance_to_dest) / 10);
        TrainstateSetSpeed(trainstate_server, train, 0);

    } else {
        ULOG_INFO_M(LOG_MASK_PATH, "Executing regular move...");
        ULOG_INFO_M(LOG_MASK_PATH, "sensor: %s, %d, distance: %d", waiting_sensor->name, waiting_sensor->num, distance_from_sensor);

        TrainstateSetSpeed(trainstate_server, train, train_speed);

        // start at index one since we skip the starting node (assume no short move)
        ZoneId prev_zone = -1;
        for (usize i = 1; i < cbuf_len(path); ++i) {
            TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);

            if (edge->src->type == NODE_SENSOR) {

                // wait for sensor
                // first is train, second is pos
                Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, train);
                usize new_pos = res.second;
                if (new_pos != edge->src->num) {
                    // TODO what happens if we hit an unexpected sensor? (in the case that a sensor misses the trigger)
                    // should we do some form of recovery?
                    ULOG_WARN("expect sensor %d, got sensor %d", edge->src->num, new_pos);
                }

                // check if we entered a new zone, if so, flip the switches that we need in the next zone
                TrackNode* node = track_node_by_sensor_id(track, new_pos);
                TrackNode* next_sensor = track_next_sensor(switch_server, track, node);

                // NOTE: need reverse since zones are denoted by sensors that are leaving zone
                ZoneId next_zone = next_sensor->reverse->zone;
                ULOG_INFO_M(LOG_MASK_PATH, "in zone %d", next_zone);
                setSwitchesInZone(switch_server, track, next_zone, desired_switch_modes);

                // can release previous zone now
                if (prev_zone != -1) {
                    ULOG_INFO_M(LOG_MASK_PATH, "train %d release zone %d", train, prev_zone);
                    zone_unreserve(train, prev_zone);
                }
                prev_zone = node->zone;

                // stop once we hit target sensor
                if (edge->src->num == waiting_sensor->num) break;
                
            }
        }

        // now wait before sending stop command
        ULOG_INFO_M(LOG_MASK_PATH, "Waiting to stop train...");
        u64 delay_ticks = distance_from_sensor*100/train_vel;
        Delay(clock_server, delay_ticks);

        ULOG_INFO_M(LOG_MASK_PATH, "Train stopped...");
        TrainstateSetSpeed(trainstate_server, train, 0);
        
        /* TrainstateSetOffset(trainstate_server, train, offset); */
    }

    // explicitly set position (TODO this is probably pretty dangerous)
    usize dest_ind = ((TrackEdge*)cbuf_back(path))->dest - track->nodes;
    TrainstateSetPos(trainstate_server, train, dest_ind);

    // free the path we took (but keep the place we stop at)
    zone_unreserve_all(track, train);
    ZoneId dest_zone = ((TrackEdge*)cbuf_back(path))->dest->zone;
    ULOG_INFO_M(LOG_MASK_PATH, "train stopped in zone %d", dest_zone);
    zone_reserve(train, dest_zone);

}

void
patherTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

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

    TrainstateSetDest(trainstate_server, train, dest);

    if (src == dest || src == track->nodes[dest].reverse - track->nodes) {
        ULOG_INFO_M(LOG_MASK_PATH, "[PATHER] Source equals destination");
        Exit();
    }

    Arena arena = arena_new(sizeof(TrackEdge*)*TRACK_MAX*2);

    ULOG_INFO_M(LOG_MASK_PATH, "computing path...");
    CBuf* path = dijkstra(track, train, src, dest, true, &arena);
    if (path == NULL) {
        ULOG_INFO_M(LOG_MASK_PATH, "[PATHER] dijkstra can't find path");
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
            ULOG_INFO_M(LOG_MASK_PATH, "found reversal");

            // no need to move if we are only running a reversal
            if (cbuf_len(simple_path) > 1) {
                TrainState state = TrainstateGet(trainstate_server, train);
                usize reverse_offset = (state.reversed) ? 0 : TRAIN_LENGTH ;
                patherSimplePath(track, simple_path, train, train_speed, reverse_offset, &arena);
            }

            Delay(clock_server, 400); // TODO arbritatary and questionably necessary delay
            ULOG_INFO_M(LOG_MASK_PATH, "Reversing train...");
            TrainstateReverseStatic(trainstate_server, train);
            cbuf_clear(simple_path);
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

int
pathRandomizer(void)
{
    usize train_num = 2;
    usize train_speed = 8;

    Tid path_task = WhoIs(PATH_ADDRESS);
    Track* track = get_track_a();

    for (;;) {

        usize dest = rand_int() % 80;
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
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    Track* track = get_track_a();

    PathMsg msg_buf;
    PathResp reply_buf;

    zone_init(track); // TODO wont need this once zone is an actual task

    for (;;) {
        int from_tid;
        int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(PathMsg));
        if (msg_len < 0) {
            ULOG_WARN("Error when receiving");
            continue;
        }

        TrainState trainstate = TrainstateGet(trainstate_server, msg_buf.train);
        usize start_sensor = trainstate.pos;
        TrackNode* dest = track_node_by_name(track, msg_buf.dest);
        if (dest == NULL) {
            // TODO send back error?
            ULOG_WARN("invalid destination");
            continue;
        }
        usize dest_sensor = dest - track->nodes;
        ULOG_INFO_M(LOG_MASK_PATH, "routing train %d from %d to %d", msg_buf.train, start_sensor, dest_sensor);

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

