#ifndef __USER_TRAINPOS_H__
#define __USER_TRAINPOS_H__

#include <trainsys.h>
#include <traindef.h>

#define TRAINPOS_ADDRESS "trainpos"

typedef struct {
    usize train;
    usize pos;
} TrainPosWaitResult;
TrainPosWaitResult trainPosWait(Tid trainpos_server, isize train);
isize trainPosQuery(Tid trainpos_server, isize train); // same as trainPosWait but doesn't wait for a sensor to trigger

void trainPosTask();

#endif // __USER_TRAINPOS_H__
