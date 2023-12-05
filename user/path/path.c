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
    bool allow_reversal;
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
                /* ULOG_INFO_M(LOG_MASK_PATH, "setting switch %d to state %d", pair->first, pair->second); */
                SwitchChange(switch_server, pair->first, pair->second);
            }
        }
    }
}

// moves the train to a specified destination without using any reversals
void
patherSimplePath(Track* track, CBuf* path, usize train, usize train_speed, isize offset, Arena* arena)
{
    /* ULOG_INFO_M(LOG_MASK_PATH, "Executing simple path offset %d", offset); */
    /* for (usize i = 0; i < cbuf_len(path); ++i) { */
    /*     TrackEdge* edge = (TrackEdge*)cbuf_get(path, i); */
    /*     print("%s->%s,", edge->src->name, edge->dest->name); */
    /* } */
    /* print("\r\n"); */

    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid sensor_server = WhoIs(SENSOR_ADDRESS);
    Tid switch_server = WhoIs(SWITCH_ADDRESS);
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);

    // always use the leader train as front train
    TrainState _state = TrainstateGet(trainstate_server, train);
    usize leader_train = _state.cohort;

    TrainState state = TrainstateGet(trainstate_server, leader_train);

    i32 distance_to_dest = offset - state.offset;
    for (usize i = 0; i < cbuf_len(path); ++i) {
        TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
        distance_to_dest += edge->dist;
    }
    if (distance_to_dest < 0) {
        ULOG_WARN("offsets cause destination to be behind direction of travel");
    }

    i32 distance_to_waiting_sensor;
    i32 stopping_distance;
    i32 train_vel;

    // compute which sensor to issue stop command from
    // TODO it is possible to run out of path
    TrackNode* waiting_sensor;
    const u32 STOPPING_DISTANCES[] = {14, 11, 8, 5};
    usize original_train_speed = train_speed;

    bool is_short_move = true;

    for (u32 speed_i = 0; speed_i < 4; ++speed_i) {
        train_speed = STOPPING_DISTANCES[speed_i];
        if (train_speed > original_train_speed) continue;

        stopping_distance = train_data_stop_dist(leader_train, train_speed)-offset;
        train_vel = train_data_vel(leader_train, train_speed);

        waiting_sensor = 0;
        distance_to_waiting_sensor = distance_to_dest-offset;
        for (usize i = usize_sub(cbuf_len(path), 1); ; --i) {
            TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);
            stopping_distance -= edge->dist;
            distance_to_waiting_sensor -= edge->dist;
            if (stopping_distance <= 0 && edge->src->type == NODE_SENSOR) {
                waiting_sensor = edge->src; // sensor that we should wait to trip
                break;
            }
            if (i == 0) break;
        }

        if (!(
            waiting_sensor == 0
            || ((TrackEdge*)cbuf_front(path))->src == waiting_sensor
            || distance_to_waiting_sensor < train_data_acceleration_dist(leader_train, train_speed)
        )) {
            is_short_move = false;
            break;
        }
    }

    /* ULOG_INFO_M(LOG_MASK_PATH, "train %d vel is %d", train, train_vel); */

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
                /* ULOG_INFO("want switch %d as straight", pair->first); */
            }
            else if (track_edge_cmp(edge->src->edge[DIR_CURVED], *edge)) {
                //ULOG_INFO_M(LOG_MASK_PATH, "switch %d to curved", switch_num);
                Pair_u32_SwitchMode* pair = arena_alloc(arena, Pair_u32_SwitchMode);
                pair->first = switch_num;
                pair->second = SWITCH_MODE_CURVED;
                cbuf_push_back(desired_switch_modes, pair);
                /* ULOG_INFO("want switch %d as curved", pair->first); */
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


    if (is_short_move) {
        ULOG_INFO_M(LOG_MASK_PATH, "Executing short move...");

        TrainstateSetSpeed(trainstate_server, leader_train, TRAIN_DATA_SHORT_MOVE_SPEED);
        Delay(clock_server, train_data_short_move_time(leader_train, distance_to_dest) / 10);

        TrainstateSetSpeed(trainstate_server, leader_train, 0);
        /* ULOG_INFO_M(LOG_MASK_PATH, "Before stop wait short move"); */
        Delay(clock_server, train_data_stop_time(leader_train, TRAIN_DATA_SHORT_MOVE_SPEED) / 10 + 100);
        /* ULOG_INFO_M(LOG_MASK_PATH, "After stop wait short move"); */

    } else {
        ULOG_INFO_M(LOG_MASK_PATH, "Executing regular move sensor: %s, %d, distance: %d", waiting_sensor->name, waiting_sensor->num, distance_from_sensor);

        TrainstateSetSpeed(trainstate_server, leader_train, train_speed);

        // start at index one since we skip the starting node (assume no short move)
        ZoneId prev_zone = -1;
        usize last_reserved_zone = 0;
        for (usize i = 1; i < cbuf_len(path); ++i) {
            TrackEdge* edge = (TrackEdge*)cbuf_get(path, i);

            if (edge->src->type == NODE_SENSOR) {

                // wait for sensor
                // first is train, second is pos
                usize new_pos = 0;
                for (;;) {
                    Pair_usize_usize res = TrainstateWaitForSensor(trainstate_server, leader_train);
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
                /* ULOG_INFO_M(LOG_MASK_PATH, "at sensor %s in zone %d, next zone is %d", node->name, node->zone, next_zone); */
                setSwitchesInZone(switch_server, track, next_zone, desired_switch_modes);

                // release all zones behind the last train in the cohort
                if (node->zone != -1) {
                    /* ULOG_INFO_M(LOG_MASK_PATH, "train %d release zone %d", train, node->zone); */
                    ULOG_INFO("ZONE RELEASE");
                    usize last_train;
                    if (cbuf_len(state.followers) == 0) {
                        last_train = leader_train;
                    }
                    else {
                        last_train = cbuf_back(state.followers);
                    }

                    TrainState last_train_state = TrainstateGet(trainstate_server, last_train);
                    // Find the index of the node that the last train is currently at
                    for (usize j = last_reserved_zone; j <= i; j++) {
                        TrackEdge* search_zone = (TrackEdge*)cbuf_get(path, j);
                        if (track_node_index(track, search_zone->dest) == last_train_state.pos) {
                            // Free all nodes from the last reserved zone up to the zone the last train is currently at
                            for (usize k = last_reserved_zone; k <= j; k++) {
                                TrackEdge* zone_to_release = (TrackEdge*)cbuf_get(path, k);
                                if (zone_to_release->dest->type == NODE_SENSOR) {
                                    ULOG_INFO("  UNRESERVE %d", zone_to_release->dest->zone);
                                    zone_unreserve(reserve_server, leader_train, zone_to_release->dest->zone);
                                }
                            }
                            last_reserved_zone = j+1;
                            break;
                        }
                    }
                }
                /*
                if (prev_zone != -1) {
                    ULOG_INFO_M(LOG_MASK_PATH, "train %d release zone %d", train, prev_zone);
                    zone_unreserve(reserve_server, train, prev_zone);
                }
                */
                prev_zone = node->zone;

                // stop once we hit target sensor
                if (edge->src->num == waiting_sensor->num) break;
                
            }
        }

        // now wait before sending stop command
        u64 delay_ticks = distance_from_sensor*100/train_vel;
        /* ULOG_INFO_M(LOG_MASK_PATH, "Waiting to stop train (delay for %d)...", delay_ticks); */
        Delay(clock_server, delay_ticks);

        TrainstateSetSpeed(trainstate_server, leader_train, 0);
        /* ULOG_INFO_M(LOG_MASK_PATH, "Before stop wait regular move"); */
        Delay(clock_server, train_data_stop_time(leader_train, TRAIN_DATA_SHORT_MOVE_SPEED) / 10 + 100);
        /* ULOG_INFO_M(LOG_MASK_PATH, "After stop wait regular move"); */
        
        /* TrainstateSetOffset(trainstate_server, train, offset); */
    }

    // free the path we took
    /* ZoneId prev_zone = ((TrackEdge*)cbuf_back(path))->dest->zone; */
    /* zone_unreserve(reserve_server, train, prev_zone); */
    zone_unreserve_all(reserve_server, leader_train);

    // Set train position, which also reserves the zone it stopped at.
    // TODO What happens if the train goes somewhere else due to a switch error?
    TrackNode* dest = ((TrackEdge*)cbuf_back(path))->dest;
    TrainstateSetPos(trainstate_server, reserve_server, leader_train, dest);
    /* ULOG_INFO_M(LOG_MASK_PATH, "train stopped in zone %d", dest_zone); */

    TrainstateSetOffset(trainstate_server, leader_train, offset);

}

// Run a path that includes reversals
void
patherComplexPath(Tid trainstate_server, Tid clock_server, Track* track, CBuf* path, usize train, usize train_speed, isize offset, Arena* arena)
{
    // no work to dopath.c
    if (cbuf_len(path) == 0) return;

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
                u32 reverse_offset = TRAIN_LENGTH;
                patherSimplePath(track, simple_path, train, train_speed, reverse_offset, arena);
            }
            ULOG_INFO_M(LOG_MASK_PATH, "Reversing train %d...", train);
            TrainstateReverse(trainstate_server, train);
            cbuf_clear(simple_path);
        }
    }

    if (cbuf_len(simple_path) > 0) {
        patherSimplePath(track, simple_path, train, train_speed, offset, arena);
    }
}


