#ifndef __PATH_DIJKSTRA_H__
#define __PATH_DIJKSTRA_H__

#include <trainsys.h>
#include <traindef.h>
#include <trainstd.h>
#include "track_data.h"

CBuf* dijkstra(Track* track, usize train, u32 src, u32 dest, bool allow_reversal, bool check_reserve, Arena* arena);

#endif // __PATH_DIJKSTRA_H__
