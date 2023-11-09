#ifndef __USER_TRAINSTATE_H__
#define __USER_TRAINSTATE_H__

#include <traindef.h>
#include <trainsys.h>

#define NUMBER_OF_TRAINS 80
#define TRAIN_SPEED_MASK     0b01111
#define TRAIN_LIGHTS_MASK    0b10000

#define TRAINSTATE_ADDRESS "trainstate"

typedef u8 TrainState;

int TrainstateSetSpeed(Tid trainstate_server, usize train, usize speed);
int TrainstateSetLights(Tid trainstate_server, usize train, bool lights);
TrainState TrainstateGet(Tid trainstate_server, usize train);
void trainStateServer();

#endif // __USER_TRAINSTATE_H__
