#ifndef __TRAINSTD_ARENA_H__
#define __TRAINSTD_ARENA_H__

#include <traindef.h>

#define ARENA_NOZERO 0x1

typedef struct {
    char* beg;
    char* end;
} Arena;

// {USAGE] arena_alloc(alloc, type, count, flags)

// some macro magic to make optional arguments work
#define arena_alloc(...)            arena_allocx(__VA_ARGS__,arena_alloc4,arena_alloc3,arena_alloc2)(__VA_ARGS__)
#define arena_allocx(a,b,c,d,e,...) e
#define arena_alloc2(a, t)          (t *)_arena_alloc(a, sizeof(t), alignof(t), 1, 0)
#define arena_alloc3(a, t, n)       (t *)_arena_alloc(a, sizeof(t), alignof(t), n, 0)
#define arena_alloc4(a, t, n, f)    (t *)_arena_alloc(a, sizeof(t), alignof(t), n, f)

Arena arena_new(usize capacity);
void arena_release(Arena *a);
void* _arena_alloc(Arena *a, isize size, isize align, isize count, int flags);

#endif // __TRAINSTD_ARENA_H__
