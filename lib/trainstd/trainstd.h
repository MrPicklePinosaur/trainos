#ifndef __TRAINSTD_H__
#define __TRAINSTD_H__

#include <stddef.h>

void println(char* format, ...);
void print(char* format, ...);

void* alloc(size_t size);
void free(void* ptr);

#endif // __TRAINSTD_H__
