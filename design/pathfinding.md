
## Train stopping position
Potential schemes for when a train needs to wait for another train to pass

1. Each train should stop in the middle of a sensor.
    - is this safe? tere are some spots where a sensor is quite close to a switch, so it may not be very safe?

2. Each train should stop slightly before the sensor to the exit of the zone (perhaps ~5cm)
    - this is better since the train wont be on top of a switch
    - the train will continue to occupy the zone behind until it enters the next zone
    - however does not take into account the direction the train is facing

3. Each train should stop slight after the sensor to the enterance of the zone (perhaps ~5cm)
    - same beneift as 2.
    - should also work regardless of the direction the train is facing

4. Ideal solution: the train should be as close to the exit sensor of the zone as possible, this ensures that we move as far as we can while we are waiting for the next zone to be free
    - for this to be possible, we need to combine data about distances between sensors and which way the train is facing (and also length of train)

Right now this scheme only routes the train to the right zone, and it no longer considers the position

## Sensor zones
Zones are created by the sensors bounding a switch. This ensures trains cannot be fighting for switch states since each zone guarentees mutual exclusion

### How do we know when we release a zone?
ideal solution: when no part of the train is touch the previous zone

1. when the train enters the next zone after
    - since no train is long enough to occupy three zones at the same time
    - however this may cause long delays when the next zone is really far away (example E8 -> C14 zone)

2. A fixed distance after the train crossing into the next zone
    - ensure that it is both safe to switch the switch and also that we don't hold onto zone lock for too long

## Changing switch states

When should we switch the switches?

Ideal solution: should trigger as close to when we need the switch as possible, in other words, right as the train is about to enter the sensor zone.

But ideal doesnt really matter, since we have reserved the zone for the duration we need it for, so we can also switch it the earliest time the zone becomes ours, which would be when the last train releases the zone



IDEAL solution
- we know about train orientation
- we stop the train just before the exit of the zone in cases of waiting
- we release the previous zone the instant we enter the next zone
- at same time we lock the next zone, we also release previous zone, the reservation system should guarentee that we can lock the next zone

## Rservation table

How granular should the time slice be?
- constant time slices? (t, zone)
- Should instead a time period be inserted of arbirtrary granulatity? (t_start, t_end, zone)

CA* basic impl
- use true distance as heuristic (easy to compute all shortest distance between pair of nodes in graph)
- graph is technically small enough that we can even get away with space-time dijkstra?

Every time train enters new zone it will use its current velocity to predict when it will need zones and reserve it when some buffer time

## Other considerations

What to do about terminals? terminals may end up blocking the switches that are close to it for a long time
- maybe need some special handing for the train to 'park'?

How to handle reservations for reversing?
- might need to enter a zone, exit, switch the switch, and renter the zone, which will take quite a while

Use higher velocities if we have a longer path that is reserved?


## RESOURCES

https://www.davidsilver.uk/wp-content/uploads/2020/03/coop-path-AIWisdom.pdf
https://www.davidsilver.uk/wp-content/uploads/2020/03/coop-path-AIIDE.pdf
