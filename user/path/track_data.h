#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__

#include <trainstd.h>
#include <trainsys.h>

typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} NodeType;

typedef enum {
    EDGE_FORWARD = 0,
    EDGE_REVERSE
} EdgeType;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1
#define DIR_REVERSE 2

typedef struct TrackNode TrackNode;
typedef struct TrackEdge TrackEdge;
typedef struct Track Track;
typedef usize ZoneId;

struct TrackEdge {
    TrackEdge *reverse;
    TrackNode *src, *dest;
    int dist;             /* in millimetres */
    EdgeType type;
};

struct TrackNode {
    char *name;
    NodeType type;
    int num;              /* sensor or switch number */
    TrackNode *reverse;  /* same location, but opposite direction */
    ZoneId zone;
    TrackEdge edge[3];
};

#define ZONE_MAX_SENSORS 8
#define ZONE_MAX_SWITCHES 6
#define ZONE_MAX 25
typedef struct {
    ZoneId zone;
    TrackNode* sensors[ZONE_MAX_SENSORS+1];  
    TrackNode* switches[ZONE_MAX_SWITCHES+1];
} Zone;

struct Track {
    TrackNode* nodes; // owned pointer to array of nodes
    Zone* zones;
};

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144
#define TRACK_A_SIZE 144
#define TRACK_B_SIZE 140

void track_init();
Track* get_track_a();
Track* get_track_b();

bool track_edge_cmp(TrackEdge a, TrackEdge b);

TrackNode* track_node_by_name(Track* track, char* name);
TrackNode* track_node_by_sensor_id(Track* track, u32 sensor_id);
TrackNode* track_node_by_branch_id(Track* track, usize branch_id);
TrackNode* track_pathfind(TrackNode* start, TrackNode* end);

TrackNode* track_prev_node(Tid switch_server, Track* track, TrackNode* node);
TrackNode* track_prev_sensor(Tid switch_server, Track* track, TrackNode* node);

TrackNode* track_next_node(Tid switch_server, Track* track, TrackNode* node);
TrackNode* track_next_sensor(Tid switch_server, Track* track, TrackNode* node);


#endif // __TRACK_DATA_H__

