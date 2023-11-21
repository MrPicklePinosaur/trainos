#ifndef __PATH_RESERVE_H__
#define __PATH_RESERVE_H__

#define RESERVE_ADDRESS "reserve"

#include <traindef.h>
#include "track_data.h"

void reservationTask();

void zone_init(Track* track);
bool zone_reserve(usize train, ZoneId zone);
bool zone_unreserve(usize train, ZoneId zone);
void zone_unreserve_all(Track* track, usize train);
bool zone_is_reserved(ZoneId zone, usize train);

#endif // __PATH_RESERVE_H__
