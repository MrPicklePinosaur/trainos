#include "alloc.h"
#include "log.h"

/* dumb allocator that maintains a buffer in memory and advances ptr
 * free() calls don't do anything
 * */
typedef struct ArenaAllocator ArenaAllocator;

static ArenaAllocator alloc;

#define ARENA_ALLOCATOR_SIZE 2048
struct ArenaAllocator {
    size_t cursor;
    unsigned int* buf[ARENA_ALLOCATOR_SIZE];
};

void
arena_init(void)
{
    alloc = (ArenaAllocator) {
        .cursor = 0
    };
}

void*
arena_alloc(size_t size)
{
    /* LOG_DEBUG("alloc %d", size); */
    // bounds check
    if (alloc.cursor + size >= ARENA_ALLOCATOR_SIZE) {
        LOG_WARN("arena allocator is out of memory");
        return 0;
    }

    void* ptr = alloc.buf + alloc.cursor;
    alloc.cursor += size;

    return ptr;
}

void
arena_free(void* ptr)
{
    /* NOP :D */
}

