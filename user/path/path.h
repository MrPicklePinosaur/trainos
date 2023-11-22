#ifndef __PATH_PATH_H__
#define __PATH_PATH_H__

// path calculatoin and control task

#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include "track_data.h"

typedef struct {
    usize train;
    u32 speed;
    i32 offset;
    char* dest;
} Path;

Tid PlanPath(Path path);
Tid PlanPathSeq(Path* path, usize len); // plan a sequential path

void createPathRandomizer(usize train, usize speed);

// some helpers
void setSwitchesInZone(Tid switch_server, Track* track, ZoneId zone, CBuf* desired_switches);

#endif // __PATH_PATH_H__