typedef struct {
    Tid trainstate_server;
    Tid clock_server;
    Track* track;
    CBuf* path;
    usize train;
    usize train_speed;
    isize offset;
} PartialPatherMsg;

void
patherPartialPath()
{
    Arena arena = arena_new(sizeof(TrackEdge*)*TRACK_MAX);

    int from_tid;
    PartialPatherMsg msg_buf;
    struct {} reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(PartialPatherMsg));
    if (msg_len < 0) {
        ULOG_WARN("[PARTIAL PATHER] Error when receiving");
        Exit();
    }
    Reply(from_tid, (char*)&reply_buf, 0);

    patherComplexPath(msg_buf.trainstate_server, msg_buf.clock_server, msg_buf.track, msg_buf.path, msg_buf.train, msg_buf.train_speed, msg_buf.offset, &arena);

    Exit();
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

    Track* track = get_track();

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
    bool allow_reversal = msg_buf.allow_reversal;

    TrainstateSetDest(trainstate_server, train, dest);

    if (src == dest || src == track->nodes[dest].reverse - track->nodes) {
        ULOG_INFO_M(LOG_MASK_PATH, "[PATHER] Source equals destination");
        Exit();
    }

    Arena arena = arena_new(sizeof(TrackEdge*)*TRACK_MAX*2);

    /* ULOG_INFO_M(LOG_MASK_PATH, "computing path..."); */
    CBuf* path = dijkstra(track, train, src, dest, allow_reversal, true, &arena);
    if (path == NULL) {
        // dijkstra failed, compute a partial path instead
        ULOG_WARN("[PATHER] dijkstra can't find path, recompute a blocking path");
        path = dijkstra(track, train, src, dest, allow_reversal, false, &arena);
    }

    CBuf* complex_path = cbuf_new(128);

    // reserve zones in path and block if we can't aquire
    for (usize i = 0; i < cbuf_len(path); ++i) {
        // reserve the path that we found
        // TODO technically there could be barging if another train reseves the task we found before we do (assuming not possible for another pathfind request to come this quickly for now)
        TrackEdge* edge = cbuf_get(path, i);
        ZoneId zone = edge->dest->reverse->zone; // TODO should also reserve start?
        if (zone != -1) {
            if (!zone_reserve(reserve_server, train, zone)) {
                /* ULOG_WARN("failed reservation waiting for zone %d", zone); */

                // can do partial pathfind
                /* ULOG_WARN("Executing partial path of length %d", cbuf_len(complex_path)); */

                Tid partial_path_task = Create(5, &patherPartialPath, "partial path");
                PartialPatherMsg partial_pather_msg = (PartialPatherMsg) {
                    .trainstate_server = trainstate_server,
                    .clock_server = clock_server,
                    .track = track,
                    .path = complex_path,
                    .train = train,
                    .train_speed = train_speed,
                    .offset = offset,
                };
                struct {} resp_buf;
                Send(partial_path_task, (const char*)&partial_pather_msg, sizeof(PartialPatherMsg), (char*)&resp_buf, 0);

                zone_wait(reserve_server, train, zone);
                if (!zone_reserve(reserve_server, train, zone)) {
                    PANIC("should have claimed zone here");
                }

                // kill pather complex path
                // TODO this may be bad, since we truncate the partial path. so if cancel and there was a switch / revere point on the path, we may not execute it
                // it would be nice to figure out how much the path was completed after we kill the partial path
                Kill(partial_path_task);
                /* WaitTid(partial_path_task); */

                cbuf_clear(complex_path);
            }
            /* ULOG_INFO_M(LOG_MASK_PATH, "train %d reserved zone %d", train, zone); */
        }
        cbuf_push_back(complex_path, edge);
    }

    patherComplexPath(trainstate_server, clock_server, track, complex_path, train, train_speed, offset, &arena);

    arena_release(&arena);
    Exit();
}

