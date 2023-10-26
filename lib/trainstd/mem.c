#include "mem.h"
#include "kern/alloc.h"

void*
alloc(size_t size)
{
    return kalloc(size);
}

void
free(void* ptr)
{
    kfree(ptr);
}
