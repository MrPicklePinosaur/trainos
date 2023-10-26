#include <trainstd.h>
#include <traindef.h>
#include "arena.h"

// from https://nullprogram.com/blog/2023/09/27/

Arena
arena_new(usize capacity)
{
    Arena arena = {0};
    arena.beg = alloc(capacity);
    arena.end = arena.beg ? arena.beg + capacity : 0;
    return arena;
}

void*
_arena_alloc(Arena *a, isize size, isize align, isize count, int flags)
{
    isize avail = a->end - a->beg;
    isize padding = -(uintptr_t)a->beg & (align - 1);
    if (count > (avail - padding)/size) {
        PANIC("arena allocator out of memory");
    }
    isize total = size * count;
    char *p = a->beg + padding;
    a->beg += padding + total;

    return flags & ARENA_NOZERO ? p : memset(p, 0, total);
}
