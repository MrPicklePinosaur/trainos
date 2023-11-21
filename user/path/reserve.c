#include "reserve.h"

/*
typedef struct {
    ZoneId zone;
    usize train;
} Reservation;
*/

usize* reservations; // zero means no train has the zone reserved

void
reservationTask()
{

    // currently unused

    Exit();
}

void
zone_init(Track* track)
{
    reservations = alloc(track->zone_count*sizeof(usize));
    for (usize i = 0; i < track->zone_count; ++i) {
        reservations[i] = 0;
    }
}

// returns if the zone was successfully reserved
bool
zone_reserve(usize train, ZoneId zone)
{
    if (reservations[zone] == train) return true; // train is allowed to reserve a zone multiple times
    if (reservations[zone] != 0) {
        ULOG_WARN("unable for train %d to reserve zone %d, already reserved by train %d", train, zone, reservations[zone]);
        return false;
    }
    reservations[zone] = train;
    return true;
}

// returns false if zone was not able to be unreserved
bool
zone_unreserve(usize train, ZoneId zone)
{
    if (reservations[zone] == 0) return true;
    if (reservations[zone] != train) {
        ULOG_WARN("failed to unreserve zone %d: currently reserved by train %d", zone, train);
        return false;
    }
    reservations[zone] = 0;

    return true;
}

void
zone_unreserve_all(Track* track, usize train)
{
    for (usize i = 0; i < track->zone_count; ++i) {
        if (reservations[i] == train) reservations[i] = 0;
    } 
}

bool
zone_is_reserved(ZoneId zone, usize train)
{
    return reservations[zone] != 0 && reservations[zone] != train;
}
