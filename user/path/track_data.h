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

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

typedef struct TrackNode TrackNode;
typedef struct TrackEdge TrackEdge;
typedef struct Track Track;

struct TrackEdge {
    TrackEdge *reverse;
    TrackNode *src, *dest;
    int dist;             /* in millimetres */
};

struct TrackNode {
    char *name;
    NodeType type;
    int num;              /* sensor or switch number */
    TrackNode *reverse;  /* same location, but opposite direction */
    TrackEdge edge[2];
};

struct Track {
    Map* map; // name to node ref
    TrackNode* nodes;
};

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144

TrackNode* track_find(TrackNode* track, const char* name);
TrackNode* track_pathfind(TrackNode* start, TrackNode* end);

Track track_a_init(Arena* arena);
Track track_b_init(Arena* arena);

TrackNode* track_prev_node(TrackNode* track);
TrackNode* track_prev_sensor(TrackNode* track);

TrackNode* track_next_node(TrackNode* track);
TrackNode* track_next_sensor(TrackNode* track);


#endif // __TRACK_DATA_H__