typedef struct {
    usize train;
    usize speed;
    char* start;
} RandomizerMsg;

typedef struct {

} RandomizerResp;


char* BLACKLIST[] = { "A1", "A2", "A13", "A14", "A15", "A16", "A11", "A12", "B7", "B8", "B11", "B12", "B9", "B10", "A9", "A10", "A7", "A8", "A5", "A6", "C3", "C4", 0 };

char*
getRandomDest(Track* track)
{
    usize dest_ind = rand_int() % 80;
    char* dest = track->nodes[dest_ind].name;
    char** blacklist_node = BLACKLIST;
    for (; *blacklist_node != 0; ++blacklist_node) {
        if (strcmp(*blacklist_node, dest) == 0) return 0;
    }
    return dest;
}

void
pathRandomizer()
{
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);
    Tid reserve_server = WhoIs(RESERVE_ADDRESS);
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    int from_tid;
    RandomizerMsg msg_buf;
    RandomizerResp reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(RandomizerMsg));
    if (msg_len < 0) {
        ULOG_WARN("[RANDOMIZER] Error when receiving");
        Exit();
    }
    reply_buf = (RandomizerResp){};
    Reply(from_tid, (char*)&reply_buf, sizeof(RandomizerResp));

    usize train_num = msg_buf.train;
    usize train_speed = msg_buf.speed;
    char* start = msg_buf.start;

    Track* track = get_track();

    TrackNode* node = track_node_by_name(track, start);
    TrainstateSetPos(trainstate_server, reserve_server, train_num, node);
    for (;;) {

        char* dest = getRandomDest(track);
        if (dest == 0) continue;

        // choose random target
        ULOG_DEBUG(">>>>>>>>>>>>>>>> [RANDOM PATH] Train %d picking random dest %s", train_num, dest);
        Path train_paths[] = {(Path){train_num, train_speed, 0, dest, true}, (Path){train_num, train_speed, 0, start, true}};
        Tid train_pather = PlanPathSeq(train_paths, 2);
        WaitTid(train_pather);
        TrainstateReverseStatic(trainstate_server, train_num); // TODO dont use reverse static
        Delay(clock_server, 10);
    }
}


