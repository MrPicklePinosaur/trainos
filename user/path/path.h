#ifndef __PATH_PATH_H__
#define __PATH_PATH_H__

// path calculatoin and control task

#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include "track_data.h"

#define PATH_ADDRESS "path"

CBuf* dijkstra(Track* track, usize train, u32 src, u32 dest, bool allow_reversal, Arena* arena);

int PlanPath(Tid path_tid, u32 train, u32 speed, i32 offset, char* dest);

void pathTask(void);

#endif // __PATH_PATH_H__
