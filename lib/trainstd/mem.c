#include "mem.h"
#include "kern/alloc.h"

void*
alloc(size_t size)
{
    return arena_alloc(size);
}

void
free(void* ptr)
{
    arena_free(ptr);
}