void
createPathRandomizer(usize train, usize speed, char* start)
{
    RandomizerResp resp_buf;
    RandomizerMsg send_buf = (RandomizerMsg) {
        .train = train,
        .speed = speed,
        .start = start 
    };

    int task = Create(5, &pathRandomizer, "Path Randomizer Task");
    int ret = Send(task, (const char*)&send_buf, sizeof(RandomizerMsg), (char*)&resp_buf, sizeof(RandomizerResp));
}


Tid
PlanPath(Path path)
{
    Track* track = get_track();
    Tid trainstate_server = WhoIs(TRAINSTATE_ADDRESS);

    TrainState trainstate = TrainstateGet(trainstate_server, path.train);
    usize start_sensor = trainstate.pos;
    if (start_sensor == TRAIN_POS_NULL) {
        ULOG_WARN("Train %d has unknown current position, aborting PlanPath", path.train);
        return 0;
    }
    TrackNode* dest = track_node_by_name(track, path.dest);
    if (dest == NULL) {
        // TODO send back error?
        ULOG_WARN("invalid destination");
        return 0;
    }
    usize dest_sensor = dest - track->nodes;
    ULOG_INFO_M(LOG_MASK_PATH, "routing train %d from %d to %d", path.train, start_sensor, dest_sensor);

    PatherResp resp_buf;
    PatherMsg send_buf = (PatherMsg) {
        .src = start_sensor,
        .dest = dest_sensor,
        .train = path.train,
        .train_speed = path.speed,
        .offset = path.offset,
        .allow_reversal = path.allow_reversal 
    };

    Tid pather_task = Create(2, &patherTask, "Pather Task");
    Send(pather_task, (const char*)&send_buf, sizeof(PatherMsg), (char*)&resp_buf, sizeof(PatherResp));

    return pather_task;
}


typedef struct {
    Path* path;
    usize len;
} PlanPathSeqMsg;

// we need a task since we dont want to block task that spawned this
void
planPathSeqTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    int from_tid;
    PlanPathSeqMsg msg_buf;
    struct {} reply_buf;
    int msg_len = Receive(&from_tid, (char*)&msg_buf, sizeof(PlanPathSeqMsg));
    Reply(from_tid, (char*)&reply_buf, 0);
    
    for (usize i = 0; i < msg_buf.len; ++i) {
        /* ULOG_DEBUG("Executing path index %d: train %d", i, msg_buf.path[i].train); */
        Tid path_task = PlanPath(msg_buf.path[i]);
        if (path_task == 0) continue;
        WaitTid(path_task);
        Delay(clock_server, 5); // add tiny delay
    }
    Exit();
}

Tid
PlanPathSeq(Path* path, usize len)
{
    PlanPathSeqMsg send_buf = (PlanPathSeqMsg) { path, len };
    struct {} resp_buf;

    Tid seq_path_task = Create(2, &planPathSeqTask, "Seq Pather Task");
    Send(seq_path_task, (const char*)&send_buf, sizeof(PlanPathSeqMsg), (char*)&resp_buf, 0);

    return seq_path_task;
}
