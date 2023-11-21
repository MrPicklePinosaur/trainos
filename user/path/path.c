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
#include "user/path/dijkstra.h"

typedef struct {
    usize src;
    usize dest;
    usize train;
    usize train_speed;
    i32 offset;
} PatherMsg;

typedef struct {

} PatherResp;

typedef PAIR(u32, SwitchMode) Pair_u32_SwitchMode;

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

// attempts to reserve as much of the path as possible
bool
reserveZonesInPath(Tid reserve_server, usize train, CBuf* path)
{
    for (usize i = 0; i < cbuf_len(path); ++i) {
        // reserve the path that we found
        // TODO technically there could be barging if another train reseves the task we found before we do (assuming not possible for another pathfind request to come this quickly for now)
        TrackEdge* edge = cbuf_get(path, i);
        ZoneId zone = edge->dest->reverse->zone; // TODO should also reserve start?
        if (zone != -1) {
            if (!zone_reserve(reserve_server, train, zone)) {
                ULOG_WARN("failed reservation waiting for zone %d", zone);
                // TODO do something about this?
                /* zone_unreserve_all(reserve_server,  train); */
                /* return false; */
                zone_wait(reserve_server, train, zone);
                if (!zone_reserve(reserve_server, train, zone)) {
                    PANIC("should have claimed zone here");
                }
            }
            ULOG_INFO_M(LOG_MASK_PATH, "train %d reserved zone %d", train, zone);
        }
    }
    return true;
}

// moves the train to a specified destination without using any reversals
void
patherSimplePath(Track* track, CBuf* path, usize train, usize train_speed, isize offset, Arena* arena)
{
    ULOG_INFO_M(LOG_MASK_PATH, "Executing simple path offset %d", offset);
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
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);

    // compute which sensor to issue stop command from
    TrainState state = TrainstateGet(trainstate_server, train);
    i32 stopping_distance = train_data_stop_dist(train, train_speed)-offset;
    i32 train_vel = train_data_vel(train, train_speed);
    ULOG_INFO_M(LOG_MASK_PATH, "train %d vel is %d", train, train_vel);

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
    /* ULOG_INFO_M(LOG_MASK_PATH, "Computing switch states..."); */
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
    /* ULOG_INFO_M(LOG_MASK_PATH, "Setting immediate switches..."); */
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

#if 0
        TrainState state = TrainstateGet(trainstate_server, train);
        // short move is impossible
        if (state.offset > distance_to_dest) {
            ULOG_WARN("short move not possible");
        } else {
            ULOG_INFO("short move with offset %d", state.offset);
            TrainstateSetSpeed(trainstate_server, train, TRAIN_DATA_SHORT_MOVE_SPEED);
            Delay(clock_server, train_data_short_move_time(train, distance_to_dest-state.offset) / 10);
            TrainstateSetSpeed(trainstate_server, train, 0);
        }
#endif
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
                usize new_pos = 0;
                for (;;) {
                    Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, train);
                    new_pos = res.second;
                    if (new_pos == edge->src->num) {
                        break;
                    }
                    // TODO what happens if we hit an unexpected sensor? (in the case that a sensor misses the trigger)
                    // should we do some form of recovery?
                    ULOG_WARN("expect sensor %s, got sensor %s", edge->src->name, track->nodes[new_pos].name);
                }

                // check if we entered a new zone, if so, flip the switches that we need in the next zone
                TrackNode* node = track_node_by_sensor_id(track, new_pos);
                TrackNode* next_sensor = track_next_sensor(switch_server, track, node);

                // NOTE: need reverse since zones are denoted by sensors that are leaving zone
                ZoneId next_zone = next_sensor->reverse->zone;
                ULOG_INFO_M(LOG_MASK_PATH, "at sensor %s in zone %d, next zone is %d", node->name, node->zone, next_zone);
                setSwitchesInZone(switch_server, track, next_zone, desired_switch_modes);

                // can release previous zone now
                if (prev_zone != -1) {
                    ULOG_INFO_M(LOG_MASK_PATH, "train %d release zone %d", train, prev_zone);
                    zone_unreserve(reserve_server, train, prev_zone);
                }
                prev_zone = node->zone;

                // stop once we hit target sensor
                if (edge->src->num == waiting_sensor->num) break;
                
            }
        }

        // now wait before sending stop command
        u64 delay_ticks = distance_from_sensor*100/train_vel;
        ULOG_INFO_M(LOG_MASK_PATH, "Waiting to stop train (delay for %d)...", delay_ticks);
        Delay(clock_server, delay_ticks);

        ULOG_INFO_M(LOG_MASK_PATH, "Train stopped...");
        TrainstateSetSpeed(trainstate_server, train, 0);
        
        /* TrainstateSetOffset(trainstate_server, train, offset); */
    }

    // free the path we took (but keep the place we stop at)
    zone_unreserve_all(reserve_server, train);
    ZoneId dest_zone = ((TrackEdge*)cbuf_back(path))->dest->zone;
    ULOG_INFO_M(LOG_MASK_PATH, "train stopped in zone %d", dest_zone);
    zone_reserve(reserve_server, train, dest_zone);

}

void
patherTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);

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
    CBuf* path = dijkstra(track, train, src, dest, true, true, &arena);
    if (path == NULL) {
        // dijkstra failed, compute a partial path instead
        ULOG_WARN("[PATHER] dijkstra can't find path, recompute a blocking path");
        path = dijkstra(track, train, src, dest, true, false, &arena);
    }

    // reserve zones in path and block if we can't aquire
    reserveZonesInPath(reserve_server, train, path);

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
            /* ULOG_INFO_M(LOG_MASK_PATH, "found reversal"); */

            // no need to move if we are only running a reversal
            if (cbuf_len(simple_path) > 1) {
                /* TrainState state = TrainstateGet(trainstate_server, train); */
                /* reverse_offset = TRAIN_LENGTH; */
                patherSimplePath(track, simple_path, train, train_speed, offset, &arena);
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

int
pathRandomizer(void)
{
    usize train_num = 2;
    usize train_speed = 8;

    Track* track = get_track_a();

    for (;;) {

        usize dest = rand_int() % 80;
        // There's a deadspot in this corner that will trap trains in it
        if (dest == 0 || dest == 1 || dest == 12 || dest == 13 || dest == 14 || dest == 15) {
            continue;
        }

        Tid path_task = PlanPath(train_num, train_speed, 0, track->nodes[dest].name);
        WaitTid(path_task);
    }
}

Tid
PlanPath(u32 train, u32 speed, i32 offset, char* dest_str)
{
    Track* track = get_track_a();
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    TrainState trainstate = TrainstateGet(trainstate_server, train);
    usize start_sensor = trainstate.pos;
    TrackNode* dest = track_node_by_name(track, dest_str);
    if (dest == NULL) {
        // TODO send back error?
        ULOG_WARN("invalid destination");
        return 0;
    }
    usize dest_sensor = dest - track->nodes;
    ULOG_INFO_M(LOG_MASK_PATH, "routing train %d from %d to %d", train, start_sensor, dest_sensor);

    PatherResp resp_buf;
    PatherMsg send_buf = (PatherMsg) {
        .src = start_sensor,
        .dest = dest_sensor,
        .train = train,
        .train_speed = speed,
        .offset = offset
    };

    Tid pather_task = Create(2, &patherTask, "Pather Task");
    Send(pather_task, (const char*)&send_buf, sizeof(PatherMsg), (char*)&resp_buf, sizeof(PatherResp));

    return pather_task;
}

