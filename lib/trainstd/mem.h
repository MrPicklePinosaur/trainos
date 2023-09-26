#ifndef __TRAINSTD_MEM_H__
#define __TRAINSTD_MEM_H__

#include <stddef.h>

void* alloc(size_t size);
void free(void* ptr);

#endif // __TRAINSTD_MEM_H__
