#ifndef __PATH_PATH_H__
#define __PATH_PATH_H__

// path calculatoin and control task

#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include "track_data.h"

#define PATH_ADDRESS "path"

CBuf* dijkstra(Track* track, usize train, u32 src, u32 dest, bool allow_reversal, bool check_reserve, Arena* arena);

int PlanPath(Tid path_tid, u32 train, u32 speed, i32 offset, char* dest);

void pathTask(void);

// some helpers
void setSwitchesInZone(Tid switch_server, Track* track, ZoneId zone, CBuf* desired_switches);
bool reserveZonesInPath(Track* track, usize train, CBuf* path);

#endif // __PATH_PATH_H__
