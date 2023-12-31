#ifndef __TRAINSTD_H__
#define __TRAINSTD_H__

#include <stddef.h>
#include <traindef.h>

#include "list.h"
#include "mem.h"
#include "int.h"
#include "string.h"
#include "cbuf.h"
#include "pair.h"
#include "hashmap.h"
#include "map.h"
#include "conv.h"
#include "stopwatch.h"
#include "log.h"
#include "util.h"
#include "rand.h"

void println(char* format, ...);
void print(char* format, ...);
unsigned char getc(void);
unsigned char getc_poll(void);

#endif // __TRAINSTD_H__
