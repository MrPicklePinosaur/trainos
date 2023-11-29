#ifndef __USER_TRAINSTATE_H__
#define __USER_TRAINSTATE_H__

#include <trainstd.h>
#include <traindef.h>
#include <trainsys.h>

#include "user/path/track_data.h"

#define NUMBER_OF_TRAINS 80

// number of trains that we have calibration data for
#define TRAIN_COUNT 4

// special NULL position for trains
#define TRAIN_POS_NULL 255

#define TRAINSTATE_ADDRESS "trainstate"

typedef usize Cohort;

typedef struct {
    u8 speed;
    bool lights;
    bool reversed;
    usize pos;
    usize dest;
    isize offset;
    Cohort cohort;
    List* followers; // list of trains that following
} TrainState;

typedef PAIR(usize, usize) Pair_usize_usize;
// wait for the next sensor of train
// pair returns (train, position)
Pair_usize_usize TrainstateWaitForSensor(Tid trainstate_server, isize train);
int TrainstateReverseStatic(Tid trainstate_server, usize train);

int TrainstateSetSpeed(Tid trainstate_server, usize train, usize speed);
int TrainstateReverse(Tid trainstate_server, usize train);
int TrainstateSetLights(Tid trainstate_server, usize train, bool lights);
int TrainstateSetOffset(Tid trainstate_server, usize train, isize offset);
int TrainstateSetDest(Tid trainstate_server, usize train, usize dest);
int TrainstateSetPos(Tid trainstate_server, Tid reserve_server, usize train, TrackNode* node);
int TrainstateSetCohort(Tid trainstate_server, usize train, Cohort cohort);
TrainState TrainstateGet(Tid trainstate_server, usize train);

void trainStateServer();

#endif // __USER_TRAINSTATE_H__
