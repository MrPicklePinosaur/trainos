#ifndef __TRAINSTD_H__
#define __TRAINSTD_H__

#include <stddef.h>

#include "list.h"
#include "mem.h"
#include "cbuf.h"
#include "pair.h"
#include "hashmap.h"
#include "conv.h"
#include "timer.h"
#include "types.h"

void println(char* format, ...);
void print(char* format, ...);
char getc(void);

#endif // __TRAINSTD_H__
