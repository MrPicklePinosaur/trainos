#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__

#include <trainstd.h>

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
    TrackEdge edge[3];
};

struct Track {
    Map* map; // name to node ref
    TrackNode* nodes;
    Arena arena;
};

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144
#define TRACK_A_SIZE 144
#define TRACK_B_SIZE 140

TrackNode* track_node_by_name(Track* track, str8 name);
TrackNode* track_node_by_sensor_id(Track* track, uint32_t sensor_id);
TrackNode* track_pathfind(TrackNode* start, TrackNode* end);

Track* track_a_init();
Track* track_b_init();

TrackNode* track_prev_node(Tid switch_server, Track* track, TrackNode* node);
TrackNode* track_prev_sensor(Tid switch_server, Track* track, TrackNode* node);

TrackNode* track_next_node(Tid switch_server, Track* track, TrackNode* node);
TrackNode* track_next_sensor(Tid switch_server, Track* track, TrackNode* node);


#endif // __TRACK_DATA_H__

