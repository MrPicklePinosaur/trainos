#ifndef __USER_TRAINPOS_H__
#define __USER_TRAINPOS_H__

#include <trainsys.h>
#include <traindef.h>

int trainPosWait(Tid trainpos_server, isize train);

void trainPosTask();

#endif // __USER_TRAINPOS_H__
