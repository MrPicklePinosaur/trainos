#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stddef.h>

#define nullptr 0

/* Collection of memory allocators */

void arena_init(void);
void* arena_alloc(size_t size);
void arena_free(void* ptr);

#endif // __ALLOC_H__
