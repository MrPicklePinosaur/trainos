#include "mem.h"
#include "hashmap.h"
#include "list.h"

struct HashMap {
    List** buckets;
    size_t bucket_count;
};

HashMap*
hashmap_new(size_t bucket_count)
{
    List** buckets = alloc(sizeof(List*)*bucket_count);
    for (unsigned int i = 0; i < bucket_count; ++i) {
        buckets[i] = list_init();
    }

    HashMap* map = alloc(sizeof(HashMap));
    *map = (HashMap) {
        .buckets = buckets,
        .bucket_count = bucket_count
    };
    return map;
}

void
hashmap_delete(HashMap* hm)
{
    // we don't free items inside hashmap?
    for (unsigned int i = 0; i < hm->bucket_count; ++i) list_deinit(hm->buckets[i]);

    free(hm->buckets);
    free(hm);
}

int
hashmap_insert(HashMap* hm, hash_t hash, value_t value)
{
    size_t ind = hash % hm->bucket_count;
    list_push_back(hm->buckets[ind], value);

    // TODO: we actually need a linked list of pairs

    return 0;
}

bool
hashmap_contains(HashMap* hm, hash_t hash)
{
}

value_t
hashmap_get(HashMap* hm, hash_t hash, int* success)
{
}
