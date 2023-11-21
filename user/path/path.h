#ifndef __PATH_PATH_H__
#define __PATH_PATH_H__

// path calculatoin and control task

#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include "track_data.h"

Tid PlanPath(u32 train, u32 speed, i32 offset, char* dest);

// some helpers
void setSwitchesInZone(Tid switch_server, Track* track, ZoneId zone, CBuf* desired_switches);
bool reserveZonesInPath(Tid reserve_server, usize train, CBuf* path);

#endif // __PATH_PATH_H__
