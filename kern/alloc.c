#include "alloc.h"
#include "log.h"

/* dumb allocator that maintains a buffer in memory and advances ptr
 * free() calls don't do anything
 * */
typedef struct ArenaAllocator ArenaAllocator;

// TODO becareful that tasks don't run into the heap
static unsigned char* const HEAP_BASE = (unsigned char*)0x00800000;
static ArenaAllocator* alloc = (ArenaAllocator*)HEAP_BASE;

// 1mb heap
#define ARENA_ALLOCATOR_SIZE 0x10000
struct ArenaAllocator {
    size_t cursor;
    unsigned int* buf[ARENA_ALLOCATOR_SIZE];
};

void
arena_init(void)
{
    *alloc = (ArenaAllocator) {
        .cursor = 0
    };
}

void*
arena_alloc(size_t size)
{
    PRINT("alloc %d", size);
    // bounds check
    if (alloc->cursor + size >= ARENA_ALLOCATOR_SIZE) {
        PANIC("arena allocator is out of memory");
    }

    void* ptr = alloc->buf + alloc->cursor;
    alloc->cursor += size;

    return ptr;
}

void
arena_free(void* ptr)
{
    /* NOP :D */
}

