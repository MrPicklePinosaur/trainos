#ifndef __PATH_PATH_H__
#define __PATH_PATH_H__

// path calculatoin and control task

#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include "track_data.h"

#define PATH_ADDRESS "path"

TrackEdge** dijkstra(Track* track, uint32_t src, uint32_t dest, Arena* arena);

int PlanPath(Tid path_tid, u32 train, char* dest);

void pathTask(void);

#endif // __PATH_PATH_H__
