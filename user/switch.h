#ifndef __USER_SWITCH_H__
#define __USER_SWITCH_H__

#include <traindef.h>
#include <trainsys.h>
#include <trainstd.h>
#include "marklin.h"

#define SWITCH_ADDRESS "switch"
#define SWITCH_COUNT 22
#define SWITCH_RANGE_1_LOW 1
#define SWITCH_RANGE_1_HIGH 18
#define SWITCH_RANGE_2_LOW 153
#define SWITCH_RANGE_2_HIGH 156

typedef PAIR(isize, SwitchMode) WaitForSwitchResult;

void switchServerTask();

int SwitchChange(Tid switch_server, isize switch_id, SwitchMode mode);
SwitchMode SwitchQuery(Tid switch_server, isize switch_id);
const SwitchMode* SwitchQueryAll(Tid switch_server);
int SwitchInit(Tid switch_server);

// passing switch = -1 means we don't care which switch
WaitForSwitchResult WaitForSwitch(Tid switch_server, isize switch_id);

#endif // __USER_SWITCH_H__
