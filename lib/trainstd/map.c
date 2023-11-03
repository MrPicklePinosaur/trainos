#include <traindef.h>
#include "map.h"

u64
hash(str8 key)
{
    uint64_t h = 0x100;
    for (ptrdiff_t i = 0; i < str8_len(key); i++) {
        h ^= str8_at(key, i);
        h *= 1111111111111111111u;
    }
    return h;
}

bool
equals(mapkey_t a, mapkey_t b)
{
    return str8_cmp(a, b);
}

mapval_t*
insert(Map** m, mapkey_t key, Arena* arena)
{
    for (u64 h = hash(key); *m; h <<= 2) {
        if (equals(key, (*m)->key)) {
            return &(*m)->value;
        }
        m = &(*m)->child[h>>62];
    }
    if (!arena) {
        return 0;
    }
    *m = arena_alloc(arena, Map);
    (*m)->key = key;
    return &(*m)->value;
}
