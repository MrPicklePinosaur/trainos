#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stddef.h>

#define nullptr 0

/* Collection of memory allocators */

#define arena_init cursor_alloc_init
#define arena_alloc cursor_alloc
#define arena_free cursor_free

void cursor_alloc_init(void);
void* cursor_alloc(size_t size);
void cursor_free(void* ptr);

void freelist_init(void);
void* freelist_alloc(size_t size);
void freelist_free(void* ptr);

#endif // __ALLOC_H__
