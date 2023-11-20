#ifndef __USER_TRAINPOS_H__
#define __USER_TRAINPOS_H__

#include <trainsys.h>
#include <traindef.h>

#define TRAINPOS_ADDRESS "trainpos"

typedef enum {
    TRAINDIR_FWD,
    TRAINDIR_REV
} TrainDir;

typedef struct {
    usize pos;
    TrainDir dir;
} TrainPos;

typedef struct {
    usize train;
    usize pos;
    TrainDir dir;
} TrainPosWaitResult;

TrainPosWaitResult trainPosWait(Tid trainpos_server, isize train);
TrainPos trainPosQuery(Tid trainpos_server, isize train); // same as trainPosWait but doesn't wait for a sensor to trigger
void trainPosReverse(Tid trainpos_server); // reverse a given train

void trainPosTask();

#endif // __USER_TRAINPOS_H__
