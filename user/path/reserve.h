#ifndef __PATH_RESERVE_H__
#define __PATH_RESERVE_H__

#define RESERVE_ADDRESS "reserve"

#include <traindef.h>
#include "track_data.h"

void reservationTask();

bool zone_reserve(Tid reserve_server, usize train, ZoneId zone);
bool zone_unreserve(Tid reserve_server, usize train, ZoneId zone);
void zone_unreserve_all(Tid reserve_server, usize train);
void zone_wait(Tid reserver_server, usize train, ZoneId zone); // wait for a given zone to be free
bool zone_is_reserved(Tid reserve_server, ZoneId zone, usize train);

usize* zone_dump();

#endif // __PATH_RESERVE_H__
