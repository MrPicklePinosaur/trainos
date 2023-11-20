#ifndef __USER_TRAINSTATE_H__
#define __USER_TRAINSTATE_H__

#include <traindef.h>
#include <trainsys.h>

#define NUMBER_OF_TRAINS 80
#define TRAIN_SPEED_MASK     0b001111
#define TRAIN_LIGHTS_MASK    0b010000

// upper 8 bits used for other metadata
#define TRAIN_REVERSED_MASK  0b100000

#define TRAINSTATE_ADDRESS "trainstate"

typedef u16 TrainState;

int TrainstateSetSpeed(Tid trainstate_server, usize train, usize speed);
int TrainstateReverse(Tid trainstate_server, usize train);
int TrainstateSetLights(Tid trainstate_server, usize train, bool lights);
TrainState TrainstateGet(Tid trainstate_server, usize train);
void trainStateServer();

#endif // __USER_TRAINSTATE_H__
