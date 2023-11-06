#ifndef __PATH_PATH_H__
#define __PATH_PATH_H__

// path calculatoin and control task

#include <trainsys.h>
#include <traindef.h>

#define PATH_ADDRESS "path"

int PlanPath(Tid path_tid, usize train, const char* end);

void pathTask(void);

#endif // __PATH_PATH_H__
