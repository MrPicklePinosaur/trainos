#ifndef __USER_TRAINSTATE_H__
#define __USER_TRAINSTATE_H__

#include <traindef.h>
#include <trainsys.h>

#define NUMBER_OF_TRAINS 80

// number of trains that we have calibration data for
#define TRAIN_COUNT 1

#define TRAINSTATE_ADDRESS "trainstate"

typedef struct {
    u8 speed;
    bool lights;
    bool reversed;
    usize pos;
} TrainState;

int TrainstateSetSpeed(Tid trainstate_server, usize train, usize speed);
int TrainstateReverse(Tid trainstate_server, usize train);
int TrainstateSetLights(Tid trainstate_server, usize train, bool lights);
TrainState TrainstateGet(Tid trainstate_server, usize train);
void trainStateServer();

#endif // __USER_TRAINSTATE_H__
