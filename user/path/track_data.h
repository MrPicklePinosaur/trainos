#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__

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

struct TrackEdge {
  TrackEdge *reverse;
  TrackNode *src, *dest;
  int dist;             /* in millimetres */
};

struct TrackNode {
  const char *name;
  NodeType type;
  int num;              /* sensor or switch number */
  TrackNode *reverse;  /* same location, but opposite direction */
  TrackEdge edge[2];
};

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144

void init_tracka(TrackNode *track);
void init_trackb(TrackNode *track);


#endif // __TRACK_DATA_H__

